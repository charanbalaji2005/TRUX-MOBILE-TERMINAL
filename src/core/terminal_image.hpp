#pragma once
// src/core/terminal_image.hpp
//
// Native terminal image rendering engine for Meridian Terminal.
// Supports TrueColor 24-bit half-blocks (▀), ASCII, Colored ASCII, Hybrid mode,
// and animated GIF playback directly on the terminal canvas.

#include <cstdint>
#include <string>
#include <vector>

namespace meridian::core {

enum class ImageRenderMode {
    RealRaster,   // Authentic full-color raster image (Kitty Graphics Protocol / GPU texture) [DEFAULT]
    Pixel,        // Nearest-neighbor pixel art
    HalfBlock,    // TrueColor 24-bit half-blocks (2 vertical subpixels per char cell)
    Ascii,        // Grayscale ASCII characters
    ColorAscii,   // Colored ASCII characters with TrueColor ANSI
    Hybrid        // Half-block background with ASCII edge contours
};

struct ImageOptions {
    ImageRenderMode mode = ImageRenderMode::RealRaster;
    int target_width = 32;   // in character columns
    int target_height = 16;  // in character rows (each row = 2 vertical subpixels in HalfBlock)
    bool center = false;
    std::string custom_charset = " .:-=+*#%@";
};

struct RgbColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

class TerminalImage {
public:
    TerminalImage();
    TerminalImage(int width, int height);

    bool load_file(const std::string& path);
    bool load_png_or_ppm(const std::string& path);
    void set_pixel(int x, int y, RgbColor color);
    RgbColor get_pixel(int x, int y) const;

    int width() const { return width_; }
    int height() const { return height_; }
    bool is_valid() const { return width_ > 0 && height_ > 0 && !pixels_.empty(); }

    // Renders the image into an ANSI escape sequence string
    std::string render(const ImageOptions& opts) const;

    // Generates the reference pixel art moon & mountains image
    static TerminalImage create_default_reference_artwork();

    // Renders the reference artwork as an array of ANSI terminal lines
    static std::vector<std::string> render_reference_artwork_lines(int max_rows = 11);

    // Emits native GPU hardware raster inline image escape sequence (iTerm2 OSC 1337 + Kitty protocol)
    static std::string render_hardware_image_escape(const std::string& filepath, int target_width_cols = 0);
    static std::string render_kitty_graphics_artwork(int col = 2, int row = 1, int cols_spanned = 24, int rows_spanned = 10);
    static std::string render_file_raster_escape(const std::string& filepath, int x = 30, int y = 30, int max_w = 220, int max_h = 220);
    std::string to_kitty_graphics_escape(int x = 30, int y = 30, int cols_spanned = 28, int rows_spanned = 10) const;

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<RgbColor> pixels_;

    RgbColor sample_bilinear(float u, float v) const;
    char char_for_luminance(float lum, const std::string& charset) const;
};

} // namespace meridian::core

