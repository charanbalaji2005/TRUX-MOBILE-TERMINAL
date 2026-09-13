#pragma once
// src/core/graphics/animation_frame.hpp
//
// Native raster animation frame container for GIF, APNG, animated WebP,
// and Kitty Graphics Protocol frame streams.

#include <cstdint>
#include <vector>
#include <memory>

namespace meridian::graphics {

enum class FrameDisposalMode {
    Unspecified,         // No disposal specified (overlay onto existing canvas)
    DoNotDispose,        // Leave current frame on canvas for next frame
    RestoreToBackground, // Clear canvas area to transparent background
    RestoreToPrevious    // Restore canvas area to state before this frame
};

enum class FrameBlendMode {
    Source,              // Overwrite destination pixel completely (including alpha)
    Over                 // Alpha-blend source over destination pixel
};

struct AnimationRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

class AnimationFrame {
public:
    AnimationFrame();
    AnimationFrame(int w, int h, std::vector<uint8_t> rgba, double duration = 0.1);

    bool is_valid() const;
    size_t byte_size() const;

    int width() const { return width_; }
    int height() const { return height_; }
    double duration() const { return duration_seconds_; }
    int frame_index() const { return frame_index_; }
    FrameDisposalMode disposal_mode() const { return disposal_; }
    FrameBlendMode blend_mode() const { return blend_; }
    int x_offset() const { return x_offset_; }
    int y_offset() const { return y_offset_; }
    bool is_keyframe() const { return is_keyframe_; }

    const std::vector<uint8_t>& rgba() const { return rgba_; }
    std::vector<uint8_t>& rgba() { return rgba_; }
    const uint8_t* data() const { return rgba_.data(); }

    void set_dimensions(int w, int h) { width_ = w; height_ = h; }
    void set_duration(double sec) { duration_seconds_ = sec; }
    void set_frame_index(int idx) { frame_index_ = idx; }
    void set_disposal_mode(FrameDisposalMode mode) { disposal_ = mode; }
    void set_blend_mode(FrameBlendMode mode) { blend_ = mode; }
    void set_offsets(int x, int y) { x_offset_ = x; y_offset_ = y; }
    void set_keyframe(bool kf) { is_keyframe_ = kf; }

    // Rescale frame RGBA to target dimensions
    AnimationFrame rescale(int target_w, int target_h) const;

    // Crop or blit region
    void blit_into(std::vector<uint8_t>& canvas, int canvas_w, int canvas_h) const;

private:
    int width_ = 0;
    int height_ = 0;
    int x_offset_ = 0;
    int y_offset_ = 0;
    double duration_seconds_ = 0.1;
    int frame_index_ = 0;
    FrameDisposalMode disposal_ = FrameDisposalMode::DoNotDispose;
    FrameBlendMode blend_ = FrameBlendMode::Over;
    bool is_keyframe_ = true;
    std::vector<uint8_t> rgba_; // 32-bit RGBA (width * height * 4)
};

} // namespace meridian::graphics

