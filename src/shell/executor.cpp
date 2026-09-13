// meridian-shell / executor.cpp
#include "executor.hpp"
#include "builtins.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

namespace meridian::shell {

// _exit() skips C++ iostream flushing (unlike a normal return from main),
// so any buffered std::cout/std::cerr content written by an in-process
// builtin just before we _exit() a forked child would otherwise be lost
// silently. Every _exit() call site in this file goes through here.
[[noreturn]] static void flush_and_exit(int code) {
    std::cout.flush();
    std::cerr.flush();
    _exit(code);
}

// Raw (unexpanded) text of a word, for job-list display only. Deliberately
// NOT expand_word(): building a display string by expanding would re-run
// any $(...) command substitution a second time, with real side effects.
static std::string raw_word_text(const Word& w) {
    std::string s;
    for (auto& part : w) s += part.text;
    return s;
}

std::string Executor::pipeline_display_string(const Pipeline& pl) {
    std::string out;
    for (std::size_t i = 0; i < pl.commands.size(); ++i) {
        if (i) out += " | ";
        for (std::size_t j = 0; j < pl.commands[i].argv.size(); ++j) {
            if (j) out += " ";
            out += raw_word_text(pl.commands[i].argv[j]);
        }
    }
    return out;
}

void Executor::enable_job_control() {
    shell_pgid_ = getpid();
    setpgid(shell_pgid_, shell_pgid_);
    // The shell itself ignores these; forked children restore default
    // disposition before exec (see run_pipeline's fork branch below) so
    // real programs behave normally under Ctrl+C / Ctrl+Z.
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, shell_pgid_);
    job_control_enabled_ = true;
}

Job* Executor::find_job(std::optional<int> id) {
    if (id) {
        for (auto& j : jobs_) if (j.id == *id) return &j;
        return nullptr;
    }
    Job* best = nullptr;
    for (auto& j : jobs_) if (!best || j.id > best->id) best = &j;
    return best;
}

static std::optional<int> parse_job_spec(const std::vector<std::string>& argv) {
    if (argv.size() < 2) return std::nullopt;
    std::string spec = argv[1];
    if (!spec.empty() && spec[0] == '%') spec = spec.substr(1);
    try { return std::stoi(spec); } catch (...) { return std::nullopt; }
}

void Executor::reap_job_status_changes() {
    for (auto it = jobs_.begin(); it != jobs_.end();) {
        if (it->state == JobState::Stopped) { ++it; continue; } // left alone until fg/bg touches it

        bool all_done = true;
        bool newly_stopped = false;
        for (auto pid : it->pids) {
            int status = 0;
            pid_t r = waitpid(pid, &status, WNOHANG | WUNTRACED);
            if (r == 0) { all_done = false; }
            else if (r > 0 && WIFSTOPPED(status)) { newly_stopped = true; all_done = false; }
            // r > 0 (exited/signaled) or r < 0 (already reaped) both mean
            // "this pid is no longer blocking the job."
        }
        if (newly_stopped) {
            it->state = JobState::Stopped;
            std::cout << "\n[" << it->id << "]+  Stopped    " << it->command_line << "\n";
            ++it;
            continue;
        }
        if (all_done) {
            std::cout << "[" << it->id << "]+  Done    " << it->command_line << "\n";
            it = jobs_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string Executor::jobs_report() {
    reap_job_status_changes();
    if (jobs_.empty()) return "no jobs\n";
    std::string out;
    for (auto& j : jobs_) {
        out += "[" + std::to_string(j.id) + "] " + std::to_string(j.pgid) + "  "
             + (j.state == JobState::Stopped ? "Stopped" : "Running") + "    "
             + j.command_line + "\n";
    }
    return out;
}

int Executor::do_fg(const std::vector<std::string>& argv) {
    if (!job_control_enabled_) { std::cerr << "fg: no job control in this session\n"; return 1; }
    Job* job = find_job(parse_job_spec(argv));
    if (!job) { std::cerr << "fg: no such job\n"; return 1; }

    std::cout << job->command_line << "\n";
    std::cout.flush();
    kill(-job->pgid, SIGCONT);
    tcsetpgrp(STDIN_FILENO, job->pgid);
    job->state = JobState::Running;

    int last = 0;
    bool stopped_again = false;
    std::size_t stopped_at = job->pids.size();
    for (std::size_t i = 0; i < job->pids.size(); ++i) {
        int status = 0;
        waitpid(job->pids[i], &status, WUNTRACED);
        if (WIFSTOPPED(status)) { stopped_again = true; stopped_at = i; break; }
        if (WIFEXITED(status)) last = WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) last = 128 + WTERMSIG(status);
    }
    tcsetpgrp(STDIN_FILENO, shell_pgid_);

    int target_id = job->id;
    if (stopped_again) {
        job->pids.assign(job->pids.begin() + static_cast<long>(stopped_at), job->pids.end());
        job->state = JobState::Stopped;
        std::cout << "\n[" << target_id << "]+  Stopped    " << job->command_line << "\n";
    } else {
        jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(),
                                    [target_id](const Job& j) { return j.id == target_id; }),
                    jobs_.end());
    }
    return last;
}

