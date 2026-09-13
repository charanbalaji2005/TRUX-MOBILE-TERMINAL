# Meridian Terminal — full source listing (session 3 update)

Every source file, concatenated for browsing. Companion to the
tarball, not a replacement for it. This update adds: CommandContext
(real cwd/git/OS info gathering), a shared Config store extracted from
the AI-only version, terminal settings + keybindings config, a
credential store, the extracted Shell/REPL class, and
tests/manual_core_test.sh (spec section 78's exact command sequence).
See docs/status.md for the full ledger and docs/architecture.md for
the explicit mapping between spec-named files and what actually exists
here.

---

### Build files

## `Makefile`

```makefile
# Meridian Terminal — Makefile
#
# This is the build path actually verified inside the development
# sandbox that produced this repo (no cmake/Qt6/network available there
# — see docs/build.md). It builds and tests everything that does NOT
# depend on Qt6: the PTY layer, the VT/ANSI engine, Meridian Shell (with
# real job control), the local-only Meridian AI layer, terminal/keybind
# config, and the credential-store fallback.
#
# On a real Linux desktop with cmake + Qt6 installed, use CMakeLists.txt
# instead — it builds this same code plus the GUI target.

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -g
LDLIBS := -lutil

BUILD := build

CORE_SRC := \
    src/core/vt/screen_buffer.cpp \
    src/core/vt/ansi_parser.cpp \
    src/core/pty/pty_manager.cpp \
    src/core/config.cpp

SHELL_SRC := \
    src/shell/lexer.cpp \
    src/shell/parser.cpp \
    src/shell/executor.cpp \
    src/shell/builtins.cpp \
    src/shell/shell.cpp

AI_SRC := \
    src/ai/secret_redactor.cpp \
    src/ai/risk_classifier.cpp \
    src/ai/command_analyzer.cpp \
    src/ai/command_context.cpp \
    src/ai/ai_controller.cpp

CONFIG_SRC := \
    src/config/terminal_config.cpp \
    src/config/keybindings.cpp

SECURITY_SRC := \
    src/security/credential_store.cpp

TEST_SRC := \
    tests/test_main.cpp \
    tests/test_screen_buffer.cpp \
    tests/test_ansi_parser.cpp \
    tests/test_pty_manager.cpp \
    tests/test_shell_lexer.cpp \
    tests/test_shell_parser.cpp \
    tests/test_shell_executor.cpp \
    tests/test_job_control.cpp \
    tests/test_shell_class.cpp \
    tests/test_ai.cpp \
    tests/test_command_context.cpp \
    tests/test_config.cpp \
    tests/test_security.cpp

CORE_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CORE_SRC))
SHELL_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SHELL_SRC))
AI_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(AI_SRC))
CONFIG_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CONFIG_SRC))
SECURITY_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SECURITY_SRC))
TEST_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(TEST_SRC))

.PHONY: all test demo shell meridian manual-test clean

all: $(BUILD)/meridian_tests $(BUILD)/meridian_demo $(BUILD)/meridian-shell $(BUILD)/meridian

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/meridian_tests: $(CORE_OBJ) $(SHELL_OBJ) $(AI_OBJ) $(CONFIG_OBJ) $(SECURITY_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/meridian_demo: $(CORE_OBJ) $(BUILD)/src/app/demo_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/meridian-shell: $(SHELL_OBJ) $(BUILD)/src/app/shell_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/meridian: $(BUILD)/src/core/config.o $(AI_OBJ) $(BUILD)/src/app/meridian_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

test: $(BUILD)/meridian_tests $(BUILD)/meridian-shell
	$(BUILD)/meridian_tests

manual-test: $(BUILD)/meridian-shell
	./tests/manual_core_test.sh

demo: $(BUILD)/meridian_demo
	$(BUILD)/meridian_demo

shell: $(BUILD)/meridian-shell
	$(BUILD)/meridian-shell

meridian: $(BUILD)/meridian
	$(BUILD)/meridian $(ARGS)

clean:
	rm -rf $(BUILD)
```

---

## `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(meridian-terminal
    VERSION 0.1.0
    DESCRIPTION "Meridian Terminal — an independently engineered Linux terminal emulator"
    LANGUAGES CXX
)

# --------------------------------------------------------------------------
# This file builds everything that has actually been implemented and
# tested so far: the PTY layer, the ANSI/VT engine, Meridian Shell
# (including real POSIX job control), terminal/keybinding config, a
# credential-store fallback, and the local-only half of Meridian AI
# (typo analysis, risk classification, secret redaction, local context
# gathering — no network calls anywhere in this build). It does NOT yet
# reference a GUI target or any cloud AI provider — see docs/status.md
# and docs/ai.md for what's real vs. planned and why nothing here would
# fail to configure on a machine without Qt6 or network access.
# --------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

add_compile_options(-Wall -Wextra)

option(MERIDIAN_BUILD_TESTS "Build the Meridian test suite" ON)
option(MERIDIAN_ENABLE_ASAN "Build with AddressSanitizer + UBSan" OFF)

if(MERIDIAN_ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

# ---- meridian-core: PTY + VT/ANSI engine + shared config (no Qt) ---------
add_library(meridian-core STATIC
    src/core/pty/pty_manager.cpp
    src/core/vt/screen_buffer.cpp
    src/core/vt/ansi_parser.cpp
    src/core/config.cpp
)
target_include_directories(meridian-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-core PUBLIC util)

# ---- meridian-shell-lib: lexer/parser/executor/builtins/Shell ------------
add_library(meridian-shell-lib STATIC
    src/shell/lexer.cpp
    src/shell/parser.cpp
    src/shell/executor.cpp
    src/shell/builtins.cpp
    src/shell/shell.cpp
)
target_include_directories(meridian-shell-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-shell-lib PUBLIC util)

# ---- meridian-ai-lib: local-only Meridian AI layer (no network) --------
add_library(meridian-ai-lib STATIC
    src/ai/secret_redactor.cpp
    src/ai/risk_classifier.cpp
    src/ai/command_analyzer.cpp
    src/ai/command_context.cpp
    src/ai/ai_controller.cpp
)
target_include_directories(meridian-ai-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-ai-lib PUBLIC meridian-core)

# ---- meridian-config-lib: terminal settings + keybindings ----------------
add_library(meridian-config-lib STATIC
    src/config/terminal_config.cpp
    src/config/keybindings.cpp
)
target_include_directories(meridian-config-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-config-lib PUBLIC meridian-core)

# ---- meridian-security-lib: credential storage ---------------------------
add_library(meridian-security-lib STATIC
    src/security/credential_store.cpp
)
target_include_directories(meridian-security-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- meridian: the `meridian ai ...` CLI ----------------------------------
add_executable(meridian src/app/meridian_main.cpp)
target_link_libraries(meridian PRIVATE meridian-ai-lib)

# ---- meridian-shell: standalone shell binary ------------------------------
add_executable(meridian-shell src/app/shell_main.cpp)
target_link_libraries(meridian-shell PRIVATE meridian-shell-lib)

# ---- meridian_demo: headless PTY + ANSI integration demo -----------------
add_executable(meridian_demo src/app/demo_main.cpp)
target_link_libraries(meridian_demo PRIVATE meridian-core)

# ---- Tests -----------------------------------------------------------------
if(MERIDIAN_BUILD_TESTS)
    enable_testing()
    add_executable(meridian_tests
        tests/test_main.cpp
        tests/test_screen_buffer.cpp
        tests/test_ansi_parser.cpp
        tests/test_pty_manager.cpp
        tests/test_shell_lexer.cpp
        tests/test_shell_parser.cpp
        tests/test_shell_executor.cpp
        tests/test_job_control.cpp
        tests/test_shell_class.cpp
        tests/test_ai.cpp
        tests/test_command_context.cpp
        tests/test_config.cpp
        tests/test_security.cpp
    )
    target_link_libraries(meridian_tests PRIVATE
        meridian-core meridian-shell-lib meridian-ai-lib
        meridian-config-lib meridian-security-lib
    )
    add_test(NAME meridian_tests COMMAND meridian_tests)
    add_test(NAME meridian_manual_core_test
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/manual_core_test.sh
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    # googletest is the long-term intent here (see docs/status.md for why
    # tests/mini_test.hpp is used instead in the environment this repo was
    # first built in). If FetchContent/find_package(GTest) is available in
    # your environment, swapping the harness is a mechanical change: the
    # MTEST/ASSERT_* macros map directly onto TEST/EXPECT_*.

    # ---- Qt6 GUI (future milestone) ----
    # find_package(Qt6 COMPONENTS Widgets Network QUIET)
    # if(Qt6_FOUND)
    #     add_subdirectory(src/gui)
    # else()
    #     message(STATUS "Qt6 not found: skipping GUI target (meridian-app). "
    #                     "Core terminal engine and Meridian Shell still build fine.")
    # endif()
endif()

message(STATUS "Meridian Terminal ${PROJECT_VERSION}")
message(STATUS "  meridian-core        : PTY + VT/ANSI engine + shared config store")
message(STATUS "  meridian-shell-lib   : Meridian Shell (lexer/parser/executor/builtins, real job control, Shell/REPL class)")
message(STATUS "  meridian-shell       : standalone shell binary")
message(STATUS "  meridian-ai-lib      : local-only Meridian AI (analyzer/risk/redaction/context/controller)")
message(STATUS "  meridian-config-lib  : terminal settings + keybindings")
message(STATUS "  meridian-security-lib: credential storage (file fallback)")
message(STATUS "  meridian             : the `meridian ai ...` CLI")
message(STATUS "  meridian_demo        : headless PTY+ANSI integration demo")
message(STATUS "  meridian_tests       : test suite (enabled: ${MERIDIAN_BUILD_TESTS})")
message(STATUS "  GUI (Qt6)            : not yet implemented — see docs/status.md")
```

---

### src/core — VT engine, PTY, shared config store

## `src/core/vt/types.hpp`

```cpp
#pragma once
// meridian-terminal-core / vt / types.hpp
//
// Fundamental cell and attribute types for the terminal screen model.
// No Qt dependency — this is pure C++20 so it can be built, unit tested,
// and reused by any future GUI or headless frontend.

#include <cstdint>

namespace meridian::vt {

// A terminal color: either "use the default", a 0-255 palette index
// (16-color and 256-color modes both fit here), or a 24-bit RGB triple.
struct Color {
    enum class Kind : uint8_t { Default, Indexed, Rgb };

    Kind kind = Kind::Default;
    uint8_t index = 0;
    uint8_t r = 0, g = 0, b = 0;

    static Color default_color() { return Color{}; }

    static Color indexed(uint8_t idx) {
        Color c;
        c.kind = Kind::Indexed;
        c.index = idx;
        return c;
    }

    static Color rgb(uint8_t rr, uint8_t gg, uint8_t bb) {
        Color c;
        c.kind = Kind::Rgb;
        c.r = rr;
        c.g = gg;
        c.b = bb;
        return c;
    }

    bool operator==(const Color& o) const {
        if (kind != o.kind) return false;
        switch (kind) {
            case Kind::Default: return true;
            case Kind::Indexed: return index == o.index;
            case Kind::Rgb: return r == o.r && g == o.g && b == o.b;
        }
        return false;
    }
    bool operator!=(const Color& o) const { return !(*this == o); }
};

// SGR-derived attributes attached to a single cell.
struct Attributes {
    Color fg = Color::default_color();
    Color bg = Color::default_color();
    bool bold = false;
    bool dim = false;
    bool italic = false;
    bool underline = false;
    bool blink = false;
    bool reverse = false;
    bool hidden = false;
    bool strikethrough = false;

    bool operator==(const Attributes& o) const {
        return fg == o.fg && bg == o.bg && bold == o.bold && dim == o.dim &&
               italic == o.italic && underline == o.underline && blink == o.blink &&
               reverse == o.reverse && hidden == o.hidden && strikethrough == o.strikethrough;
    }
    bool operator!=(const Attributes& o) const { return !(*this == o); }
};

// A single terminal cell. `width` follows the usual terminal convention:
//   1 -> normal cell
//   2 -> the leading half of a wide (e.g. CJK/emoji) glyph
//   0 -> a "continuation" cell that belongs to the wide glyph in the
//        column immediately to its left, and is never drawn on its own
struct Cell {
    char32_t codepoint = U' ';
    uint8_t width = 1;
    Attributes attrs{};

    bool operator==(const Cell& o) const {
        return codepoint == o.codepoint && width == o.width && attrs == o.attrs;
    }
};

// Minimal East-Asian-width-aware column width for a codepoint.
// This is a pragmatic subset (not the full Unicode East Asian Width
// tables) covering the common wide ranges (CJK, Hangul, most emoji) and
// the common zero-width ranges (combining marks, variation selectors).
// Good enough for real terminal use; documented as a known simplification
// in docs/terminal-emulation.md.
inline int codepoint_width(char32_t cp) {
    // Zero-width: combining marks, variation selectors, zero-width joiner/space
    if ((cp >= 0x0300 && cp <= 0x036F) ||   // combining diacritical marks
        (cp >= 0x1AB0 && cp <= 0x1AFF) ||   // combining diacritical marks extended
        (cp >= 0x1DC0 && cp <= 0x1DFF) ||   // combining diacritical marks supplement
        (cp >= 0x20D0 && cp <= 0x20FF) ||   // combining diacritical marks for symbols
        (cp >= 0xFE00 && cp <= 0xFE0F) ||   // variation selectors
        cp == 0x200D ||                     // zero width joiner
        cp == 0x200B) {                     // zero width space
        return 0;
    }
    // Wide: CJK, Hangul, most emoji blocks
    if ((cp >= 0x1100 && cp <= 0x115F) ||   // Hangul Jamo
        (cp >= 0x2E80 && cp <= 0xA4CF && cp != 0x303F) || // CJK radicals..Yi
        (cp >= 0xAC00 && cp <= 0xD7A3) ||   // Hangul syllables
        (cp >= 0xF900 && cp <= 0xFAFF) ||   // CJK compatibility ideographs
        (cp >= 0xFF00 && cp <= 0xFF60) ||   // fullwidth forms
        (cp >= 0xFFE0 && cp <= 0xFFE6) ||
        (cp >= 0x1F300 && cp <= 0x1FAFF) || // emoji ranges
        (cp >= 0x20000 && cp <= 0x3FFFD)) { // CJK extension planes
        return 2;
    }
    return 1;
}

} // namespace meridian::vt
```

---

## `src/core/vt/screen_buffer.hpp`

```cpp
#pragma once
// meridian-terminal-core / vt / screen_buffer.hpp
//
// Owns the actual grid of cells, the cursor, the scrollback, and the
// primary/alternate screen split. The ANSI parser drives this through a
// small, deliberate API — it never reaches into the grid directly.

#include "types.hpp"
#include <deque>
#include <string>
#include <vector>

namespace meridian::vt {

class ScreenBuffer {
public:
    explicit ScreenBuffer(int rows = 24, int cols = 80, std::size_t scrollback_limit = 10000);

    void resize(int rows, int cols);
    int rows() const { return rows_; }
    int cols() const { return cols_; }

    int cursor_row() const { return cursor_row_; }
    int cursor_col() const { return cursor_col_; }
    void set_cursor(int row, int col);
    void move_cursor_rel(int drow, int dcol);

    // Writing (advances cursor; wraps and scrolls as needed)
    void put_codepoint(char32_t cp, const Attributes& attrs);
    void newline();          // move to next line, scrolling within region if needed
    void carriage_return();  // col -> 0
    void backspace();
    void tab();

    // Erasing. mode: 0 = cursor->end, 1 = start->cursor, 2 = all
    void erase_in_line(int mode);
    void erase_in_display(int mode);
    void insert_lines(int n);
    void delete_lines(int n);
    void delete_chars(int n);
    void insert_chars(int n);

    // Alternate screen buffer (used by full-screen apps: vim, htop, less...)
    void enter_alt_screen();
    void exit_alt_screen();
    bool in_alt_screen() const { return alt_active_; }

    // Scroll region, 1-based inclusive as in DECSTBM; (0,0) = full screen
    void set_scroll_region(int top_1based, int bottom_1based);
    void scroll_up(int n);
    void scroll_down(int n);

    // Cursor save/restore (DECSC/DECRSC, ESC 7 / ESC 8)
    void save_cursor();
    void restore_cursor();

    const Cell& cell_at(int row, int col) const;
    const std::deque<std::vector<Cell>>& scrollback() const { return scrollback_; }

    // Plain-text dump of the visible screen, one line per row, trailing
    // spaces trimmed. Used by tests and the headless demo — not part of
    // the "real" rendering path (a GPU renderer would read cells directly).
    std::string dump_text() const;
    std::string dump_row_text(int row) const;

private:
    std::vector<std::vector<Cell>>& grid();
    const std::vector<std::vector<Cell>>& grid() const;
    std::vector<Cell>& row_at(int row);
    void clear_row(std::vector<Cell>& row, const Attributes& attrs);
    int effective_scroll_bottom() const;

    int rows_;
    int cols_;
    int cursor_row_ = 0;
    int cursor_col_ = 0;
    bool pending_wrap_ = false; // deferred autowrap flag
    std::size_t scrollback_limit_;
    int scroll_top_ = 0;    // 0-based
    int scroll_bottom_;     // 0-based inclusive

    std::vector<std::vector<Cell>> primary_;
    std::vector<std::vector<Cell>> alternate_;
    bool alt_active_ = false;
    std::deque<std::vector<Cell>> scrollback_;

    int saved_cursor_row_ = 0;
    int saved_cursor_col_ = 0;
};

} // namespace meridian::vt
```

---

## `src/core/vt/screen_buffer.cpp`

```cpp
// meridian-terminal-core / vt / screen_buffer.cpp
#include "screen_buffer.hpp"
#include <algorithm>

namespace meridian::vt {

ScreenBuffer::ScreenBuffer(int rows, int cols, std::size_t scrollback_limit)
    : rows_(rows), cols_(cols), scrollback_limit_(scrollback_limit), scroll_bottom_(rows - 1) {
    primary_.assign(static_cast<std::size_t>(rows_), std::vector<Cell>(static_cast<std::size_t>(cols_)));
    alternate_.assign(static_cast<std::size_t>(rows_), std::vector<Cell>(static_cast<std::size_t>(cols_)));
}

std::vector<std::vector<Cell>>& ScreenBuffer::grid() {
    return alt_active_ ? alternate_ : primary_;
}
const std::vector<std::vector<Cell>>& ScreenBuffer::grid() const {
    return alt_active_ ? alternate_ : primary_;
}

std::vector<Cell>& ScreenBuffer::row_at(int row) {
    return grid()[static_cast<std::size_t>(row)];
}

void ScreenBuffer::clear_row(std::vector<Cell>& row, const Attributes& attrs) {
    for (auto& c : row) {
        c.codepoint = U' ';
        c.width = 1;
        c.attrs = attrs;
    }
}

void ScreenBuffer::resize(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return;
    auto resize_one = [&](std::vector<std::vector<Cell>>& g) {
        g.resize(static_cast<std::size_t>(rows));
        for (auto& row : g) row.resize(static_cast<std::size_t>(cols));
    };
    resize_one(primary_);
    resize_one(alternate_);
    rows_ = rows;
    cols_ = cols;
    scroll_top_ = 0;
    scroll_bottom_ = rows_ - 1;
    cursor_row_ = std::min(cursor_row_, rows_ - 1);
    cursor_col_ = std::min(cursor_col_, cols_ - 1);
    pending_wrap_ = false;
}

void ScreenBuffer::set_cursor(int row, int col) {
    cursor_row_ = std::clamp(row, 0, rows_ - 1);
    cursor_col_ = std::clamp(col, 0, cols_ - 1);
    pending_wrap_ = false;
}

void ScreenBuffer::move_cursor_rel(int drow, int dcol) {
    set_cursor(cursor_row_ + drow, cursor_col_ + dcol);
}

int ScreenBuffer::effective_scroll_bottom() const {
    return scroll_bottom_;
}

void ScreenBuffer::scroll_up(int n) {
    if (n <= 0) return;
    auto& g = grid();
    for (int i = 0; i < n; ++i) {
        // Only rows that scroll off the very top of a full-screen region
        // (scroll_top_ == 0) feed the scrollback, and only on the primary
        // screen — alternate-screen apps (vim, htop...) manage their own
        // redraw and shouldn't pollute scrollback history.
        if (scroll_top_ == 0 && !alt_active_) {
            scrollback_.push_back(g[static_cast<std::size_t>(scroll_top_)]);
            if (scrollback_.size() > scrollback_limit_) scrollback_.pop_front();
        }
        for (int r = scroll_top_; r < scroll_bottom_; ++r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r + 1)];
        }
        clear_row(g[static_cast<std::size_t>(scroll_bottom_)], Attributes{});
    }
}

void ScreenBuffer::scroll_down(int n) {
    if (n <= 0) return;
    auto& g = grid();
    for (int i = 0; i < n; ++i) {
        for (int r = scroll_bottom_; r > scroll_top_; --r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r - 1)];
        }
        clear_row(g[static_cast<std::size_t>(scroll_top_)], Attributes{});
    }
}

void ScreenBuffer::newline() {
    pending_wrap_ = false;
    if (cursor_row_ == effective_scroll_bottom()) {
        scroll_up(1);
    } else if (cursor_row_ < rows_ - 1) {
        cursor_row_++;
    }
}

void ScreenBuffer::carriage_return() {
    cursor_col_ = 0;
    pending_wrap_ = false;
}

void ScreenBuffer::backspace() {
    if (cursor_col_ > 0) cursor_col_--;
    pending_wrap_ = false;
}

void ScreenBuffer::tab() {
    int next_stop = ((cursor_col_ / 8) + 1) * 8;
    cursor_col_ = std::min(next_stop, cols_ - 1);
    pending_wrap_ = false;
}

void ScreenBuffer::put_codepoint(char32_t cp, const Attributes& attrs) {
    int width = codepoint_width(cp);

    if (width == 0) {
        // Combining mark: merge visually onto the previous cell instead of
        // consuming a fresh column. If there's nowhere to attach it, drop it
        // rather than corrupting the grid.
        int col = cursor_col_ - 1;
        if (pending_wrap_) { col = cursor_col_; }
        if (col >= 0 && col < cols_) {
            // We don't maintain per-cell combining-mark chains in this
            // simplified model; keep the base character as-is. This is a
            // documented simplification (see docs/terminal-emulation.md).
        }
        return;
    }

    if (pending_wrap_) {
        carriage_return();
        newline();
    }

    if (width == 2 && cursor_col_ == cols_ - 1) {
        // Wide glyph doesn't fit in the last column: wrap first.
        carriage_return();
        newline();
    }

    auto& row = row_at(cursor_row_);
    row[static_cast<std::size_t>(cursor_col_)].codepoint = cp;
    row[static_cast<std::size_t>(cursor_col_)].width = static_cast<uint8_t>(width);
    row[static_cast<std::size_t>(cursor_col_)].attrs = attrs;

    if (width == 2 && cursor_col_ + 1 < cols_) {
        row[static_cast<std::size_t>(cursor_col_ + 1)].codepoint = 0;
        row[static_cast<std::size_t>(cursor_col_ + 1)].width = 0;
        row[static_cast<std::size_t>(cursor_col_ + 1)].attrs = attrs;
    }

    int advance = width;
    if (cursor_col_ + advance >= cols_) {
        cursor_col_ = cols_ - 1;
        pending_wrap_ = true;
    } else {
        cursor_col_ += advance;
    }
}

void ScreenBuffer::erase_in_line(int mode) {
    auto& row = row_at(cursor_row_);
    int start = 0, end = cols_ - 1;
    if (mode == 0) { start = cursor_col_; end = cols_ - 1; }
    else if (mode == 1) { start = 0; end = cursor_col_; }
    else { start = 0; end = cols_ - 1; }
    for (int c = start; c <= end; ++c) {
        row[static_cast<std::size_t>(c)] = Cell{};
    }
}

void ScreenBuffer::erase_in_display(int mode) {
    auto& g = grid();
    if (mode == 0) {
        erase_in_line(0);
        for (int r = cursor_row_ + 1; r < rows_; ++r) clear_row(g[static_cast<std::size_t>(r)], Attributes{});
    } else if (mode == 1) {
        erase_in_line(1);
        for (int r = 0; r < cursor_row_; ++r) clear_row(g[static_cast<std::size_t>(r)], Attributes{});
    } else {
        for (int r = 0; r < rows_; ++r) clear_row(g[static_cast<std::size_t>(r)], Attributes{});
    }
}

