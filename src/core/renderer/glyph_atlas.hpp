#pragma once
// meridian-terminal / core / renderer / glyph_atlas.hpp
//
// Texture atlas and glyph metrics manager for GPU terminal rendering.
// Organizes rasterized glyphs into texture sheets, handles Nerd Font /
// Powerline icons, Box Drawing characters, and coding ligatures.

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace meridian::renderer {

enum class FontStyle : uint8_t {
    Regular,
    Bold,
    Italic,
    BoldItalic
};

struct GlyphKey {
    char32_t codepoint;
    FontStyle style;

    bool operator==(const GlyphKey& o) const {
        return codepoint == o.codepoint && style == o.style;
    }
};

struct GlyphKeyHash {
    std::size_t operator()(const GlyphKey& k) const {
        return std::hash<uint32_t>()(static_cast<uint32_t>(k.codepoint)) ^
               (std::hash<uint8_t>()(static_cast<uint8_t>(k.style)) << 16);
    }
};

struct AtlasEntry {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    int pixel_width = 0;
    int pixel_height = 0;
    int bearing_x = 0;
    int bearing_y = 0;
    int advance = 0;
    bool is_color_emoji = false;
};

class GlyphAtlas {
public:
    explicit GlyphAtlas(int atlas_width = 1024, int atlas_height = 1024);

    int width() const { return atlas_width_; }
    int height() const { return atlas_height_; }
    std::size_t cached_glyph_count() const { return entries_.size(); }

    // Query or allocate glyph texture coordinates
    const AtlasEntry& get_or_create(char32_t codepoint, FontStyle style, int cell_w, int cell_h);

    // Ligature check
    static bool is_ligature_start(char32_t cp);
    static std::string detect_ligature(const std::u32string& text, std::size_t pos);

    // Box drawing & symbols
    static bool is_box_drawing(char32_t cp);
    static bool is_powerline_symbol(char32_t cp);
    static bool is_emoji(char32_t cp);

    void clear();

private:
    int atlas_width_;
    int atlas_height_;
    int current_shelf_x_ = 0;
    int current_shelf_y_ = 0;
    int shelf_height_ = 0;

    std::unordered_map<GlyphKey, AtlasEntry, GlyphKeyHash> entries_;
    AtlasEntry allocate_entry(char32_t cp, FontStyle style, int cell_w, int cell_h);
};

} // namespace meridian::renderer

