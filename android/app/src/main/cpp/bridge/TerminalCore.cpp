#include "bridge/TerminalCore.hpp"

#include <poll.h>
#include <pthread.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "MeridianCore"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace meridian::android {

namespace {

// xterm 256-colour palette, resolved natively so the renderer only ever sees
// concrete ARGB and never has to know about SGR semantics.
uint32_t palette_argb(uint8_t idx) {
    static const uint32_t base16[16] = {
        0xFF000000, 0xFFCD3131, 0xFF0DBC79, 0xFFE5E510,
        0xFF2472C8, 0xFFBC3FBC, 0xFF11A8CD, 0xFFE5E5E5,
        0xFF666666, 0xFFF14C4C, 0xFF23D18B, 0xFFF5F543,
        0xFF3B8EEA, 0xFFD670D6, 0xFF29B8DB, 0xFFFFFFFF
    };
    if (idx < 16) return base16[idx];
    if (idx < 232) {
        int i = idx - 16;
        static const int lv[6] = {0, 95, 135, 175, 215, 255};
        int r = lv[(i / 36) % 6], g = lv[(i / 6) % 6], b = lv[i % 6];
        return 0xFF000000u | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
    }
    int v = 8 + (idx - 232) * 10;
    return 0xFF000000u | (uint32_t(v) << 16) | (uint32_t(v) << 8) | uint32_t(v);
}

uint32_t color_argb(const meridian::vt::Color& c) {
    using Kind = meridian::vt::Color::Kind;
    switch (c.kind) {
        case Kind::Default: return 0x00000000u;   // alpha 0 -> theme default
        case Kind::Indexed: return palette_argb(c.index);
        case Kind::Rgb:
            return 0xFF000000u | (uint32_t(c.r) << 16) | (uint32_t(c.g) << 8) | uint32_t(c.b);
    }
    return 0x00000000u;
}

} // namespace

TerminalCore::TerminalCore(int cols, int rows, std::size_t scrollback)
    : cols_(cols), rows_(rows) {
    screen_ = std::make_unique<meridian::vt::ScreenBuffer>(rows, cols, scrollback);
    parser_ = std::make_unique<meridian::vt::AnsiParser>(*screen_);
}

TerminalCore::~TerminalCore() {
    running_.store(false);
    pty_.close_pty();    // unblocks the poll() in reader_loop
    cv_.notify_all();
    if (reader_.joinable()) reader_.join();
}

bool TerminalCore::start(const PtySpawnSpec& spec) {
    PtySpawnSpec s = spec;
    s.cols = static_cast<uint16_t>(cols_);
    s.rows = static_cast<uint16_t>(rows_);
    if (!pty_.spawn(s)) return false;

    alive_.store(true);
    running_.store(true);
    reader_ = std::thread(&TerminalCore::reader_loop, this);

    // Name the thread for debuggability in Android Studio profiler / logcat.
    pthread_setname_np(reader_.native_handle(), "meridian-pty-rd");

    return true;
}

void TerminalCore::reader_loop() {
    // 64 KiB: large enough that `yes | head -100000` does not thrash, small
    // enough to stay off the heap hot path. Reused for the whole session --
    // no per-read allocation.
    static constexpr size_t kBufSize = 64 * 1024;
    auto buf = std::make_unique<char[]>(kBufSize);

    LOGI("Reader thread started");

    while (running_.load(std::memory_order_relaxed)) {
        int fd = pty_.master_fd();
        if (fd < 0) break;

        struct pollfd pfd{};
        pfd.fd = fd;
        pfd.events = POLLIN;
        int pr = poll(&pfd, 1, 100);
        if (pr < 0) {
            if (errno == EINTR) continue;
            LOGE("poll() failed: %s", strerror(errno));
            break;
        }
        if (pr == 0) {
            // Idle tick: notice a child that exited without closing the PTY.
            if (!pty_.is_alive()) {
                LOGI("Child exited (detected via idle tick)");
                break;
            }
            continue;
        }

        if (pfd.revents & (POLLHUP | POLLERR)) {
            // Drain whatever is still buffered before declaring the session dead.
            ssize_t n;
            while ((n = pty_.read_master(buf.get(), kBufSize)) > 0) {
                std::lock_guard<std::mutex> lock(mutex_);
                parser_->feed(buf.get(), static_cast<size_t>(n));
            }
            mark_dirty();
            LOGI("PTY HUP/ERR — session ended");
            break;
        }

        if (pfd.revents & POLLIN) {
            ssize_t n = pty_.read_master(buf.get(), kBufSize);
            if (n > 0) {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    parser_->feed(buf.get(), static_cast<size_t>(n));
                }
                mark_dirty();
            } else if (n < 0) {
                LOGI("read_master returned EIO — slave closed");
                break;
            }
        }
    }

    // Final: reap child so exit_status() is populated
    pty_.is_alive();
    alive_.store(false);

    // Always notify waiters on exit so awaitChange() returns promptly.
    mark_dirty();

    LOGI("Reader thread exiting");
}