void ScreenBuffer::insert_lines(int n) {
    if (n <= 0) return;
    auto& g = grid();
    if (cursor_row_ < scroll_top_ || cursor_row_ > scroll_bottom_) return;
    for (int i = 0; i < n; ++i) {
        for (int r = scroll_bottom_; r > cursor_row_; --r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r - 1)];
        }
        clear_row(g[static_cast<std::size_t>(cursor_row_)], Attributes{});
    }
}

void ScreenBuffer::delete_lines(int n) {
    if (n <= 0) return;
    auto& g = grid();
    if (cursor_row_ < scroll_top_ || cursor_row_ > scroll_bottom_) return;
    for (int i = 0; i < n; ++i) {
        for (int r = cursor_row_; r < scroll_bottom_; ++r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r + 1)];
        }
        clear_row(g[static_cast<std::size_t>(scroll_bottom_)], Attributes{});
    }
}

void ScreenBuffer::delete_chars(int n) {
    if (n <= 0) return;
    auto& row = row_at(cursor_row_);
    for (int i = 0; i < n; ++i) {
        for (int c = cursor_col_; c < cols_ - 1; ++c) {
            row[static_cast<std::size_t>(c)] = row[static_cast<std::size_t>(c + 1)];
        }
        row[static_cast<std::size_t>(cols_ - 1)] = Cell{};
    }
}

void ScreenBuffer::insert_chars(int n) {
    if (n <= 0) return;
    auto& row = row_at(cursor_row_);
    for (int i = 0; i < n; ++i) {
        for (int c = cols_ - 1; c > cursor_col_; --c) {
            row[static_cast<std::size_t>(c)] = row[static_cast<std::size_t>(c - 1)];
        }
        row[static_cast<std::size_t>(cursor_col_)] = Cell{};
    }
}

void ScreenBuffer::enter_alt_screen() {
    if (alt_active_) return;
    alt_active_ = true;
    erase_in_display(2);
    cursor_row_ = 0;
    cursor_col_ = 0;
    pending_wrap_ = false;
}

void ScreenBuffer::exit_alt_screen() {
    if (!alt_active_) return;
    alt_active_ = false;
    pending_wrap_ = false;
}

void ScreenBuffer::set_scroll_region(int top_1based, int bottom_1based) {
    if (top_1based <= 0 || bottom_1based <= 0) {
        scroll_top_ = 0;
        scroll_bottom_ = rows_ - 1;
        return;
    }
    int top = std::clamp(top_1based - 1, 0, rows_ - 1);
    int bottom = std::clamp(bottom_1based - 1, 0, rows_ - 1);
    if (top >= bottom) { scroll_top_ = 0; scroll_bottom_ = rows_ - 1; return; }
    scroll_top_ = top;
    scroll_bottom_ = bottom;
}

void ScreenBuffer::save_cursor() {
    saved_cursor_row_ = cursor_row_;
    saved_cursor_col_ = cursor_col_;
}

void ScreenBuffer::restore_cursor() {
    set_cursor(saved_cursor_row_, saved_cursor_col_);
}

const Cell& ScreenBuffer::cell_at(int row, int col) const {
    return grid()[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
}

std::string ScreenBuffer::dump_row_text(int row) const {
    std::string out;
    const auto& g = grid();
    const auto& r = g[static_cast<std::size_t>(row)];
    for (const auto& cell : r) {
        if (cell.width == 0) continue; // continuation cell of a wide glyph
        char32_t cp = cell.codepoint;
        if (cp == 0) cp = U' ';
        if (cp < 0x80) {
            out.push_back(static_cast<char>(cp));
        } else {
            // Minimal UTF-8 encode for dump/debug purposes.
            if (cp < 0x800) {
                out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else if (cp < 0x10000) {
                out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else {
                out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            }
        }
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

std::string ScreenBuffer::dump_text() const {
    std::string out;
    for (int r = 0; r < rows_; ++r) {
        out += dump_row_text(r);
        if (r + 1 < rows_) out.push_back('\n');
    }
    return out;
}

} // namespace meridian::vt
```

---

## `src/core/vt/ansi_parser.hpp`

```cpp
#pragma once
// meridian-terminal-core / vt / ansi_parser.hpp
//
// A real state-machine parser for the ANSI/VT escape sequences a shell and
// its child programs actually emit (SGR colors/attributes, cursor motion,
// erase, alternate screen, scroll regions, OSC window title). It decodes
// UTF-8 as it goes and feeds decoded codepoints + SGR state into a
// ScreenBuffer. Malformed or truncated sequences are recovered from, never
// crash-worthy — see tests/test_ansi_parser.cpp for adversarial input cases.

#include "screen_buffer.hpp"
#include "types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace meridian::vt {

class AnsiParser {
public:
    explicit AnsiParser(ScreenBuffer& screen);

    // Feed a raw byte chunk as read from the PTY. Safe to call with any
    // byte sequence, including one that splits a UTF-8 sequence or an
    // escape sequence across calls.
    void feed(const std::string& data);
    void feed(const char* data, std::size_t len);

    // Most recent OSC 0/2 (window title) request, if any.
    const std::string& window_title() const { return window_title_; }

    const Attributes& current_attrs() const { return current_attrs_; }

private:
    enum class State { Ground, Escape, CsiParam, OscString, OscDiscard, Utf8Continuation };

    void feed_byte(uint8_t b);
    void handle_ground(uint8_t b);
    void handle_escape(uint8_t b);
    void handle_csi(uint8_t b);
    void handle_osc(uint8_t b);
    void handle_osc_discard(uint8_t b);
    void handle_utf8_continuation(uint8_t b);

    void dispatch_csi(uint8_t final_byte);
    void dispatch_sgr(const std::vector<long>& params);
    void dispatch_decset(const std::vector<long>& params, bool set);

    std::vector<long> parse_params() const;
    void reset_csi();
    void reset_osc();

    ScreenBuffer& screen_;
    State state_ = State::Ground;

    std::string csi_params_;        // raw digits/semicolons/colons
    bool csi_private_ = false;      // '?' prefix seen (DECSET/DECRST family)

    std::string osc_buffer_;

    Attributes current_attrs_{};
    std::string window_title_;

    // UTF-8 decode state (persists across feed() calls)
    char32_t utf8_codepoint_ = 0;
    int utf8_remaining_ = 0;

    static constexpr std::size_t kMaxCsiLen = 128;
    static constexpr std::size_t kMaxOscLen = 8192;
};

} // namespace meridian::vt
```

---

## `src/core/vt/ansi_parser.cpp`

```cpp
// meridian-terminal-core / vt / ansi_parser.cpp
#include "ansi_parser.hpp"

namespace meridian::vt {

AnsiParser::AnsiParser(ScreenBuffer& screen) : screen_(screen) {}

void AnsiParser::feed(const std::string& data) { feed(data.data(), data.size()); }

void AnsiParser::feed(const char* data, std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        feed_byte(static_cast<uint8_t>(data[i]));
    }
}

void AnsiParser::feed_byte(uint8_t b) {
    switch (state_) {
        case State::Ground:            handle_ground(b); break;
        case State::Escape:            handle_escape(b); break;
        case State::CsiParam:          handle_csi(b); break;
        case State::OscString:         handle_osc(b); break;
        case State::OscDiscard:        handle_osc_discard(b); break;
        case State::Utf8Continuation:  handle_utf8_continuation(b); break;
    }
}

void AnsiParser::handle_ground(uint8_t b) {
    if (b == 0x1B) { state_ = State::Escape; return; }
    if (b == '\n') { screen_.newline(); return; }
    if (b == '\r') { screen_.carriage_return(); return; }
    if (b == '\b') { screen_.backspace(); return; }
    if (b == '\t') { screen_.tab(); return; }
    if (b < 0x20) { return; } // other C0 controls (incl. BEL outside OSC): ignore, don't crash

    if (b < 0x80) {
        screen_.put_codepoint(static_cast<char32_t>(b), current_attrs_);
        return;
    }
    if ((b & 0xE0) == 0xC0) { utf8_codepoint_ = b & 0x1F; utf8_remaining_ = 1; state_ = State::Utf8Continuation; return; }
    if ((b & 0xF0) == 0xE0) { utf8_codepoint_ = b & 0x0F; utf8_remaining_ = 2; state_ = State::Utf8Continuation; return; }
    if ((b & 0xF8) == 0xF0) { utf8_codepoint_ = b & 0x07; utf8_remaining_ = 3; state_ = State::Utf8Continuation; return; }
    // Invalid UTF-8 lead byte (stray continuation byte, 0xFF, etc.):
    // drop it silently instead of corrupting the grid or crashing.
}

void AnsiParser::handle_utf8_continuation(uint8_t b) {
    if ((b & 0xC0) == 0x80) {
        utf8_codepoint_ = (utf8_codepoint_ << 6) | (b & 0x3F);
        if (--utf8_remaining_ == 0) {
            screen_.put_codepoint(utf8_codepoint_, current_attrs_);
            state_ = State::Ground;
        }
        return;
    }
    // Malformed/truncated UTF-8 sequence: abandon it and re-process this
    // byte as if we were in Ground. This recurses at most once, so a
    // malformed stream can never wedge or crash the parser.
    utf8_remaining_ = 0;
    state_ = State::Ground;
    handle_ground(b);
}

void AnsiParser::handle_escape(uint8_t b) {
    if (b == '[') { reset_csi(); state_ = State::CsiParam; return; }
    if (b == ']') { reset_osc(); state_ = State::OscString; return; }
    if (b == '7') { screen_.save_cursor(); state_ = State::Ground; return; }
    if (b == '8') { screen_.restore_cursor(); state_ = State::Ground; return; }
    if (b == 'c') {
        screen_.erase_in_display(2);
        screen_.set_cursor(0, 0);
        current_attrs_ = Attributes{};
        state_ = State::Ground;
        return;
    }
    if (b == 'D') { screen_.newline(); state_ = State::Ground; return; }
    if (b == 'M') { screen_.move_cursor_rel(-1, 0); state_ = State::Ground; return; }
    if (b == 'E') { screen_.carriage_return(); screen_.newline(); state_ = State::Ground; return; }
    // Any other single-char escape (charset selection, '#'-sequences, ...)
    // is accepted and ignored. We always return to Ground so an
    // unsupported escape can never wedge the state machine.
    state_ = State::Ground;
}

void AnsiParser::handle_csi(uint8_t b) {
    if (b == 0x1B) { reset_csi(); state_ = State::Escape; return; }
    if (b == '?' && csi_params_.empty()) { csi_private_ = true; return; }
    if ((b >= '0' && b <= '9') || b == ';' || b == ':') {
        if (csi_params_.size() < kMaxCsiLen) csi_params_.push_back(static_cast<char>(b));
        return;
    }
    if (b >= 0x40 && b <= 0x7E) {
        dispatch_csi(b);
        state_ = State::Ground;
        return;
    }
    if (csi_params_.size() >= kMaxCsiLen) {
        // Runaway/malformed sequence that never reaches a valid final byte:
        // give up on it cleanly instead of growing forever.
        reset_csi();
        state_ = State::Ground;
        return;
    }
    // Intermediate bytes (0x20-0x2F): accepted, ignored, keep waiting.
}

void AnsiParser::handle_osc(uint8_t b) {
    auto commit_title = [&]() {
        auto semi = osc_buffer_.find(';');
        if (semi != std::string::npos) {
            std::string code = osc_buffer_.substr(0, semi);
            if (code == "0" || code == "2") window_title_ = osc_buffer_.substr(semi + 1);
        }
    };
    if (b == 0x07) { commit_title(); state_ = State::Ground; return; }
    if (b == 0x1B) {
        // Either the start of an ST (ESC \) terminator, or a fresh escape
        // aborting this OSC outright. Either way, close out the OSC and
        // hand the next byte to Escape-state handling.
        commit_title();
        state_ = State::Escape;
        return;
    }
    if (osc_buffer_.size() < kMaxOscLen) {
        osc_buffer_.push_back(static_cast<char>(b));
    } else {
        // Runaway OSC string: stop accumulating (bounded memory) but keep
        // discarding bytes — silently, not as screen text — until a real
        // terminator shows up. Dumping straight back to Ground here would
        // let the tail of a huge malformed OSC get rendered as literal
        // characters, which is worse than just waiting it out.
        reset_osc();
        state_ = State::OscDiscard;
    }
}

void AnsiParser::handle_osc_discard(uint8_t b) {
    if (b == 0x07) { state_ = State::Ground; return; }
    if (b == 0x1B) { state_ = State::Escape; return; }
    // Anything else: keep discarding.
}

std::vector<long> AnsiParser::parse_params() const {
    std::vector<long> out;
    if (csi_params_.empty()) return out;
    std::size_t start = 0;
    while (start <= csi_params_.size()) {
        std::size_t semi = csi_params_.find(';', start);
        std::string tok = csi_params_.substr(start, semi == std::string::npos ? std::string::npos : semi - start);
        if (tok.empty()) {
            out.push_back(-1); // empty param means "use default"
        } else {
            try {
                out.push_back(std::stol(tok));
            } catch (...) {
                out.push_back(-1);
            }
        }
        if (semi == std::string::npos) break;
        start = semi + 1;
    }
    return out;
}

void AnsiParser::dispatch_csi(uint8_t final_byte) {
    auto params = parse_params();
    auto get = [&](std::size_t idx, long def) -> long {
        if (idx < params.size() && params[idx] >= 0) return params[idx];
        return def;
    };

    if (csi_private_ && (final_byte == 'h' || final_byte == 'l')) {
        dispatch_decset(params, final_byte == 'h');
        return;
    }

    switch (final_byte) {
        case 'A': screen_.move_cursor_rel(-static_cast<int>(get(0, 1)), 0); break;
        case 'B': screen_.move_cursor_rel(static_cast<int>(get(0, 1)), 0); break;
        case 'C': screen_.move_cursor_rel(0, static_cast<int>(get(0, 1))); break;
        case 'D': screen_.move_cursor_rel(0, -static_cast<int>(get(0, 1))); break;
        case 'G': screen_.set_cursor(screen_.cursor_row(), static_cast<int>(get(0, 1)) - 1); break;
        case 'd': screen_.set_cursor(static_cast<int>(get(0, 1)) - 1, screen_.cursor_col()); break;
        case 'H':
        case 'f':
            screen_.set_cursor(static_cast<int>(get(0, 1)) - 1, static_cast<int>(get(1, 1)) - 1);
            break;
        case 'J': screen_.erase_in_display(static_cast<int>(get(0, 0))); break;
        case 'K': screen_.erase_in_line(static_cast<int>(get(0, 0))); break;
        case 'L': screen_.insert_lines(static_cast<int>(get(0, 1))); break;
        case 'M': screen_.delete_lines(static_cast<int>(get(0, 1))); break;
        case 'P': screen_.delete_chars(static_cast<int>(get(0, 1))); break;
        case '@': screen_.insert_chars(static_cast<int>(get(0, 1))); break;
        case 'S': screen_.scroll_up(static_cast<int>(get(0, 1))); break;
        case 'T': screen_.scroll_down(static_cast<int>(get(0, 1))); break;
        case 'r': screen_.set_scroll_region(static_cast<int>(get(0, 0)), static_cast<int>(get(1, 0))); break;
        case 'm': dispatch_sgr(params); break;
        case 's': screen_.save_cursor(); break;
        case 'u': screen_.restore_cursor(); break;
        default:
            // Unrecognized final byte: accepted, ignored, never fatal.
            break;
    }
}

void AnsiParser::dispatch_sgr(const std::vector<long>& params) {
    if (params.empty()) { current_attrs_ = Attributes{}; return; }
    auto get = [&](std::size_t idx) -> long { return (idx < params.size() && params[idx] >= 0) ? params[idx] : 0; };

    std::size_t i = 0;
    while (i < params.size()) {
        long code = params[i] < 0 ? 0 : params[i];
        if (code >= 30 && code <= 37) { current_attrs_.fg = Color::indexed(static_cast<uint8_t>(code - 30)); }
        else if (code >= 40 && code <= 47) { current_attrs_.bg = Color::indexed(static_cast<uint8_t>(code - 40)); }
        else if (code >= 90 && code <= 97) { current_attrs_.fg = Color::indexed(static_cast<uint8_t>(code - 90 + 8)); }
        else if (code >= 100 && code <= 107) { current_attrs_.bg = Color::indexed(static_cast<uint8_t>(code - 100 + 8)); }
        else if (code == 38 || code == 48) {
            if (i + 1 < params.size()) {
                long mode = get(i + 1);
                if (mode == 5 && i + 2 < params.size()) {
                    uint8_t idx = static_cast<uint8_t>(get(i + 2));
                    if (code == 38) current_attrs_.fg = Color::indexed(idx); else current_attrs_.bg = Color::indexed(idx);
                    i += 2;
                } else if (mode == 2 && i + 4 < params.size()) {
                    uint8_t r = static_cast<uint8_t>(get(i + 2));
                    uint8_t g = static_cast<uint8_t>(get(i + 3));
                    uint8_t b = static_cast<uint8_t>(get(i + 4));
                    if (code == 38) current_attrs_.fg = Color::rgb(r, g, b); else current_attrs_.bg = Color::rgb(r, g, b);
                    i += 4;
                } else {
                    i += 1;
                }
            }
        } else {
            switch (code) {
                case 0: current_attrs_ = Attributes{}; break;
                case 1: current_attrs_.bold = true; break;
                case 2: current_attrs_.dim = true; break;
                case 3: current_attrs_.italic = true; break;
                case 4: current_attrs_.underline = true; break;
                case 5: case 6: current_attrs_.blink = true; break;
                case 7: current_attrs_.reverse = true; break;
                case 8: current_attrs_.hidden = true; break;
                case 9: current_attrs_.strikethrough = true; break;
                case 21: current_attrs_.bold = false; break;
                case 22: current_attrs_.bold = false; current_attrs_.dim = false; break;
                case 23: current_attrs_.italic = false; break;
                case 24: current_attrs_.underline = false; break;
                case 25: current_attrs_.blink = false; break;
                case 27: current_attrs_.reverse = false; break;
                case 28: current_attrs_.hidden = false; break;
                case 29: current_attrs_.strikethrough = false; break;
                case 39: current_attrs_.fg = Color::default_color(); break;
                case 49: current_attrs_.bg = Color::default_color(); break;
                default: break; // unknown SGR code: ignore, never fatal
            }
        }
        ++i;
    }
}

void AnsiParser::dispatch_decset(const std::vector<long>& params, bool set) {
    for (long p : params) {
        if (p < 0) continue;
        switch (p) {
            case 1049:
                if (set) { screen_.save_cursor(); screen_.enter_alt_screen(); }
                else { screen_.exit_alt_screen(); screen_.restore_cursor(); }
                break;
            case 47:
            case 1047:
                if (set) screen_.enter_alt_screen(); else screen_.exit_alt_screen();
                break;
            default:
                // Cursor visibility (25), mouse reporting (1000/1002/1003/1006),
                // bracketed paste (2004), etc: accepted and ignored — they
                // don't affect the cell grid this milestone models.
                break;
        }
    }
}

void AnsiParser::reset_csi() { csi_params_.clear(); csi_private_ = false; }
void AnsiParser::reset_osc() { osc_buffer_.clear(); }

} // namespace meridian::vt
```

---

## `src/core/pty/pty_manager.hpp`

```cpp
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
```

---

## `src/core/pty/pty_manager.cpp`

```cpp
// meridian-terminal-core / pty / pty_manager.cpp
#include "pty_manager.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <pty.h>
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
```

---

## `src/core/config.hpp`

```cpp
#pragma once
// meridian-core / config.hpp
//
// A deliberately minimal flat key=value config store — NOT a full TOML
// parser, even though the spec's example configs look like TOML
// (`terminal.toml`, `keybindings.toml`, `ai.toml`). Implementing real
// TOML (nested tables, arrays, typed values, quoting rules) is real
// scope on its own; this covers exactly what a `key = "value"` line
// needs and says so honestly rather than pretending to parse a format
// it doesn't. Shared by src/ai/ and src/config/ rather than duplicated —
// both just point the same class at a different file.

#include <map>
#include <string>

namespace meridian {

class Config {
public:
    explicit Config(std::string path);

    // Loads from disk if the file exists; returns false (not an error)
    // if it doesn't yet — callers get default values from get() either way.
    bool load();
    bool save() const;

    std::string get(const std::string& key, const std::string& default_value = "") const;
    void set(const std::string& key, const std::string& value);
    bool get_bool(const std::string& key, bool default_value) const;
    void set_bool(const std::string& key, bool value);

    const std::string& path() const { return path_; }

private:
    std::string path_;
    std::map<std::string, std::string> values_;
};

} // namespace meridian
```

---

## `src/core/config.cpp`

```cpp
// meridian-core / config.cpp
#include "config.hpp"

#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace meridian {

namespace {

// Creates each missing component of a directory path (like `mkdir -p`),
// e.g. for "~/.config/meridian/ai.toml" ensures "~/.config" and
// "~/.config/meridian" both exist before we try to write the file.
void mkdir_p(const std::string& dir) {
    if (dir.empty()) return;
    std::string partial;
    std::size_t start = (dir[0] == '/') ? 1 : 0;
    if (dir[0] == '/') partial = "/";
    std::size_t pos = start;
    while (pos <= dir.size()) {
        auto slash = dir.find('/', pos);
        std::string component = dir.substr(pos, slash == std::string::npos ? std::string::npos : slash - pos);
        if (!component.empty()) {
            partial += component;
            mkdir(partial.c_str(), 0755); // ignore EEXIST and other errors here; save() will fail loudly if it matters
            partial += "/";
        }
        if (slash == std::string::npos) break;
        pos = slash + 1;
    }
}

std::string dirname_of(const std::string& path) {
    auto slash = path.find_last_of('/');
    if (slash == std::string::npos) return "";
    return path.substr(0, slash);
}

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

} // namespace

Config::Config(std::string path) : path_(std::move(path)) {}

bool Config::load() {
    std::ifstream f(path_);
    if (!f.is_open()) return false;
    std::string line;
    while (std::getline(f, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;
        auto eq = trimmed.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(trimmed.substr(0, eq));
        std::string value = trim(trimmed.substr(eq + 1));
        // Strip one layer of surrounding double quotes, if present, so
        // both `key=value` and `key="value"` round-trip identically.
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }
        values_[key] = value;
    }
    return true;
}

bool Config::save() const {
    mkdir_p(dirname_of(path_));
    std::ofstream f(path_, std::ios::trunc);
    if (!f.is_open()) return false;
    f << "# Meridian AI configuration — generated by Meridian, safe to hand-edit.\n";
    for (const auto& [key, value] : values_) {
        f << key << " = \"" << value << "\"\n";
    }
    return f.good();
}

std::string Config::get(const std::string& key, const std::string& default_value) const {
    auto it = values_.find(key);
    return it != values_.end() ? it->second : default_value;
}

void Config::set(const std::string& key, const std::string& value) { values_[key] = value; }

bool Config::get_bool(const std::string& key, bool default_value) const {
    auto it = values_.find(key);
    if (it == values_.end()) return default_value;
    return it->second == "true" || it->second == "1" || it->second == "on";
}

void Config::set_bool(const std::string& key, bool value) { values_[key] = value ? "true" : "false"; }

} // namespace meridian
```

---

### src/shell — Meridian Shell (job control, Shell/REPL class)

## `src/shell/ast.hpp`

```cpp
#pragma once
// meridian-shell / ast.hpp
//
// AST for Meridian Shell. Words are kept as segments (literal vs
// expandable) rather than fully-resolved strings, so expansion — $VAR,
// ${VAR}, $(command substitution) — happens lazily at execution time.
// This matters for correctness: with &&/||, an unevaluated branch must
// never run its command substitutions.

#include <string>
#include <vector>

namespace meridian::shell {

// A word segment. `literal == true` means "print this text verbatim,
// never expand it" — that's what single-quoted text and backslash-escaped
// characters produce. `literal == false` means "scan this text for
// $VAR / ${VAR} / $(...) and expand them".
struct WordPart {
    std::string text;
    bool literal = false;
};
using Word = std::vector<WordPart>;

enum class RedirType { In, Out, Append, ErrOut, ErrAppend, All, AllAppend };

struct Redirection {
    RedirType type;
    Word target;
};

// One command within a pipeline.
struct Command {
    std::vector<Word> argv;
    std::vector<Redirection> redirections;
};

// One or more commands connected by '|'.
struct Pipeline {
    std::vector<Command> commands;
    bool background = false; // trailing '&'
};

enum class Connector { None, Semicolon, And, Or };

struct SequenceItem {
    Pipeline pipeline;
    Connector connector = Connector::None; // how this item connects to the NEXT one
};

using Sequence = std::vector<SequenceItem>;

} // namespace meridian::shell
```

---

## `src/shell/lexer.hpp`

```cpp
#pragma once
// meridian-shell / lexer.hpp
//
// Tokenizes one command line. Handles single/double quoting, backslash
// escapes, and the operator set from the spec: | ; & && || > >> < 2> 2>>
// &> &>>. This is a real tokenizer, not a whitespace split() — quoted
// operators and spaces inside words are handled correctly (see
// tests/test_shell_lexer.cpp).

#include "ast.hpp"
#include <string>
#include <vector>

namespace meridian::shell {

enum class TokenType {
    Word,
    Pipe, Semicolon, Background, And, Or,
    RedirOut, RedirAppend, RedirIn, RedirErr, RedirErrAppend, RedirAll, RedirAllAppend,
    End
};

struct Token {
    TokenType type;
    Word word; // populated only when type == Word
};

class Lexer {
public:
    explicit Lexer(std::string input);

    // Tokenizes the whole line. On a quoting error (unterminated ' or "),
    // returns an empty vector and sets *error.
    std::vector<Token> tokenize(std::string* error = nullptr);

private:
    std::string input_;
};

} // namespace meridian::shell
```

---

## `src/shell/lexer.cpp`

```cpp
// meridian-shell / lexer.cpp
#include "lexer.hpp"

namespace meridian::shell {

Lexer::Lexer(std::string input) : input_(std::move(input)) {}

std::vector<Token> Lexer::tokenize(std::string* error) {
    std::vector<Token> tokens;
    const std::size_t len = input_.size();
    std::size_t pos = 0;

    Word current_word;
    std::string expand_buf;
    bool in_word = false;
    int subst_depth = 0; // nesting depth of unclosed, unquoted $( ... )

    auto flush_expand_buf = [&]() {
        if (!expand_buf.empty()) {
            current_word.push_back(WordPart{expand_buf, false});
            expand_buf.clear();
        }
    };
    auto push_literal_char = [&](char c) {
        flush_expand_buf();
        current_word.push_back(WordPart{std::string(1, c), true});
    };
    auto finish_word = [&]() {
        flush_expand_buf();
        if (!current_word.empty()) {
            tokens.push_back(Token{TokenType::Word, current_word});
            current_word.clear();
        }
        in_word = false;
    };

    while (pos < len) {
        char c = input_[pos];

        if (!in_word) {
            if (c == ' ' || c == '\t') { pos++; continue; }
            if (c == '#') { break; } // comment: rest of line ignored
            if (c == '\n') { finish_word(); tokens.push_back(Token{TokenType::Semicolon, {}}); pos++; continue; }
            if (c == '|') {
                if (pos + 1 < len && input_[pos + 1] == '|') { tokens.push_back(Token{TokenType::Or, {}}); pos += 2; }
                else { tokens.push_back(Token{TokenType::Pipe, {}}); pos += 1; }
                continue;
            }
            if (c == '&') {
                if (pos + 1 < len && input_[pos + 1] == '&') { tokens.push_back(Token{TokenType::And, {}}); pos += 2; }
                else if (pos + 1 < len && input_[pos + 1] == '>') {
                    if (pos + 2 < len && input_[pos + 2] == '>') { tokens.push_back(Token{TokenType::RedirAllAppend, {}}); pos += 3; }
                    else { tokens.push_back(Token{TokenType::RedirAll, {}}); pos += 2; }
                } else { tokens.push_back(Token{TokenType::Background, {}}); pos += 1; }
                continue;
            }
            if (c == ';') { tokens.push_back(Token{TokenType::Semicolon, {}}); pos += 1; continue; }
            if (c == '>') {
                if (pos + 1 < len && input_[pos + 1] == '>') { tokens.push_back(Token{TokenType::RedirAppend, {}}); pos += 2; }
                else { tokens.push_back(Token{TokenType::RedirOut, {}}); pos += 1; }
                continue;
            }
            if (c == '<') { tokens.push_back(Token{TokenType::RedirIn, {}}); pos += 1; continue; }
            if (c == '2' && pos + 1 < len && input_[pos + 1] == '>') {
                if (pos + 2 < len && input_[pos + 2] == '>') { tokens.push_back(Token{TokenType::RedirErrAppend, {}}); pos += 3; }
                else { tokens.push_back(Token{TokenType::RedirErr, {}}); pos += 2; }
                continue;
            }
            in_word = true; // fall through: this char starts a word
        }

        // --- word-building region ---

        // Track $( ... ) nesting so that spaces and operator characters
        // *inside* an unquoted command substitution don't end the word —
        // "echo $(echo a b)" must lex as two words (echo, $(echo a b)),
        // not four.
        if (c == '$' && pos + 1 < len && input_[pos + 1] == '(') {
            expand_buf.push_back('$');
            expand_buf.push_back('(');
            pos += 2;
            subst_depth++;
            continue;
        }
        if (subst_depth > 0) {
            if (c == '(') { expand_buf.push_back(c); pos++; subst_depth++; continue; }
            if (c == ')') { expand_buf.push_back(c); pos++; subst_depth--; continue; }
            if (c != '\'' && c != '"' && c != '\\') {
                // Any other character (including whitespace and the
                // operator characters) while inside an unclosed $(...):
                // accumulate literally rather than treating it as a
                // delimiter. Quotes/backslash still fall through to the
                // normal handling below so quoting still works inside a
                // substitution, e.g. $(echo "a b").
                expand_buf.push_back(c);
                pos++;
                continue;
            }
        }

        if (c == '\'') {
            pos++; // consume opening quote
            std::string lit;
            while (pos < len && input_[pos] != '\'') { lit.push_back(input_[pos]); pos++; }
            if (pos >= len) { if (error) *error = "unterminated single quote"; return {}; }
            pos++; // consume closing quote
            flush_expand_buf();
            current_word.push_back(WordPart{lit, true});
            continue;
        }
        if (c == '"') {
            pos++; // consume opening quote
            while (pos < len && input_[pos] != '"') {
                char dc = input_[pos];
                if (dc == '\\' && pos + 1 < len &&
                    (input_[pos + 1] == '"' || input_[pos + 1] == '\\' || input_[pos + 1] == '$')) {
                    push_literal_char(input_[pos + 1]);
                    pos += 2;
                    continue;
                }
                expand_buf.push_back(dc);
                pos++;
            }
            if (pos >= len) { if (error) *error = "unterminated double quote"; return {}; }
            pos++; // consume closing quote
            continue;
        }
        if (c == '\\') {
            if (pos + 1 < len) { push_literal_char(input_[pos + 1]); pos += 2; continue; }
            pos++; // trailing backslash at end of input: drop it
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\n' || c == '|' || c == '&' || c == ';' || c == '>' || c == '<') {
            finish_word();
            continue; // reprocess this char now that in_word == false
        }
        expand_buf.push_back(c);
        pos++;
    }

    if (subst_depth > 0) {
        if (error) *error = "unterminated command substitution: missing ')'";
        return {};
    }

    finish_word();
    tokens.push_back(Token{TokenType::End, {}});
    return tokens;
}

} // namespace meridian::shell
```

---

## `src/shell/parser.hpp`

```cpp
#pragma once
// meridian-shell / parser.hpp
//
// Recursive-descent parser: Token stream -> Sequence AST. Never uses
// string-splitting; every operator is structural.
//
// Grammar:
//   sequence  := seq_item ((';' | '&&' | '||' | '&') seq_item)*
//   seq_item  := pipeline
//   pipeline  := command ('|' command)*
//   command   := (WORD | redir WORD)+
//   redir     := '>' | '>>' | '<' | '2>' | '2>>' | '&>' | '&>>'

#include "ast.hpp"
#include "lexer.hpp"
#include <vector>

namespace meridian::shell {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Returns the parsed sequence. On syntax error, sets *error and
    // returns an empty sequence.
    Sequence parse(std::string* error = nullptr);

private:
    const Token& peek() const;
    const Token& advance();

    bool parse_command(Command* out, std::string* error);
    bool parse_pipeline(Pipeline* out, std::string* error);

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
};

} // namespace meridian::shell
```

---

## `src/shell/parser.cpp`

```cpp
// meridian-shell / parser.cpp
#include "parser.hpp"

