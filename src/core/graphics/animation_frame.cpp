// src/core/graphics/animation_frame.cpp
#include "animation_frame.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace meridian::graphics {

AnimationFrame::AnimationFrame()
    : width_(0), height_(0), duration_seconds_(0.1) {}

AnimationFrame::AnimationFrame(int w, int h, std::vector<uint8_t> rgba, double duration)
    : width_(w), height_(h), duration_seconds_(duration), rgba_(std::move(rgba)) {}

bool AnimationFrame::is_valid() const {
    return width_ > 0 && height_ > 0 && rgba_.size() >= static_cast<size_t>(width_ * height_ * 4);
}

size_t AnimationFrame::byte_size() const {
    return rgba_.size();
}

AnimationFrame AnimationFrame::rescale(int target_w, int target_h) const {
    if (!is_valid() || target_w <= 0 || target_h <= 0) {
        return AnimationFrame();
    }

    AnimationFrame dst;
    dst.width_ = target_w;
    dst.height_ = target_h;
    dst.duration_seconds_ = duration_seconds_;
    dst.frame_index_ = frame_index_;
    dst.disposal_ = disposal_;
    dst.blend_ = blend_;
    dst.rgba_.resize(static_cast<size_t>(target_w * target_h * 4));

    // Bilinear filtering
    for (int y = 0; y < target_h; ++y) {
        float v = (target_h > 1) ? static_cast<float>(y) / (target_h - 1) : 0.0f;
        float py = v * (height_ - 1);
        int y0 = std::clamp(static_cast<int>(std::floor(py)), 0, height_ - 1);
        int y1 = std::clamp(y0 + 1, 0, height_ - 1);
        float fy = py - y0;

        for (int x = 0; x < target_w; ++x) {
            float u = (target_w > 1) ? static_cast<float>(x) / (target_w - 1) : 0.0f;
            float px = u * (width_ - 1);
            int x0 = std::clamp(static_cast<int>(std::floor(px)), 0, width_ - 1);
            int x1 = std::clamp(x0 + 1, 0, width_ - 1);
            float fx = px - x0;

            int i00 = (y0 * width_ + x0) * 4;
            int i10 = (y0 * width_ + x1) * 4;
            int i01 = (y1 * width_ + x0) * 4;
            int i11 = (y1 * width_ + x1) * 4;

            int dst_idx = (y * target_w + x) * 4;
            for (int ch = 0; ch < 4; ++ch) {
                float top = rgba_[i00 + ch] * (1.0f - fx) + rgba_[i10 + ch] * fx;
                float bot = rgba_[i01 + ch] * (1.0f - fx) + rgba_[i11 + ch] * fx;
                float val = top * (1.0f - fy) + bot * fy;
                dst.rgba_[dst_idx + ch] = static_cast<uint8_t>(std::clamp(val, 0.0f, 255.0f));
            }
        }
    }

    return dst;
}

void AnimationFrame::blit_into(std::vector<uint8_t>& canvas, int canvas_w, int canvas_h) const {
    if (!is_valid() || canvas.empty() || canvas_w <= 0 || canvas_h <= 0) return;

    for (int y = 0; y < height_; ++y) {
        int dst_y = y + y_offset_;
        if (dst_y < 0 || dst_y >= canvas_h) continue;

        for (int x = 0; x < width_; ++x) {
            int dst_x = x + x_offset_;
            if (dst_x < 0 || dst_x >= canvas_w) continue;

            int src_idx = (y * width_ + x) * 4;
            int dst_idx = (dst_y * canvas_w + dst_x) * 4;

            if (dst_idx + 3 >= static_cast<int>(canvas.size())) continue;

            uint8_t sa = rgba_[src_idx + 3];
            if (blend_ == FrameBlendMode::Source || sa == 255) {
                std::memcpy(&canvas[dst_idx], &rgba_[src_idx], 4);
            } else if (sa > 0) {
                // Alpha over blend
                float src_a = sa / 255.0f;
                float dst_a = canvas[dst_idx + 3] / 255.0f;
                float out_a = src_a + dst_a * (1.0f - src_a);

                if (out_a > 0.001f) {
                    for (int c = 0; c < 3; ++c) {
                        float src_c = rgba_[src_idx + c] / 255.0f;
                        float dst_c = canvas[dst_idx + c] / 255.0f;
                        float out_c = (src_c * src_a + dst_c * dst_a * (1.0f - src_a)) / out_a;
                        canvas[dst_idx + c] = static_cast<uint8_t>(std::clamp(out_c * 255.0f, 0.0f, 255.0f));
                    }
                    canvas[dst_idx + 3] = static_cast<uint8_t>(std::clamp(out_a * 255.0f, 0.0f, 255.0f));
                }
            }
        }
    }
}

} // namespace meridian::graphics

