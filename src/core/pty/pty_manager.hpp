#pragma once
// meridian-terminal-core / pty / pty_manager.hpp
//
// Owns a real Linux pseudo-terminal and the child process attached to it.
// Uses openpty()/fork()/execve() directly — no shelling out through
// system("...") or /bin/sh -c, per the project's execution-safety rule.

#include <string>
#include <sys/types.h>
#include <vector>

namespace meridian::pty {

struct SpawnOptions {
    std::string program = "/bin/bash";        // argv[0] / executable path
    std::vector<std::string> args;             // argv[1..]
    std::vector<std::string> env;              // "KEY=VALUE"; empty = inherit current environ
    std::string cwd;                           // empty = inherit
    int rows = 24;
    int cols = 80;
};

class PtyManager {
public:
    PtyManager() = default;
    ~PtyManager();

    PtyManager(const PtyManager&) = delete;
    PtyManager& operator=(const PtyManager&) = delete;

    // Opens a PTY pair, forks, and execs `opts.program` in the child with
    // its controlling terminal set to the PTY slave. Returns false and
    // sets last_error() on failure (parent process only — child failures
    // surface as a non-zero exit status from wait_for_exit()).
    bool spawn(const SpawnOptions& opts);

    ssize_t read(char* buf, std::size_t len);
    ssize_t write(const std::string& data);

    void resize(int rows, int cols);

    // Blocks until the child exits. Returns the exit code, or -1 if it
    // was killed by a signal (in which case *out_signal is set).
    int wait_for_exit(int* out_signal = nullptr);

    bool is_running() const { return child_pid_ > 0; }
    pid_t child_pid() const { return child_pid_; }
    int master_fd() const { return master_fd_; }

    const std::string& last_error() const { return last_error_; }

private:
    int master_fd_ = -1;
    pid_t child_pid_ = -1;
    std::string last_error_;
};

} // namespace meridian::pty
