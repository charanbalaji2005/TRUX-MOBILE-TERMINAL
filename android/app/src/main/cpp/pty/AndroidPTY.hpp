#pragma once
// Android PTY backend.
//
// Mirrors the contract of meridian::platform::PlatformPTY, but is a standalone
// class rather than a subclass: the desktop factory create_native_pty() hard
// codes the Linux/Mac/Windows triple, and the Android child process needs an
// environment bootstrap (clearenv + Meridian $PREFIX) that the desktop backend
// has no concept of.

#include <string>
#include <utility>
#include <vector>
#include <cstdint>
#include <sys/types.h>

namespace meridian::android {

struct PtySpawnSpec {
    std::string shell;                                   // absolute path to exec
    std::vector<std::string> argv;                       // argv[1..]
    std::vector<std::pair<std::string, std::string>> env;
    std::string cwd;
    uint16_t cols = 80;
    uint16_t rows = 24;
};

class AndroidPTY {
public:
    AndroidPTY() = default;
    ~AndroidPTY();
    AndroidPTY(const AndroidPTY&) = delete;
    AndroidPTY& operator=(const AndroidPTY&) = delete;

    bool spawn(const PtySpawnSpec& spec);
    ssize_t read_master(char* buf, size_t max_bytes);    // non-blocking
    ssize_t write_master(const char* data, size_t len);
    bool resize(uint16_t cols, uint16_t rows);
    bool is_alive();
    int exit_status() const { return exit_status_; }
    void send_signal(int sig);
    void close_pty();

    int master_fd() const { return master_fd_; }
    pid_t child_pid() const { return child_pid_; }

private:
    int master_fd_ = -1;
    pid_t child_pid_ = -1;
    int exit_status_ = -1;
    bool reaped_ = false;
};

} // namespace meridian::android
