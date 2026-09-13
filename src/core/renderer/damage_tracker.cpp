#include "damage_tracker.hpp"

namespace meridian::renderer {

void DamageRect::merge(const DamageRect& other) {
    if (other.empty()) return;
    if (empty()) {
        *this = other;
        return;
    }
    int right = std::max(x + width, other.x + other.width);
    int bottom = std::max(y + height, other.y + other.height);
    x = std::min(x, other.x);
    y = std::min(y, other.y);
    width = right - x;
    height = bottom - y;
}

DamageTracker::DamageTracker(int rows, int cols) {
    resize(rows, cols);
}

void DamageTracker::resize(int rows, int cols) {
    rows_ = std::max(1, rows);
    cols_ = std::max(1, cols);
    dirty_rows_.assign(rows_, false);
    mark_all_dirty();
}

void DamageTracker::mark_dirty(int row, int col, int width) {
    if (row < 0 || row >= rows_ || col < 0 || col >= cols_ || width <= 0) return;
    int end_col = std::min(cols_ - 1, col + width - 1);

    if (!has_damage_) {
        min_x_ = col;
        max_x_ = end_col;
        min_y_ = row;
        max_y_ = row;
        has_damage_ = true;
    } else {
        min_x_ = std::min(min_x_, col);
        max_x_ = std::max(max_x_, end_col);
        min_y_ = std::min(min_y_, row);
        max_y_ = std::max(max_y_, row);
    }
    dirty_rows_[row] = true;
}

void DamageTracker::mark_row_dirty(int row) {
    mark_dirty(row, 0, cols_);
}

void DamageTracker::mark_all_dirty() {
    has_damage_ = true;
    min_x_ = 0;
    min_y_ = 0;
    max_x_ = cols_ - 1;
    max_y_ = rows_ - 1;
    std::fill(dirty_rows_.begin(), dirty_rows_.end(), true);
}

void DamageTracker::mark_scroll(int delta_lines) {
    if (delta_lines == 0) return;
    mark_all_dirty();
}

bool DamageTracker::is_row_dirty(int row) const {
    if (row < 0 || row >= rows_) return false;
    return dirty_rows_[row];
}

DamageRect DamageTracker::current_damage() const {
    if (!has_damage_) return DamageRect{0, 0, 0, 0};
    return DamageRect{min_x_, min_y_, max_x_ - min_x_ + 1, max_y_ - min_y_ + 1};
}

DamageRect DamageTracker::consume_damage() {
    DamageRect rect = current_damage();
    has_damage_ = false;
    min_x_ = 0;
    min_y_ = 0;
    max_x_ = 0;
    max_y_ = 0;
    std::fill(dirty_rows_.begin(), dirty_rows_.end(), false);
    return rect;
}

} // namespace meridian::renderer

