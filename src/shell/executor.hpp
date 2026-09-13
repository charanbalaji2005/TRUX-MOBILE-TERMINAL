#pragma once
// meridian-shell / executor.hpp
//
// Executes a parsed Sequence using fork()/execvp()/pipe()/dup2()/waitpid()
// directly — never passes the user's command through /bin/sh or system().
// Builtins that must mutate the shell's own state (cd, export, exit) run
// in-process when safe to do so; everything else — external programs,
// pipeline stages, and builtins combined with redirection — runs in a
// forked child, matching real shell subshell semantics.
//
// Also implements real POSIX job control (process groups + tcsetpgrp
// terminal handoff) when enable_job_control() has been called — see
// docs/shell.md for the design and how it's tested against a real PTY.

#include "ast.hpp"
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

namespace meridian::shell {

enum class JobState { Running, Stopped };

struct Job {
    int id = 0;
    pid_t pgid = -1;
    std::vector<pid_t> pids;   // still-outstanding (not yet reaped) pids of this job
    std::string command_line;  // best-effort display text, not re-parseable
    JobState state = JobState::Running;
};

class Executor {
public:
    Executor() = default;

    // Lexes, parses, and runs one command line in one call. On a lex/parse
    // error, sets *error, leaves last_status() at -1, and returns -1.
    int run_line(const std::string& line, std::string* error = nullptr);

    int run(const Sequence& seq);

    // Expands a Word: literal segments pass through untouched; expandable
    // segments are scanned for $VAR, ${VAR}, $(command substitution),
    // $? (last exit status), and $$ (this process's pid).
    std::string expand_word(const Word& w);

    // Forks, runs `src` through this same engine with stdout captured via
    // a pipe, and returns the captured output with trailing newlines
    // stripped (POSIX command-substitution semantics). Used by $(...).
    std::string capture_command_substitution(const std::string& src);

    void request_exit(int code) { exit_requested_ = true; exit_code_ = code; }
    bool exit_requested() const { return exit_requested_; }
    int exit_code() const { return exit_code_; }

    void push_history(const std::string& line) { if (!line.empty()) history_.push_back(line); }
    const std::vector<std::string>& history() const { return history_; }

    // Enables real job control: this process becomes its own process
    // group leader, ignores SIGINT/SIGQUIT/SIGTSTP/SIGTTIN/SIGTTOU for
    // itself (children restore default disposition before exec — see
    // run_in_child), and claims the controlling terminal's foreground
    // group via tcsetpgrp. Only call this when stdin is actually a
    // controlling tty (see src/app/shell_main.cpp) — matches real shells,
    // which only enable job control for interactive sessions.
    void enable_job_control();
    bool job_control_enabled() const { return job_control_enabled_; }

    // `fg`/`bg` builtins. argv[1], if present, is "%N" or "N" selecting
    // a job by id; with no argument, the most recently created job is
    // used. Returns the resumed job's exit status (fg) or 0 (bg).
    int do_fg(const std::vector<std::string>& argv);
    int do_bg(const std::vector<std::string>& argv);

    // Human-readable job list, also used by the `jobs` builtin. Polls
    // (non-blocking) for background jobs that finished or stopped since
    // the last call.
    std::string jobs_report();

    int last_status() const { return last_status_; }

private:
    int run_pipeline(const Pipeline& pl);
    int run_single_inprocess(const Command& cmd);
    [[noreturn]] void run_in_child(const Command& cmd);

    Job* find_job(std::optional<int> id);
    void reap_job_status_changes();
    std::string pipeline_display_string(const Pipeline& pl);

    std::vector<std::string> history_;
    int last_status_ = 0;
    bool exit_requested_ = false;
    int exit_code_ = 0;

    bool job_control_enabled_ = false;
    pid_t shell_pgid_ = 0;
    std::vector<Job> jobs_;
    int next_job_id_ = 1;
};

} // namespace meridian::shell
