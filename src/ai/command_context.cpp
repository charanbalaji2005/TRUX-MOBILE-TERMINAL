// meridian-ai / command_context.cpp
#include "command_context.hpp"

#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace meridian::ai {

namespace {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string parent_dir(const std::string& dir) {
    if (dir.empty() || dir == "/") return "";
    auto slash = dir.find_last_of('/');
    if (slash == std::string::npos) return "";
    if (slash == 0) return "/";
    return dir.substr(0, slash);
}

} // namespace

std::pair<bool, std::optional<std::string>> CommandContextBuilder::find_git_branch(const std::string& start_dir) {
    std::string dir = start_dir;
    for (int depth = 0; depth < 64 && !dir.empty(); ++depth) {
        std::string git_path = dir + "/.git";
        struct stat st{};
        if (stat(git_path.c_str(), &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                // .git is a file, not a directory — a worktree or
                // submodule pointer ("gitdir: <path>"). Not resolved in
                // this v1; we can at least say "yes, this is part of a
                // git repo" without a branch name. See docs/ai.md.
                return {true, std::nullopt};
            }
            std::ifstream f(git_path + "/HEAD");
            if (f.is_open()) {
                std::string line;
                std::getline(f, line);
                line = trim(line);
                const std::string prefix = "ref: refs/heads/";
                if (line.rfind(prefix, 0) == 0) return {true, line.substr(prefix.size())};
                if (!line.empty()) return {true, line}; // detached HEAD: raw commit hash
            }
            return {true, std::nullopt};
        }
        dir = parent_dir(dir);
    }
    return {false, std::nullopt};
}

CommandContext CommandContextBuilder::build(const std::string& current_command,
                                             std::optional<std::string> last_command,
                                             std::optional<int> last_exit_code,
                                             std::optional<std::string> last_stderr,
                                             const std::string& search_dir) {
    CommandContext ctx;
    ctx.current_command = current_command;
    ctx.last_command = std::move(last_command);
    ctx.last_exit_code = last_exit_code;
    ctx.last_stderr = std::move(last_stderr);

    char cwd_buf[4096];
    if (getcwd(cwd_buf, sizeof(cwd_buf))) ctx.working_directory = cwd_buf;

    const char* shell_env = std::getenv("SHELL");
    ctx.shell_name = shell_env ? shell_env : "meridian-shell";

    struct utsname uts{};
    if (uname(&uts) == 0) {
        ctx.os_info = std::string(uts.sysname) + " " + uts.release + " " + uts.machine;
    }

    std::string dir = search_dir.empty() ? ctx.working_directory : search_dir;
    auto [found, branch] = find_git_branch(dir);
    ctx.in_git_repo = found;
    ctx.git_branch = branch;

    return ctx;
}

} // namespace meridian::ai
