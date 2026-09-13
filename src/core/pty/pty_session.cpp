// meridian-terminal-core / pty / pty_session.cpp
#include "pty_session.hpp"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#if defined(__APPLE__)
#include <util.h>
#elif defined(_WIN32)
// Windows
#else
#include <pty.h>
#endif
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>

namespace meridian::pty {

namespace {

static volatile sig_atomic_t g_winch_received = 0;
static int g_active_master_fd = -1;

void handle_sigwinch(int /*sig*/) {
    g_winch_received = 1;
    if (g_active_master_fd >= 0) {
        struct winsize ws{};
        if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
            ioctl(g_active_master_fd, TIOCSWINSZ, &ws);
        }
    }
}

} // namespace

PtySession::PtySession() = default;

PtySession::~PtySession() {
    if (master_fd_ >= 0) {
        ::close(master_fd_);
        master_fd_ = -1;
    }
    if (child_pid_ > 0) {
        int status = 0;
        waitpid(child_pid_, &status, WNOHANG);
        child_pid_ = -1;
    }
}

std::string PtySession::detect_user_shell() {
    const char* env_shell = std::getenv("SHELL");
    if (env_shell && env_shell[0] && access(env_shell, X_OK) == 0) {
        return std::string(env_shell);
    }
    if (access("/bin/zsh", X_OK) == 0) return "/bin/zsh";
    if (access("/bin/bash", X_OK) == 0) return "/bin/bash";
    if (access("/bin/fish", X_OK) == 0) return "/bin/fish";
    return "/bin/sh";
}

int PtySession::run_interactive(const std::string& custom_shell) {
    std::string shell_path = custom_shell.empty() ? detect_user_shell() : custom_shell;

    struct winsize ws{};
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != 0 || ws.ws_col == 0) {
        ws.ws_col = 80;
        ws.ws_row = 24;
    }

    int master = -1, slave = -1;
    if (openpty(&master, &slave, nullptr, nullptr, &ws) != 0) {
        std::cerr << "meridian: openpty failed: " << std::strerror(errno) << "\n";
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "meridian: fork failed: " << std::strerror(errno) << "\n";
        ::close(master);
        ::close(slave);
        return 1;
    }

    if (pid == 0) {
        // ---- CHILD: Controlling Terminal & User Login Shell ----
        ::close(master);
        setsid();
        if (ioctl(slave, TIOCSCTTY, 0) != 0) {
            _exit(126);
        }

        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        if (slave > STDERR_FILENO) ::close(slave);

        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);
        setenv("MERIDIAN_TERMINAL", "2.0.0", 1);

        char* argv[2];
        argv[0] = const_cast<char*>(shell_path.c_str());
        argv[1] = nullptr;

        execvp(shell_path.c_str(), argv);
        _exit(127);
    }

    // ---- PARENT: Terminal Raw Mode & I/O Event Loop ----
    ::close(slave);
    master_fd_ = master;
    child_pid_ = pid;
    g_active_master_fd = master;

    // Set non-blocking on master fd
    int flags = fcntl(master_fd_, F_GETFL, 0);
    if (flags != -1) {
        fcntl(master_fd_, F_SETFL, flags | O_NONBLOCK);
    }

    // Configure SIGWINCH handler
    struct sigaction sa_winch{}, old_sa_winch{};
    sa_winch.sa_handler = handle_sigwinch;
    sigemptyset(&sa_winch.sa_mask);
    sa_winch.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa_winch, &old_sa_winch);

    // Save and switch stdin to raw unbuffered mode
    struct termios orig_termios{}, raw_termios{};
    bool is_interactive_tty = (isatty(STDIN_FILENO) == 1);
    if (is_interactive_tty) {
        tcgetattr(STDIN_FILENO, &orig_termios);
        raw_termios = orig_termios;
        cfmakeraw(&raw_termios);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw_termios);
    }

    char buffer[4096];
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = master_fd_;
    fds[1].events = POLLIN;

    int child_exit_code = 0;

    while (child_pid_ > 0) {
        if (g_winch_received) {
            g_winch_received = 0;
            if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
                ioctl(master_fd_, TIOCSWINSZ, &ws);
            }
        }

        int ret = poll(fds, 2, 50);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        // 1. Data from User Stdin -> Write to PTY Master
        if (fds[0].revents & POLLIN) {
            ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (n > 0) {
                ssize_t written = 0;
                while (written < n) {
                    ssize_t w = write(master_fd_, buffer + written, n - written);
                    if (w <= 0) break;
                    written += w;
                }
            }
        }

        // 2. Data from PTY Master -> Write to User Stdout
        if (fds[1].revents & POLLIN) {
            ssize_t n = read(master_fd_, buffer, sizeof(buffer));
            if (n > 0) {
                ssize_t written = 0;
                while (written < n) {
                    ssize_t w = write(STDOUT_FILENO, buffer + written, n - written);
                    if (w <= 0) break;
                    written += w;
                }
            } else if (n == 0) {
                break;
            }
        }

        // 3. Child Exit Detection
        if (fds[1].revents & (POLLHUP | POLLERR)) {
            // Drain remaining bytes from master fd
            while (true) {
                ssize_t n = read(master_fd_, buffer, sizeof(buffer));
                if (n <= 0) break;
                ssize_t written = 0;
                while (written < n) {
                    ssize_t w = write(STDOUT_FILENO, buffer + written, n - written);
                    if (w <= 0) break;
                    written += w;
                }
            }
            break;
        }

        int status = 0;
        pid_t r = waitpid(child_pid_, &status, WNOHANG);
        if (r > 0) {
            if (WIFEXITED(status)) {
                child_exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                child_exit_code = 128 + WTERMSIG(status);
            }
            child_pid_ = -1;
            break;
        }
    }

    // Restore terminal modes
    if (is_interactive_tty) {
        tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
    }
    sigaction(SIGWINCH, &old_sa_winch, nullptr);
    g_active_master_fd = -1;

    if (master_fd_ >= 0) {
        ::close(master_fd_);
        master_fd_ = -1;
    }

    if (child_pid_ > 0) {
        int status = 0;
        waitpid(child_pid_, &status, 0);
        if (WIFEXITED(status)) child_exit_code = WEXITSTATUS(status);
        child_pid_ = -1;
    }

    return child_exit_code;
}

