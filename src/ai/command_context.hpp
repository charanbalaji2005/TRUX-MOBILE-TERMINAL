#pragma once
// meridian-ai / command_context.hpp
//
// Spec's CommandContext (§9/§66): the bundle of *local* information
// worth having on hand before analyzing or (eventually) asking a
// provider about a command — current command, last command/exit code/
// stderr, working directory, shell, OS info, and git branch if inside a
// repo. Everything here is gathered from the local machine only:
// getcwd(), uname(), and reading .git/HEAD directly (no `git` binary
// invocation needed, no network). This is exactly the "minimum
// necessary context" spec §66 asks for — deliberately NOT the full
// environment, not scrollback, not file contents.

#include <optional>
#include <string>

namespace meridian::ai {

struct CommandContext {
    std::string current_command;
    std::optional<std::string> last_command;
    std::optional<int> last_exit_code;
    std::optional<std::string> last_stderr;
    std::string working_directory;
    std::string shell_name;
    std::string os_info;               // e.g. "Linux 6.8.0 x86_64"
    bool in_git_repo = false;
    std::optional<std::string> git_branch;
};

class CommandContextBuilder {
public:
    // Gathers real local context. `search_dir` overrides where the git
    // walk-up starts (defaults to the real cwd) — used by tests so they
    // don't depend on this process's actual working directory.
    static CommandContext build(const std::string& current_command,
                                 std::optional<std::string> last_command = std::nullopt,
                                 std::optional<int> last_exit_code = std::nullopt,
                                 std::optional<std::string> last_stderr = std::nullopt,
                                 const std::string& search_dir = "");

    // Exposed separately for testing: walks upward from `start_dir`
    // looking for a .git directory, and if found, reads .git/HEAD
    // directly (format: "ref: refs/heads/<branch>\n", or a bare commit
    // hash when in detached-HEAD state) rather than invoking the `git`
    // binary. Returns {true, branch_or_hash} or {false, nullopt}.
    static std::pair<bool, std::optional<std::string>> find_git_branch(const std::string& start_dir);
};

} // namespace meridian::ai
