#pragma once
// meridian-terminal / platform / PlatformPTY.hpp
//
// Unified cross-platform PTY abstraction layer.
// Exposes a uniform interface across Linux (openpty/termios),
// macOS (BSD PTY/sysctl), and Windows (ConPTY CreatePseudoConsole).

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

namespace meridian::platform {

enum class PlatformOS {
    Linux,
    MacOS,
    Windows,
    Unknown
};

struct PTYSize {
    uint16_t cols = 80;
    uint16_t rows = 24;
    uint16_t xpixel = 0;
    uint16_t ypixel = 0;
};

class PlatformPTY {
public:
    virtual ~PlatformPTY() = default;

    // Spawns shell process attached to pseudo-terminal
    virtual bool spawn_shell(const std::string& shell_path = "", const std::vector<std::string>& args = {}) = 0;

    // Reads pending output from PTY master. Non-blocking when timeout_ms = 0.
    virtual ssize_t read_master(char* buffer, size_t max_bytes, int timeout_ms = 0) = 0;

    // Writes input to PTY master (keystrokes, escapes, mouse reporting)
    virtual ssize_t write_master(const char* data, size_t length) = 0;

    // Resizes the pseudo-terminal window
    virtual bool resize(const PTYSize& size) = 0;

    // Checks if child process is still alive
    virtual bool is_alive() const = 0;

    // Gets child process ID
    virtual int64_t child_pid() const = 0;

    // Closes PTY and terminates child process
    virtual void close_pty() = 0;

    // Factory method to instantiate the platform-native PTY backend
    static std::unique_ptr<PlatformPTY> create_native_pty();

    // Query current platform operating system
    static PlatformOS current_platform();
    static std::string platform_name();
};

} // namespace meridian::platform

