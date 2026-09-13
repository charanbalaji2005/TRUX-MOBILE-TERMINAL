// src/core/graphics/terminal_image_compositor.cpp
#include "terminal_image_compositor.hpp"
#include <algorithm>
#include <cmath>

namespace meridian::graphics {

TerminalImageCompositor::TerminalImageCompositor() = default;

void TerminalImageCompositor::set_metrics(const TerminalCellMetrics& metrics) {
    metrics_ = metrics;
}

void TerminalImageCompositor::handle_resize(int new_cols, int new_rows, float new_cell_w, float new_cell_h) {
    if (new_cols > 0) metrics_.columns = new_cols;
    if (new_rows > 0) metrics_.rows = new_rows;
    if (new_cell_w > 0.0f) metrics_.cell_width_px = new_cell_w;
    if (new_cell_h > 0.0f) metrics_.cell_height_px = new_cell_h;
}

void TerminalImageCompositor::on_terminal_scroll(int lines_scrolled, std::vector<ImageObject>& images) {
    if (lines_scrolled == 0) return;

    for (auto& img : images) {
        if (!img.scroll_with_terminal) continue;

        if (img.placement == ImagePlacementType::CursorRelative || img.placement == ImagePlacementType::ScrollbackRow) {
            img.scrollback_row -= lines_scrolled;
            // Update pixel Y according to cell height
            img.y -= lines_scrolled * metrics_.cell_height_px;
        }
    }
}

void TerminalImageCompositor::compute_aspect_fit(
    int src_w, int src_h,
    float target_w, float target_h,
    ImageFitMode mode,
    float& out_w, float& out_h,
    float& out_u0, float& out_v0,
    float& out_u1, float& out_v1
) {
    out_u0 = 0.0f; out_v0 = 0.0f;
    out_u1 = 1.0f; out_v1 = 1.0f;

    if (src_w <= 0 || src_h <= 0 || target_w <= 0.0f || target_h <= 0.0f) {
        out_w = target_w;
        out_h = target_h;
        return;
    }

    float src_aspect = static_cast<float>(src_w) / src_h;
    float target_aspect = target_w / target_h;

    switch (mode) {
        case ImageFitMode::Stretch:
            out_w = target_w;
            out_h = target_h;
            break;

        case ImageFitMode::Cover:
            out_w = target_w;
            out_h = target_h;
            if (src_aspect > target_aspect) {
                // Image wider than target box -> crop horizontal sides
                float visible_width = target_aspect / src_aspect;
                out_u0 = (1.0f - visible_width) * 0.5f;
                out_u1 = 1.0f - out_u0;
            } else {
                // Image taller than target box -> crop vertical top/bottom
                float visible_height = src_aspect / target_aspect;
                out_v0 = (1.0f - visible_height) * 0.5f;
                out_v1 = 1.0f - out_v0;
            }
            break;

        case ImageFitMode::Contain:
        default:
            if (src_aspect > target_aspect) {
                // Width limited
                out_w = target_w;
                out_h = target_w / src_aspect;
            } else {
                // Height limited
                out_h = target_h;
                out_w = target_h * src_aspect;
            }
            break;
    }
}

CompositorBounds TerminalImageCompositor::compute_render_bounds(
    const ImageObject& img,
    int current_scroll_offset
) const {
    CompositorBounds bounds;
    bounds.visible = img.visible;
    if (!bounds.visible) return bounds;

    float canvas_w = metrics_.columns * metrics_.cell_width_px;
    float canvas_h = metrics_.rows * metrics_.cell_height_px;

    float base_x = 0.0f;
    float base_y = 0.0f;
    float target_box_w = 0.0f;
    float target_box_h = 0.0f;

    if (img.placement == ImagePlacementType::AbsolutePixels) {
        base_x = img.x;
        base_y = img.y;
    } else {
        // Cursor or scrollback anchored
        base_x = img.cursor_col * metrics_.cell_width_px;
        base_y = (img.cursor_row - current_scroll_offset) * metrics_.cell_height_px;
    }

    // Determine target width
    if (img.width_percentage > 0.0f) {
        target_box_w = canvas_w * (img.width_percentage / 100.0f);
    } else if (img.display_width > 0.0f) {
        target_box_w = img.display_width;
    } else if (img.cols_spanned > 0) {
        target_box_w = img.cols_spanned * metrics_.cell_width_px;
    } else if (img.original_width > 0) {
        target_box_w = std::min(canvas_w, static_cast<float>(img.original_width));
    } else {
        target_box_w = canvas_w * 0.8f;
    }

    // Determine target height
    if (img.height_percentage > 0.0f) {
        target_box_h = canvas_h * (img.height_percentage / 100.0f);
    } else if (img.display_height > 0.0f) {
        target_box_h = img.display_height;
    } else if (img.rows_spanned > 0) {
        target_box_h = img.rows_spanned * metrics_.cell_height_px;
    } else if (img.original_height > 0 && img.original_width > 0) {
        float aspect = static_cast<float>(img.original_width) / img.original_height;
        target_box_h = target_box_w / aspect;
    } else {
        target_box_h = canvas_h * 0.5f;
    }

    float rendered_w = target_box_w;
    float rendered_h = target_box_h;
    compute_aspect_fit(
        img.original_width, img.original_height,
        target_box_w, target_box_h,
        img.fit_mode,
        rendered_w, rendered_h,
        bounds.source_u0, bounds.source_v0,
        bounds.source_u1, bounds.source_v1
    );

    bounds.x = base_x;
    bounds.y = base_y;
    bounds.width = rendered_w;
    bounds.height = rendered_h;

    // Check if fully clipped outside canvas
    if (bounds.x + bounds.width < 0.0f || bounds.x > canvas_w ||
        bounds.y + bounds.height < 0.0f || bounds.y > canvas_h) {
        bounds.visible = false;
    }

    return bounds;
}

} // namespace meridian::graphics

