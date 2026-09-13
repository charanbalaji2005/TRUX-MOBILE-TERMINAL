#pragma once
// src/core/graphics/image_object.hpp
//
// Native GPU Image Object representation for Meridian Terminal.
// Represents a photorealistic raster image placed on the terminal canvas layer.

#include "image_decoder.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace meridian::graphics {

enum class ImagePlacementType {
    CursorRelative,  // Placed at terminal cursor cell position (scrolls with content)
    AbsolutePixels,  // Placed at explicit canvas pixel coordinates (x, y)
    ScrollbackRow    // Anchored to a specific scrollback history row
};

struct ImageObject {
    uint64_t id = 0;
    uint64_t gpu_texture_id = 0;

    // Source Metadata
    std::string source_path;
    std::string format;          // "PNG", "JPEG", "WebP", "BMP", "GIF"
    int original_width = 0;      // Native raster width in pixels
    int original_height = 0;     // Native raster height in pixels
    bool has_alpha = true;

    // Canvas Display Geometry (physical or logical pixels)
    float x = 0.0f;
    float y = 0.0f;
    float display_width = 0.0f;
    float display_height = 0.0f;

    // Percentage dimensions (if specified, e.g. 50% width)
    float width_percentage = 0.0f;
    float height_percentage = 0.0f;

    // Terminal Grid Anchors
    int cursor_row = 0;
    int cursor_col = 0;
    int scrollback_row = 0;
    int cols_spanned = 0;
    int rows_spanned = 0;

    // Stacking & Visual Properties
    int z_index = 0;             // < 0: under text, >= 0: over text
    float opacity = 1.0f;        // 0.0f (transparent) to 1.0f (opaque)
    bool visible = true;
    bool scroll_with_terminal = true;

    ImagePlacementType placement = ImagePlacementType::CursorRelative;
    ImageFitMode fit_mode = ImageFitMode::Contain;
    ImageScaleFilter filter = ImageScaleFilter::Smooth;

    // HiDPI / Display scaling
    float device_pixel_ratio = 1.0f;

    // Animation Metadata
    bool is_animated = false;
    size_t current_frame = 0;
    double frame_elapsed = 0.0;
};

} // namespace meridian::graphics