namespace meridian::shell {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {
    if (tokens_.empty()) tokens_.push_back(Token{TokenType::End, {}});
}

const Token& Parser::peek() const { return tokens_[pos_]; }

const Token& Parser::advance() {
    const Token& t = tokens_[pos_];
    if (pos_ + 1 < tokens_.size()) pos_++;
    return t;
}

static RedirType redir_type_for(TokenType t) {
    switch (t) {
        case TokenType::RedirOut: return RedirType::Out;
        case TokenType::RedirAppend: return RedirType::Append;
        case TokenType::RedirIn: return RedirType::In;
        case TokenType::RedirErr: return RedirType::ErrOut;
        case TokenType::RedirErrAppend: return RedirType::ErrAppend;
        case TokenType::RedirAll: return RedirType::All;
        case TokenType::RedirAllAppend: return RedirType::AllAppend;
        default: return RedirType::Out;
    }
}

static bool is_redir_token(TokenType t) {
    switch (t) {
        case TokenType::RedirOut:
        case TokenType::RedirAppend:
        case TokenType::RedirIn:
        case TokenType::RedirErr:
        case TokenType::RedirErrAppend:
        case TokenType::RedirAll:
        case TokenType::RedirAllAppend:
            return true;
        default:
            return false;
    }
}

bool Parser::parse_command(Command* out, std::string* error) {
    bool got_anything = false;
    while (true) {
        TokenType t = peek().type;
        if (t == TokenType::Word) {
            out->argv.push_back(peek().word);
            advance();
            got_anything = true;
        } else if (is_redir_token(t)) {
            RedirType rt = redir_type_for(t);
            advance();
            if (peek().type != TokenType::Word) {
                if (error) *error = "syntax error: expected a filename after redirection operator";
                return false;
            }
            out->redirections.push_back(Redirection{rt, peek().word});
            advance();
            got_anything = true;
        } else {
            break;
        }
    }
    if (!got_anything) {
        if (error) *error = "syntax error: expected a command";
        return false;
    }
    return true;
}

bool Parser::parse_pipeline(Pipeline* out, std::string* error) {
    Command first;
    if (!parse_command(&first, error)) return false;
    out->commands.push_back(std::move(first));

    while (peek().type == TokenType::Pipe) {
        advance();
        Command next;
        if (!parse_command(&next, error)) return false;
        out->commands.push_back(std::move(next));
    }
    return true;
}

Sequence Parser::parse(std::string* error) {
    Sequence seq;
    if (peek().type == TokenType::End) return seq; // empty line -> empty sequence, not an error

    while (peek().type != TokenType::End) {
        SequenceItem item;
        if (!parse_pipeline(&item.pipeline, error)) return {};

        switch (peek().type) {
            case TokenType::Semicolon: item.connector = Connector::Semicolon; advance(); break;
            case TokenType::And:       item.connector = Connector::And;       advance(); break;
            case TokenType::Or:        item.connector = Connector::Or;        advance(); break;
            case TokenType::Background:
                item.pipeline.background = true;
                item.connector = Connector::Semicolon;
                advance();
                break;
            case TokenType::End:
                item.connector = Connector::None;
                break;
            default:
                if (error) *error = "syntax error: unexpected token";
                return {};
        }
        seq.push_back(std::move(item));
    }
    return seq;
}

} // namespace meridian::shell
```

---

## `src/shell/executor.hpp`

```cpp
#pragma once
// meridian-shell / executor.hpp
//
// Executes a parsed Sequence using fork()/execvp()/pipe()/dup2()/waitpid()
// directly — never passes the user's command through /bin/sh or system().
// Builtins that must mutate the shell's own state (cd, export, exit) run
// in-process when safe to do so; everything else — external programs,
// pipeline stages, and builtins combined with redirection — runs in a
// forked child, matching real shell subshell semantics.
//
// Also implements real POSIX job control (process groups + tcsetpgrp
// terminal handoff) when enable_job_control() has been called — see
// docs/shell.md for the design and how it's tested against a real PTY.

#include "ast.hpp"
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

namespace meridian::shell {

enum class JobState { Running, Stopped };

struct Job {
    int id = 0;
    pid_t pgid = -1;
    std::vector<pid_t> pids;   // still-outstanding (not yet reaped) pids of this job
    std::string command_line;  // best-effort display text, not re-parseable
    JobState state = JobState::Running;
};

class Executor {
public:
    Executor() = default;

    // Lexes, parses, and runs one command line in one call. On a lex/parse
    // error, sets *error, leaves last_status() at -1, and returns -1.
    int run_line(const std::string& line, std::string* error = nullptr);

    int run(const Sequence& seq);

    // Expands a Word: literal segments pass through untouched; expandable
    // segments are scanned for $VAR, ${VAR}, $(command substitution),
    // $? (last exit status), and $$ (this process's pid).
    std::string expand_word(const Word& w);

    // Forks, runs `src` through this same engine with stdout captured via
    // a pipe, and returns the captured output with trailing newlines
    // stripped (POSIX command-substitution semantics). Used by $(...).
    std::string capture_command_substitution(const std::string& src);

    void request_exit(int code) { exit_requested_ = true; exit_code_ = code; }
    bool exit_requested() const { return exit_requested_; }
    int exit_code() const { return exit_code_; }

    void push_history(const std::string& line) { if (!line.empty()) history_.push_back(line); }
    const std::vector<std::string>& history() const { return history_; }

    // Enables real job control: this process becomes its own process
    // group leader, ignores SIGINT/SIGQUIT/SIGTSTP/SIGTTIN/SIGTTOU for
    // itself (children restore default disposition before exec — see
    // run_in_child), and claims the controlling terminal's foreground
    // group via tcsetpgrp. Only call this when stdin is actually a
    // controlling tty (see src/app/shell_main.cpp) — matches real shells,
    // which only enable job control for interactive sessions.
    void enable_job_control();
    bool job_control_enabled() const { return job_control_enabled_; }

    // `fg`/`bg` builtins. argv[1], if present, is "%N" or "N" selecting
    // a job by id; with no argument, the most recently created job is
    // used. Returns the resumed job's exit status (fg) or 0 (bg).
    int do_fg(const std::vector<std::string>& argv);
    int do_bg(const std::vector<std::string>& argv);

    // Human-readable job list, also used by the `jobs` builtin. Polls
    // (non-blocking) for background jobs that finished or stopped since
    // the last call.
    std::string jobs_report();

    int last_status() const { return last_status_; }

private:
    int run_pipeline(const Pipeline& pl);
    int run_single_inprocess(const Command& cmd);
    [[noreturn]] void run_in_child(const Command& cmd);

    Job* find_job(std::optional<int> id);
    void reap_job_status_changes();
    std::string pipeline_display_string(const Pipeline& pl);

    std::vector<std::string> history_;
    int last_status_ = 0;
    bool exit_requested_ = false;
    int exit_code_ = 0;

    bool job_control_enabled_ = false;
    pid_t shell_pgid_ = 0;
    std::vector<Job> jobs_;
    int next_job_id_ = 1;
};

} // namespace meridian::shell
```

---

## `src/shell/executor.cpp`

```cpp
// meridian-shell / executor.cpp
#include "executor.hpp"
#include "builtins.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

namespace meridian::shell {

// _exit() skips C++ iostream flushing (unlike a normal return from main),
// so any buffered std::cout/std::cerr content written by an in-process
// builtin just before we _exit() a forked child would otherwise be lost
// silently. Every _exit() call site in this file goes through here.
[[noreturn]] static void flush_and_exit(int code) {
    std::cout.flush();
    std::cerr.flush();
    _exit(code);
}

// Raw (unexpanded) text of a word, for job-list display only. Deliberately
// NOT expand_word(): building a display string by expanding would re-run
// any $(...) command substitution a second time, with real side effects.
static std::string raw_word_text(const Word& w) {
    std::string s;
    for (auto& part : w) s += part.text;
    return s;
}

std::string Executor::pipeline_display_string(const Pipeline& pl) {
    std::string out;
    for (std::size_t i = 0; i < pl.commands.size(); ++i) {
        if (i) out += " | ";
        for (std::size_t j = 0; j < pl.commands[i].argv.size(); ++j) {
            if (j) out += " ";
            out += raw_word_text(pl.commands[i].argv[j]);
        }
    }
    return out;
}

void Executor::enable_job_control() {
    shell_pgid_ = getpid();
    setpgid(shell_pgid_, shell_pgid_);
    // The shell itself ignores these; forked children restore default
    // disposition before exec (see run_pipeline's fork branch below) so
    // real programs behave normally under Ctrl+C / Ctrl+Z.
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, shell_pgid_);
    job_control_enabled_ = true;
}

Job* Executor::find_job(std::optional<int> id) {
    if (id) {
        for (auto& j : jobs_) if (j.id == *id) return &j;
        return nullptr;
    }
    Job* best = nullptr;
    for (auto& j : jobs_) if (!best || j.id > best->id) best = &j;
    return best;
}

static std::optional<int> parse_job_spec(const std::vector<std::string>& argv) {
    if (argv.size() < 2) return std::nullopt;
    std::string spec = argv[1];
    if (!spec.empty() && spec[0] == '%') spec = spec.substr(1);
    try { return std::stoi(spec); } catch (...) { return std::nullopt; }
}

void Executor::reap_job_status_changes() {
    for (auto it = jobs_.begin(); it != jobs_.end();) {
        if (it->state == JobState::Stopped) { ++it; continue; } // left alone until fg/bg touches it

        bool all_done = true;
        bool newly_stopped = false;
        for (auto pid : it->pids) {
            int status = 0;
            pid_t r = waitpid(pid, &status, WNOHANG | WUNTRACED);
            if (r == 0) { all_done = false; }
            else if (r > 0 && WIFSTOPPED(status)) { newly_stopped = true; all_done = false; }
            // r > 0 (exited/signaled) or r < 0 (already reaped) both mean
            // "this pid is no longer blocking the job."
        }
        if (newly_stopped) {
            it->state = JobState::Stopped;
            std::cout << "\n[" << it->id << "]+  Stopped    " << it->command_line << "\n";
            ++it;
            continue;
        }
        if (all_done) {
            std::cout << "[" << it->id << "]+  Done    " << it->command_line << "\n";
            it = jobs_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string Executor::jobs_report() {
    reap_job_status_changes();
    if (jobs_.empty()) return "no jobs\n";
    std::string out;
    for (auto& j : jobs_) {
        out += "[" + std::to_string(j.id) + "] " + std::to_string(j.pgid) + "  "
             + (j.state == JobState::Stopped ? "Stopped" : "Running") + "    "
             + j.command_line + "\n";
    }
    return out;
}

int Executor::do_fg(const std::vector<std::string>& argv) {
    if (!job_control_enabled_) { std::cerr << "fg: no job control in this session\n"; return 1; }
    Job* job = find_job(parse_job_spec(argv));
    if (!job) { std::cerr << "fg: no such job\n"; return 1; }

    std::cout << job->command_line << "\n";
    std::cout.flush();
    kill(-job->pgid, SIGCONT);
    tcsetpgrp(STDIN_FILENO, job->pgid);
    job->state = JobState::Running;

    int last = 0;
    bool stopped_again = false;
    std::size_t stopped_at = job->pids.size();
    for (std::size_t i = 0; i < job->pids.size(); ++i) {
        int status = 0;
        waitpid(job->pids[i], &status, WUNTRACED);
        if (WIFSTOPPED(status)) { stopped_again = true; stopped_at = i; break; }
        if (WIFEXITED(status)) last = WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) last = 128 + WTERMSIG(status);
    }
    tcsetpgrp(STDIN_FILENO, shell_pgid_);

    int target_id = job->id;
    if (stopped_again) {
        job->pids.assign(job->pids.begin() + static_cast<long>(stopped_at), job->pids.end());
        job->state = JobState::Stopped;
        std::cout << "\n[" << target_id << "]+  Stopped    " << job->command_line << "\n";
    } else {
        jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(),
                                    [target_id](const Job& j) { return j.id == target_id; }),
                    jobs_.end());
    }
    return last;
}

int Executor::do_bg(const std::vector<std::string>& argv) {
    if (!job_control_enabled_) { std::cerr << "bg: no job control in this session\n"; return 1; }
    Job* job = find_job(parse_job_spec(argv));
    if (!job) { std::cerr << "bg: no such job\n"; return 1; }
    if (job->state != JobState::Stopped) { std::cerr << "bg: job is already running\n"; return 1; }
    kill(-job->pgid, SIGCONT);
    job->state = JobState::Running;
    std::cout << "[" << job->id << "]+ " << job->command_line << " &\n";
    return 0;
}

// Expands exactly one '$...' construct starting at text[i] ('$' itself).
// Advances i past the construct and returns the expansion.
static std::string expand_one_dollar(const std::string& text, std::size_t& i, Executor& ex) {
    char next = text[i + 1];

    if (next == '(') {
        int depth = 1;
        std::size_t j = i + 2;
        while (j < text.size() && depth > 0) {
            if (text[j] == '(') depth++;
            else if (text[j] == ')') { depth--; if (depth == 0) break; }
            j++;
        }
        std::string inner = text.substr(i + 2, j - (i + 2));
        i = (j < text.size()) ? j + 1 : text.size();
        return ex.capture_command_substitution(inner);
    }
    if (next == '{') {
        std::size_t close = text.find('}', i + 2);
        std::string name = (close == std::string::npos) ? text.substr(i + 2) : text.substr(i + 2, close - (i + 2));
        i = (close == std::string::npos) ? text.size() : close + 1;
        const char* v = std::getenv(name.c_str());
        return v ? std::string(v) : std::string();
    }
    if (std::isalpha(static_cast<unsigned char>(next)) || next == '_') {
        std::size_t j = i + 1;
        while (j < text.size() && (std::isalnum(static_cast<unsigned char>(text[j])) || text[j] == '_')) j++;
        std::string name = text.substr(i + 1, j - (i + 1));
        i = j;
        const char* v = std::getenv(name.c_str());
        return v ? std::string(v) : std::string();
    }
    if (next == '?') { i += 2; return std::to_string(ex.last_status()); }
    if (next == '$') { i += 2; return std::to_string(static_cast<long>(getpid())); }

    i += 1;
    return "$";
}

std::string Executor::expand_word(const Word& w) {
    std::string result;
    for (const auto& part : w) {
        if (part.literal) { result += part.text; continue; }
        const std::string& text = part.text;
        std::size_t i = 0;
        while (i < text.size()) {
            if (text[i] == '$' && i + 1 < text.size()) {
                result += expand_one_dollar(text, i, *this);
            } else {
                result.push_back(text[i]);
                i++;
            }
        }
    }
    return result;
}

