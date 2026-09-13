#include "glyph_atlas.hpp"

#include <algorithm>

namespace meridian::renderer {

GlyphAtlas::GlyphAtlas(int atlas_width, int atlas_height)
    : atlas_width_(std::max(256, atlas_width)),
      atlas_height_(std::max(256, atlas_height)) {}

void GlyphAtlas::clear() {
    entries_.clear();
    current_shelf_x_ = 0;
    current_shelf_y_ = 0;
    shelf_height_ = 0;
}

bool GlyphAtlas::is_box_drawing(char32_t cp) {
    return (cp >= 0x2500 && cp <= 0x257F) || // Box Drawing
           (cp >= 0x2580 && cp <= 0x259F);   // Block Elements
}

bool GlyphAtlas::is_powerline_symbol(char32_t cp) {
    return (cp >= 0xE0A0 && cp <= 0xE0D4);   // Powerline & Extra Symbols
}

bool GlyphAtlas::is_emoji(char32_t cp) {
    return (cp >= 0x1F300 && cp <= 0x1F9FF) || // Emoji & Symbols
           (cp >= 0x2600 && cp <= 0x27BF);     // Miscellaneous Symbols
}

bool GlyphAtlas::is_ligature_start(char32_t cp) {
    return cp == U'=' || cp == U'-' || cp == U'!' || cp == U'<' || cp == U'>' || cp == U':';
}

std::string GlyphAtlas::detect_ligature(const std::u32string& text, std::size_t pos) {
    if (pos >= text.size()) return "";
    
    // Check 3-char ligatures
    if (pos + 2 < text.size()) {
        if (text[pos] == U'=' && text[pos+1] == U'=' && text[pos+2] == U'=') return "===";
        if (text[pos] == U'!' && text[pos+1] == U'=' && text[pos+2] == U'=') return "!==";
        if (text[pos] == U'<' && text[pos+1] == U'=' && text[pos+2] == U'=') return "<==";
        if (text[pos] == U'=' && text[pos+1] == U'=' && text[pos+2] == U'>') return "==>";
    }
    
    // Check 2-char ligatures
    if (pos + 1 < text.size()) {
        if (text[pos] == U'=' && text[pos+1] == U'=') return "==";
        if (text[pos] == U'!' && text[pos+1] == U'=') return "!=";
        if (text[pos] == U'-' && text[pos+1] == U'>') return "->";
        if (text[pos] == U'=' && text[pos+1] == U'>') return "=>";
        if (text[pos] == U'<' && text[pos+1] == U'=') return "<=";
        if (text[pos] == U'>' && text[pos+1] == U'=') return ">=";
        if (text[pos] == U':' && text[pos+1] == U':') return "::";
        if (text[pos] == U'|' && text[pos+1] == U'>') return "|>";
    }
    
    return "";
}

AtlasEntry GlyphAtlas::allocate_entry(char32_t cp, FontStyle /*style*/, int cell_w, int cell_h) {
    AtlasEntry entry;
    int glyph_w = cell_w;
    int glyph_h = cell_h;

    if (is_emoji(cp)) {
        glyph_w = cell_w * 2;
        entry.is_color_emoji = true;
    }

    // Shelf packing
    if (current_shelf_x_ + glyph_w > atlas_width_) {
        // Move to next shelf
        current_shelf_x_ = 0;
        current_shelf_y_ += shelf_height_;
        shelf_height_ = 0;
    }

    if (current_shelf_y_ + glyph_h > atlas_height_) {
        // Atlas full - wrap around / clear for now
        clear();
    }

    entry.pixel_width = glyph_w;
    entry.pixel_height = glyph_h;
    entry.bearing_x = 0;
    entry.bearing_y = glyph_h;
    entry.advance = glyph_w;

    entry.u0 = static_cast<float>(current_shelf_x_) / static_cast<float>(atlas_width_);
    entry.v0 = static_cast<float>(current_shelf_y_) / static_cast<float>(atlas_height_);
    entry.u1 = static_cast<float>(current_shelf_x_ + glyph_w) / static_cast<float>(atlas_width_);
    entry.v1 = static_cast<float>(current_shelf_y_ + glyph_h) / static_cast<float>(atlas_height_);

    current_shelf_x_ += glyph_w + 1; // 1px padding
    shelf_height_ = std::max(shelf_height_, glyph_h + 1);

    return entry;
}

const AtlasEntry& GlyphAtlas::get_or_create(char32_t codepoint, FontStyle style, int cell_w, int cell_h) {
    GlyphKey key{codepoint, style};
    auto it = entries_.find(key);
    if (it != entries_.end()) {
        return it->second;
    }

    AtlasEntry entry = allocate_entry(codepoint, style, cell_w, cell_h);
    auto [inserted_it, _] = entries_.emplace(key, entry);
    return inserted_it->second;
}

} // namespace meridian::renderer

