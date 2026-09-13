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
    uint32_t hyperlink_id = 0;

    bool operator==(const Attributes& o) const {
        return fg == o.fg && bg == o.bg && bold == o.bold && dim == o.dim &&
               italic == o.italic && underline == o.underline && blink == o.blink &&
               reverse == o.reverse && hidden == o.hidden && strikethrough == o.strikethrough &&
               hyperlink_id == o.hyperlink_id;
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