std::string Executor::capture_command_substitution(const std::string& src) {
    int fds[2];
    if (pipe(fds) != 0) return "";
    std::cout.flush();
    std::cerr.flush(); // avoid the parent's pending buffer content getting duplicated into the child's copy
    pid_t pid = fork();
    if (pid < 0) { close(fds[0]); close(fds[1]); return ""; }
    if (pid == 0) {
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);
        std::string err;
        int status = run_line(src, &err);
        flush_and_exit(status < 0 ? 1 : status);
    }
    close(fds[1]);
    std::string out;
    char buf[4096];
    ssize_t n;
    while ((n = ::read(fds[0], buf, sizeof(buf))) > 0) out.append(buf, static_cast<std::size_t>(n));
    close(fds[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    while (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

int Executor::run_line(const std::string& line, std::string* error) {
    Lexer lexer(line);
    std::string lex_err;
    auto tokens = lexer.tokenize(&lex_err);
    if (!lex_err.empty()) { if (error) *error = lex_err; last_status_ = -1; return -1; }

    Parser parser(std::move(tokens));
    std::string parse_err;
    Sequence seq = parser.parse(&parse_err);
    if (!parse_err.empty()) { if (error) *error = parse_err; last_status_ = -1; return -1; }

    return run(seq);
}

int Executor::run(const Sequence& seq) {
    if (job_control_enabled_) reap_job_status_changes();
    int status = last_status_;
    for (std::size_t idx = 0; idx < seq.size(); ++idx) {
        if (idx > 0) {
            Connector prev = seq[idx - 1].connector;
            if (prev == Connector::And && status != 0) continue;
            if (prev == Connector::Or && status == 0) continue;
        }
        status = run_pipeline(seq[idx].pipeline);
        last_status_ = status;
        if (exit_requested_) break;
    }
    return status;
}

int Executor::run_single_inprocess(const Command& cmd) {
    std::vector<std::string> argv;
    for (auto& w : cmd.argv) argv.push_back(expand_word(w));
    if (argv.empty()) return 0;
    return run_builtin(argv[0], argv, *this);
}

void Executor::run_in_child(const Command& cmd) {
    for (const auto& r : cmd.redirections) {
        std::string target = expand_word(r.target);
        int fd = -1;
        switch (r.type) {
            case RedirType::In:        fd = open(target.c_str(), O_RDONLY); break;
            case RedirType::Out:       fd = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); break;
            case RedirType::Append:    fd = open(target.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644); break;
            case RedirType::ErrOut:    fd = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); break;
            case RedirType::ErrAppend: fd = open(target.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644); break;
            case RedirType::All:       fd = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); break;
            case RedirType::AllAppend: fd = open(target.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644); break;
        }
        if (fd < 0) {
            std::cerr << "meridian-shell: cannot open '" << target << "': " << std::strerror(errno) << "\n";
            flush_and_exit(1);
        }
        switch (r.type) {
            case RedirType::In: dup2(fd, STDIN_FILENO); break;
            case RedirType::Out:
            case RedirType::Append: dup2(fd, STDOUT_FILENO); break;
            case RedirType::ErrOut:
            case RedirType::ErrAppend: dup2(fd, STDERR_FILENO); break;
            case RedirType::All:
            case RedirType::AllAppend: dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); break;
        }
        close(fd);
    }

    std::vector<std::string> argv;
    for (auto& w : cmd.argv) argv.push_back(expand_word(w));
    if (argv.empty()) flush_and_exit(0);

    if (is_builtin(argv[0])) {
        int status = run_builtin(argv[0], argv, *this);
        flush_and_exit(status);
    }

    std::vector<char*> cargv;
    cargv.reserve(argv.size() + 1);
    for (auto& s : argv) cargv.push_back(s.data());
    cargv.push_back(nullptr);

    execvp(argv[0].c_str(), cargv.data());
    std::cerr << "meridian-shell: " << argv[0] << ": " << std::strerror(errno) << "\n";
    flush_and_exit(errno == ENOENT ? 127 : 126);
}

int Executor::run_pipeline(const Pipeline& pl) {
    if (pl.commands.empty()) return 0;

    if (pl.commands.size() == 1) {
        const Command& cmd = pl.commands[0];
        if (cmd.argv.empty()) return 0;
        std::string prog = expand_word(cmd.argv[0]);
        bool inprocess_ok = cmd.redirections.empty() && !pl.background && is_builtin(prog);
        if (inprocess_ok) return run_single_inprocess(cmd);
    }

    std::string display = pipeline_display_string(pl);

    std::size_t n = pl.commands.size();
    std::vector<std::array<int, 2>> pipes(n > 1 ? n - 1 : 0);
    for (auto& p : pipes) {
        if (pipe(p.data()) != 0) { std::cerr << "meridian-shell: pipe() failed\n"; return 1; }
    }

    std::cout.flush();
    std::cerr.flush();

    std::vector<pid_t> pids;
    pids.reserve(n);
    pid_t pgid = 0;
    for (std::size_t i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid < 0) { std::cerr << "meridian-shell: fork failed\n"; return 1; }
        if (pid == 0) {
            if (job_control_enabled_) {
                pid_t self = getpid();
                pid_t target = (i == 0) ? self : pgid;
                setpgid(self, target); // child-side set; parent sets it too (race-safe double-set)
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
            }
            if (i > 0) dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i + 1 < n) dup2(pipes[i][1], STDOUT_FILENO);
            for (auto& p : pipes) { close(p[0]); close(p[1]); }
            run_in_child(pl.commands[i]); // never returns
        }
        if (job_control_enabled_) {
            pid_t target = (i == 0) ? pid : pgid;
            setpgid(pid, target);
            if (i == 0) pgid = pid;
        }
        pids.push_back(pid);
    }
    for (auto& p : pipes) { close(p[0]); close(p[1]); }

    if (pl.background) {
        Job job;
        job.id = next_job_id_++;
        job.pgid = job_control_enabled_ ? pgid : pids.front();
        job.pids = pids;
        job.command_line = display;
        job.state = JobState::Running;
        jobs_.push_back(job);
        std::cout << "[" << job.id << "] " << job.pgid << "\n";
        return 0;
    }

    if (job_control_enabled_) tcsetpgrp(STDIN_FILENO, pgid);

    int last_status = 0;
    bool stopped = false;
    std::size_t stopped_at = pids.size();
    for (std::size_t i = 0; i < pids.size(); ++i) {
        int status = 0;
        int opts = job_control_enabled_ ? WUNTRACED : 0;
        waitpid(pids[i], &status, opts);
        if (WIFSTOPPED(status)) { stopped = true; stopped_at = i; break; }
        if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) last_status = 128 + WTERMSIG(status);
    }

    if (job_control_enabled_) tcsetpgrp(STDIN_FILENO, shell_pgid_);

    if (stopped) {
        Job job;
        job.id = next_job_id_++;
        job.pgid = pgid;
        job.pids.assign(pids.begin() + static_cast<long>(stopped_at), pids.end());
        job.command_line = display;
        job.state = JobState::Stopped;
        jobs_.push_back(job);
        std::cout << "\n[" << job.id << "]+  Stopped    " << display << "\n";
    }

    return last_status;
}

} // namespace meridian::shell
```

---

## `src/shell/builtins.hpp`

```cpp
#pragma once
// meridian-shell / builtins.hpp
//
// Builtins that must affect the shell's own process state (cd, export,
// exit, ...) and therefore cannot be implemented as external programs.

#include <string>
#include <vector>

namespace meridian::shell {

class Executor; // forward decl to avoid a header cycle

bool is_builtin(const std::string& name);

// Runs a builtin. `argv` is already word-expanded. Returns the exit
// status. Safe to call either in-process (lone builtin, no redirection)
// or inside a forked child (builtin used in a pipeline or with
// redirection) — callers decide which via Executor.
int run_builtin(const std::string& name, const std::vector<std::string>& argv, Executor& ctx);

} // namespace meridian::shell
```

---

## `src/shell/builtins.cpp`

```cpp
// meridian-shell / builtins.cpp
#include "builtins.hpp"
#include "executor.hpp"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

extern char** environ;

namespace meridian::shell {

namespace {
const char* kBuiltinNames[] = {
    "cd", "pwd", "echo", "exit", "export", "unset", "env",
    "history", "jobs", "fg", "bg", "help", "type", "which", "clear", "alias"
};
} // namespace

bool is_builtin(const std::string& name) {
    for (auto* n : kBuiltinNames)
        if (name == n) return true;
    return false;
}

static int builtin_cd(const std::vector<std::string>& argv) {
    std::string target;
    if (argv.size() < 2) {
        const char* home = std::getenv("HOME");
        if (!home) { std::cerr << "cd: HOME not set\n"; return 1; }
        target = home;
    } else {
        target = argv[1];
    }
    if (chdir(target.c_str()) != 0) {
        std::cerr << "cd: " << target << ": " << std::strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

static int builtin_pwd() {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf))) { std::cout << buf << "\n"; return 0; }
    std::cerr << "pwd: " << std::strerror(errno) << "\n";
    return 1;
}

static int builtin_echo(const std::vector<std::string>& argv) {
    for (std::size_t i = 1; i < argv.size(); ++i) {
        if (i > 1) std::cout << ' ';
        std::cout << argv[i];
    }
    std::cout << "\n";
    return 0;
}

static int builtin_export(const std::vector<std::string>& argv) {
    for (std::size_t i = 1; i < argv.size(); ++i) {
        auto eq = argv[i].find('=');
        if (eq == std::string::npos) {
            if (!std::getenv(argv[i].c_str())) { std::cerr << "export: " << argv[i] << ": not set\n"; return 1; }
            continue;
        }
        setenv(argv[i].substr(0, eq).c_str(), argv[i].substr(eq + 1).c_str(), 1);
    }
    return 0;
}

static int builtin_unset(const std::vector<std::string>& argv) {
    for (std::size_t i = 1; i < argv.size(); ++i) unsetenv(argv[i].c_str());
    return 0;
}

static int builtin_env() {
    for (char** e = environ; *e; ++e) std::cout << *e << "\n";
    return 0;
}

static int builtin_history(Executor& ctx) {
    const auto& h = ctx.history();
    for (std::size_t i = 0; i < h.size(); ++i) std::cout << "  " << (i + 1) << "  " << h[i] << "\n";
    return 0;
}

static int builtin_jobs(Executor& ctx) {
    std::cout << ctx.jobs_report();
    return 0;
}

static int builtin_type(const std::vector<std::string>& argv) {
    if (argv.size() < 2) return 0;
    if (is_builtin(argv[1])) { std::cout << argv[1] << " is a Meridian Shell builtin\n"; return 0; }
    const char* path = std::getenv("PATH");
    std::string p = path ? path : "";
    std::size_t start = 0;
    while (start <= p.size()) {
        auto colon = p.find(':', start);
        std::string dir = p.substr(start, colon == std::string::npos ? std::string::npos : colon - start);
        std::string candidate = dir + "/" + argv[1];
        if (access(candidate.c_str(), X_OK) == 0) { std::cout << argv[1] << " is " << candidate << "\n"; return 0; }
        if (colon == std::string::npos) break;
        start = colon + 1;
    }
    std::cout << argv[1] << ": not found\n";
    return 1;
}

static int builtin_help() {
    std::cout <<
        "Meridian Shell builtins:\n"
        "  cd [dir]        change working directory\n"
        "  pwd              print working directory\n"
        "  echo [args...]   print arguments\n"
        "  export N=V       set an environment variable\n"
        "  unset N          remove an environment variable\n"
        "  env              list environment variables\n"
        "  history          list command history for this session\n"
        "  jobs             list background/stopped jobs\n"
        "  fg [%N]          bring a job to the foreground\n"
        "  bg [%N]          resume a stopped job in the background\n"
        "  type NAME        show whether NAME is a builtin or found in PATH\n"
        "  exit [code]      exit the shell\n";
    return 0;
}

int run_builtin(const std::string& name, const std::vector<std::string>& argv, Executor& ctx) {
    if (name == "cd") return builtin_cd(argv);
    if (name == "pwd") return builtin_pwd();
    if (name == "echo") return builtin_echo(argv);
    if (name == "export") return builtin_export(argv);
    if (name == "unset") return builtin_unset(argv);
    if (name == "env") return builtin_env();
    if (name == "history") return builtin_history(ctx);
    if (name == "jobs") return builtin_jobs(ctx);
    if (name == "fg") return ctx.do_fg(argv);
    if (name == "bg") return ctx.do_bg(argv);
    if (name == "type" || name == "which") return builtin_type(argv);
    if (name == "help") return builtin_help();
    if (name == "clear") { std::cout << "\x1b[2J\x1b[H"; return 0; }
    if (name == "alias") return 0; // accepted, no-op this milestone
    if (name == "exit") {
        int code = 0;
        if (argv.size() > 1) { try { code = std::stoi(argv[1]); } catch (...) { code = 0; } }
        ctx.request_exit(code);
        return code;
    }
    return 127;
}

} // namespace meridian::shell
```

---

## `src/shell/shell.hpp`

```cpp
#pragma once
// meridian-shell / shell.hpp
//
// Spec's top-level "Shell" component: wraps Executor with the REPL loop
// and prompt-string logic that used to live directly in
// src/app/shell_main.cpp. Extracting it here means the REPL behavior —
// prompt formatting, line-by-line execution, history, exit handling —
// is unit-testable against plain std::istream/std::ostream, without
// needing a real PTY for every case (job control specifically still
// needs a real PTY to test meaningfully — see tests/test_job_control.cpp
// — but everything else here doesn't).

#include "executor.hpp"
#include <iostream>
#include <string>

namespace meridian::shell {

class Shell {
public:
    // `interactive` controls only prompt printing here. Real job
    // control (process-group/terminal ownership, signal disposition
    // changes) is NOT enabled implicitly by this constructor — it has
    // real, process-global side effects (mutates this process's actual
    // signal handlers and process group), so it's something the caller
    // opts into explicitly via enable_job_control() below, once, in a
    // real process. Bundling it into the constructor would make `Shell`
    // unsafe to construct more than once per process (as tests do) —
    // see tests/test_shell_class.cpp for exactly that scenario.
    explicit Shell(bool interactive);

    // Enables real job control (see docs/shell.md): process groups,
    // tcsetpgrp terminal handoff, ignoring job-control signals in this
    // process. Only call this once, in a real interactive process with
    // an actual controlling terminal (see src/app/shell_main.cpp) —
    // never from a test that shares a process with other tests.
    void enable_job_control() { executor_.enable_job_control(); }

    // Reads lines from `in` until EOF or an `exit` command. Returns the
    // process exit code.
    //
    // IMPORTANT about `out`/`err`: they receive the interactive prompt
    // and this Shell's own error messages (lex/parse failures) — NOT
    // builtin or program output. Builtins write to the real process
    // std::cout/std::cerr directly (which IS fd 1/2, possibly dup2'd to
    // a redirect target), and external programs get their own stdio via
    // exec — neither can be redirected into an arbitrary ostream
    // without OS-level fd redirection, which is what the file- and
    // PTY-based tests exercise instead (see tests/test_shell_executor.cpp,
    // tests/test_job_control.cpp). Passing a stringstream here lets you
    // test prompt behavior, error routing, and control flow without a
    // real terminal — it is deliberately not a general output-capture
    // mechanism, because one can't exist uniformly across both the
    // in-process-builtin and forked-child code paths.
    int run_interactive(std::istream& in, std::ostream& out, std::ostream& err);

    // Runs a single command line (the `-c` mode). Returns its exit code.
    int run_command(const std::string& command, std::ostream& err);

    Executor& executor() { return executor_; }
    bool interactive() const { return interactive_; }

private:
    std::string prompt() const;

    Executor executor_;
    bool interactive_;
};

} // namespace meridian::shell
```

---

## `src/shell/shell.cpp`

```cpp
// meridian-shell / shell.cpp
#include "shell.hpp"
#include <unistd.h>

namespace meridian::shell {

Shell::Shell(bool interactive) : interactive_(interactive) {}

std::string Shell::prompt() const {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd))) return "meridian:" + std::string(cwd) + "$ ";
    return "meridian$ ";
}

int Shell::run_command(const std::string& command, std::ostream& err) {
    std::string error;
    int status = executor_.run_line(command, &error);
    if (!error.empty()) { err << "meridian-shell: " << error << "\n"; return 1; }
    return status < 0 ? 1 : status;
}

int Shell::run_interactive(std::istream& in, std::ostream& out, std::ostream& err) {
    std::string line;
    while (true) {
        if (interactive_) { out << prompt(); out.flush(); }
        if (!std::getline(in, line)) break; // EOF: Ctrl+D, or the stream is exhausted

        std::string error;
        int status = executor_.run_line(line, &error);
        if (!error.empty()) err << "meridian-shell: " << error << "\n";
        else if (status >= 0) executor_.push_history(line);

        if (executor_.exit_requested()) return executor_.exit_code();
    }
    return 0;
}

} // namespace meridian::shell
```

---

### src/ai — local-only Meridian AI

## `src/ai/ai_provider.hpp`

```cpp
#pragma once
// meridian-ai-core / ai_provider.hpp
//
// NOT YET IMPLEMENTED. This header exists to pin down the shape of the
// AIProvider interface described in the spec (§47-48) so the rest of the
// architecture — Meridian AI's controller, the popup UI, command
// correction/explain/generate/diagnose — can be built against a stable
// contract later without redesigning it. There is no .cpp file for this
// header, no HTTP client, and nothing here has been compiled against a
// real network call.
//
// Why it stops here: implementing Groq/Gemini/Ollama Cloud for real means
// writing and testing HTTPS + JSON request/response code, and this
// project was built in a sandbox with no network access — there was no
// way to make a real API call, see a real response, or catch a real bug
// in that code. Writing it anyway would mean shipping networking code
// that looks plausible but has never actually talked to a server, which
// is exactly the kind of unverified code this project has otherwise
// avoided. See docs/ai.md and docs/status.md for the concrete next step.

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace meridian::ai {

enum class RiskLevel { Low, Medium, High, Critical };

struct AIResponse {
    std::string explanation;
    std::optional<std::string> suggested_command;
    double confidence = 0.0;
    std::vector<std::string> warnings;
    RiskLevel risk = RiskLevel::Low;
};

struct RequestContext {
    std::string current_command;
    std::optional<std::string> last_command;
    std::optional<int> last_exit_code;
    std::optional<std::string> last_stderr;
    std::string working_directory;
    // Deliberately NOT included by default: full scrollback, full
    // environment, file contents. See §60-61 (secret redaction / context
    // control) — those policies belong here once this is implemented.
};

enum class AIOperation { Fix, Explain, Generate, Diagnose, Ask };

// The common interface every provider (Groq, Gemini, Ollama Cloud, and
// any future OpenAI-compatible endpoint) implements, per spec §48. The
// AI core is meant to talk to providers ONLY through this interface —
// never with provider-specific branches in command-correction/explain
// logic elsewhere.
class AIProvider {
public:
    virtual ~AIProvider() = default;

    virtual std::string id() const = 0;   // e.g. "groq"
    virtual std::string name() const = 0; // e.g. "Groq Cloud"

    virtual bool authenticate(const std::string& api_key) = 0;
    virtual bool validate_credentials() = 0;
    virtual std::vector<std::string> list_models() = 0;

    virtual AIResponse generate(AIOperation op, const RequestContext& ctx, const std::string& model) = 0;

    // Streaming variant: invokes `on_token` as partial text arrives.
    // Returns the same normalized AIResponse once the stream completes.
    virtual AIResponse stream(AIOperation op, const RequestContext& ctx, const std::string& model,
                               const std::function<void(const std::string& partial_text)>& on_token) = 0;

    virtual bool health_check() = 0;
};

} // namespace meridian::ai
```

---

## `src/ai/secret_redactor.hpp`

```cpp
#pragma once
// meridian-ai / secret_redactor.hpp
//
// Pattern-based redaction of likely secrets (API keys, tokens, passwords,
// private key blocks) before any text would be sent to a cloud AI
// provider — spec §60/§65. This is pure local pattern matching: no
// network, fully real, fully testable.
//
// This is a pragmatic v1: known common formats (KEY=VALUE style env
// vars, Authorization: Bearer headers, AWS/GitHub/Slack/OpenAI-style key
// prefixes, PEM private key blocks). It is NOT a claim of catching every
// possible secret shape — see docs/ai.md for what's explicitly out of
// scope and why redaction should never be the only layer of defense.

#include <string>

namespace meridian::ai {

class SecretRedactor {
public:
    // Returns a copy of `text` with likely secrets replaced by
    // "[REDACTED]". If `redaction_count` is non-null, it's set to how
    // many redactions were made (0 if none) — for logging/audit
    // purposes without the caller ever needing to see the actual value.
    std::string redact(const std::string& text, int* redaction_count = nullptr) const;
};

} // namespace meridian::ai
```

---

## `src/ai/secret_redactor.cpp`

```cpp
// meridian-ai / secret_redactor.cpp
#include "secret_redactor.hpp"
#include <regex>

namespace meridian::ai {

namespace {

// Each entry: a pattern plus the regex_replace format string that
// redacts the value while preserving surrounding context (the key name,
// quotes, header text) where that's meaningful to keep.
struct Rule {
    std::regex pattern;
    std::string replacement;
};

const std::vector<Rule>& rules() {
    static const std::vector<Rule> r = {
        // NAME=value / NAME: value where NAME looks like a secret's name
        // (API_KEY, ACCESS_TOKEN, DB_PASSWORD, ...). Keeps the name and
        // any quotes, redacts only the value.
        {std::regex(R"(([A-Za-z0-9_]*(?:key|token|secret|password|passwd|pwd)[A-Za-z0-9_]*\s*[:=]\s*)("?)([^\s"'\n]+)("?))",
                     std::regex::icase),
         "$1$2[REDACTED]$4"},

        // Authorization: Bearer <token>
        {std::regex(R"((Authorization:\s*Bearer\s+)([^\s"']+))", std::regex::icase), "$1[REDACTED]"},

        // Known key-prefix formats.
        {std::regex(R"(\bAKIA[0-9A-Z]{16}\b)"), "[REDACTED]"},                    // AWS access key id
        {std::regex(R"(\bghp_[A-Za-z0-9]{20,}\b)"), "[REDACTED]"},                // GitHub PAT (classic)
        {std::regex(R"(\bgithub_pat_[A-Za-z0-9_]{20,}\b)"), "[REDACTED]"},        // GitHub PAT (fine-grained)
        {std::regex(R"(\bsk-[A-Za-z0-9_-]{16,}\b)"), "[REDACTED]"},               // OpenAI/Anthropic-style secret keys
        {std::regex(R"(\bxox[baprs]-[A-Za-z0-9-]{10,}\b)"), "[REDACTED]"},        // Slack tokens

        // PEM private key blocks, redacted whole (not just the header).
        {std::regex(R"(-----BEGIN [^-]+PRIVATE KEY-----[\s\S]*?-----END [^-]+PRIVATE KEY-----)"),
         "-----BEGIN [REDACTED] PRIVATE KEY-----\n[REDACTED]\n-----END [REDACTED] PRIVATE KEY-----"},
    };
    return r;
}

} // namespace

std::string SecretRedactor::redact(const std::string& text, int* redaction_count) const {
    std::string out = text;
    int total = 0;
    for (const auto& rule : rules()) {
        total += static_cast<int>(
            std::distance(std::sregex_iterator(out.begin(), out.end(), rule.pattern), std::sregex_iterator()));
        out = std::regex_replace(out, rule.pattern, rule.replacement);
    }
    if (redaction_count) *redaction_count = total;
    return out;
}

} // namespace meridian::ai
```

---

## `src/ai/risk_classifier.hpp`

```cpp
#pragma once
// meridian-ai / risk_classifier.hpp
//
// Classifies a command line's risk level (spec §41/§73) using local
// pattern matching only — no network, no AI model, fully testable.
// This is deliberately conservative and pattern-based rather than a
// claim of true semantic understanding of what a command will do.

#include <string>
#include <vector>

namespace meridian::ai {

enum class RiskLevel { Low, Medium, High, Critical };

std::string to_string(RiskLevel level);

struct RiskResult {
    RiskLevel level = RiskLevel::Low;
    std::vector<std::string> reasons; // human-readable, e.g. "matches rm -rf"
};

class RiskClassifier {
public:
    RiskResult classify(const std::string& command_line) const;
};

} // namespace meridian::ai
```

---

## `src/ai/risk_classifier.cpp`

```cpp
// meridian-ai / risk_classifier.cpp
#include "risk_classifier.hpp"
#include <regex>

namespace meridian::ai {

std::string to_string(RiskLevel level) {
    switch (level) {
        case RiskLevel::Low: return "LOW";
        case RiskLevel::Medium: return "MEDIUM";
        case RiskLevel::High: return "HIGH";
        case RiskLevel::Critical: return "CRITICAL";
    }
    return "LOW";
}

namespace {

struct Pattern {
    std::regex re;
    std::string reason;
};

const std::vector<Pattern>& critical_patterns() {
    static const std::vector<Pattern> p = {
        {std::regex(R"(\brm\s+-[a-zA-Z]*r[a-zA-Z]*f[a-zA-Z]*\b(\s+\S+)*\s+/\s*(\*|)\s*($|[;&|]))"),
         "recursive force-delete targeting the filesystem root"},
        {std::regex(R"(\brm\s+-[a-zA-Z]*f[a-zA-Z]*r[a-zA-Z]*\b(\s+\S+)*\s+/\s*(\*|)\s*($|[;&|]))"),
         "recursive force-delete targeting the filesystem root"},
        {std::regex(R"(:\s*\(\s*\)\s*\{[^}]*:\s*\|\s*:[^}]*\}\s*;\s*:)"), "classic shell fork bomb pattern"},
        {std::regex(R"(\bdd\b.*\bof=/dev/(sd|nvme|hd|disk|xvd)\w*)"), "writes raw data directly onto a disk device"},
        {std::regex(R"(\bmkfs\.\w+)"), "formats a filesystem, destroying existing data on the target"},
        {std::regex(R"(>\s*/dev/(sd|nvme|hd|disk|xvd)\w*)"), "redirects output directly onto a disk device"},
        {std::regex(R"(\bchmod\s+-R\s+777\s+/\s*($|[;&|]))"), "recursively opens permissions on the filesystem root"},
    };
    return p;
}

const std::vector<Pattern>& high_patterns() {
    static const std::vector<Pattern> p = {
        {std::regex(R"(\bsudo\b)"), "runs with elevated (root) privileges"},
        {std::regex(R"(\brm\s+-[a-zA-Z]*r[a-zA-Z]*f[a-zA-Z]*\b)"), "recursive force-delete"},
        {std::regex(R"(\brm\s+-[a-zA-Z]*f[a-zA-Z]*r[a-zA-Z]*\b)"), "recursive force-delete"},
        {std::regex(R"(\b(shutdown|halt|poweroff)\b)"), "shuts down or halts the machine"},
        {std::regex(R"(\breboot\b)"), "reboots the machine"},
        {std::regex(R"(\bkill\s+-(9|KILL)\b)", std::regex::icase), "sends an unblockable kill signal"},
        {std::regex(R"(\bchown\s+-R\b)"), "recursively changes file ownership"},
        {std::regex(R"(\bsystemctl\s+(stop|restart|disable|mask)\b)"), "changes a running system service's state"},
        {std::regex(R"((curl|wget)\b[^|]*\|\s*(sudo\s+)?(sh|bash|zsh)\b)"), "pipes a downloaded script directly into a shell"},
        {std::regex(R"(\bdd\s+)"), "low-level block-device copy tool — destructive if the target is wrong"},
    };
    return p;
}

const std::vector<Pattern>& medium_patterns() {
    static const std::vector<Pattern> p = {
        {std::regex(R"(\brm\b)"), "deletes files"},
        {std::regex(R"(\bchmod\b)"), "changes file permissions"},
        {std::regex(R"(\bchown\b)"), "changes file ownership"},
        {std::regex(R"(\bnpm\s+install\s+-g\b)"), "installs a package globally"},
        {std::regex(R"(\bpip\d?\s+install\b)"), "installs a package, potentially system-wide"},
        {std::regex(R"(\bgit\s+push\s+[^\n]*(--force|-f)\b)"), "force-pushes, which can overwrite remote history"},
        {std::regex(R"(\bkill\b)"), "sends a signal to terminate a process"},
        {std::regex(R"(\bapt(-get)?\s+(remove|purge)\b)"), "removes an installed package"},
    };
    return p;
}

RiskResult classify_against(const std::string& line, const std::vector<Pattern>& patterns, RiskLevel level) {
    RiskResult r;
    for (const auto& p : patterns) {
        if (std::regex_search(line, p.re)) {
            r.reasons.push_back(p.reason);
        }
    }
    if (!r.reasons.empty()) r.level = level;
    return r;
}

} // namespace

RiskResult RiskClassifier::classify(const std::string& command_line) const {
    RiskResult critical = classify_against(command_line, critical_patterns(), RiskLevel::Critical);
    if (!critical.reasons.empty()) return critical;

    RiskResult high = classify_against(command_line, high_patterns(), RiskLevel::High);
    if (!high.reasons.empty()) return high;

    RiskResult medium = classify_against(command_line, medium_patterns(), RiskLevel::Medium);
    if (!medium.reasons.empty()) return medium;

    return RiskResult{RiskLevel::Low, {}};
}

} // namespace meridian::ai
```

---

## `src/ai/command_analyzer.hpp`

```cpp
#pragma once
// meridian-ai / command_analyzer.hpp
//
// Local-only command-name typo detection (spec §38): scans real PATH
// directories for executables, and when a typed command name isn't a
// known builtin or PATH executable, suggests the closest match by edit
// distance. No network call, no LLM — just PATH lookup and a classic
// string-distance algorithm, which is what "local analyzer" means in
// the spec (cloud AI is a separate, explicitly-optional escalation).
//
// Scoped honestly: this catches "gerp" -> "grep" (the command name
// itself is misspelled). It does NOT catch subcommand-level mistakes
// like "npm instal express" -> "npm install express", since recognizing
// that "instal" isn't a valid npm subcommand requires per-program
// knowledge (or an actual LLM) that a generic PATH-based analyzer
// can't have. See docs/ai.md.

#include <optional>
#include <string>
#include <vector>

namespace meridian::ai {

struct Suggestion {
    std::string original;
    std::string suggested;
    int edit_distance = 0;
};

class CommandAnalyzer {
public:
    // `builtins` is the shell's own builtin name list — these count as
    // "known" alongside anything found on PATH.
    explicit CommandAnalyzer(std::vector<std::string> builtins);

