#pragma once
// meridian-terminal / core / renderer / render_pipeline.hpp
//
// GPU vertex generation and batching engine. Transforms ScreenBuffer cells
// into GPU vertex arrays with TrueColor resolution, damage filtering,
// selection highlights, cursor animations, and quad merging.

#include "../vt/screen_buffer.hpp"
#include "damage_tracker.hpp"
#include "glyph_atlas.hpp"

#include <cstdint>
#include <vector>

namespace meridian::renderer {

struct RgbaColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    bool operator==(const RgbaColor& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
};

struct RenderVertex {
    float x = 0.0f;
    float y = 0.0f;
    float u = 0.0f;
    float v = 0.0f;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};

enum class CursorShape {
    Block,
    Beam,
    Underline
};

struct SelectionRange {
    int start_row = -1;
    int start_col = -1;
    int end_row = -1;
    int end_col = -1;
    bool active = false;

    bool contains(int row, int col) const;
};

struct RenderBatch {
    std::vector<RenderVertex> background_vertices; // Quads for cell backgrounds
    std::vector<RenderVertex> glyph_vertices;      // Quads for glyph textures
    std::vector<RenderVertex> decoration_vertices; // Underlines, strikethroughs, selection
    std::vector<RenderVertex> cursor_vertices;     // Animated cursor
    std::size_t total_quads() const {
        return (background_vertices.size() + glyph_vertices.size() +
                decoration_vertices.size() + cursor_vertices.size()) / 6;
    }
};

class RenderPipeline {
public:
    RenderPipeline(int cell_width = 10, int cell_height = 20);

    void set_cell_size(int width, int height);
    int cell_width() const { return cell_width_; }
    int cell_height() const { return cell_height_; }

    void set_default_colors(RgbaColor default_bg, RgbaColor default_fg);
    RgbaColor default_bg() const { return default_bg_; }
    RgbaColor default_fg() const { return default_fg_; }

    // Resolve vt::Color to 32-bit RGBA
    RgbaColor resolve_color(const vt::Color& color, bool is_fg, bool bold = false) const;

    // Generate vertex batches for rendering
    RenderBatch generate_batch(
        const vt::ScreenBuffer& screen,
        GlyphAtlas& atlas,
        const DamageTracker* damage = nullptr,
        const SelectionRange* selection = nullptr,
        CursorShape cursor_shape = CursorShape::Block,
        bool cursor_visible = true,
        float cursor_blink_opacity = 1.0f
    );

private:
    int cell_width_;
    int cell_height_;
    RgbaColor default_bg_{15, 23, 42, 255};    // Slate 900
    RgbaColor default_fg_{248, 250, 252, 255}; // Slate 50

    static void append_quad(
        std::vector<RenderVertex>& verts,
        float x0, float y0, float x1, float y1,
        float u0, float v0, float u1, float v1,
        RgbaColor color
    );
};

} // namespace meridian::renderer