int PtySession::run_command(const std::string& command, const std::string& custom_shell) {
    std::string shell_path = custom_shell.empty() ? detect_user_shell() : custom_shell;

    struct winsize ws{};
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != 0 || ws.ws_col == 0) {
        ws.ws_col = 80;
        ws.ws_row = 24;
    }

    int master = -1, slave = -1;
    if (openpty(&master, &slave, nullptr, nullptr, &ws) != 0) {
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        ::close(master);
        ::close(slave);
        return 1;
    }

    if (pid == 0) {
        ::close(master);
        setsid();
        ioctl(slave, TIOCSCTTY, 0);
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        if (slave > 2) ::close(slave);

        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);

        char* argv[4];
        argv[0] = const_cast<char*>(shell_path.c_str());
        argv[1] = const_cast<char*>("-c");
        argv[2] = const_cast<char*>(command.c_str());
        argv[3] = nullptr;

        execvp(shell_path.c_str(), argv);
        _exit(127);
    }

    ::close(slave);
    master_fd_ = master;
    child_pid_ = pid;

    char buffer[4096];
    int child_exit_code = 0;

    while (child_pid_ > 0) {
        ssize_t n = read(master_fd_, buffer, sizeof(buffer));
        if (n > 0) {
            ssize_t written = 0;
            while (written < n) {
                ssize_t w = write(STDOUT_FILENO, buffer + written, n - written);
                if (w <= 0) break;
                written += w;
            }
        } else {
            break;
        }

        int status = 0;
        pid_t r = waitpid(child_pid_, &status, WNOHANG);
        if (r > 0) {
            if (WIFEXITED(status)) child_exit_code = WEXITSTATUS(status);
            child_pid_ = -1;
            break;
        }
    }

    if (master_fd_ >= 0) {
        ::close(master_fd_);
        master_fd_ = -1;
    }

    if (child_pid_ > 0) {
        int status = 0;
        waitpid(child_pid_, &status, 0);
        if (WIFEXITED(status)) child_exit_code = WEXITSTATUS(status);
        child_pid_ = -1;
    }

    return child_exit_code;
}

void PtySession::resize(int rows, int cols) {
    if (master_fd_ < 0) return;
    struct winsize ws{};
    ws.ws_row = static_cast<unsigned short>(rows);
    ws.ws_col = static_cast<unsigned short>(cols);
    ioctl(master_fd_, TIOCSWINSZ, &ws);
}

} // namespace meridian::pty