    // Scans $PATH (or `path_override` if given, for testability) for
    // executable files. Real filesystem access — no caching across
    // process runs, so this reflects whatever's actually installed.
    void refresh_path_index(const char* path_override = nullptr);

    std::size_t known_executable_count() const { return path_executables_.size(); }

    // Returns nullopt if `command_name` is already known (a builtin, a
    // PATH executable, or an existing absolute/relative executable
    // path), or if no known name is close enough to guess confidently.
    std::optional<Suggestion> analyze(const std::string& command_name) const;

private:
    bool is_known(const std::string& name) const;

    std::vector<std::string> builtins_;
    std::vector<std::string> path_executables_;
};

// Classic Levenshtein edit distance. Exposed for testing and for reuse
// elsewhere (e.g. a future subcommand-level analyzer).
int edit_distance(const std::string& a, const std::string& b);

} // namespace meridian::ai
```

---

## `src/ai/command_analyzer.cpp`

```cpp
// meridian-ai / command_analyzer.cpp
#include "command_analyzer.hpp"

#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace meridian::ai {

int edit_distance(const std::string& a, const std::string& b) {
    // Optimal String Alignment distance: classic Levenshtein plus an
    // adjacent-transposition case counted as a single operation. Plain
    // Levenshtein charges 2 for "gerp" -> "grep" (a transposed 'e'/'r'),
    // which is the single most common real-world typo shape and would
    // otherwise sit right at the edge of (or past) the suggestion
    // threshold for short command names.
    const std::size_t n = a.size(), m = b.size();
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1));
    for (std::size_t i = 0; i <= n; ++i) dp[i][0] = static_cast<int>(i);
    for (std::size_t j = 0; j <= m; ++j) dp[0][j] = static_cast<int>(j);
    for (std::size_t i = 1; i <= n; ++i) {
        for (std::size_t j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1,          // deletion
                                  dp[i][j - 1] + 1,          // insertion
                                  dp[i - 1][j - 1] + cost}); // substitution
            if (i > 1 && j > 1 && a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1]) {
                dp[i][j] = std::min(dp[i][j], dp[i - 2][j - 2] + 1); // adjacent transposition
            }
        }
    }
    return dp[n][m];
}

CommandAnalyzer::CommandAnalyzer(std::vector<std::string> builtins) : builtins_(std::move(builtins)) {}

void CommandAnalyzer::refresh_path_index(const char* path_override) {
    path_executables_.clear();
    const char* path = path_override ? path_override : std::getenv("PATH");
    if (!path) return;

    std::string p(path);
    std::size_t start = 0;
    while (start <= p.size()) {
        auto colon = p.find(':', start);
        std::string dir = p.substr(start, colon == std::string::npos ? std::string::npos : colon - start);
        if (!dir.empty()) {
            DIR* d = opendir(dir.c_str());
            if (d) {
                struct dirent* entry;
                while ((entry = readdir(d)) != nullptr) {
                    std::string name = entry->d_name;
                    if (name == "." || name == "..") continue;
                    std::string full = dir + "/" + name;
                    struct stat st{};
                    if (stat(full.c_str(), &st) == 0 && S_ISREG(st.st_mode) && access(full.c_str(), X_OK) == 0) {
                        path_executables_.push_back(name);
                    }
                }
                closedir(d);
            }
        }
        if (colon == std::string::npos) break;
        start = colon + 1;
    }
    std::sort(path_executables_.begin(), path_executables_.end());
    path_executables_.erase(std::unique(path_executables_.begin(), path_executables_.end()), path_executables_.end());
}

bool CommandAnalyzer::is_known(const std::string& name) const {
    if (name.find('/') != std::string::npos) {
        // Looks like a path (relative or absolute) rather than a bare
        // command name — check it directly instead of against PATH.
        return access(name.c_str(), X_OK) == 0;
    }
    if (std::find(builtins_.begin(), builtins_.end(), name) != builtins_.end()) return true;
    return std::binary_search(path_executables_.begin(), path_executables_.end(), name);
}

std::optional<Suggestion> CommandAnalyzer::analyze(const std::string& command_name) const {
    if (command_name.empty() || is_known(command_name)) return std::nullopt;

    std::string best;
    int best_distance = -1;
    for (const auto& candidate : builtins_) {
        int d = edit_distance(command_name, candidate);
        if (best_distance < 0 || d < best_distance) { best_distance = d; best = candidate; }
    }
    for (const auto& candidate : path_executables_) {
        int d = edit_distance(command_name, candidate);
        if (best_distance < 0 || d < best_distance) { best_distance = d; best = candidate; }
    }

    if (best_distance < 0) return std::nullopt;

    // Only suggest when the guess is close relative to the word's
    // length — otherwise a short, simply-nonexistent command would get
    // matched to something unrelated just because it happened to be
    // "closest" among thousands of candidates.
    int threshold = std::max(1, static_cast<int>(command_name.size()) / 3);
    if (best_distance > threshold) return std::nullopt;

    return Suggestion{command_name, best, best_distance};
}

} // namespace meridian::ai
```

---

## `src/ai/command_context.hpp`

```cpp
#pragma once
// meridian-ai / command_context.hpp
//
// Spec's CommandContext (§9/§66): the bundle of *local* information
// worth having on hand before analyzing or (eventually) asking a
// provider about a command — current command, last command/exit code/
// stderr, working directory, shell, OS info, and git branch if inside a
// repo. Everything here is gathered from the local machine only:
// getcwd(), uname(), and reading .git/HEAD directly (no `git` binary
// invocation needed, no network). This is exactly the "minimum
// necessary context" spec §66 asks for — deliberately NOT the full
// environment, not scrollback, not file contents.

#include <optional>
#include <string>

namespace meridian::ai {

struct CommandContext {
    std::string current_command;
    std::optional<std::string> last_command;
    std::optional<int> last_exit_code;
    std::optional<std::string> last_stderr;
    std::string working_directory;
    std::string shell_name;
    std::string os_info;               // e.g. "Linux 6.8.0 x86_64"
    bool in_git_repo = false;
    std::optional<std::string> git_branch;
};

class CommandContextBuilder {
public:
    // Gathers real local context. `search_dir` overrides where the git
    // walk-up starts (defaults to the real cwd) — used by tests so they
    // don't depend on this process's actual working directory.
    static CommandContext build(const std::string& current_command,
                                 std::optional<std::string> last_command = std::nullopt,
                                 std::optional<int> last_exit_code = std::nullopt,
                                 std::optional<std::string> last_stderr = std::nullopt,
                                 const std::string& search_dir = "");

    // Exposed separately for testing: walks upward from `start_dir`
    // looking for a .git directory, and if found, reads .git/HEAD
    // directly (format: "ref: refs/heads/<branch>\n", or a bare commit
    // hash when in detached-HEAD state) rather than invoking the `git`
    // binary. Returns {true, branch_or_hash} or {false, nullopt}.
    static std::pair<bool, std::optional<std::string>> find_git_branch(const std::string& start_dir);
};

} // namespace meridian::ai
```

---

## `src/ai/command_context.cpp`

```cpp
// meridian-ai / command_context.cpp
#include "command_context.hpp"

#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace meridian::ai {

namespace {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string parent_dir(const std::string& dir) {
    if (dir.empty() || dir == "/") return "";
    auto slash = dir.find_last_of('/');
    if (slash == std::string::npos) return "";
    if (slash == 0) return "/";
    return dir.substr(0, slash);
}

} // namespace

std::pair<bool, std::optional<std::string>> CommandContextBuilder::find_git_branch(const std::string& start_dir) {
    std::string dir = start_dir;
    for (int depth = 0; depth < 64 && !dir.empty(); ++depth) {
        std::string git_path = dir + "/.git";
        struct stat st{};
        if (stat(git_path.c_str(), &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                // .git is a file, not a directory — a worktree or
                // submodule pointer ("gitdir: <path>"). Not resolved in
                // this v1; we can at least say "yes, this is part of a
                // git repo" without a branch name. See docs/ai.md.
                return {true, std::nullopt};
            }
            std::ifstream f(git_path + "/HEAD");
            if (f.is_open()) {
                std::string line;
                std::getline(f, line);
                line = trim(line);
                const std::string prefix = "ref: refs/heads/";
                if (line.rfind(prefix, 0) == 0) return {true, line.substr(prefix.size())};
                if (!line.empty()) return {true, line}; // detached HEAD: raw commit hash
            }
            return {true, std::nullopt};
        }
        dir = parent_dir(dir);
    }
    return {false, std::nullopt};
}

CommandContext CommandContextBuilder::build(const std::string& current_command,
                                             std::optional<std::string> last_command,
                                             std::optional<int> last_exit_code,
                                             std::optional<std::string> last_stderr,
                                             const std::string& search_dir) {
    CommandContext ctx;
    ctx.current_command = current_command;
    ctx.last_command = std::move(last_command);
    ctx.last_exit_code = last_exit_code;
    ctx.last_stderr = std::move(last_stderr);

    char cwd_buf[4096];
    if (getcwd(cwd_buf, sizeof(cwd_buf))) ctx.working_directory = cwd_buf;

    const char* shell_env = std::getenv("SHELL");
    ctx.shell_name = shell_env ? shell_env : "meridian-shell";

    struct utsname uts{};
    if (uname(&uts) == 0) {
        ctx.os_info = std::string(uts.sysname) + " " + uts.release + " " + uts.machine;
    }

    std::string dir = search_dir.empty() ? ctx.working_directory : search_dir;
    auto [found, branch] = find_git_branch(dir);
    ctx.in_git_repo = found;
    ctx.git_branch = branch;

    return ctx;
}

} // namespace meridian::ai
```

---

## `src/ai/ai_controller.hpp`

```cpp
#pragma once
// meridian-ai / ai_controller.hpp
//
// Ties together Config + CommandAnalyzer + RiskClassifier +
// SecretRedactor into the state machine spec §36/§37/§59/§61/§64
// describe (on/off, detection on/off, privacy mode, provider/model
// selection) and the local-only analysis spec §38-41 describes.
//
// What this does NOT do: call any AI provider. `explain()` and
// `fix_via_provider()`-shaped operations that need real language
// generation report plainly that no provider is configured/implemented
// rather than fabricating a plausible-looking answer — see docs/ai.md.

#include "command_analyzer.hpp"
#include "../core/config.hpp"
#include "risk_classifier.hpp"
#include "secret_redactor.hpp"
#include <string>
#include <vector>

namespace meridian::ai {

class AIController {
public:
    // `config_path` is injected (not hardcoded to ~/.config/meridian)
    // so tests can point it at a throwaway file.
    explicit AIController(std::string config_path);

    void load();
    void save();

    bool enabled() const { return config_.get_bool("ai.enabled", false); }
    bool detection_enabled() const { return config_.get_bool("ai.detect", true); }
    bool privacy_mode() const { return config_.get_bool("ai.privacy", false); }
    std::string provider() const { return config_.get("ai.provider", "(none configured)"); }
    std::string model() const { return config_.get("ai.model", "(none configured)"); }

    void set_enabled(bool v) { config_.set_bool("ai.enabled", v); }
    void set_detection_enabled(bool v) { config_.set_bool("ai.detect", v); }
    void set_privacy_mode(bool v) { config_.set_bool("ai.privacy", v); }
    void set_provider(const std::string& p) { config_.set("ai.provider", p); }

    std::string status_report() const;

    // Real local analysis: PATH-based typo suggestion + risk
    // classification. Returns a formatted, ready-to-print report. Safe
    // to call regardless of enabled()/privacy_mode() — this never
    // touches the network, so those settings don't gate it; they gate
    // whether a GUI/CLI layer *would* also escalate to a cloud provider
    // (which isn't implemented — see explain_command()).
    std::string analyze_command(const std::string& command_line);

    // Always honest about there being no provider behind this yet.
    std::string explain_command(const std::string& command_line) const;

    // Pure secret redaction, exposed directly for the `meridian ai
    // redact` CLI command and for reuse by a future provider layer.
    std::string redact(const std::string& text) const { return redactor_.redact(text); }

    // Report of provider connectivity — since no provider is
    // implemented, this always reports so, per provider, honestly
    // (spec §62 requires "do not crash the terminal" and a clear
    // "Provider unavailable" message — this is that message).
    std::string test_providers() const;

private:
    Config config_;
    CommandAnalyzer analyzer_;
    RiskClassifier risk_;
    SecretRedactor redactor_;
};

} // namespace meridian::ai
```

---

## `src/ai/ai_controller.cpp`

```cpp
// meridian-ai / ai_controller.cpp
#include "ai_controller.hpp"
#include <sstream>

namespace meridian::ai {

namespace {

const std::vector<std::string>& shell_builtin_names() {
    // Mirrors src/shell/builtins.cpp's list. Duplicated rather than
    // linked against meridian-shell-lib on purpose: the AI layer must
    // stay usable even for people running bash/zsh/fish instead of
    // Meridian Shell, so it can't assume the shell library is even
    // linked in. See docs/ai.md.
    static const std::vector<std::string> names = {
        "cd", "pwd", "echo", "exit", "export", "unset", "env",
        "history", "jobs", "fg", "bg", "help", "type", "which", "clear", "alias"
    };
    return names;
}

std::string first_token(const std::string& line) {
    std::size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    std::size_t end = line.find_first_of(" \t", start);
    return line.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

std::string indicator_for_risk(RiskLevel level) {
    switch (level) {
        case RiskLevel::Critical:
        case RiskLevel::High: return "\U0001F512"; // 🔒
        case RiskLevel::Medium: return "\u26A0";    // ⚠
        case RiskLevel::Low: return "";
    }
    return "";
}

} // namespace

AIController::AIController(std::string config_path)
    : config_(std::move(config_path)), analyzer_(shell_builtin_names()) {
    analyzer_.refresh_path_index();
}

void AIController::load() { config_.load(); }
void AIController::save() { config_.save(); }

std::string AIController::status_report() const {
    std::ostringstream out;
    out << "Meridian AI\n"
        << "-------------------------\n"
        << "Status:       " << (enabled() ? "ON" : "OFF") << "\n"
        << "Detection:    " << (detection_enabled() ? "ON" : "OFF") << "\n"
        << "Provider:     " << provider() << "\n"
        << "Model:        " << model() << "\n"
        << "Connection:   N/A (no provider implemented yet — see docs/ai.md)\n"
        << "Privacy:      " << (privacy_mode() ? "ON" : "OFF") << "\n";
    return out.str();
}

std::string AIController::test_providers() const {
    std::ostringstream out;
    out << "Meridian AI Provider Test\n\n";
    for (const char* name : {"Groq", "Gemini", "Ollama Cloud"}) {
        out << name << "\n  Status: NOT IMPLEMENTED (no network access in this build — see docs/ai.md)\n\n";
    }
    return out.str();
}

std::string AIController::analyze_command(const std::string& command_line) {
    if (!enabled() || !detection_enabled()) return "";

    std::string cmd = first_token(command_line);
    if (cmd.empty()) return "";

    std::ostringstream out;
    bool wrote_anything = false;

    auto suggestion = analyzer_.analyze(cmd);
    if (suggestion) {
        std::string corrected = command_line;
        corrected.replace(corrected.find(cmd), cmd.size(), suggestion->suggested);
        out << "\u2726 possible correction: " << corrected
            << "  (\"" << suggestion->original << "\" not found; closest known command is \""
            << suggestion->suggested << "\", edit distance " << suggestion->edit_distance << ")\n";
        wrote_anything = true;
    }

    auto risk = risk_.classify(command_line);
    if (risk.level != RiskLevel::Low) {
        out << indicator_for_risk(risk.level) << " risk: " << to_string(risk.level);
        if (!risk.reasons.empty()) {
            out << " -";
            for (std::size_t i = 0; i < risk.reasons.size(); ++i) {
                out << (i == 0 ? " " : "; ") << risk.reasons[i];
            }
        }
        out << "\n";
        wrote_anything = true;
    }

    return wrote_anything ? out.str() : "";
}

std::string AIController::explain_command(const std::string& command_line) const {
    std::ostringstream out;
    out << "No AI provider is configured (this build has no Groq/Gemini/Ollama Cloud\n"
        << "implementation — see docs/ai.md), so I can't generate a real natural-language\n"
        << "explanation of:\n\n  " << command_line << "\n\n"
        << "What's available without a provider: `meridian ai analyze \"" << command_line
        << "\"` for local typo detection and risk classification.\n";
    return out.str();
}

} // namespace meridian::ai
```

---

### src/config — terminal settings + keybindings (NEW)

## `src/config/terminal_config.hpp`

```cpp
#pragma once
// meridian-config / terminal_config.hpp
//
// Spec §75's terminal.toml settings, split honestly into two groups:
// the ones that are real and testable without a GUI (scrollback size,
// default shell, startup directory, mouse-scroll-lines), and the ones
// that only mean something once a renderer exists (font, theme, cursor
// blink, padding, tab bar visibility). Both groups are implemented and
// persisted here — a config value isn't "fake" just because nothing
// reads it yet, as long as that's stated plainly, which it is: see the
// per-field comments below.

#include "../core/config.hpp"
#include <string>

namespace meridian::config {

struct TerminalSettings {
    // --- Meaningful right now, independent of any GUI ---
    int scrollback_lines = 10000;      // used directly by ScreenBuffer's constructor
    std::string default_shell = "/bin/bash"; // what PtyManager spawns when none is specified
    std::string startup_directory;      // empty = inherit cwd

    // --- Recorded now; only take effect once a renderer exists ---
    std::string font_family = "monospace";
    int font_size = 13;
    std::string theme_name = "default";
    std::string cursor_style = "block"; // block | underline | bar
    bool cursor_blink = true;
    bool tab_bar_visible = true;
    int padding = 4;
};

class TerminalConfig {
public:
    explicit TerminalConfig(std::string path);

    bool load();
    bool save();

    TerminalSettings get() const { return settings_; }
    void set(const TerminalSettings& s) { settings_ = s; }

    const std::string& path() const { return config_.path(); }

private:
    Config config_;
    TerminalSettings settings_;

    void settings_from_config();
    void settings_to_config();
};

} // namespace meridian::config
```

---

## `src/config/terminal_config.cpp`

```cpp
// meridian-config / terminal_config.cpp
#include "terminal_config.hpp"
#include <cstdlib>

namespace meridian::config {

TerminalConfig::TerminalConfig(std::string path) : config_(std::move(path)) {}

void TerminalConfig::settings_from_config() {
    settings_.scrollback_lines = std::atoi(config_.get("scrollback_lines", "10000").c_str());
    settings_.default_shell = config_.get("default_shell", "/bin/bash");
    settings_.startup_directory = config_.get("startup_directory", "");
    settings_.font_family = config_.get("font_family", "monospace");
    settings_.font_size = std::atoi(config_.get("font_size", "13").c_str());
    settings_.theme_name = config_.get("theme_name", "default");
    settings_.cursor_style = config_.get("cursor_style", "block");
    settings_.cursor_blink = config_.get_bool("cursor_blink", true);
    settings_.tab_bar_visible = config_.get_bool("tab_bar_visible", true);
    settings_.padding = std::atoi(config_.get("padding", "4").c_str());
}

void TerminalConfig::settings_to_config() {
    config_.set("scrollback_lines", std::to_string(settings_.scrollback_lines));
    config_.set("default_shell", settings_.default_shell);
    config_.set("startup_directory", settings_.startup_directory);
    config_.set("font_family", settings_.font_family);
    config_.set("font_size", std::to_string(settings_.font_size));
    config_.set("theme_name", settings_.theme_name);
    config_.set("cursor_style", settings_.cursor_style);
    config_.set_bool("cursor_blink", settings_.cursor_blink);
    config_.set_bool("tab_bar_visible", settings_.tab_bar_visible);
    config_.set("padding", std::to_string(settings_.padding));
}

bool TerminalConfig::load() {
    bool existed = config_.load();
    settings_from_config(); // applies defaults either way
    return existed;
}

bool TerminalConfig::save() {
    settings_to_config();
    return config_.save();
}

} // namespace meridian::config
```

---

## `src/config/keybindings.hpp`

```cpp
#pragma once
// meridian-config / keybindings.hpp
//
// Spec §75/§20's keybindings.toml: a named-action -> key-combo map with
// sensible defaults (matching the specific shortcuts named elsewhere in
// the spec: Ctrl+Space for the AI popup, Ctrl+Shift+F for search, etc).
// This is real, persisted, testable data even with no GUI attached yet
// to actually intercept these key combos — a future input layer reads
// from here rather than hardcoding shortcuts.

#include "../core/config.hpp"
#include <map>
#include <string>

namespace meridian::config {

class Keybindings {
public:
    explicit Keybindings(std::string path);