void TerminalCore::mark_dirty() {
    generation_.fetch_add(1, std::memory_order_release);
    cv_.notify_all();
}

bool TerminalCore::await_change(uint64_t since, int timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                        [&] { return generation_.load(std::memory_order_acquire) > since
                                     || !running_.load(std::memory_order_relaxed); });
}

void TerminalCore::write_input(const char* data, size_t len) {
    pty_.write_master(data, len);
}

void TerminalCore::resize(int cols, int rows) {
    if (cols <= 0 || rows <= 0) return;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cols == cols_ && rows == rows_) return;
        cols_ = cols;
        rows_ = rows;
        screen_->resize(rows, cols);
    }
    pty_.resize(static_cast<uint16_t>(cols), static_cast<uint16_t>(rows));
    mark_dirty();
}

void TerminalCore::signal(int sig) { pty_.send_signal(sig); }

int TerminalCore::snapshot(int32_t* out, int capacity, int32_t* meta, int scroll_offset) {
    std::lock_guard<std::mutex> lock(mutex_);
    const int r = screen_->rows();
    const int c = screen_->cols();
    const int needed = r * c * 3;
    if (capacity < needed) return -needed;  // Return negative needed count (not -1)

    const auto& sb = screen_->scrollback();
    const int sb_count = static_cast<int>(sb.size());
    const int offset = std::clamp(scroll_offset, 0, sb_count);
    const int start_idx = sb_count - offset;

    static const meridian::vt::Cell blank_cell{};

    int k = 0;
    for (int y = 0; y < r; ++y) {
        int line_idx = start_idx + y;
        const std::vector<meridian::vt::Cell>* sb_row = nullptr;
        if (line_idx < sb_count) {
            sb_row = &sb[static_cast<std::size_t>(line_idx)];
        }

        for (int x = 0; x < c; ++x) {
            const meridian::vt::Cell* cell_ptr = nullptr;
            if (sb_row != nullptr) {
                if (x < static_cast<int>(sb_row->size())) {
                    cell_ptr = &(*sb_row)[static_cast<std::size_t>(x)];
                } else {
                    cell_ptr = &blank_cell;
                }
            } else {
                int screen_y = line_idx - sb_count;
                cell_ptr = &screen_->cell_at(screen_y, x);
            }

            const meridian::vt::Cell& cell = *cell_ptr;
            const meridian::vt::Attributes& a = cell.attrs;

            int32_t packed = static_cast<int32_t>(cell.codepoint & 0x1FFFFF);
            packed |= (static_cast<int32_t>(cell.width & 0x3) << 21);
            if (a.bold)          packed |= (1 << 23);
            if (a.italic)        packed |= (1 << 24);
            if (a.underline)     packed |= (1 << 25);
            if (a.strikethrough) packed |= (1 << 26);
            if (a.reverse)       packed |= (1 << 27);
            if (a.dim)           packed |= (1 << 28);
            if (a.hidden)        packed |= (1 << 29);

            out[k++] = packed;
            out[k++] = static_cast<int32_t>(color_argb(a.fg));
            out[k++] = static_cast<int32_t>(color_argb(a.bg));
        }
    }

    if (offset == 0) {
        meta[0] = screen_->cursor_row();
        meta[1] = screen_->cursor_col();
        meta[4] = 1;
    } else {
        int visual_cursor_row = screen_->cursor_row() + offset;
        if (visual_cursor_row >= 0 && visual_cursor_row < r) {
            meta[0] = visual_cursor_row;
            meta[1] = screen_->cursor_col();
            meta[4] = 1;
        } else {
            meta[0] = -1;
            meta[1] = -1;
            meta[4] = 0;
        }
    }

    meta[2] = r;
    meta[3] = c;
    meta[5] = sb_count;
    meta[6] = offset;
    return k;
}

std::string TerminalCore::dump_text() {
    std::lock_guard<std::mutex> lock(mutex_);
    return screen_->dump_text();
}

} // namespace meridian::android
