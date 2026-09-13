#include "PlatformPTY.hpp"

#if defined(__APPLE__)

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <util.h>

namespace meridian::platform {

class MacPTY : public PlatformPTY {
public:
    MacPTY() = default;

    ~MacPTY() override {
        close_pty();
    }

    bool spawn_shell(const std::string& shell_path, const std::vector<std::string>& args) override {
        close_pty();

        struct winsize ws{};
        ws.ws_col = size_.cols;
        ws.ws_row = size_.rows;
        ws.ws_xpixel = size_.xpixel;
        ws.ws_ypixel = size_.ypixel;

        int master_fd = -1;
        pid_t pid = forkpty(&master_fd, nullptr, nullptr, &ws);

        if (pid < 0) {
            return false;
        }

        if (pid == 0) {
            std::string target_shell = shell_path;
            if (target_shell.empty()) {
                const char* env_shell = std::getenv("SHELL");
                target_shell = (env_shell && env_shell[0]) ? env_shell : "/bin/zsh";
            }

            setenv("TERM", "xterm-256color", 1);
            setenv("COLORTERM", "truecolor", 1);
            setenv("MERIDIAN_VERSION", "2.0.0", 1);

            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(target_shell.c_str()));
            for (const auto& arg : args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            execvp(target_shell.c_str(), argv.data());
            _exit(127);
        }

        master_fd_ = master_fd;
        child_pid_ = pid;

        int flags = fcntl(master_fd_, F_GETFL, 0);
        if (flags != -1) {
            fcntl(master_fd_, F_SETFL, flags | O_NONBLOCK);
        }

        return true;
    }

    ssize_t read_master(char* buffer, size_t max_bytes, int timeout_ms) override {
        if (master_fd_ < 0) return -1;

        if (timeout_ms > 0) {
            struct pollfd pfd;
            pfd.fd = master_fd_;
            pfd.events = POLLIN;
            pfd.revents = 0;
            int ret = poll(&pfd, 1, timeout_ms);
            if (ret <= 0 || !(pfd.revents & POLLIN)) {
                return 0;
            }
        }

        ssize_t n = read(master_fd_, buffer, max_bytes);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            return -1;
        }
        return n;
    }

    ssize_t write_master(const char* data, size_t length) override {
        if (master_fd_ < 0) return -1;
        return write(master_fd_, data, length);
    }

    bool resize(const PTYSize& size) override {
        size_ = size;
        if (master_fd_ < 0) return false;

        struct winsize ws{};
        ws.ws_col = size.cols;
        ws.ws_row = size.rows;
        ws.ws_xpixel = size.xpixel;
        ws.ws_ypixel = size.ypixel;

        return (ioctl(master_fd_, TIOCSWINSZ, &ws) == 0);
    }

    bool is_alive() const override {
        if (child_pid_ <= 0) return false;
        int status = 0;
        pid_t res = waitpid(child_pid_, &status, WNOHANG);
        return (res == 0);
    }

    int64_t child_pid() const override {
        return child_pid_;
    }

    void close_pty() override {
        if (master_fd_ >= 0) {
            close(master_fd_);
            master_fd_ = -1;
        }
        if (child_pid_ > 0) {
            kill(child_pid_, SIGHUP);
            int status = 0;
            waitpid(child_pid_, &status, WNOHANG);
            child_pid_ = -1;
        }
    }

private:
    int master_fd_ = -1;
    pid_t child_pid_ = -1;
    PTYSize size_{80, 24, 0, 0};
};

std::unique_ptr<PlatformPTY> PlatformPTY::create_native_pty() {
    return std::make_unique<MacPTY>();
}

PlatformOS PlatformPTY::current_platform() {
    return PlatformOS::MacOS;
}

std::string PlatformPTY::platform_name() {
    return "macOS";
}

} // namespace meridian::platform

#endif