    bool load();
    bool save();

    // Returns the bound combo for `action`, or the built-in default if
    // never customized (defaults are seeded in the constructor, so this
    // never returns empty for a known action name).
    std::string get(const std::string& action) const;
    void set(const std::string& action, const std::string& combo);

    // All action -> combo pairs currently in effect (defaults plus any
    // overrides), for listing/display purposes.
    std::map<std::string, std::string> all() const;

private:
    Config config_;
    std::map<std::string, std::string> defaults_;
};

} // namespace meridian::config
```

---

## `src/config/keybindings.cpp`

```cpp
// meridian-config / keybindings.cpp
#include "keybindings.hpp"

namespace meridian::config {

Keybindings::Keybindings(std::string path) : config_(std::move(path)) {
    // Defaults straight from the spec's own examples: §42 (Ctrl+Space
    // for the AI popup), §23 (Ctrl+Shift+F search), §18/§19 (tabs/panes).
    defaults_ = {
        {"ai_popup", "Ctrl+Space"},
        {"search", "Ctrl+Shift+F"},
        {"new_tab", "Ctrl+T"},
        {"close_tab", "Ctrl+W"},
        {"next_tab", "Ctrl+Tab"},
        {"prev_tab", "Ctrl+Shift+Tab"},
        {"split_horizontal", "Ctrl+Shift+H"},
        {"split_vertical", "Ctrl+Shift+V"},
        {"focus_next_pane", "Ctrl+Alt+Right"},
        {"focus_prev_pane", "Ctrl+Alt+Left"},
        {"close_pane", "Ctrl+Shift+W"},
        {"copy", "Ctrl+Shift+C"},
        {"paste", "Ctrl+Shift+V"},
        {"increase_font_size", "Ctrl+Plus"},
        {"decrease_font_size", "Ctrl+Minus"},
    };
}

bool Keybindings::load() { return config_.load(); }
bool Keybindings::save() { return config_.save(); }

std::string Keybindings::get(const std::string& action) const {
    std::string default_combo;
    auto it = defaults_.find(action);
    if (it != defaults_.end()) default_combo = it->second;
    return config_.get("keybind." + action, default_combo);
}

void Keybindings::set(const std::string& action, const std::string& combo) {
    config_.set("keybind." + action, combo);
}

std::map<std::string, std::string> Keybindings::all() const {
    std::map<std::string, std::string> result;
    for (const auto& [action, default_combo] : defaults_) {
        result[action] = config_.get("keybind." + action, default_combo);
    }
    return result;
}

} // namespace meridian::config
```

---

### src/security — credential storage (NEW)

## `src/security/credential_store.hpp`

```cpp
#pragma once
// meridian-security / credential_store.hpp
//
// Spec §57: API keys must never be hardcoded, committed, or logged,
// and should prefer the Linux Secret Service / desktop keyring when
// available. This defines that interface plus the fallback actually
// usable in this build environment — see docs/security.md for exactly
// why the fallback is what it is (short version: this machine has the
// libsecret *runtime* library but not its development headers, and
// there's no network access to install them, so nothing here has been
// compiled against real Secret Service).
//
// Nothing in this repo currently calls store()/retrieve() for a real
// secret — no provider is implemented yet to have credentials for. This
// exists so that work can plug in directly once a provider does.

#include <optional>
#include <string>

namespace meridian::security {

class CredentialStore {
public:
    virtual ~CredentialStore() = default;
    virtual bool store(const std::string& key_name, const std::string& secret) = 0;
    virtual std::optional<std::string> retrieve(const std::string& key_name) = 0;
    virtual bool remove(const std::string& key_name) = 0;
    virtual std::string backend_name() const = 0;
};

// Fallback backend: stores each secret in its own file under
// `<dir>/<key_name>`, created with mode 0600 (owner read/write only).
// This is NOT OS-keyring-grade protection — no encryption at rest, no
// integration with the session keyring/login unlock, just filesystem
// permissions. It exists so the interface has a real, working
// implementation to test against; docs/security.md is explicit that a
// real Secret Service backend should replace this before any provider
// integration ships credentials through it for real users.
class FileCredentialStore : public CredentialStore {
public:
    explicit FileCredentialStore(std::string dir);

    bool store(const std::string& key_name, const std::string& secret) override;
    std::optional<std::string> retrieve(const std::string& key_name) override;
    bool remove(const std::string& key_name) override;
    std::string backend_name() const override { return "file (0600) — NOT OS-keyring-backed"; }

private:
    std::string path_for(const std::string& key_name) const;
    std::string dir_;
};

} // namespace meridian::security
```

---

## `src/security/credential_store.cpp`

```cpp
// meridian-security / credential_store.cpp
#include "credential_store.hpp"

#include <cctype>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace meridian::security {

namespace {

void mkdir_p(const std::string& dir) {
    if (dir.empty()) return;
    std::string partial;
    std::size_t start = (dir[0] == '/') ? 1 : 0;
    if (dir[0] == '/') partial = "/";
    std::size_t pos = start;
    while (pos <= dir.size()) {
        auto slash = dir.find('/', pos);
        std::string component = dir.substr(pos, slash == std::string::npos ? std::string::npos : slash - pos);
        if (!component.empty()) {
            partial += component;
            mkdir(partial.c_str(), 0700);
            partial += "/";
        }
        if (slash == std::string::npos) break;
        pos = slash + 1;
    }
}

// Only [A-Za-z0-9_.-] survive; everything else becomes '_'. Prevents a
// key_name containing '/' or ".." from escaping the credentials
// directory (e.g. a provider name that somehow contained a path
// separator could otherwise write outside `dir_`).
std::string sanitize(const std::string& key_name) {
    std::string out;
    out.reserve(key_name.size());
    for (char c : key_name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.') out.push_back(c);
        else out.push_back('_');
    }
    if (out.empty()) out = "_";
    return out;
}

} // namespace

FileCredentialStore::FileCredentialStore(std::string dir) : dir_(std::move(dir)) {}

std::string FileCredentialStore::path_for(const std::string& key_name) const {
    return dir_ + "/" + sanitize(key_name);
}

bool FileCredentialStore::store(const std::string& key_name, const std::string& secret) {
    mkdir_p(dir_);
    std::string path = path_for(key_name);
    // Create with mode 0600 directly via open(), rather than creating
    // the file with default permissions and chmod'ing afterward — the
    // latter leaves a real (if brief) window where the file exists
    // world/group-readable before the chmod lands. open()'s mode
    // argument can only have bits *cleared* by umask, never added, so
    // requesting 0600 here guarantees no group/other bits are ever set.
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) return false;
    std::size_t written = 0;
    while (written < secret.size()) {
        ssize_t n = ::write(fd, secret.data() + written, secret.size() - written);
        if (n <= 0) { close(fd); return false; }
        written += static_cast<std::size_t>(n);
    }
    return close(fd) == 0;
}

std::optional<std::string> FileCredentialStore::retrieve(const std::string& key_name) {
    std::ifstream f(path_for(key_name));
    if (!f.is_open()) return std::nullopt;
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool FileCredentialStore::remove(const std::string& key_name) {
    std::string path = path_for(key_name);
    if (::access(path.c_str(), F_OK) != 0) return true; // already gone
    return ::unlink(path.c_str()) == 0;
}

} // namespace meridian::security
```

---

### src/app — entry points

## `src/app/demo_main.cpp`

```cpp
// src/app/demo_main.cpp
//
// Headless integration demo. Spawns a REAL /bin/bash process on a REAL
// Linux PTY, captures its raw output, and feeds it through the same
// ANSI parser + screen buffer a GUI frontend would use for rendering.
// This is the concrete proof that the PTY layer and the VT engine work
// together end-to-end — no display server required to verify it.

#include "../core/pty/pty_manager.hpp"
#include "../core/vt/ansi_parser.hpp"
#include "../core/vt/screen_buffer.hpp"

#include <iostream>
#include <string>

using namespace meridian;

int main() {
    std::cout << "=== Meridian Terminal Core -- headless PTY + ANSI demo ===\n\n";

    pty::PtyManager pty;
    pty::SpawnOptions opts;
    opts.program = "/bin/bash";
    opts.args = {
        "--norc", "--noprofile", "-c",
        "printf 'Hello \\033[1mBOLD\\033[0m and \\033[31mRED\\033[0m\\n'; "
        "printf 'Second line, \\033[4munderlined\\033[0m.\\n'"
    };
    opts.rows = 24;
    opts.cols = 80;

    if (!pty.spawn(opts)) {
        std::cerr << "spawn failed: " << pty.last_error() << "\n";
        return 1;
    }
    std::cout << "spawned real child process, pid=" << pty.child_pid() << "\n";

    vt::ScreenBuffer screen(opts.rows, opts.cols);
    vt::AnsiParser parser(screen);

    std::string raw;
    char buf[4096];
    ssize_t n;
    while ((n = pty.read(buf, sizeof(buf))) > 0) {
        raw.append(buf, static_cast<std::size_t>(n));
        parser.feed(buf, static_cast<std::size_t>(n));
    }

    int status = pty.wait_for_exit();
    std::cout << "child exited with status " << status << "\n\n";

    std::cout << "--- raw bytes received from the PTY (" << raw.size() << " bytes) ---\n";
    for (unsigned char c : raw) {
        if (c == '\x1b') std::cout << "\\e";
        else if (c == '\r') std::cout << "\\r";
        else if (c == '\n') std::cout << "\\n\n";
        else std::cout << c;
    }
    std::cout << "\n\n";

    std::cout << "--- decoded screen, after real ANSI parsing ---\n";
    std::cout << screen.dump_text() << "\n\n";

    std::cout << "--- proving SGR attribute decoding actually happened ---\n";
    // "Hello " is 6 characters (columns 0-5), so column 6 is the 'B' of BOLD.
    const auto& bold_cell = screen.cell_at(0, 6);
    std::cout << "cell(0,6) = '" << static_cast<char>(bold_cell.codepoint)
              << "'  bold=" << (bold_cell.attrs.bold ? "true" : "false") << "\n";

    return status;
}
```

---

## `src/app/shell_main.cpp`

```cpp
// src/app/shell_main.cpp
//
// Standalone entry point for Meridian Shell. Runs as an ordinary
// foreground process reading stdin / writing stdout — exactly what
// PtyManager would spawn as `opts.program` if a user picked Meridian
// Shell as their shell (see src/app/demo_main.cpp for that spawn path).
// The actual REPL/prompt/job-control-enable logic lives in
// src/shell/shell.{hpp,cpp} — this file is just argv handling.

#include "../shell/shell.hpp"

#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char** argv) {
    // `meridian-shell -c "cmd"` runs one command and exits — the same
    // convention /bin/bash follows, and what lets other programs (and
    // this project's own $(...) command substitution) invoke it.
    if (argc >= 3 && std::string(argv[1]) == "-c") {
        meridian::shell::Shell shell(/*interactive=*/false);
        return shell.run_command(argv[2], std::cerr);
    }

    bool interactive = isatty(STDIN_FILENO);
    meridian::shell::Shell shell(interactive);
    if (interactive) shell.enable_job_control(); // real, once, in this real process only
    return shell.run_interactive(std::cin, std::cout, std::cerr);
}
```

---

## `src/app/meridian_main.cpp`

```cpp
// src/app/meridian_main.cpp
//
// The `meridian` CLI, implementing the `meridian ai ...` command family
// (spec §36/§37/§59/§61-64) against the real, local-only AI layer in
// src/ai/. No network calls happen anywhere in this binary — see
// docs/ai.md for what's implemented vs. planned.

#include "../ai/ai_controller.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace meridian::ai;

namespace {

std::string default_config_path() {
    // MERIDIAN_CONFIG_HOME lets tests (and users who want an isolated
    // config) avoid touching the real ~/.config.
    if (const char* override_dir = std::getenv("MERIDIAN_CONFIG_HOME")) {
        return std::string(override_dir) + "/ai.toml";
    }
    const char* home = std::getenv("HOME");
    std::string base = home ? home : ".";
    return base + "/.config/meridian/ai.toml";
}

void print_usage() {
    std::cout <<
        "Usage: meridian ai <command> [args]\n\n"
        "  on | off                    enable/disable Meridian AI\n"
        "  status                      show current AI state\n"
        "  detect on | detect off      toggle local command analysis\n"
        "  privacy on | privacy off    toggle privacy mode\n"
        "  providers                   list configured providers\n"
        "  use <provider>              set the active provider name\n"
        "  test                        test provider connectivity\n"
        "  analyze \"<command>\"         local typo + risk analysis (no network)\n"
        "  explain \"<command>\"         explain a command (needs a provider — not yet implemented)\n"
        "  redact \"<text>\"             show text with likely secrets redacted\n";
}

int join_argv(int argc, char** argv, int start, std::string* out) {
    std::string s;
    for (int i = start; i < argc; ++i) {
        if (i > start) s += " ";
        s += argv[i];
    }
    *out = s;
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) != "ai") {
        print_usage();
        return 1;
    }
    if (argc < 3) {
        print_usage();
        return 1;
    }

    AIController controller(default_config_path());
    controller.load();

    std::string cmd = argv[2];

    if (cmd == "on") { controller.set_enabled(true); controller.save(); std::cout << "Meridian AI enabled.\n"; return 0; }
    if (cmd == "off") { controller.set_enabled(false); controller.save(); std::cout << "Meridian AI disabled.\n"; return 0; }
    if (cmd == "status") { std::cout << controller.status_report(); return 0; }

    if (cmd == "detect" && argc >= 4) {
        std::string sub = argv[3];
        if (sub == "on") { controller.set_detection_enabled(true); controller.save(); std::cout << "Detection enabled.\n"; return 0; }
        if (sub == "off") { controller.set_detection_enabled(false); controller.save(); std::cout << "Detection disabled.\n"; return 0; }
    }

    if (cmd == "privacy" && argc >= 4) {
        std::string sub = argv[3];
        if (sub == "on") { controller.set_privacy_mode(true); controller.save(); std::cout << "Privacy mode enabled: remote AI requests would be disabled (no provider is implemented, so this is currently moot).\n"; return 0; }
        if (sub == "off") { controller.set_privacy_mode(false); controller.save(); std::cout << "Privacy mode disabled.\n"; return 0; }
    }

    if (cmd == "providers") {
        std::cout << "Configured providers:\n"
                   << "  (none — Groq, Gemini, and Ollama Cloud are not yet implemented; see docs/ai.md)\n"
                   << "Active provider setting: " << controller.provider() << "\n";
        return 0;
    }

    if (cmd == "use" && argc >= 4) {
        controller.set_provider(argv[3]);
        controller.save();
        std::cout << "Provider preference set to '" << argv[3]
                   << "' (recorded only — no provider implementation exists to actually use it yet).\n";
        return 0;
    }

    if (cmd == "test") { std::cout << controller.test_providers(); return 0; }

    if (cmd == "analyze" && argc >= 4) {
        std::string line;
        join_argv(argc, argv, 3, &line);
        // analyze_command() only returns non-empty output when AI is
        // both on() and detect-enabled — mirror that here rather than
        // silently no-op'ing so the CLI is honest about why nothing
        // printed.
        if (!controller.enabled()) { std::cout << "Meridian AI is off (`meridian ai on` to enable).\n"; return 0; }
        if (!controller.detection_enabled()) { std::cout << "Detection is off (`meridian ai detect on` to enable).\n"; return 0; }
        std::string report = controller.analyze_command(line);
        std::cout << (report.empty() ? "No issues detected.\n" : report);
        return 0;
    }

    if (cmd == "explain" && argc >= 4) {
        std::string line;
        join_argv(argc, argv, 3, &line);
        std::cout << controller.explain_command(line);
        return 0;
    }

    if (cmd == "redact" && argc >= 4) {
        std::string line;
        join_argv(argc, argv, 3, &line);
        std::cout << controller.redact(line) << "\n";
        return 0;
    }

    print_usage();
    return 1;
}
```

---

### tests

## `tests/mini_test.hpp`

```cpp
#pragma once
// tests / mini_test.hpp
//
// A tiny, dependency-free test framework. googletest would normally be
// used here (see docs/status.md for why it isn't available in this
// build), but the assertion/registration/reporting behavior is the same
// shape: TEST-per-function, auto-registration via static initializers,
// ASSERT_* macros, one pass/fail summary at the end.

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace mtest {

inline std::vector<std::pair<std::string, std::function<void()>>>& registry() {
    static std::vector<std::pair<std::string, std::function<void()>>> r;
    return r;
}
inline int& failure_count() { static int f = 0; return f; }
inline int& assertion_count() { static int a = 0; return a; }

struct Registrar {
    Registrar(const std::string& name, std::function<void()> fn) { registry().emplace_back(name, std::move(fn)); }
};

inline void check(bool cond, const char* expr, const char* file, int line) {
    assertion_count()++;
    if (!cond) {
        failure_count()++;
        std::cerr << "    ASSERT FAILED: " << expr << "  (" << file << ":" << line << ")\n";
    }
}

inline int run_all() {
    int total = 0, failed_tests = 0;
    for (auto& [name, fn] : registry()) {
        int before = failure_count();
        std::cerr << "[ RUN  ] " << name << "\n";
        fn();
        total++;
        if (failure_count() > before) { failed_tests++; std::cerr << "[ FAIL ] " << name << "\n"; }
        else { std::cerr << "[  OK  ] " << name << "\n"; }
    }
    std::cerr << "\n" << total << " tests, " << assertion_count() << " assertions, "
              << failed_tests << " failed test(s), " << failure_count() << " failed assertion(s)\n";
    return failed_tests == 0 ? 0 : 1;
}

} // namespace mtest

#define MTEST(name)                                                        \
    void mtest_##name();                                                   \
    static mtest::Registrar mtest_reg_##name(#name, mtest_##name);         \
    void mtest_##name()

#define ASSERT_TRUE(cond) mtest::check((cond), #cond, __FILE__, __LINE__)
#define ASSERT_FALSE(cond) mtest::check(!(cond), "!(" #cond ")", __FILE__, __LINE__)
#define ASSERT_EQ(a, b) mtest::check((a) == (b), #a " == " #b, __FILE__, __LINE__)
#define ASSERT_NE(a, b) mtest::check((a) != (b), #a " != " #b, __FILE__, __LINE__)
```

---

## `tests/test_main.cpp`

```cpp
// tests / test_main.cpp
#include "mini_test.hpp"

int main() {
    return mtest::run_all();
}
```

---

## `tests/test_screen_buffer.cpp`

```cpp
// tests / test_screen_buffer.cpp
#include "mini_test.hpp"
#include "../src/core/vt/screen_buffer.hpp"

using namespace meridian::vt;

MTEST(cursor_starts_at_origin) {
    ScreenBuffer sb(5, 10);
    ASSERT_EQ(sb.cursor_row(), 0);
    ASSERT_EQ(sb.cursor_col(), 0);
}

MTEST(put_codepoint_advances_cursor_and_writes_cell) {
    ScreenBuffer sb(5, 10);
    Attributes a;
    sb.put_codepoint(U'H', a);
    sb.put_codepoint(U'i', a);
    ASSERT_EQ(sb.cursor_col(), 2);
    ASSERT_EQ(sb.cell_at(0, 0).codepoint, (char32_t)U'H');
    ASSERT_EQ(sb.cell_at(0, 1).codepoint, (char32_t)U'i');
    ASSERT_EQ(sb.dump_row_text(0), std::string("Hi"));
}

MTEST(line_wraps_at_column_limit) {
    ScreenBuffer sb(3, 4); // 4 columns
    Attributes a;
    for (char c : std::string("ABCDE")) sb.put_codepoint(static_cast<char32_t>(c), a);
    // "ABCD" fills row 0, 'E' wraps onto row 1
    ASSERT_EQ(sb.dump_row_text(0), std::string("ABCD"));
    ASSERT_EQ(sb.dump_row_text(1), std::string("E"));
}

MTEST(newline_at_bottom_scrolls_and_feeds_scrollback) {
    ScreenBuffer sb(2, 5, /*scrollback_limit=*/100);
    Attributes a;
    for (char c : std::string("row0")) sb.put_codepoint(static_cast<char32_t>(c), a);
    sb.carriage_return();
    sb.newline();
    for (char c : std::string("row1")) sb.put_codepoint(static_cast<char32_t>(c), a);
    sb.carriage_return();
    sb.newline(); // cursor was on the bottom (last) row -> this scrolls
    for (char c : std::string("row2")) sb.put_codepoint(static_cast<char32_t>(c), a);

    ASSERT_EQ(sb.scrollback().size(), (std::size_t)1);
    ASSERT_EQ(sb.dump_row_text(0), std::string("row1"));
    ASSERT_EQ(sb.dump_row_text(1), std::string("row2"));
}

MTEST(erase_in_line_modes) {
    ScreenBuffer sb(2, 5);
    Attributes a;
    for (char c : std::string("ABCDE")) sb.put_codepoint(static_cast<char32_t>(c), a);
    sb.set_cursor(0, 2);
    sb.erase_in_line(0); // cursor -> end
    ASSERT_EQ(sb.dump_row_text(0), std::string("AB"));
}

MTEST(erase_in_display_all_clears_grid) {
    ScreenBuffer sb(2, 5);
    Attributes a;
    for (char c : std::string("ABCDE")) sb.put_codepoint(static_cast<char32_t>(c), a);
    sb.erase_in_display(2);
    ASSERT_EQ(sb.dump_row_text(0), std::string(""));
    ASSERT_EQ(sb.dump_row_text(1), std::string(""));
}

MTEST(alt_screen_preserves_primary_content) {
    ScreenBuffer sb(2, 5);
    Attributes a;
    sb.put_codepoint(U'X', a);
    sb.enter_alt_screen();
    sb.put_codepoint(U'Y', a);
    ASSERT_EQ(sb.dump_row_text(0), std::string("Y"));
    sb.exit_alt_screen();
    ASSERT_EQ(sb.dump_row_text(0), std::string("X"));
}

MTEST(wide_codepoint_occupies_two_columns) {
    ScreenBuffer sb(2, 10);
    Attributes a;
    sb.put_codepoint(0x4E2D /* CJK 'middle' */, a); // wide
    ASSERT_EQ(sb.cursor_col(), 2);
    ASSERT_EQ(sb.cell_at(0, 0).width, (uint8_t)2);
    ASSERT_EQ(sb.cell_at(0, 1).width, (uint8_t)0); // continuation cell
}

MTEST(resize_preserves_grid_dimensions_without_crash) {
    ScreenBuffer sb(5, 5);
    sb.resize(10, 20);
    ASSERT_EQ(sb.rows(), 10);
    ASSERT_EQ(sb.cols(), 20);
    // Writing after resize must not crash or go out of bounds.
    Attributes a;
    for (int i = 0; i < 25; ++i) sb.put_codepoint(U'x', a);
}
```

---

## `tests/test_ansi_parser.cpp`

```cpp
// tests / test_ansi_parser.cpp
#include "mini_test.hpp"
#include "../src/core/vt/ansi_parser.hpp"
#include "../src/core/vt/screen_buffer.hpp"

using namespace meridian::vt;

MTEST(plain_text_passes_through) {
    ScreenBuffer sb(3, 20);
    AnsiParser p(sb);
    p.feed("hello");
    ASSERT_EQ(sb.dump_row_text(0), std::string("hello"));
}

MTEST(sgr_bold_sets_cell_attribute) {
    ScreenBuffer sb(3, 20);
    AnsiParser p(sb);
    p.feed("\x1b[1mB");
    ASSERT_TRUE(sb.cell_at(0, 0).attrs.bold);
}

MTEST(sgr_reset_clears_attributes) {
    ScreenBuffer sb(3, 20);
    AnsiParser p(sb);
    p.feed("\x1b[1;4mX\x1b[0mY");
    ASSERT_TRUE(sb.cell_at(0, 0).attrs.bold);
    ASSERT_TRUE(sb.cell_at(0, 0).attrs.underline);
    ASSERT_FALSE(sb.cell_at(0, 1).attrs.bold);
}

