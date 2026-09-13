#include "AndroidPTY.hpp"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <pty.h>          // Bionic provides forkpty()/openpty() from API 23
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <android/log.h>

// TEMP_FAILURE_RETRY: retry on EINTR; not defined by Bionic, so define it here.
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expr) \
    ({ decltype(expr) _r; do { _r = (expr); } while (_r == -1 && errno == EINTR); _r; })
#endif

extern char** environ;

#define LOG_TAG "MeridianPTY"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace meridian::android {

AndroidPTY::~AndroidPTY() { close_pty(); }

bool AndroidPTY::spawn(const PtySpawnSpec& spec) {
    close_pty();

    struct winsize ws{};
    ws.ws_col = spec.cols;
    ws.ws_row = spec.rows;

    int master = -1;
    pid_t pid = forkpty(&master, nullptr, nullptr, &ws);
    if (pid < 0) {
        LOGE("forkpty() failed: %s", strerror(errno));
        return false;
    }

    if (pid == 0) {
        // ---- child ---------------------------------------------------------
        // forkpty() has already called login_tty(): the slave is our
        // controlling terminal and fds 0/1/2 are wired to it.

        // The zygote hands us a hostile environment (ANDROID_DATA, LD_*,
        // BOOTCLASSPATH...). Wipe it and install only the Meridian userspace.
        environ = nullptr;
        static char* empty_env[] = { nullptr };
        environ = empty_env;
        for (const auto& kv : spec.env) {
            setenv(kv.first.c_str(), kv.second.c_str(), 1);
        }

        if (!spec.cwd.empty()) {
            (void) chdir(spec.cwd.c_str());   // non-fatal; shell falls back to $HOME
        }

        // Restore default signal dispositions. The Android runtime leaves
        // several signals blocked or handled, which breaks Ctrl+C / Ctrl+Z.
        sigset_t all;
        sigfillset(&all);
        sigprocmask(SIG_UNBLOCK, &all, nullptr);
        signal(SIGCHLD, SIG_DFL);
        signal(SIGPIPE, SIG_DFL);
        signal(SIGINT,  SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(spec.shell.c_str()));
        for (const auto& a : spec.argv) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);

        execv(spec.shell.c_str(), argv.data());

        // Exec failed. Report on the PTY so the user sees a real diagnostic
        // rather than an empty black screen.
        dprintf(2, "meridian: cannot execute %s: %s\r\n",
                spec.shell.c_str(), strerror(errno));
        _exit(127);
    }

    // ---- parent ------------------------------------------------------------
    master_fd_ = master;
    child_pid_ = pid;
    reaped_ = false;
    exit_status_ = -1;

    // Set FD_CLOEXEC so the master FD is not inherited by any exec'd child.
    // Do this before setting O_NONBLOCK to avoid a race.
    int flags = fcntl(master_fd_, F_GETFD, 0);
    if (flags != -1) fcntl(master_fd_, F_SETFD, flags | FD_CLOEXEC);

    // Set non-blocking so read() returns EAGAIN instead of blocking when idle.
    flags = fcntl(master_fd_, F_GETFL, 0);
    if (flags != -1) fcntl(master_fd_, F_SETFL, flags | O_NONBLOCK);

    LOGI("PTY spawned: pid=%d master_fd=%d shell=%s", pid, master, spec.shell.c_str());
    return true;
}

ssize_t AndroidPTY::read_master(char* buf, size_t max_bytes) {
    if (master_fd_ < 0) return -1;
    ssize_t n = TEMP_FAILURE_RETRY(::read(master_fd_, buf, max_bytes));
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
    return n;   // 0, or -1/EIO, means the slave closed -> the process is gone
}

ssize_t AndroidPTY::write_master(const char* data, size_t len) {
    if (master_fd_ < 0) return -1;
    size_t written = 0;
    while (written < len) {
        ssize_t n = TEMP_FAILURE_RETRY(::write(master_fd_, data + written, len - written));
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            return -1;
        }
        written += static_cast<size_t>(n);
    }
    return static_cast<ssize_t>(written);
}

bool AndroidPTY::resize(uint16_t cols, uint16_t rows) {
    if (master_fd_ < 0) return false;
    struct winsize ws{};
    ws.ws_col = cols;
    ws.ws_row = rows;
    if (ioctl(master_fd_, TIOCSWINSZ, &ws) != 0) return false;
    // The kernel delivers SIGWINCH to the foreground group for us.
    return true;
}

bool AndroidPTY::is_alive() {
    if (child_pid_ <= 0 || reaped_) return false;
    int status = 0;
    pid_t r = waitpid(child_pid_, &status, WNOHANG);
    if (r == child_pid_) {
        reaped_ = true;
        exit_status_ = WIFEXITED(status)   ? WEXITSTATUS(status)
                     : WIFSIGNALED(status) ? 128 + WTERMSIG(status)
                                           : -1;
        LOGI("Child pid=%d exited with status=%d", child_pid_, exit_status_);
        return false;
    }
    return r == 0;
}

void AndroidPTY::send_signal(int sig) {
    if (child_pid_ > 0 && !reaped_) {
        killpg(child_pid_, sig);   // whole group, so pipelines die too
    }
}

void AndroidPTY::close_pty() {
    if (child_pid_ > 0 && !reaped_) {
        // Graceful: SIGHUP + SIGCONT, then wait up to 1 second.
        killpg(child_pid_, SIGHUP);
        killpg(child_pid_, SIGCONT);
        for (int i = 0; i < 40; ++i) {
            if (!is_alive()) break;
            usleep(25 * 1000);
        }
        // Force-kill if still alive
        if (!reaped_) {
            killpg(child_pid_, SIGKILL);
            int status = 0;
            TEMP_FAILURE_RETRY(waitpid(child_pid_, &status, 0));
            reaped_ = true;
        }
    }
    if (master_fd_ >= 0) {
        ::close(master_fd_);
        master_fd_ = -1;
    }
    child_pid_ = -1;
}

} // namespace meridian::android
