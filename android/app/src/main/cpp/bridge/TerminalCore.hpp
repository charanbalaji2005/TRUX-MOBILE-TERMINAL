#pragma once
// One native terminal session: PTY + reader thread + the *existing* Meridian
// VT engine (meridian::vt::AnsiParser -> meridian::vt::ScreenBuffer).
//
// Nothing in the VT engine was modified. It is compiled straight out of
// src/core/vt and driven here exactly as the desktop frontend drives it.

#include "core/vt/ansi_parser.hpp"
#include "core/vt/screen_buffer.hpp"
#include "pty/AndroidPTY.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace meridian::android {

class TerminalCore {
public:
    TerminalCore(int cols, int rows, std::size_t scrollback);
    ~TerminalCore();

    bool start(const PtySpawnSpec& spec);

    void write_input(const char* data, size_t len);
    void resize(int cols, int rows);
    void signal(int sig);
    bool alive() const { return alive_.load(std::memory_order_relaxed); }
    int exit_status() const { return pty_.exit_status(); }

    // Monotonic counter bumped whenever the screen changed. The UI thread uses
    // it to skip redundant frames.
    uint64_t generation() const { return generation_.load(std::memory_order_acquire); }

    // Blocks until generation() > since, or timeout. Lets the render loop sleep
    // instead of polling at 60 Hz over an idle terminal.
    bool await_change(uint64_t since, int timeout_ms);

    int cols() const { return cols_; }
    int rows() const { return rows_; }

    // Packs the visible grid into `out` (3 ints per cell, row-major).
    // Returns the number of ints written, or -1 if `capacity` is too small.
    //   [0] codepoint | width<<21 | style flags<<23
    //   [1] foreground ARGB (alpha 0 => "use theme default")
    //   [2] background ARGB (alpha 0 => "use theme default")
    // If scroll_offset > 0, pulls rows from scrollback buffer.
    int snapshot(int32_t* out, int capacity, int32_t* meta, int scroll_offset = 0);

    std::string dump_text();

private:
    void reader_loop();
    void mark_dirty();

    AndroidPTY pty_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;

    std::unique_ptr<meridian::vt::ScreenBuffer> screen_;
    std::unique_ptr<meridian::vt::AnsiParser> parser_;

    std::thread reader_;
    std::atomic<bool> running_{false};
    std::atomic<bool> alive_{false};
    std::atomic<uint64_t> generation_{1};

    int cols_;
    int rows_;
};

} // namespace meridian::android
