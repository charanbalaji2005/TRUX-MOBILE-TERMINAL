#include "render_pipeline.hpp"

#include <cmath>

namespace meridian::renderer {

namespace {

// Standard ANSI 16 color table
const RgbaColor ANSI_PALETTE[16] = {
    {0, 0, 0, 255},       // 0: Black
    {220, 38, 38, 255},   // 1: Red
    {22, 163, 74, 255},   // 2: Green
    {202, 138, 4, 255},   // 3: Yellow
    {37, 99, 235, 255},   // 4: Blue
    {147, 51, 234, 255},  // 5: Magenta
    {13, 148, 136, 255},  // 6: Cyan
    {203, 213, 225, 255}, // 7: White
    {100, 116, 139, 255}, // 8: Bright Black (Gray)
    {239, 68, 68, 255},   // 9: Bright Red
    {34, 197, 94, 255},   // 10: Bright Green
    {234, 179, 8, 255},   // 11: Bright Yellow
    {59, 130, 246, 255},  // 12: Bright Blue
    {168, 85, 247, 255},  // 13: Bright Magenta
    {20, 184, 166, 255},  // 14: Bright Cyan
    {248, 250, 252, 255}  // 15: Bright White
};

RgbaColor resolve_256_color(uint8_t index) {
    if (index < 16) {
        return ANSI_PALETTE[index];
    }
    if (index >= 16 && index <= 231) {
        // 6x6x6 color cube
        uint8_t idx = index - 16;
        uint8_t b = idx % 6;
        uint8_t g = (idx / 6) % 6;
        uint8_t r = (idx / 36) % 6;
        auto to_val = [](uint8_t v) -> uint8_t { return v ? (v * 40 + 55) : 0; };
        return RgbaColor{to_val(r), to_val(g), to_val(b), 255};
    }
    // 232-255: grayscale ramp
    uint8_t gray = (index - 232) * 10 + 8;
    return RgbaColor{gray, gray, gray, 255};
}

} // namespace

bool SelectionRange::contains(int row, int col) const {
    if (!active) return false;
    if (row < start_row || row > end_row) return false;
    if (start_row == end_row) {
        return col >= start_col && col <= end_col;
    }
    if (row == start_row) return col >= start_col;
    if (row == end_row) return col <= end_col;
    return true; // intermediate rows
}

RenderPipeline::RenderPipeline(int cell_width, int cell_height)
    : cell_width_(std::max(4, cell_width)),
      cell_height_(std::max(8, cell_height)) {}

void RenderPipeline::set_cell_size(int width, int height) {
    cell_width_ = std::max(4, width);
    cell_height_ = std::max(8, height);
}

void RenderPipeline::set_default_colors(RgbaColor default_bg, RgbaColor default_fg) {
    default_bg_ = default_bg;
    default_fg_ = default_fg;
}

RgbaColor RenderPipeline::resolve_color(const vt::Color& color, bool is_fg, bool bold) const {
    switch (color.kind) {
        case vt::Color::Kind::Default:
            return is_fg ? default_fg_ : default_bg_;
        case vt::Color::Kind::Indexed: {
            uint8_t idx = color.index;
            if (is_fg && bold && idx < 8) idx += 8; // bold promotes to bright
            return resolve_256_color(idx);
        }
        case vt::Color::Kind::Rgb:
            return RgbaColor{color.r, color.g, color.b, 255};
    }
    return is_fg ? default_fg_ : default_bg_;
}

void RenderPipeline::append_quad(
    std::vector<RenderVertex>& verts,
    float x0, float y0, float x1, float y1,
    float u0, float v0, float u1, float v1,
    RgbaColor color
) {
    // 2 triangles = 6 vertices (CCW order)
    verts.push_back({x0, y0, u0, v0, color.r, color.g, color.b, color.a});
    verts.push_back({x1, y0, u1, v0, color.r, color.g, color.b, color.a});
    verts.push_back({x1, y1, u1, v1, color.r, color.g, color.b, color.a});

    verts.push_back({x0, y0, u0, v0, color.r, color.g, color.b, color.a});
    verts.push_back({x1, y1, u1, v1, color.r, color.g, color.b, color.a});
    verts.push_back({x0, y1, u0, v1, color.r, color.g, color.b, color.a});
}

RenderBatch RenderPipeline::generate_batch(
    const vt::ScreenBuffer& screen,
    GlyphAtlas& atlas,
    const DamageTracker* damage,
    const SelectionRange* selection,
    CursorShape cursor_shape,
    bool cursor_visible,
    float cursor_blink_opacity
) {
    RenderBatch batch;
    int rows = screen.rows();
    int cols = screen.cols();

    for (int r = 0; r < rows; ++r) {
        if (damage && !damage->is_row_dirty(r)) continue;

        for (int c = 0; c < cols; ++c) {
            const auto& cell = screen.cell_at(r, c);
            if (cell.width == 0) continue; // skip continuation cells

            float x0 = static_cast<float>(c * cell_width_);
            float y0 = static_cast<float>(r * cell_height_);
            float x1 = x0 + static_cast<float>(cell.width * cell_width_);
            float y1 = y0 + static_cast<float>(cell_height_);

            // Resolve background & foreground
            RgbaColor bg = resolve_color(cell.attrs.bg, /*is_fg=*/false);
            RgbaColor fg = resolve_color(cell.attrs.fg, /*is_fg=*/true, cell.attrs.bold);

            if (cell.attrs.reverse) {
                std::swap(bg, fg);
            }

            // Selection override
            bool is_selected = selection && selection->contains(r, c);
            if (is_selected) {
                bg = RgbaColor{59, 130, 246, 180}; // selection blue tint
                fg = RgbaColor{255, 255, 255, 255};
            }

            // 1. Background quad (only if non-default transparent or selected)
            if (bg != default_bg_ || is_selected) {
                append_quad(batch.background_vertices, x0, y0, x1, y1, 0, 0, 0, 0, bg);
            }

            // 2. Glyph quad
            if (cell.codepoint != U' ' && cell.codepoint != 0) {
                FontStyle style = FontStyle::Regular;
                if (cell.attrs.bold && cell.attrs.italic) style = FontStyle::BoldItalic;
                else if (cell.attrs.bold) style = FontStyle::Bold;
                else if (cell.attrs.italic) style = FontStyle::Italic;

                const auto& entry = atlas.get_or_create(cell.codepoint, style, cell_width_, cell_height_);
                RgbaColor glyph_color = entry.is_color_emoji ? RgbaColor{255, 255, 255, 255} : fg;
                if (cell.attrs.dim) {
                    glyph_color.a = 140;
                }

                append_quad(batch.glyph_vertices, x0, y0, x1, y1, entry.u0, entry.v0, entry.u1, entry.v1, glyph_color);
            }

            // 3. Decorations (underline, strikethrough)
            if (cell.attrs.underline) {
                float ul_y0 = y1 - 2.0f;
                float ul_y1 = y1;
                append_quad(batch.decoration_vertices, x0, ul_y0, x1, ul_y1, 0, 0, 0, 0, fg);
            }
            if (cell.attrs.strikethrough) {
                float st_y0 = y0 + cell_height_ * 0.5f - 1.0f;
                float st_y1 = st_y0 + 2.0f;
                append_quad(batch.decoration_vertices, x0, st_y0, x1, st_y1, 0, 0, 0, 0, fg);
            }
        }
    }

    // 4. Cursor
    if (cursor_visible && cursor_blink_opacity > 0.05f) {
        int cr = screen.cursor_row();
        int cc = screen.cursor_col();
        if (cr >= 0 && cr < rows && cc >= 0 && cc < cols) {
            float cx0 = static_cast<float>(cc * cell_width_);
            float cy0 = static_cast<float>(cr * cell_height_);
            float cx1 = cx0 + static_cast<float>(cell_width_);
            float cy1 = cy0 + static_cast<float>(cell_height_);

            uint8_t alpha = static_cast<uint8_t>(255.0f * cursor_blink_opacity);
            RgbaColor cursor_col{56, 189, 248, alpha}; // Sky blue accent

            switch (cursor_shape) {
                case CursorShape::Block:
                    append_quad(batch.cursor_vertices, cx0, cy0, cx1, cy1, 0, 0, 0, 0, cursor_col);
                    break;
                case CursorShape::Beam:
                    append_quad(batch.cursor_vertices, cx0, cy0, cx0 + 2.5f, cy1, 0, 0, 0, 0, cursor_col);
                    break;
                case CursorShape::Underline:
                    append_quad(batch.cursor_vertices, cx0, cy1 - 3.0f, cx1, cy1, 0, 0, 0, 0, cursor_col);
                    break;
            }
        }
    }

    return batch;
}

} // namespace meridian::renderer

