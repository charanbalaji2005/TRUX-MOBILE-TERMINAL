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