int Executor::do_bg(const std::vector<std::string>& argv) {
    if (!job_control_enabled_) { std::cerr << "bg: no job control in this session\n"; return 1; }
    Job* job = find_job(parse_job_spec(argv));
    if (!job) { std::cerr << "bg: no such job\n"; return 1; }
    if (job->state != JobState::Stopped) { std::cerr << "bg: job is already running\n"; return 1; }
    kill(-job->pgid, SIGCONT);
    job->state = JobState::Running;
    std::cout << "[" << job->id << "]+ " << job->command_line << " &\n";
    return 0;
}

// Expands exactly one '$...' construct starting at text[i] ('$' itself).
// Advances i past the construct and returns the expansion.
static std::string expand_one_dollar(const std::string& text, std::size_t& i, Executor& ex) {
    char next = text[i + 1];

    if (next == '(') {
        int depth = 1;
        std::size_t j = i + 2;
        while (j < text.size() && depth > 0) {
            if (text[j] == '(') depth++;
            else if (text[j] == ')') { depth--; if (depth == 0) break; }
            j++;
        }
        std::string inner = text.substr(i + 2, j - (i + 2));
        i = (j < text.size()) ? j + 1 : text.size();
        return ex.capture_command_substitution(inner);
    }
    if (next == '{') {
        std::size_t close = text.find('}', i + 2);
        std::string name = (close == std::string::npos) ? text.substr(i + 2) : text.substr(i + 2, close - (i + 2));
        i = (close == std::string::npos) ? text.size() : close + 1;
        const char* v = std::getenv(name.c_str());
        return v ? std::string(v) : std::string();
    }
    if (std::isalpha(static_cast<unsigned char>(next)) || next == '_') {
        std::size_t j = i + 1;
        while (j < text.size() && (std::isalnum(static_cast<unsigned char>(text[j])) || text[j] == '_')) j++;
        std::string name = text.substr(i + 1, j - (i + 1));
        i = j;
        const char* v = std::getenv(name.c_str());
        return v ? std::string(v) : std::string();
    }
    if (next == '?') { i += 2; return std::to_string(ex.last_status()); }
    if (next == '$') { i += 2; return std::to_string(static_cast<long>(getpid())); }

    i += 1;
    return "$";
}

std::string Executor::expand_word(const Word& w) {
    std::string result;
    for (const auto& part : w) {
        if (part.literal) { result += part.text; continue; }
        const std::string& text = part.text;
        std::size_t i = 0;
        while (i < text.size()) {
            if (text[i] == '$' && i + 1 < text.size()) {
                result += expand_one_dollar(text, i, *this);
            } else {
                result.push_back(text[i]);
                i++;
            }
        }
    }
    return result;
}

std::string Executor::capture_command_substitution(const std::string& src) {
    int fds[2];
    if (pipe(fds) != 0) return "";
    std::cout.flush();
    std::cerr.flush(); // avoid the parent's pending buffer content getting duplicated into the child's copy
    pid_t pid = fork();
    if (pid < 0) { close(fds[0]); close(fds[1]); return ""; }
    if (pid == 0) {
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);
        std::string err;
        int status = run_line(src, &err);
        flush_and_exit(status < 0 ? 1 : status);
    }
    close(fds[1]);
    std::string out;
    char buf[4096];
    ssize_t n;
    while ((n = ::read(fds[0], buf, sizeof(buf))) > 0) out.append(buf, static_cast<std::size_t>(n));
    close(fds[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    while (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

int Executor::run_line(const std::string& line, std::string* error) {
    Lexer lexer(line);
    std::string lex_err;
    auto tokens = lexer.tokenize(&lex_err);
    if (!lex_err.empty()) { if (error) *error = lex_err; last_status_ = -1; return -1; }

    Parser parser(std::move(tokens));
    std::string parse_err;
    Sequence seq = parser.parse(&parse_err);
    if (!parse_err.empty()) { if (error) *error = parse_err; last_status_ = -1; return -1; }

    return run(seq);
}

int Executor::run(const Sequence& seq) {
    if (job_control_enabled_) reap_job_status_changes();
    int status = last_status_;
    for (std::size_t idx = 0; idx < seq.size(); ++idx) {
        if (idx > 0) {
            Connector prev = seq[idx - 1].connector;
            if (prev == Connector::And && status != 0) continue;
            if (prev == Connector::Or && status == 0) continue;
        }
        status = run_pipeline(seq[idx].pipeline);
        last_status_ = status;
        if (exit_requested_) break;
    }
    return status;
}

int Executor::run_single_inprocess(const Command& cmd) {
    std::vector<std::string> argv;
    for (auto& w : cmd.argv) argv.push_back(expand_word(w));
    if (argv.empty()) return 0;
    return run_builtin(argv[0], argv, *this);
}

void Executor::run_in_child(const Command& cmd) {
    for (const auto& r : cmd.redirections) {
        std::string target = expand_word(r.target);
        int fd = -1;
        switch (r.type) {
            case RedirType::In:        fd = open(target.c_str(), O_RDONLY); break;
            case RedirType::Out:       fd = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); break;
            case RedirType::Append:    fd = open(target.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644); break;
            case RedirType::ErrOut:    fd = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); break;
            case RedirType::ErrAppend: fd = open(target.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644); break;
            case RedirType::All:       fd = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); break;
            case RedirType::AllAppend: fd = open(target.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644); break;
        }
        if (fd < 0) {
            std::cerr << "meridian-shell: cannot open '" << target << "': " << std::strerror(errno) << "\n";
            flush_and_exit(1);
        }
        switch (r.type) {
            case RedirType::In: dup2(fd, STDIN_FILENO); break;
            case RedirType::Out:
            case RedirType::Append: dup2(fd, STDOUT_FILENO); break;
            case RedirType::ErrOut:
            case RedirType::ErrAppend: dup2(fd, STDERR_FILENO); break;
            case RedirType::All:
            case RedirType::AllAppend: dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); break;
        }
        close(fd);
    }

    std::vector<std::string> argv;
    for (auto& w : cmd.argv) argv.push_back(expand_word(w));
    if (argv.empty()) flush_and_exit(0);

    if (is_builtin(argv[0])) {
        int status = run_builtin(argv[0], argv, *this);
        flush_and_exit(status);
    }

    std::vector<char*> cargv;
    cargv.reserve(argv.size() + 1);
    for (auto& s : argv) cargv.push_back(s.data());
    cargv.push_back(nullptr);

    execvp(argv[0].c_str(), cargv.data());
    std::cerr << "meridian-shell: " << argv[0] << ": " << std::strerror(errno) << "\n";
    flush_and_exit(errno == ENOENT ? 127 : 126);
}

