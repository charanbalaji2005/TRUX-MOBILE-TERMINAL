#pragma once
// src/core/graphics/ascii_art_engine.hpp
//
// Ultra-High-Quality ASCII & Unicode Dual-Pixel Terminal Art Engine for Meridian Terminal.
// Features Lanczos-3 Sinc Resampling, 24-bit TrueColor Dual-Pixel Halfblocks (▀),
// Edge-Aware Detail Enhancement, Brightness/Contrast/Gamma/Saturation adjustments,
// and 70-character brightness ramps with Floyd-Steinberg error diffusion dithering.

#include "image_decoder.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace meridian::graphics {

enum class AsciiRenderMode {
    TrueColor,   // 24-bit TrueColor Dual-Pixel Halfblocks (▀) [DEFAULT]
    HalfBlock,   // Dual-Pixel Halfblocks with ANSI 256 / TrueColor fallback
    Ansi,        // 256-color ANSI colored ASCII character ramp
    Ascii        // Pure monochrome ASCII character ramp with optional Floyd-Steinberg dithering
};

enum class AsciiQuality {
    Low,         // Fast ASCII with 10-char ramp
    Medium,      // Color ASCII with area-average resampling
    High,        // TrueColor with bicubic resampling
    Ultra        // TrueColor Dual-Pixel Halfblocks + Lanczos-3 Resampling + Edge Enhancement [DEFAULT]
};

enum class AsciiFitMode {
    Contain,     // Fit within requested width/height preserving aspect ratio [DEFAULT]
    Cover        // Fill bounding box preserving aspect ratio (crop edges)
};

struct AsciiArtOptions {
    AsciiRenderMode mode = AsciiRenderMode::TrueColor;
    AsciiQuality quality = AsciiQuality::Ultra;
    AsciiFitMode fit = AsciiFitMode::Contain;

    int target_width = 0;        // Target columns (0 = auto-detect terminal width)
    int target_height = 0;       // Target rows (0 = auto-calculate based on aspect ratio)

    float brightness = 1.0f;     // 1.0 = normal, > 1.0 brighter, < 1.0 darker
    float contrast = 1.0f;       // 1.0 = normal, > 1.0 higher contrast
    float gamma = 1.0f;          // 1.0 = linear, < 1.0 brightens shadows, > 1.0 deepens shadows
    float saturation = 1.0f;     // 1.0 = normal, 0.0 = grayscale, > 1.0 hyper-vibrant
    float edge_enhancement = 0.25f; // Unsharp mask outline enhancement (0.0 to 1.0)

    bool dither = false;         // Floyd-Steinberg error diffusion dithering
    bool preserve_alpha = true;  // Alpha transparency preservation
    float char_aspect = 0.5f;    // Terminal character cell width/height aspect ratio (~1:2)

    std::string character_ramp =
        " .'`^,:;Il!i~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
};

struct PixelRgbF {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

class AsciiArtEngine {
public:
    // Main Entrypoints: Render file, DecodedImage, or raw RGBA buffer into ANSI string
    static std::string render_file(
        const std::string& filepath,
        const AsciiArtOptions& options = {}
    );

    static std::string render_image(
        const DecodedImage& image,
        const AsciiArtOptions& options = {}
    );

    static std::string render_pixels(
        const std::vector<uint8_t>& rgba_pixels,
        int src_w, int src_h,
        const AsciiArtOptions& options = {}
    );

    // High-Quality Lanczos-3 Sinc Resampling
    static std::vector<uint8_t> resample_lanczos3(
        const std::vector<uint8_t>& src_rgba,
        int src_w, int src_h,
        int dst_w, int dst_h
    );

    // Color & Tonality Processing
    static void apply_color_adjustments(
        std::vector<uint8_t>& rgba,
        int width, int height,
        float brightness, float contrast, float gamma, float saturation
    );

    // Edge-Aware Outline & Feature Enhancement (Unsharp Mask)
    static void apply_edge_enhancement(
        std::vector<uint8_t>& rgba,
        int width, int height,
        float strength
    );

    // Emitters
    static std::string emit_truecolor_halfblocks(
        const std::vector<uint8_t>& rgba,
        int width, int height,
        bool preserve_alpha
    );

    static std::string emit_ascii_art(
        const std::vector<uint8_t>& rgba,
        int width, int height,
        AsciiRenderMode mode,
        bool dither,
        const std::string& ramp,
        bool preserve_alpha
    );

    // Terminal Dimension Helper
    static void get_terminal_dimensions(int& cols, int& rows);
};

} // namespace meridian::graphics