MTEST(sgr_indexed_color) {
    ScreenBuffer sb(3, 20);
    AnsiParser p(sb);
    p.feed("\x1b[31mR"); // red foreground
    ASSERT_TRUE(sb.cell_at(0, 0).attrs.fg.kind == Color::Kind::Indexed);
    ASSERT_EQ(sb.cell_at(0, 0).attrs.fg.index, (uint8_t)1);
}

MTEST(sgr_24bit_rgb_color) {
    ScreenBuffer sb(3, 20);
    AnsiParser p(sb);
    p.feed("\x1b[38;2;10;20;30mZ");
    auto& fg = sb.cell_at(0, 0).attrs.fg;
    ASSERT_TRUE(fg.kind == Color::Kind::Rgb);
    ASSERT_EQ(fg.r, (uint8_t)10);
    ASSERT_EQ(fg.g, (uint8_t)20);
    ASSERT_EQ(fg.b, (uint8_t)30);
}

MTEST(cup_moves_cursor_absolute) {
    ScreenBuffer sb(10, 10);
    AnsiParser p(sb);
    p.feed("\x1b[3;5H");
    ASSERT_EQ(sb.cursor_row(), 2); // 1-based -> 0-based
    ASSERT_EQ(sb.cursor_col(), 4);
}

MTEST(cursor_up_down_forward_back) {
    ScreenBuffer sb(10, 10);
    AnsiParser p(sb);
    p.feed("\x1b[5;5H\x1b[2A\x1b[3C");
    ASSERT_EQ(sb.cursor_row(), 2);
    ASSERT_EQ(sb.cursor_col(), 7);
}

MTEST(ed_erases_whole_display) {
    ScreenBuffer sb(2, 5);
    AnsiParser p(sb);
    p.feed("ABCDE\x1b[2J");
    ASSERT_EQ(sb.dump_row_text(0), std::string(""));
}

MTEST(alt_screen_decset_1049) {
    ScreenBuffer sb(2, 5);
    AnsiParser p(sb);
    p.feed("main\x1b[?1049h" "alt");
    ASSERT_TRUE(sb.in_alt_screen());
    ASSERT_EQ(sb.dump_row_text(0), std::string("alt"));
    p.feed("\x1b[?1049l");
    ASSERT_FALSE(sb.in_alt_screen());
    ASSERT_EQ(sb.dump_row_text(0), std::string("main"));
}

MTEST(osc_window_title_bel_terminated) {
    ScreenBuffer sb(2, 5);
    AnsiParser p(sb);
    p.feed("\x1b]0;My Title\x07");
    ASSERT_EQ(p.window_title(), std::string("My Title"));
}

MTEST(utf8_multibyte_decodes_to_one_cell) {
    ScreenBuffer sb(2, 10);
    AnsiParser p(sb);
    // U+00E9 'é' = 0xC3 0xA9 in UTF-8
    std::string data;
    data.push_back(static_cast<char>(0xC3));
    data.push_back(static_cast<char>(0xA9));
    p.feed(data);
    ASSERT_EQ(sb.cursor_col(), 1);
    ASSERT_EQ(sb.cell_at(0, 0).codepoint, (char32_t)0x00E9);
}

MTEST(utf8_split_across_two_feed_calls) {
    ScreenBuffer sb(2, 10);
    AnsiParser p(sb);
    std::string first, second;
    first.push_back(static_cast<char>(0xE4));  // first byte of a 3-byte sequence
    second.push_back(static_cast<char>(0xB8));
    second.push_back(static_cast<char>(0xAD)); // together: U+4E2D
    p.feed(first);
    p.feed(second);
    ASSERT_EQ(sb.cell_at(0, 0).codepoint, (char32_t)0x4E2D);
}

// ---- Malformed / adversarial input must never crash the parser ----

MTEST(truncated_csi_at_end_of_stream_does_not_crash) {
    ScreenBuffer sb(2, 10);
    AnsiParser p(sb);
    p.feed("\x1b[1;2"); // never reaches a final byte
    // No crash is the test; parser should just be "waiting".
    p.feed("m"); // now complete it
    ASSERT_TRUE(true);
}

MTEST(csi_interrupted_by_fresh_escape_recovers) {
    ScreenBuffer sb(2, 10);
    AnsiParser p(sb);
    p.feed("\x1b[999999999999999999999999"); // huge, non-terminating param
    p.feed("\x1b[2J"); // fresh escape should still be recognized after recovery
    p.feed("Q");
    ASSERT_EQ(sb.dump_row_text(0), std::string("Q"));
}

MTEST(stray_continuation_bytes_are_dropped_not_fatal) {
    ScreenBuffer sb(2, 10);
    AnsiParser p(sb);
    std::string data;
    data.push_back(static_cast<char>(0x80)); // continuation byte with no lead byte
    data.push_back(static_cast<char>(0xFF)); // invalid byte
    data += "ok";
    p.feed(data);
    ASSERT_EQ(sb.dump_row_text(0), std::string("ok"));
}

MTEST(unterminated_osc_does_not_grow_unbounded_or_crash) {
    ScreenBuffer sb(2, 10);
    AnsiParser p(sb);
    p.feed("\x1b]0;");
    std::string huge(20000, 'x'); // exceeds kMaxOscLen: must not grow memory unbounded or crash
    p.feed(huge);
    p.feed("\x07");    // eventually terminated
    p.feed("done");    // parser must be back in Ground and printing normally
    ASSERT_EQ(sb.dump_row_text(0), std::string("done"));
}

MTEST(random_byte_soup_never_crashes) {
    ScreenBuffer sb(5, 20);
    AnsiParser p(sb);
    // A deterministic pseudo-random byte stream mixing control bytes,
    // high bytes, and CSI-ish fragments — the point is simply that
    // feed() returns normally for all of it.
    std::string soup;
    unsigned x = 12345;
    for (int i = 0; i < 5000; ++i) {
        x = x * 1103515245u + 12345u;
        soup.push_back(static_cast<char>((x >> 16) & 0xFF));
    }
    p.feed(soup);
    ASSERT_TRUE(true); // reaching this line means it didn't crash
}
```

---

## `tests/test_pty_manager.cpp`

```cpp
// tests / test_pty_manager.cpp
//
// Real integration tests: an actual PTY pair is opened, a real process
// is forked and exec'd onto it, and we read its actual output back
// through the master fd.

#include "mini_test.hpp"
#include "../src/core/pty/pty_manager.hpp"

#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

using namespace meridian::pty;

MTEST(pty_spawns_real_process_and_reads_its_output) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/echo";
    opts.args = {"hello-from-real-process"};
    bool ok = pty.spawn(opts);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(pty.is_running());

    std::string out;
    char buf[256];
    // Drain until EOF (child exits and closes its end of the pty).
    ssize_t n;
    while ((n = pty.read(buf, sizeof(buf))) > 0) out.append(buf, static_cast<std::size_t>(n));

    ASSERT_TRUE(out.find("hello-from-real-process") != std::string::npos);

    int status = pty.wait_for_exit();
    ASSERT_EQ(status, 0);
}

MTEST(pty_reports_nonzero_exit_status) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/bash";
    opts.args = {"-c", "exit 3"};
    ASSERT_TRUE(pty.spawn(opts));

    char buf[64];
    while (pty.read(buf, sizeof(buf)) > 0) { /* drain */ }

    int status = pty.wait_for_exit();
    ASSERT_EQ(status, 3);
}

MTEST(pty_resize_updates_kernel_window_size) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/bash";
    opts.args = {"-c", "sleep 0.2"};
    opts.rows = 24;
    opts.cols = 80;
    ASSERT_TRUE(pty.spawn(opts));

    pty.resize(40, 120);

    struct winsize ws{};
    ioctl(pty.master_fd(), TIOCGWINSZ, &ws);
    ASSERT_EQ(ws.ws_row, (unsigned short)40);
    ASSERT_EQ(ws.ws_col, (unsigned short)120);

    char buf[64];
    while (pty.read(buf, sizeof(buf)) > 0) { /* drain */ }
    pty.wait_for_exit();
}

MTEST(pty_write_reaches_child_stdin) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/bash";
    // cat with no args echoes stdin to stdout until EOF; we write a line
    // then close by exiting the shell via the written command instead
    // (simpler and avoids needing to close master mid-test).
    opts.args = {"-c", "read line; echo \"got:$line\""};
    ASSERT_TRUE(pty.spawn(opts));

    pty.write("ping\n");

    std::string out;
    char buf[256];
    ssize_t n;
    while ((n = pty.read(buf, sizeof(buf))) > 0) out.append(buf, static_cast<std::size_t>(n));

    ASSERT_TRUE(out.find("got:ping") != std::string::npos);
    pty.wait_for_exit();
}
```

---

## `tests/test_shell_lexer.cpp`

```cpp
// tests / test_shell_lexer.cpp
#include "mini_test.hpp"
#include "../src/shell/lexer.hpp"

using namespace meridian::shell;

static std::string word_text(const Word& w) {
    std::string s;
    for (auto& part : w) s += part.text;
    return s;
}

MTEST(lexer_splits_simple_words) {
    Lexer lex("ls -la /tmp");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)4); // 3 words + End
    ASSERT_TRUE(toks[0].type == TokenType::Word);
    ASSERT_EQ(word_text(toks[0].word), std::string("ls"));
    ASSERT_EQ(word_text(toks[1].word), std::string("-la"));
    ASSERT_EQ(word_text(toks[2].word), std::string("/tmp"));
    ASSERT_TRUE(toks[3].type == TokenType::End);
}

MTEST(lexer_single_quotes_are_literal) {
    Lexer lex("echo 'a b | c'");
    auto toks = lex.tokenize();
    ASSERT_EQ(word_text(toks[1].word), std::string("a b | c"));
    ASSERT_TRUE(toks[1].word[0].literal);
}

MTEST(lexer_double_quotes_stay_one_word_but_expandable) {
    Lexer lex("echo \"hi there\"");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, word, End
    ASSERT_EQ(word_text(toks[1].word), std::string("hi there"));
    ASSERT_FALSE(toks[1].word[0].literal);
}

MTEST(lexer_recognizes_pipe_and_operators) {
    Lexer lex("a | b && c || d ; e &");
    auto toks = lex.tokenize();
    std::vector<TokenType> types;
    for (auto& t : toks) types.push_back(t.type);
    std::vector<TokenType> expected = {
        TokenType::Word, TokenType::Pipe, TokenType::Word, TokenType::And, TokenType::Word,
        TokenType::Or, TokenType::Word, TokenType::Semicolon, TokenType::Word, TokenType::Background, TokenType::End
    };
    ASSERT_EQ(types.size(), expected.size());
    for (std::size_t i = 0; i < types.size(); ++i) ASSERT_TRUE(types[i] == expected[i]);
}

MTEST(lexer_recognizes_redirections) {
    Lexer lex("cmd > out.txt 2>> err.txt < in.txt &> both.txt");
    auto toks = lex.tokenize();
    std::vector<TokenType> types;
    for (auto& t : toks) types.push_back(t.type);
    ASSERT_TRUE(types[1] == TokenType::RedirOut);
    ASSERT_TRUE(types[3] == TokenType::RedirErrAppend);
    ASSERT_TRUE(types[5] == TokenType::RedirIn);
    ASSERT_TRUE(types[7] == TokenType::RedirAll);
}

MTEST(lexer_backslash_escapes_next_char) {
    Lexer lex("echo a\\ b");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, "a b", End
    ASSERT_EQ(word_text(toks[1].word), std::string("a b"));
}

MTEST(lexer_comment_ignored_to_end_of_line) {
    Lexer lex("echo hi # this is a comment | not a pipe");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, hi, End
}

MTEST(lexer_reports_unterminated_quote) {
    Lexer lex("echo 'unterminated");
    std::string err;
    auto toks = lex.tokenize(&err);
    ASSERT_TRUE(toks.empty());
    ASSERT_FALSE(err.empty());
}

MTEST(lexer_command_substitution_spans_spaces_as_one_word) {
    // Regression test: spaces *inside* an unquoted $(...) must not split
    // the word, even though bare spaces normally do.
    Lexer lex("echo [$(echo inner)]");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, "[$(echo inner)]", End
    ASSERT_EQ(word_text(toks[1].word), std::string("[$(echo inner)]"));
}

MTEST(lexer_reports_unterminated_command_substitution) {
    std::string err;
    auto toks = Lexer("echo $(echo unterminated").tokenize(&err);
    ASSERT_TRUE(toks.empty());
    ASSERT_FALSE(err.empty());
}

MTEST(lexer_stderr_redirect_only_at_token_boundary) {
    // "file2" is a plain word (ends in a digit but is not just "2"), so
    // this must lex as WORD("file2") RedirOut WORD("out"), not as a
    // stderr redirection.
    Lexer lex("cmd file2>out");
    auto toks = lex.tokenize();
    ASSERT_EQ(word_text(toks[1].word), std::string("file2"));
    ASSERT_TRUE(toks[2].type == TokenType::RedirOut);
}
```

---

## `tests/test_shell_parser.cpp`

```cpp
// tests / test_shell_parser.cpp
#include "mini_test.hpp"
#include "../src/shell/lexer.hpp"
#include "../src/shell/parser.hpp"

using namespace meridian::shell;

static std::string wt(const Word& w) { std::string s; for (auto& p : w) s += p.text; return s; }

static Sequence parse_line(const std::string& line, std::string* err = nullptr) {
    Lexer lex(line);
    std::string lex_err;
    auto toks = lex.tokenize(&lex_err);
    if (!lex_err.empty()) { if (err) *err = lex_err; return {}; }
    Parser p(std::move(toks));
    return p.parse(err);
}

MTEST(parser_builds_single_command) {
    auto seq = parse_line("ls -la");
    ASSERT_EQ(seq.size(), (std::size_t)1);
    ASSERT_EQ(seq[0].pipeline.commands.size(), (std::size_t)1);
    ASSERT_EQ(seq[0].pipeline.commands[0].argv.size(), (std::size_t)2);
    ASSERT_EQ(wt(seq[0].pipeline.commands[0].argv[0]), std::string("ls"));
}

MTEST(parser_builds_pipeline) {
    auto seq = parse_line("cat file | grep foo | sort");
    ASSERT_EQ(seq.size(), (std::size_t)1);
    ASSERT_EQ(seq[0].pipeline.commands.size(), (std::size_t)3);
    ASSERT_EQ(wt(seq[0].pipeline.commands[1].argv[0]), std::string("grep"));
}

MTEST(parser_attaches_redirection_to_command) {
    auto seq = parse_line("sort < in.txt > out.txt");
    auto& cmd = seq[0].pipeline.commands[0];
    ASSERT_EQ(cmd.redirections.size(), (std::size_t)2);
    ASSERT_TRUE(cmd.redirections[0].type == RedirType::In);
    ASSERT_EQ(wt(cmd.redirections[0].target), std::string("in.txt"));
    ASSERT_TRUE(cmd.redirections[1].type == RedirType::Out);
}

MTEST(parser_sequence_connectors) {
    auto seq = parse_line("a && b || c ; d");
    ASSERT_EQ(seq.size(), (std::size_t)4);
    ASSERT_TRUE(seq[0].connector == Connector::And);
    ASSERT_TRUE(seq[1].connector == Connector::Or);
    ASSERT_TRUE(seq[2].connector == Connector::Semicolon);
    ASSERT_TRUE(seq[3].connector == Connector::None);
}

MTEST(parser_trailing_ampersand_marks_background) {
    auto seq = parse_line("sleep 10 &");
    ASSERT_EQ(seq.size(), (std::size_t)1);
    ASSERT_TRUE(seq[0].pipeline.background);
}

MTEST(parser_empty_line_is_empty_sequence_not_error) {
    std::string err;
    auto seq = parse_line("", &err);
    ASSERT_TRUE(seq.empty());
    ASSERT_TRUE(err.empty());
}

MTEST(parser_reports_error_on_leading_pipe) {
    std::string err;
    auto seq = parse_line("| ls", &err);
    ASSERT_TRUE(seq.empty());
    ASSERT_FALSE(err.empty());
}

MTEST(parser_reports_error_on_redirect_without_target) {
    std::string err;
    auto seq = parse_line("ls >", &err);
    ASSERT_TRUE(seq.empty());
    ASSERT_FALSE(err.empty());
}
```

---

## `tests/test_shell_executor.cpp`

```cpp
// tests / test_shell_executor.cpp
//
// These are true integration tests: they run real external programs
// (coreutils) through Executor's fork/exec/pipe/dup2 path, and check
// results by reading back files the child processes actually wrote —
// not mocked output.

#include "mini_test.hpp"
#include "../src/shell/executor.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace meridian::shell;

namespace {

std::string read_file(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string tmp_path(const std::string& name) {
    return "/tmp/meridian_test_" + name + "_" + std::to_string(getpid());
}

} // namespace

MTEST(executor_runs_external_program_and_reports_exit_status) {
    Executor ex;
    int status = ex.run_line("true");
    ASSERT_EQ(status, 0);
    status = ex.run_line("false");
    ASSERT_NE(status, 0);
}

MTEST(executor_redirects_stdout_to_file) {
    Executor ex;
    std::string f = tmp_path("redir");
    int status = ex.run_line("echo hello > " + f);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(read_file(f), std::string("hello\n"));
    std::remove(f.c_str());
}

MTEST(executor_runs_real_pipeline_through_coreutils) {
    Executor ex;
    std::string f = tmp_path("pipeline");
    int status = ex.run_line("printf 'b\\nc\\na\\n' | sort > " + f);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(read_file(f), std::string("a\nb\nc\n"));
    std::remove(f.c_str());
}

MTEST(executor_and_short_circuits_on_failure) {
    Executor ex;
    std::string f = tmp_path("and");
    std::remove(f.c_str());
    ex.run_line("false && echo nope > " + f);
    std::ifstream check(f);
    ASSERT_FALSE(check.good() && check.peek() != std::ifstream::traits_type::eof());
}

MTEST(executor_or_runs_fallback_on_failure) {
    Executor ex;
    std::string f = tmp_path("or");
    std::remove(f.c_str());
    ex.run_line("false || echo yep > " + f);
    ASSERT_EQ(read_file(f), std::string("yep\n"));
    std::remove(f.c_str());
}

MTEST(executor_cd_builtin_changes_real_process_directory) {
    Executor ex;
    char before[4096];
    if (!getcwd(before, sizeof(before))) before[0] = '\0';
    ex.run_line("cd /tmp");
    char after[4096];
    if (!getcwd(after, sizeof(after))) after[0] = '\0';
    ASSERT_EQ(std::string(after), std::string("/tmp"));
    if (chdir(before) != 0) { /* best-effort restore */ }
}

MTEST(executor_export_and_variable_expansion) {
    Executor ex;
    ex.run_line("export MERIDIAN_TEST_VAR=hello123");
    std::string f = tmp_path("var");
    ex.run_line("echo $MERIDIAN_TEST_VAR > " + f);
    ASSERT_EQ(read_file(f), std::string("hello123\n"));
    std::remove(f.c_str());
    unsetenv("MERIDIAN_TEST_VAR");
}

MTEST(executor_command_substitution_uses_own_engine_not_bash) {
    Executor ex;
    std::string f = tmp_path("cmdsub");
    ex.run_line("echo [$(echo inner)] > " + f);
    ASSERT_EQ(read_file(f), std::string("[inner]\n"));
    std::remove(f.c_str());
}

MTEST(executor_exit_builtin_sets_exit_state) {
    Executor ex;
    ASSERT_FALSE(ex.exit_requested());
    ex.run_line("exit 7");
    ASSERT_TRUE(ex.exit_requested());
    ASSERT_EQ(ex.exit_code(), 7);
}

MTEST(executor_dollar_question_reports_last_status) {
    Executor ex;
    ex.run_line("false");
    std::string f = tmp_path("status");
    ex.run_line("echo $? > " + f);
    ASSERT_EQ(read_file(f), std::string("1\n"));
    std::remove(f.c_str());
}

MTEST(executor_single_quotes_prevent_expansion) {
    Executor ex;
    setenv("MERIDIAN_TEST_VAR2", "should_not_appear", 1);
    std::string f = tmp_path("quote");
    ex.run_line("echo '$MERIDIAN_TEST_VAR2' > " + f);
    ASSERT_EQ(read_file(f), std::string("$MERIDIAN_TEST_VAR2\n"));
    std::remove(f.c_str());
    unsetenv("MERIDIAN_TEST_VAR2");
}
```

---

## `tests/test_job_control.cpp`

```cpp
// tests / test_job_control.cpp
//
// This is the real test for job control: it spawns the actual
// meridian-shell BINARY (not the Executor class directly) on a REAL PTY,
// exactly the way PtyManager would host any shell. It then writes a raw
// Ctrl+Z byte (0x1A) to the PTY master. That byte is never delivered to
// meridian-shell as ordinary input — the kernel's tty line discipline
// intercepts it (ISIG mode) and sends a real SIGTSTP to the terminal's
// foreground process group. This is the same mechanism a real interactive
// session relies on, so if this test passes, process-group assignment and
// tcsetpgrp foreground handoff are both actually working, not just
// plausible-looking code.

#include "mini_test.hpp"
#include "../src/core/pty/pty_manager.hpp"

#include <chrono>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <poll.h>
#include <string>
#include <unistd.h>

using namespace meridian::pty;

namespace {

// master_fd_ is a BLOCKING descriptor, so read() must never be called
// without first confirming (via poll()) that data is actually available —
// otherwise a single call can block indefinitely once the shell has
// nothing more to say. This is what the very first version of this test
// got wrong, and why it hung rather than failed.
std::string read_available(PtyManager& pty, int timeout_ms) {
    std::string out;
    char buf[4096];
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (true) {
        auto now = std::chrono::steady_clock::now();
        if (now >= deadline) break;
        int remaining = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
        struct pollfd pfd{pty.master_fd(), POLLIN, 0};
        int pr = poll(&pfd, 1, remaining);
        if (pr <= 0) break; // timeout or error: return whatever's been read so far
        ssize_t n = pty.read(buf, sizeof(buf));
        if (n <= 0) break; // EOF or error
        out.append(buf, static_cast<std::size_t>(n));
    }
    return out;
}

std::string shell_binary_path() { return "./build/meridian-shell"; }

} // namespace

MTEST(job_control_ctrl_z_stops_foreground_job_via_real_pty) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = shell_binary_path();
    opts.rows = 24;
    opts.cols = 80;
    bool ok = pty.spawn(opts);
    ASSERT_TRUE(ok);
    if (!ok) return;

    // Let the shell start and enable job control.
    read_available(pty, 200);

    pty.write("sleep 100\n");
    read_available(pty, 200); // let sleep actually start and become foreground

    // The real test: a genuine Ctrl+Z byte through the kernel's tty driver.
    pty.write("\x1a");
    std::string after_stop = read_available(pty, 500);
    ASSERT_TRUE(after_stop.find("Stopped") != std::string::npos);

    pty.write("jobs\n");
    std::string jobs_out = read_available(pty, 300);
    ASSERT_TRUE(jobs_out.find("Stopped") != std::string::npos);
    ASSERT_TRUE(jobs_out.find("sleep 100") != std::string::npos);

    // Extract the pgid from the jobs listing ("[1] <pgid>  Stopped ...")
    // so we can clean up the still-alive (stopped) sleep process rather
    // than leaving it around for the rest of the test run.
    pid_t pgid = -1;
    auto bracket = jobs_out.find(']');
    if (bracket != std::string::npos) {
        pgid = static_cast<pid_t>(std::strtol(jobs_out.c_str() + bracket + 1, nullptr, 10));
    }

    pty.write("bg\n");
    std::string bg_out = read_available(pty, 300);
    ASSERT_TRUE(bg_out.find("sleep 100") != std::string::npos);

    pty.write("jobs\n");
    std::string jobs_out2 = read_available(pty, 300);
    ASSERT_TRUE(jobs_out2.find("Running") != std::string::npos);

    // Cleanup: kill the whole stopped/backgrounded process group directly
    // (this test process is the sleep's grandparent, not its parent, but
    // Unix signal delivery only cares about matching uid, not ancestry).
    if (pgid > 0) kill(-pgid, SIGKILL);

    pty.write("exit\n");
    // Bounded wait: if meridian-shell somehow never exits, fail loudly
    // rather than hanging the whole test suite forever.
    read_available(pty, 300);
    int status = pty.wait_for_exit();
    ASSERT_EQ(status, 0);
}

