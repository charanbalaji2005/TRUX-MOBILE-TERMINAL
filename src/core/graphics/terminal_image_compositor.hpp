#pragma once
// src/core/graphics/terminal_image_compositor.hpp
//
// Dedicated Native Image Layer Compositor for Meridian Terminal.
// Manages the separate graphics plane above/below the character grid,
// computes viewports, handles window resizes, and tracks scrollback movement.

#include "image_object.hpp"
#include "gpu_texture_manager.hpp"
#include <memory>
#include <vector>

namespace meridian::graphics {

struct TerminalCellMetrics {
    float cell_width_px = 9.0f;
    float cell_height_px = 18.0f;
    int columns = 80;
    int rows = 24;
    float device_pixel_ratio = 1.0f;
};

struct CompositorBounds {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float source_u0 = 0.0f;
    float source_v0 = 0.0f;
    float source_u1 = 1.0f;
    float source_v1 = 1.0f;
    bool visible = true;
};

class TerminalImageCompositor {
public:
    TerminalImageCompositor();

    // Sets terminal viewport & cell metrics
    void set_metrics(const TerminalCellMetrics& metrics);
    const TerminalCellMetrics& metrics() const { return metrics_; }

    // Computes physical pixel bounds & texture UV coordinates for an image
    CompositorBounds compute_render_bounds(
        const ImageObject& img,
        int current_scroll_offset = 0
    ) const;

    // Handles terminal window resize event
    void handle_resize(int new_cols, int new_rows, float new_cell_w, float new_cell_h);

    // Updates scrollback row anchors when new lines are appended
    void on_terminal_scroll(int lines_scrolled, std::vector<ImageObject>& images);

    // Calculates containment/cover geometry preserving aspect ratio
    static void compute_aspect_fit(
        int src_w, int src_h,
        float target_w, float target_h,
        ImageFitMode mode,
        float& out_w, float& out_h,
        float& out_u0, float& out_v0,
        float& out_u1, float& out_v1
    );

private:
    TerminalCellMetrics metrics_;
};

} // namespace meridian::graphics

