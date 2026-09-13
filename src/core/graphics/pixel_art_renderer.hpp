#pragma once
// src/core/graphics/pixel_art_renderer.hpp
//
// Meridian Terminal 2.5 — Professional Pixel-Art Terminal Rendering Pipeline:
// High-quality image decoder -> Lanczos/Bicubic resizer -> Sobel edge preservation
// -> Pixel-grid reduction -> Median-cut / K-means color quantization
// -> Ordered dithering (optional) -> Unicode half-block TrueColor ANSI emitter.

#include <cstdint>
#include <string>
#include <vector>
#include "image_decoder.hpp"

namespace meridian::graphics {

enum class PixelRenderStyle {
    PixelArt,     // Controlled pixel grid + color quantization + edge preservation (Default)
    HalfBlock,    // Full TrueColor 24-bit halfblocks
    Ascii,        // Grayscale ASCII character map
    AsciiColor,   // Full TrueColor ANSI foreground ASCII
    Braille,      // 2x4 dot Unicode Braille matrix
    TrueColor     // Direct 24-bit RGB output
};

struct PixelArtOptions {
    PixelRenderStyle style = PixelRenderStyle::HalfBlock; // HD Full TrueColor Halfblocks by default
    int scale = 1;              // 1:1 HD pixel grid by default
    int num_colors = 64;        // High-fidelity palette
    float sharpness = 0.20f;    // Subtle sharpening
    bool dither = false;        // Ordered Bayer dithering
    int max_width_cols = 0;     // Target terminal columns (0 = auto-detect)
    int max_height_rows = 0;    // Target terminal rows (0 = auto-detect)
    bool preserve_alpha = true; // Preserve transparency
    std::string charset = " .:-=+*#%@";
};

struct PixelRgba {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    bool is_transparent() const { return a < 128; }
    bool operator==(const PixelRgba& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
};

class PixelArtRenderer {
public:
    static std::string render_file(const std::string& filepath, const PixelArtOptions& opts);
    static std::string render_image(const DecodedImage& image, const PixelArtOptions& opts);
    static std::string render_pixels(const std::vector<PixelRgba>& pixels, int width, int height, const PixelArtOptions& opts);

    // Terminal dimension helper
    static void get_terminal_dimensions(int& out_cols, int& out_rows);

    // Image processing stages
    static std::vector<PixelRgba> sharpen_image(const std::vector<PixelRgba>& src, int w, int h, float amount);
    static std::vector<float> compute_sobel_edges(const std::vector<PixelRgba>& src, int w, int h);
    static std::vector<PixelRgba> reduce_to_pixel_grid(const std::vector<PixelRgba>& src, int src_w, int src_h, int target_w, int target_h, const std::vector<float>& edges);
    static std::vector<PixelRgba> quantize_colors(const std::vector<PixelRgba>& src, int num_colors, bool dither, int w, int h);
    static std::string emit_ansi_halfblocks(const std::vector<PixelRgba>& grid, int grid_w, int grid_h, bool preserve_alpha);
    static std::string emit_ascii(const std::vector<PixelRgba>& grid, int grid_w, int grid_h, bool colored, const std::string& charset);
    static std::string emit_braille(const std::vector<PixelRgba>& grid, int grid_w, int grid_h);
};

} // namespace meridian::graphics