int Executor::run_pipeline(const Pipeline& pl) {
    if (pl.commands.empty()) return 0;

    if (pl.commands.size() == 1) {
        const Command& cmd = pl.commands[0];
        if (cmd.argv.empty()) return 0;
        std::string prog = expand_word(cmd.argv[0]);
        bool inprocess_ok = cmd.redirections.empty() && !pl.background && is_builtin(prog);
        if (inprocess_ok) return run_single_inprocess(cmd);
    }

    std::string display = pipeline_display_string(pl);

    std::size_t n = pl.commands.size();
    std::vector<std::array<int, 2>> pipes(n > 1 ? n - 1 : 0);
    for (auto& p : pipes) {
        if (pipe(p.data()) != 0) { std::cerr << "meridian-shell: pipe() failed\n"; return 1; }
    }

    std::cout.flush();
    std::cerr.flush();

    std::vector<pid_t> pids;
    pids.reserve(n);
    pid_t pgid = 0;
    for (std::size_t i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid < 0) { std::cerr << "meridian-shell: fork failed\n"; return 1; }
        if (pid == 0) {
            if (job_control_enabled_) {
                pid_t self = getpid();
                pid_t target = (i == 0) ? self : pgid;
                setpgid(self, target); // child-side set; parent sets it too (race-safe double-set)
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
            }
            if (i > 0) dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i + 1 < n) dup2(pipes[i][1], STDOUT_FILENO);
            for (auto& p : pipes) { close(p[0]); close(p[1]); }
            run_in_child(pl.commands[i]); // never returns
        }
        if (job_control_enabled_) {
            pid_t target = (i == 0) ? pid : pgid;
            setpgid(pid, target);
            if (i == 0) pgid = pid;
        }
        pids.push_back(pid);
    }
    for (auto& p : pipes) { close(p[0]); close(p[1]); }

    if (pl.background) {
        Job job;
        job.id = next_job_id_++;
        job.pgid = job_control_enabled_ ? pgid : pids.front();
        job.pids = pids;
        job.command_line = display;
        job.state = JobState::Running;
        jobs_.push_back(job);
        std::cout << "[" << job.id << "] " << job.pgid << "\n";
        return 0;
    }

    if (job_control_enabled_) tcsetpgrp(STDIN_FILENO, pgid);

    int last_status = 0;
    bool stopped = false;
    std::size_t stopped_at = pids.size();
    for (std::size_t i = 0; i < pids.size(); ++i) {
        int status = 0;
        int opts = job_control_enabled_ ? WUNTRACED : 0;
        waitpid(pids[i], &status, opts);
        if (WIFSTOPPED(status)) { stopped = true; stopped_at = i; break; }
        if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) last_status = 128 + WTERMSIG(status);
    }

    if (job_control_enabled_) tcsetpgrp(STDIN_FILENO, shell_pgid_);

    if (stopped) {
        Job job;
        job.id = next_job_id_++;
        job.pgid = pgid;
        job.pids.assign(pids.begin() + static_cast<long>(stopped_at), pids.end());
        job.command_line = display;
        job.state = JobState::Stopped;
        jobs_.push_back(job);
        std::cout << "\n[" << job.id << "]+  Stopped    " << display << "\n";
    }

    return last_status;
}

} // namespace meridian::shell
