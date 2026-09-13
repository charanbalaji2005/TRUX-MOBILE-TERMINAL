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

uint32_t ScreenBuffer::register_hyperlink(const std::string& uri, const std::string& /*id*/) {
    if (uri.empty()) return 0;
    for (std::size_t i = 1; i < hyperlink_table_.size(); ++i) {
        if (hyperlink_table_[i] == uri) return static_cast<uint32_t>(i);
    }
    hyperlink_table_.push_back(uri);
    return static_cast<uint32_t>(hyperlink_table_.size() - 1);
}

std::string ScreenBuffer::get_hyperlink(uint32_t id) const {
    if (id == 0 || id >= hyperlink_table_.size()) return "";
    return hyperlink_table_[id];
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
