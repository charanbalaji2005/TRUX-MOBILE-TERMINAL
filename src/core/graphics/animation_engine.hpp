#pragma once
// src/core/graphics/animation_engine.hpp
//
// Central Animation Engine for Meridian Terminal.
// Handles multi-frame GIF, APNG, WebP, and Kitty Graphics Protocol animation streams.
// Implements frame composition, disposal modes, memory bounds (256MB default),
// and non-blocking timer/event-driven playback.

#include "animation_frame.hpp"
#include "animation_controller.hpp"
#include "image_decoder.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace meridian::graphics {

struct AnimationObject {
    uint64_t id = 0;
    std::string source_path;
    std::string format;
    int canvas_width = 0;
    int canvas_height = 0;
    std::vector<AnimationFrame> frames;
    AnimationController controller;

    // Composition state
    std::vector<uint8_t> current_canvas; // Composed RGBA canvas
    std::vector<uint8_t> previous_canvas; // For RestoreToPrevious disposal
    size_t last_rendered_frame = static_cast<size_t>(-1);

    size_t total_memory_bytes() const;
};

struct AnimationLimits {
    size_t max_memory_bytes = 256 * 1024 * 1024; // 256 MB default
    int max_width = 16384;
    int max_height = 16384;
    size_t max_frames = 2048;
    double max_duration_seconds = 3600.0;
};

class AnimationEngine {
public:
    static AnimationEngine& instance();

    AnimationEngine();
    ~AnimationEngine() = default;

    // ── Load Animations ─────────────────────────────────────────────────────
    uint64_t load_file(const std::string& filepath, std::string* error_out = nullptr);
    uint64_t load_memory(const uint8_t* data, size_t size, const std::string& hint = "", std::string* error_out = nullptr);
    uint64_t create_empty_animation(int width, int height, const std::string& name = "");

    // ── Frame Management ────────────────────────────────────────────────────
    bool add_frame(uint64_t anim_id, const AnimationFrame& frame);
    bool set_frame_duration(uint64_t anim_id, size_t frame_index, double duration_seconds);
    size_t frame_count(uint64_t anim_id) const;

    // ── Playback Controls ───────────────────────────────────────────────────
    bool play(uint64_t anim_id);
    bool pause(uint64_t anim_id);
    bool stop(uint64_t anim_id);
    bool seek(uint64_t anim_id, double seconds);
    bool seek_frame(uint64_t anim_id, size_t frame_index);
    bool setLoopCount(uint64_t anim_id, int count);
    bool setFrameRate(uint64_t anim_id, double fps);

    // ── Lifecycle & Ticking ─────────────────────────────────────────────────
    // Advance simulation time across all playing animations. Returns list of animation IDs whose frame changed.
    std::vector<uint64_t> tick(double dt);

    // ── Render Output ───────────────────────────────────────────────────────
    // Get the fully composed 32-bit RGBA pixel canvas for the active frame
    const std::vector<uint8_t>& get_composed_canvas(uint64_t anim_id);
    const AnimationObject* get_animation(uint64_t anim_id) const;

    // ── Management & Memory ─────────────────────────────────────────────────
    bool remove_animation(uint64_t anim_id);
    void clear_all();
    std::vector<uint64_t> list_active_animations() const;
    size_t total_memory_usage() const;

    // Limits & Security
    const AnimationLimits& limits() const { return limits_; }
    void set_limits(const AnimationLimits& limits) { limits_ = limits; }
    void set_memory_limit(size_t bytes) { limits_.max_memory_bytes = bytes; }

private:
    void render_frame_to_canvas(AnimationObject& anim, size_t target_frame);
    bool decode_gif_frames(const uint8_t* data, size_t size, AnimationObject& out, std::string* error_out);
    bool enforce_memory_limit();

    mutable std::mutex mutex_;
    uint64_t next_id_ = 1;
    AnimationLimits limits_;
    std::map<uint64_t, std::shared_ptr<AnimationObject>> animations_;
};

} // namespace meridian::graphics

