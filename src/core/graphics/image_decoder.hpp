#pragma once
// src/core/graphics/image_decoder.hpp
//
// Native image decoder subsystem for Meridian Terminal.
// Decodes PNG, JPEG, WebP, BMP, GIF, and PPM formats directly into
// 32-bit RGBA8888 pixel buffers with support for multi-frame animations.

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace meridian::graphics {

enum class ImageFitMode {
    Contain, // Preserve aspect ratio, fit entirely within target box
    Cover,   // Preserve aspect ratio, fill entire target box
    Stretch  // Ignore aspect ratio, stretch to target box
};

enum class ImageScaleFilter {
    Smooth,  // High-quality bilinear / bicubic filtering (default)
    Pixel    // Nearest-neighbor scaling for pixel art
};

struct ImageFrame {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba; // 32-bit RGBA (4 bytes per pixel)
    double duration = 0.1;     // Frame duration in seconds for animations (e.g. GIF)

    bool is_valid() const {
        return width > 0 && height > 0 && rgba.size() >= static_cast<size_t>(width * height * 4);
    }
};

struct DecodedImage {
    int original_width = 0;
    int original_height = 0;
    std::string format; // "png", "jpeg", "webp", "gif", "bmp", "ppm"
    bool is_animated = false;
    std::vector<ImageFrame> frames;

    bool is_valid() const {
        return !frames.empty() && frames[0].is_valid();
    }

    const ImageFrame& frame(size_t index = 0) const {
        if (frames.empty()) {
            static ImageFrame empty;
            return empty;
        }
        return frames[index % frames.size()];
    }
};

struct ImageDimensions {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

class ImageDecoder {
public:
    // Decode an image file from path
    static DecodedImage decode_file(const std::string& filepath);

    // Decode an image from memory buffer (PNG, JPEG, WebP, BMP, GIF, PPM)
    static DecodedImage decode_memory(const uint8_t* data, size_t size, const std::string& hint = "");

    // Calculate fitted dimensions preserving aspect ratio
    static ImageDimensions calculate_fit(
        int src_w, int src_h,
        float target_box_w, float target_box_h,
        ImageFitMode mode = ImageFitMode::Contain,
        float origin_x = 0.0f, float origin_y = 0.0f
    );

    // Rescale RGBA frame using bilinear or nearest-neighbor filtering
    static ImageFrame rescale_frame(
        const ImageFrame& src,
        int target_w, int target_h,
        ImageScaleFilter filter = ImageScaleFilter::Smooth
    );
};

} // namespace meridian::graphics

