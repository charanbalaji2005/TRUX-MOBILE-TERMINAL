// meridian-terminal-core / pty / pty_manager.cpp
#include "pty_manager.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#if defined(__APPLE__)
#include <util.h>
#elif defined(_WIN32)
// Windows PTY handled in platform/WindowsConPTY.cpp
#else
#include <pty.h>
#endif
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

namespace meridian::pty {

PtyManager::~PtyManager() {
    if (master_fd_ >= 0) ::close(master_fd_);
}

bool PtyManager::spawn(const SpawnOptions& opts) {
    int master = -1, slave = -1;
    struct winsize ws{};
    ws.ws_row = static_cast<unsigned short>(opts.rows);
    ws.ws_col = static_cast<unsigned short>(opts.cols);

    if (openpty(&master, &slave, nullptr, nullptr, &ws) != 0) {
        last_error_ = std::string("openpty failed: ") + std::strerror(errno);
        return false;
    }

    // Build argv/envp in the parent, before forking, so the child only has
    // to call execve — no allocation races between fork and exec.
    std::vector<std::string> argv_storage;
    argv_storage.push_back(opts.program);
    for (auto& a : opts.args) argv_storage.push_back(a);
    std::vector<char*> argv;
    argv.reserve(argv_storage.size() + 1);
    for (auto& s : argv_storage) argv.push_back(s.data());
    argv.push_back(nullptr);

    bool use_custom_env = !opts.env.empty();
    std::vector<std::string> env_storage = opts.env;
    std::vector<char*> envp;
    if (use_custom_env) {
        envp.reserve(env_storage.size() + 1);
        for (auto& s : env_storage) envp.push_back(s.data());
        envp.push_back(nullptr);
    }

    pid_t pid = fork();
    if (pid < 0) {
        last_error_ = std::string("fork failed: ") + std::strerror(errno);
        ::close(master);
        ::close(slave);
        return false;
    }

    if (pid == 0) {
        // ---- child ----
        ::close(master);
        setsid();
        if (ioctl(slave, TIOCSCTTY, 0) != 0) _exit(126);
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        if (slave > STDERR_FILENO) ::close(slave);

        if (!opts.cwd.empty() && chdir(opts.cwd.c_str()) != 0) _exit(127);

        if (use_custom_env) {
            execve(opts.program.c_str(), argv.data(), envp.data());
        } else {
            execv(opts.program.c_str(), argv.data());
        }
        // execve/execv only returns on failure.
        _exit(127);
    }

    // ---- parent ----
    ::close(slave);
    master_fd_ = master;
    child_pid_ = pid;
    return true;
}

ssize_t PtyManager::read(char* buf, std::size_t len) {
    if (master_fd_ < 0) return -1;
    return ::read(master_fd_, buf, len);
}

ssize_t PtyManager::write(const std::string& data) {
    if (master_fd_ < 0) return -1;
    return ::write(master_fd_, data.data(), data.size());
}

void PtyManager::resize(int rows, int cols) {
    if (master_fd_ < 0) return;
    struct winsize ws{};
    ws.ws_row = static_cast<unsigned short>(rows);
    ws.ws_col = static_cast<unsigned short>(cols);
    ioctl(master_fd_, TIOCSWINSZ, &ws);
}

int PtyManager::wait_for_exit(int* out_signal) {
    if (child_pid_ <= 0) return -1;
    int status = 0;
    pid_t r = waitpid(child_pid_, &status, 0);
    if (r < 0) return -1;
    child_pid_ = -1;
    if (WIFEXITED(status)) {
        if (out_signal) *out_signal = 0;
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        if (out_signal) *out_signal = WTERMSIG(status);
        return -1;
    }
    return -1;
}

} // namespace meridian::pty