MTEST(job_control_disabled_for_noninteractive_shell) {
    // "-c" mode must NOT attempt job control at all (no controlling tty
    // to claim) — this is the same non-interactive path the executor
    // integration tests already exercise; confirmed here at the binary
    // level so a regression would show up even if only shell_main.cpp
    // were touched.
    PtyManager pty;
    SpawnOptions opts;
    opts.program = shell_binary_path();
    opts.args = {"-c", "echo noninteractive-ok"};
    ASSERT_TRUE(pty.spawn(opts));
    std::string out = read_available(pty, 300);
    ASSERT_TRUE(out.find("noninteractive-ok") != std::string::npos);
    pty.wait_for_exit();
}
```

---

## `tests/test_shell_class.cpp`

```cpp
// tests / test_shell_class.cpp
//
// Tests what Shell::run_interactive actually controls: prompt printing,
// its own error routing, exit-code propagation, and history — NOT
// builtin/program output, which structurally can't be captured into an
// arbitrary ostream (see the doc comment on run_interactive in
// shell.hpp for why). Real command *output* is already verified via
// real file redirection in tests/test_shell_executor.cpp and a real
// PTY in tests/test_job_control.cpp / tests/manual_core_test.sh.

#include "mini_test.hpp"
#include "../src/shell/shell.hpp"

#include <sstream>

using namespace meridian::shell;

MTEST(shell_noninteractive_prints_no_prompt) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("echo hi\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    // Non-interactive mode must not print "meridian:...$ " prompts —
    // scripts piping commands in shouldn't see prompt noise.
    ASSERT_TRUE(out.str().find("meridian:") == std::string::npos);
}

MTEST(shell_interactive_prints_a_prompt_per_line) {
    Shell shell(/*interactive=*/true);
    std::istringstream in("echo a\necho b\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    // One prompt printed before each of the two lines read, plus a
    // trailing prompt before hitting EOF on the third read attempt.
    std::size_t count = 0, pos = 0;
    while ((pos = out.str().find("meridian:", pos)) != std::string::npos) { count++; pos += 1; }
    ASSERT_EQ(count, (std::size_t)3);
}

MTEST(shell_exit_builtin_stops_the_loop_with_its_code) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("echo before\nexit 5\necho after\n");
    std::ostringstream out, err;
    int status = shell.run_interactive(in, out, err);
    ASSERT_EQ(status, 5);
    // History is a real, checkable side effect: the third line must
    // never have been reached once `exit` fired on the second.
    const auto& h = shell.executor().history();
    ASSERT_EQ(h.size(), (std::size_t)2);
    ASSERT_EQ(h[0], std::string("echo before"));
    ASSERT_EQ(h[1], std::string("exit 5"));
}

MTEST(shell_history_accumulates_across_lines) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("echo a\necho b\necho c\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    ASSERT_EQ(shell.executor().history().size(), (std::size_t)3);
}

MTEST(shell_run_command_matches_dash_c_semantics) {
    Shell shell(/*interactive=*/false);
    std::ostringstream err;
    int status = shell.run_command("exit 3", err);
    ASSERT_EQ(status, 3);
}

MTEST(shell_syntax_error_reported_to_err_stream) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("| bad\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    // Lex/parse errors ARE routed through the err ostream directly by
    // Shell itself (unlike builtin output) — this is real, checkable
    // behavior.
    ASSERT_FALSE(err.str().empty());
    ASSERT_TRUE(err.str().find("meridian-shell:") != std::string::npos);
}

MTEST(shell_valid_command_after_syntax_error_still_runs) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("| bad\necho recovered\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    ASSERT_FALSE(err.str().empty()); // the syntax error was reported
    // A syntax error on one line must not stop later lines from running.
    const auto& h = shell.executor().history();
    ASSERT_TRUE(h.size() >= 1);
    ASSERT_EQ(h.back(), std::string("echo recovered"));
}
```

---

## `tests/test_ai.cpp`

```cpp
// tests / test_ai.cpp
#include "mini_test.hpp"
#include "../src/ai/ai_controller.hpp"
#include "../src/ai/command_analyzer.hpp"
#include "../src/ai/risk_classifier.hpp"
#include "../src/ai/secret_redactor.hpp"
#include "../src/core/config.hpp"

#include <cstdio>
#include <unistd.h>

using namespace meridian::ai;

// ---- SecretRedactor ----

MTEST(redactor_finds_key_value_style_secret) {
    SecretRedactor r;
    int count = 0;
    std::string out = r.redact("API_KEY=sk-abc123def456ghi789", &count);
    ASSERT_TRUE(out.find("sk-abc123def456ghi789") == std::string::npos);
    ASSERT_TRUE(out.find("API_KEY=") != std::string::npos); // key name preserved
    ASSERT_TRUE(out.find("[REDACTED]") != std::string::npos);
    ASSERT_TRUE(count >= 1);
}

MTEST(redactor_finds_bearer_token) {
    SecretRedactor r;
    std::string out = r.redact("curl -H \"Authorization: Bearer abc123xyz789\" https://api.example.com");
    ASSERT_TRUE(out.find("abc123xyz789") == std::string::npos);
    ASSERT_TRUE(out.find("Authorization: Bearer [REDACTED]") != std::string::npos);
}

MTEST(redactor_bearer_token_does_not_eat_trailing_quote) {
    // Regression test: the token pattern used to be \S+, which greedily
    // consumed a trailing closing quote as part of the "token" and
    // silently dropped it from the output.
    SecretRedactor r;
    std::string out = r.redact("curl -H \"Authorization: Bearer xyz789token\"");
    ASSERT_TRUE(out.find("[REDACTED]\"") != std::string::npos);
}

MTEST(redactor_finds_aws_access_key) {
    SecretRedactor r;
    std::string out = r.redact("export AWS_KEY_ID_UNUSED_NAME AKIAIOSFODNN7EXAMPLE end");
    ASSERT_TRUE(out.find("AKIAIOSFODNN7EXAMPLE") == std::string::npos);
}

MTEST(redactor_finds_pem_private_key_block) {
    SecretRedactor r;
    std::string input =
        "before\n-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEA...\n-----END RSA PRIVATE KEY-----\nafter";
    std::string out = r.redact(input);
    ASSERT_TRUE(out.find("MIIEpAIBAAKCAQEA") == std::string::npos);
    ASSERT_TRUE(out.find("before") != std::string::npos);
    ASSERT_TRUE(out.find("after") != std::string::npos);
}

MTEST(redactor_leaves_ordinary_text_alone) {
    SecretRedactor r;
    int count = 0;
    std::string input = "ls -la /home/user/projects";
    std::string out = r.redact(input, &count);
    ASSERT_EQ(out, input);
    ASSERT_EQ(count, 0);
}

// ---- RiskClassifier ----

MTEST(risk_classifies_plain_ls_as_low) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("ls -la").level == RiskLevel::Low);
}

MTEST(risk_classifies_sudo_as_high) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("sudo systemctl restart nginx").level == RiskLevel::High);
}

MTEST(risk_classifies_rm_rf_root_as_critical) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("rm -rf /").level == RiskLevel::Critical);
}

MTEST(risk_classifies_rm_rf_subdir_as_high_not_critical) {
    RiskClassifier c;
    auto result = c.classify("rm -rf /home/user/build");
    ASSERT_TRUE(result.level == RiskLevel::High);
}

MTEST(risk_classifies_plain_rm_as_medium) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("rm old_file.txt").level == RiskLevel::Medium);
}

MTEST(risk_classifies_fork_bomb_as_critical) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify(":(){ :|:& };:").level == RiskLevel::Critical);
}

MTEST(risk_classifies_pipe_to_shell_as_high) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("curl https://example.com/install.sh | bash").level == RiskLevel::High);
}

// ---- CommandAnalyzer ----

MTEST(analyzer_suggests_correction_for_transposed_typo) {
    CommandAnalyzer a({"echo", "exit"});
    a.refresh_path_index(); // real $PATH scan
    auto s = a.analyze("gerp");
    ASSERT_TRUE(s.has_value());
    if (s) ASSERT_EQ(s->suggested, std::string("grep"));
}

MTEST(analyzer_makes_no_suggestion_for_known_command) {
    CommandAnalyzer a({"echo"});
    a.refresh_path_index();
    auto s = a.analyze("echo");
    ASSERT_FALSE(s.has_value());
}

MTEST(analyzer_makes_no_suggestion_for_wildly_different_bogus_name) {
    CommandAnalyzer a({"echo"});
    a.refresh_path_index();
    auto s = a.analyze("totally_bogus_command_xyz_987654");
    ASSERT_FALSE(s.has_value());
}

MTEST(analyzer_respects_explicit_path_override) {
    // Use an isolated fake PATH so this test doesn't depend on exactly
    // what's installed on whatever machine runs it.
    std::string dir = "/tmp/meridian_ai_test_bin";
    std::string mkdir_cmd = "mkdir -p " + dir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort */ }
    std::string touch_cmd = "touch " + dir + "/mytool && chmod +x " + dir + "/mytool";
    if (system(touch_cmd.c_str()) != 0) { /* best-effort */ }

    CommandAnalyzer a({});
    a.refresh_path_index(dir.c_str());
    auto s = a.analyze("mytoo"); // one char short of "mytool"
    ASSERT_TRUE(s.has_value());
    if (s) ASSERT_EQ(s->suggested, std::string("mytool"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}

MTEST(edit_distance_basic_cases) {
    ASSERT_EQ(edit_distance("", ""), 0);
    ASSERT_EQ(edit_distance("abc", "abc"), 0);
    ASSERT_EQ(edit_distance("abc", "abd"), 1);
    ASSERT_EQ(edit_distance("gerp", "grep"), 1); // adjacent transposition
    ASSERT_EQ(edit_distance("kitten", "sitting"), 3);
}

// ---- Config ----

MTEST(config_round_trips_values) {
    std::string path = "/tmp/meridian_test_config_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    {
        meridian::Config c(path);
        c.set("ai.provider", "groq");
        c.set_bool("ai.enabled", true);
        ASSERT_TRUE(c.save());
    }
    {
        meridian::Config c(path);
        ASSERT_TRUE(c.load());
        ASSERT_EQ(c.get("ai.provider"), std::string("groq"));
        ASSERT_TRUE(c.get_bool("ai.enabled", false));
    }
    std::remove(path.c_str());
}

MTEST(config_missing_file_is_not_an_error) {
    meridian::Config c("/tmp/meridian_definitely_does_not_exist_12345.toml");
    ASSERT_FALSE(c.load()); // false = "didn't exist", not a crash
    ASSERT_EQ(c.get("anything", "default"), std::string("default"));
}

// ---- AIController integration ----

MTEST(controller_analyze_is_silent_when_ai_disabled) {
    std::string path = "/tmp/meridian_test_ctrl_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    AIController ctrl(path);
    ASSERT_FALSE(ctrl.enabled()); // default off
    std::string report = ctrl.analyze_command("gerp foo");
    ASSERT_EQ(report, std::string(""));
    std::remove(path.c_str());
}

MTEST(controller_analyze_detects_typo_and_risk_when_enabled) {
    std::string path = "/tmp/meridian_test_ctrl2_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    AIController ctrl(path);
    ctrl.set_enabled(true);
    ctrl.set_detection_enabled(true);

    std::string report = ctrl.analyze_command("gerp foo");
    ASSERT_TRUE(report.find("grep") != std::string::npos);

    std::string risky = ctrl.analyze_command("sudo rm -rf /");
    ASSERT_TRUE(risky.find("CRITICAL") != std::string::npos);

    std::remove(path.c_str());
}

MTEST(controller_status_report_reflects_state) {
    std::string path = "/tmp/meridian_test_ctrl3_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    AIController ctrl(path);
    ctrl.set_enabled(true);
    ctrl.set_provider("groq");
    std::string status = ctrl.status_report();
    ASSERT_TRUE(status.find("ON") != std::string::npos);
    ASSERT_TRUE(status.find("groq") != std::string::npos);
    std::remove(path.c_str());
}

MTEST(controller_explain_is_honest_about_no_provider) {
    AIController ctrl("/tmp/meridian_test_ctrl4.toml");
    std::string explanation = ctrl.explain_command("chmod 755 app.sh");
    ASSERT_TRUE(explanation.find("No AI provider") != std::string::npos);
}
```

---

## `tests/test_command_context.cpp`

```cpp
// tests / test_command_context.cpp
#include "mini_test.hpp"
#include "../src/ai/command_context.hpp"

#include <cstdio>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace meridian::ai;

namespace {
std::string sandbox_dir() {
    return "/tmp/meridian_test_gitctx_" + std::to_string(getpid());
}
} // namespace

MTEST(context_builder_fills_cwd_shell_and_os_info) {
    auto ctx = CommandContextBuilder::build("ls -la");
    ASSERT_EQ(ctx.current_command, std::string("ls -la"));
    ASSERT_FALSE(ctx.working_directory.empty());
    ASSERT_FALSE(ctx.shell_name.empty());
    ASSERT_FALSE(ctx.os_info.empty());
}

MTEST(context_builder_carries_last_command_state) {
    auto ctx = CommandContextBuilder::build("npm run dev", std::string("npm install"), 1, std::string("EADDRINUSE"));
    ASSERT_TRUE(ctx.last_command.has_value());
    if (ctx.last_command) ASSERT_EQ(*ctx.last_command, std::string("npm install"));
    ASSERT_TRUE(ctx.last_exit_code.has_value());
    if (ctx.last_exit_code) ASSERT_EQ(*ctx.last_exit_code, 1);
    ASSERT_TRUE(ctx.last_stderr.has_value());
}

MTEST(context_builder_reports_no_git_repo_outside_one) {
    // /tmp itself is very unlikely to be inside a git repo.
    auto ctx = CommandContextBuilder::build("ls", std::nullopt, std::nullopt, std::nullopt, "/tmp");
    ASSERT_FALSE(ctx.in_git_repo);
    ASSERT_FALSE(ctx.git_branch.has_value());
}

MTEST(git_branch_detected_from_real_dot_git_head_file) {
    // Build a minimal real .git/HEAD (just the one file this code
    // reads) rather than requiring an actual `git` binary or repo.
    std::string dir = sandbox_dir();
    std::string git_dir = dir + "/.git";
    std::string mkdir_cmd = "mkdir -p " + git_dir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort setup */ }

    {
        std::ofstream head(git_dir + "/HEAD");
        head << "ref: refs/heads/feature/cool-thing\n";
    }

    auto result = CommandContextBuilder::find_git_branch(dir);
    ASSERT_TRUE(result.first);
    ASSERT_TRUE(result.second.has_value());
    if (result.second) ASSERT_EQ(*result.second, std::string("feature/cool-thing"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}

MTEST(git_branch_detected_from_subdirectory_by_walking_up) {
    std::string dir = sandbox_dir() + "_walkup";
    std::string git_dir = dir + "/.git";
    std::string subdir = dir + "/src/deep/nested";
    std::string mkdir_cmd = "mkdir -p " + git_dir + " && mkdir -p " + subdir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort */ }

    {
        std::ofstream head(git_dir + "/HEAD");
        head << "ref: refs/heads/main\n";
    }

    auto result = CommandContextBuilder::find_git_branch(subdir);
    ASSERT_TRUE(result.first);
    if (result.second) ASSERT_EQ(*result.second, std::string("main"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}

MTEST(git_branch_handles_detached_head_as_raw_hash) {
    std::string dir = sandbox_dir() + "_detached";
    std::string git_dir = dir + "/.git";
    std::string mkdir_cmd = "mkdir -p " + git_dir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort */ }

    {
        std::ofstream head(git_dir + "/HEAD");
        head << "abc123def456\n"; // detached HEAD: raw commit hash, no "ref:" prefix
    }

    auto result = CommandContextBuilder::find_git_branch(dir);
    ASSERT_TRUE(result.first);
    if (result.second) ASSERT_EQ(*result.second, std::string("abc123def456"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}
```

---

## `tests/test_config.cpp`

```cpp
// tests / test_config.cpp
#include "mini_test.hpp"
#include "../src/config/keybindings.hpp"
#include "../src/config/terminal_config.hpp"

#include <cstdio>
#include <unistd.h>

using namespace meridian::config;

MTEST(terminal_config_defaults_without_a_file) {
    TerminalConfig cfg("/tmp/meridian_definitely_missing_terminal_" + std::to_string(getpid()) + ".toml");
    cfg.load(); // file doesn't exist -> defaults apply, no crash
    auto s = cfg.get();
    ASSERT_EQ(s.scrollback_lines, 10000);
    ASSERT_EQ(s.default_shell, std::string("/bin/bash"));
    ASSERT_TRUE(s.cursor_blink);
}

MTEST(terminal_config_round_trips_custom_values) {
    std::string path = "/tmp/meridian_test_terminal_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    {
        TerminalConfig cfg(path);
        auto s = cfg.get();
        s.scrollback_lines = 50000;
        s.default_shell = "/usr/bin/zsh";
        s.font_size = 16;
        s.cursor_blink = false;
        cfg.set(s);
        ASSERT_TRUE(cfg.save());
    }
    {
        TerminalConfig cfg(path);
        cfg.load();
        auto s = cfg.get();
        ASSERT_EQ(s.scrollback_lines, 50000);
        ASSERT_EQ(s.default_shell, std::string("/usr/bin/zsh"));
        ASSERT_EQ(s.font_size, 16);
        ASSERT_FALSE(s.cursor_blink);
    }
    std::remove(path.c_str());
}

MTEST(keybindings_returns_spec_defaults_when_unset) {
    Keybindings kb("/tmp/meridian_definitely_missing_keybindings_" + std::to_string(getpid()) + ".toml");
    ASSERT_EQ(kb.get("ai_popup"), std::string("Ctrl+Space"));
    ASSERT_EQ(kb.get("search"), std::string("Ctrl+Shift+F"));
}

MTEST(keybindings_override_persists) {
    std::string path = "/tmp/meridian_test_keybindings_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    {
        Keybindings kb(path);
        kb.set("ai_popup", "Alt+A");
        ASSERT_TRUE(kb.save());
    }
    {
        Keybindings kb(path);
        kb.load();
        ASSERT_EQ(kb.get("ai_popup"), std::string("Alt+A"));
        // Unrelated actions still fall back to their defaults.
        ASSERT_EQ(kb.get("search"), std::string("Ctrl+Shift+F"));
    }
    std::remove(path.c_str());
}

MTEST(keybindings_all_lists_every_default_action) {
    Keybindings kb("/tmp/meridian_definitely_missing_kb2_" + std::to_string(getpid()) + ".toml");
    auto all = kb.all();
    ASSERT_TRUE(all.count("ai_popup") == 1);
    ASSERT_TRUE(all.count("new_tab") == 1);
    ASSERT_TRUE(all.size() >= 10);
}
```

---

## `tests/test_security.cpp`

```cpp
// tests / test_security.cpp
#include "mini_test.hpp"
#include "../src/security/credential_store.hpp"

#include <sys/stat.h>
#include <unistd.h>

using namespace meridian::security;

namespace {
std::string test_dir() {
    return "/tmp/meridian_test_credentials_" + std::to_string(getpid());
}
} // namespace

MTEST(credential_store_round_trips_a_secret) {
    FileCredentialStore store(test_dir());
    ASSERT_TRUE(store.store("groq", "sk-test-secret-value-123"));
    auto retrieved = store.retrieve("groq");
    ASSERT_TRUE(retrieved.has_value());
    if (retrieved) ASSERT_EQ(*retrieved, std::string("sk-test-secret-value-123"));
    store.remove("groq");
}

MTEST(credential_store_missing_key_returns_nullopt) {
    FileCredentialStore store(test_dir());
    auto retrieved = store.retrieve("does_not_exist_at_all");
    ASSERT_FALSE(retrieved.has_value());
}

MTEST(credential_store_file_has_owner_only_permissions) {
    // This is the actual point of the store: verify the REAL file mode
    // on disk is 0600, not just that round-tripping the value works.
    FileCredentialStore store(test_dir());
    store.store("perm_check", "secret");

    std::string path = test_dir() + "/perm_check";
    struct stat st{};
    ASSERT_EQ(stat(path.c_str(), &st), 0);
    mode_t perm_bits = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    ASSERT_EQ(perm_bits, (mode_t)(S_IRUSR | S_IWUSR));

    store.remove("perm_check");
}

MTEST(credential_store_remove_is_idempotent) {
    FileCredentialStore store(test_dir());
    store.store("temp", "value");
    ASSERT_TRUE(store.remove("temp"));
    ASSERT_TRUE(store.remove("temp")); // already gone -> still reports success
    ASSERT_FALSE(store.retrieve("temp").has_value());
}

MTEST(credential_store_sanitizes_path_traversal_attempts) {
    FileCredentialStore store(test_dir());
    // A key_name that looks like a path-traversal attempt must not
    // escape the credentials directory.
    ASSERT_TRUE(store.store("../../etc/passwd_like_name", "value"));
    auto retrieved = store.retrieve("../../etc/passwd_like_name");
    ASSERT_TRUE(retrieved.has_value());
    // The file actually written should be inside test_dir(), sanitized.
    std::string expected_path = test_dir() + "/.._.._etc_passwd_like_name";
    struct stat st{};
    ASSERT_EQ(stat(expected_path.c_str(), &st), 0);
    store.remove("../../etc/passwd_like_name");
}
```

---

## `tests/manual_core_test.sh`

```bash
#!/usr/bin/env bash
# tests/manual_core_test.sh
#
# The exact manual test sequence from the spec (section 78), run against
# the real meridian-shell binary and actually checked against expected
# output rather than just eyeballed — each command below produces a
# PASS/FAIL line, and the script's own exit code reflects whether
# everything passed. Generated files are cleaned up at the end
# regardless of outcome.
#
# Usage: ./tests/manual_core_test.sh   (run from the repo root, after
#                                        `make shell` or `make all`)

set -u
SHELL_BIN="./build/meridian-shell"
FAILURES=0
CHECKS=0

pass() { CHECKS=$((CHECKS+1)); echo "  [ OK ]  $1"; }
fail() { CHECKS=$((CHECKS+1)); FAILURES=$((FAILURES+1)); echo "  [FAIL]  $1"; }

check_contains() {
    local desc="$1" haystack="$2" needle="$3"
    if [[ "$haystack" == *"$needle"* ]]; then pass "$desc"; else
        fail "$desc (expected to find: '$needle', got: '$haystack')"
    fi
}

if [ ! -x "$SHELL_BIN" ]; then
    echo "meridian-shell not built — run 'make shell' or 'make all' first."
    exit 1
fi

echo "=== Meridian Shell manual core test ==="
echo "(spec section 78 — the exact listed command sequence)"
echo

echo "--- pwd ---"
out=$("$SHELL_BIN" -c "pwd")
check_contains "pwd prints a real absolute path" "$out" "/"

echo "--- echo hello ---"
out=$("$SHELL_BIN" -c "echo hello")
check_contains "echo hello" "$out" "hello"

echo "--- echo \"\$HOME\" ---"
out=$("$SHELL_BIN" -c 'echo "$HOME"')
check_contains "echo \$HOME expands" "$out" "$HOME"

echo "--- export TEST_VAR=hello / echo \"\$TEST_VAR\" ---"
out=$("$SHELL_BIN" -c 'export TEST_VAR=hello; echo "$TEST_VAR"')
check_contains "export + expansion" "$out" "hello"

echo "--- printf \"hello\\n\" ---"
out=$("$SHELL_BIN" -c 'printf "hello\n"')
check_contains "printf builtin passthrough to real printf(1)" "$out" "hello"

echo "--- echo hello | grep hello ---"
out=$("$SHELL_BIN" -c "echo hello | grep hello")
check_contains "real pipe through real grep(1)" "$out" "hello"

echo "--- echo hello > test_output.txt / cat test_output.txt ---"
"$SHELL_BIN" -c "echo hello > test_output.txt" >/dev/null
out=$(cat test_output.txt 2>/dev/null)
check_contains "redirection creates file with content" "$out" "hello"

echo "--- echo world >> test_output.txt / cat test_output.txt ---"
"$SHELL_BIN" -c "echo world >> test_output.txt" >/dev/null
out=$(cat test_output.txt 2>/dev/null)
check_contains "append keeps prior content" "$out" "hello"
check_contains "append adds new content" "$out" "world"

echo "--- sleep 1 & / jobs ---"
out=$("$SHELL_BIN" -c 'sleep 1 & jobs')
check_contains "background job reported by jobs" "$out" "sleep 1"

echo
echo "=== cleaning up generated files ==="
rm -f test_output.txt
if [ ! -e test_output.txt ]; then pass "test_output.txt removed"; else fail "test_output.txt still present"; fi

echo
echo "$CHECKS checks, $FAILURES failed"
exit $([ "$FAILURES" -eq 0 ] && echo 0 || echo 1)
```

---

