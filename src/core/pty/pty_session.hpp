#pragma once
// meridian-terminal-core / pty / pty_session.hpp
//
// Real POSIX PTY Terminal Session.
// Launches the user's actual login shell ($SHELL, /bin/zsh, /bin/bash, /bin/fish)
// over a real pseudo-terminal with full raw/canonical mode handoff, job control
// (Ctrl+C SIGINT, Ctrl+Z SIGTSTP, Ctrl+\ SIGQUIT, Ctrl+D EOF), and TIOCSWINSZ
// window resizing.

#include <string>
#include <vector>
#include <sys/types.h>

namespace meridian::pty {

class PtySession {
public:
    PtySession();
    ~PtySession();

    PtySession(const PtySession&) = delete;
    PtySession& operator=(const PtySession&) = delete;

    // Detects user's login shell from $SHELL (falling back to /bin/bash or /bin/sh)
    static std::string detect_user_shell();

    // Runs a full interactive terminal emulator session attached to the user's real shell
    int run_interactive(const std::string& custom_shell = "");

    // Runs a single command through the user's real shell ($SHELL -c "command")
    int run_command(const std::string& command, const std::string& custom_shell = "");

    // Resizes the running PTY window
    void resize(int rows, int cols);

    pid_t child_pid() const { return child_pid_; }
    int master_fd() const { return master_fd_; }

private:
    int master_fd_ = -1;
    pid_t child_pid_ = -1;
};

} // namespace meridian::pty

