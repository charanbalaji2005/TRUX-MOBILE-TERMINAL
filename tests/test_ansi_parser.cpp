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
