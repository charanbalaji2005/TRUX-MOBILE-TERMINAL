#pragma once
// meridian-terminal / core / renderer / damage_tracker.hpp
//
// High-performance damage tracking for 120/144/240 Hz terminal rendering.
// Computes minimal dirty bounding boxes and row bitmasks to avoid full-screen
// GPU redraws on every incoming byte stream or cursor tick.

#include <algorithm>
#include <cstdint>
#include <vector>

namespace meridian::renderer {

struct DamageRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    bool empty() const { return width <= 0 || height <= 0; }
    void merge(const DamageRect& other);
};

class DamageTracker {
public:
    DamageTracker(int rows = 24, int cols = 80);

    void resize(int rows, int cols);
    int rows() const { return rows_; }
    int cols() const { return cols_; }

    void mark_dirty(int row, int col, int width = 1);
    void mark_row_dirty(int row);
    void mark_all_dirty();
    void mark_scroll(int delta_lines);

    bool is_dirty() const { return has_damage_; }
    bool is_row_dirty(int row) const;

    // Returns accumulated damage rectangle and clears dirty flags
    DamageRect consume_damage();

    // Inspect current damage without clearing
    DamageRect current_damage() const;

private:
    int rows_;
    int cols_;
    bool has_damage_ = false;
    int min_x_ = 0;
    int min_y_ = 0;
    int max_x_ = 0;
    int max_y_ = 0;
    std::vector<bool> dirty_rows_;
};

} // namespace meridian::renderer

