// src/core/graphics/animation_engine.cpp
#include "animation_engine.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>

// stb_image is included in image_decoder.cpp with implementation.
// Here we declare the prototypes needed for GIF loading.
extern "C" {
    unsigned char *stbi_load_gif_from_memory(
        unsigned char const *buffer, int len, int **delays,
        int *x, int *y, int *z, int *comp, int req_comp
    );
    void stbi_image_free(void *retval_from_stbi_load);
}

namespace meridian::graphics {

size_t AnimationObject::total_memory_bytes() const {
    size_t bytes = 0;
    for (const auto& f : frames) {
        bytes += f.byte_size();
    }
    bytes += current_canvas.size();
    bytes += previous_canvas.size();
    return bytes;
}

AnimationEngine& AnimationEngine::instance() {
    static AnimationEngine eng;
    return eng;
}

AnimationEngine::AnimationEngine() = default;

uint64_t AnimationEngine::load_file(const std::string& filepath, std::string* error_out) {
    if (filepath.empty()) {
        if (error_out) *error_out = "Empty filepath";
        return 0;
    }

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        if (error_out) *error_out = "Failed to open file: " + filepath;
        return 0;
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        if (error_out) *error_out = "File is empty: " + filepath;
        return 0;
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        if (error_out) *error_out = "Failed to read file bytes: " + filepath;
        return 0;
    }

    uint64_t id = load_memory(buffer.data(), buffer.size(), filepath, error_out);
    if (id != 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = animations_.find(id);
        if (it != animations_.end()) {
            it->second->source_path = filepath;
        }
    }
    return id;
}

uint64_t AnimationEngine::load_memory(
    const uint8_t* data,
    size_t size,
    const std::string& hint,
    std::string* error_out
) {
    if (!data || size < 4) {
        if (error_out) *error_out = "Invalid buffer";
        return 0;
    }

    auto anim = std::make_shared<AnimationObject>();
    anim->source_path = hint;

    // Check if GIF header: "GIF87a" or "GIF89a"
    bool is_gif = (size >= 6 && data[0] == 'G' && data[1] == 'I' && data[2] == 'F');

    if (is_gif) {
        if (!decode_gif_frames(data, size, *anim, error_out)) {
            return 0;
        }
    } else {
        // Single static image fallback
        auto decoded = ImageDecoder::decode_memory(data, size, hint);
        if (!decoded.is_valid()) {
            if (error_out) *error_out = "Unsupported image format or corrupt data";
            return 0;
        }

        anim->canvas_width = decoded.original_width;
        anim->canvas_height = decoded.original_height;
        anim->format = decoded.format;

        for (size_t i = 0; i < decoded.frames.size(); ++i) {
            const auto& df = decoded.frames[i];
            AnimationFrame f(df.width, df.height, df.rgba, df.duration);
            f.set_frame_index(static_cast<int>(i));
            anim->frames.push_back(std::move(f));
        }
    }

    if (anim->frames.empty() || anim->canvas_width <= 0 || anim->canvas_height <= 0) {
        if (error_out) *error_out = "No valid frames decoded";
        return 0;
    }

    // Security bounds checking
    if (anim->canvas_width > limits_.max_width || anim->canvas_height > limits_.max_height) {
        if (error_out) *error_out = "Animation dimensions exceed safety limit";
        return 0;
    }

    if (anim->frames.size() > limits_.max_frames) {
        anim->frames.resize(limits_.max_frames);
    }

    // Initialize controller and canvas
    anim->controller.set_frames(anim->frames);
    anim->current_canvas.resize(static_cast<size_t>(anim->canvas_width * anim->canvas_height * 4), 0);

    // Compose frame 0
    render_frame_to_canvas(*anim, 0);

    uint64_t new_id = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        new_id = next_id_++;
        anim->id = new_id;
        animations_[new_id] = anim;
        enforce_memory_limit();
    }

    return new_id;
}

bool AnimationEngine::decode_gif_frames(
    const uint8_t* data,
    size_t size,
    AnimationObject& out,
    std::string* error_out
) {
    int* delays = nullptr;
    int w = 0, h = 0, frame_count = 0, comp = 0;

    unsigned char* raw_pixels = stbi_load_gif_from_memory(
        data, static_cast<int>(size),
        &delays, &w, &h, &frame_count, &comp, 4
    );

    if (!raw_pixels || w <= 0 || h <= 0 || frame_count <= 0) {
        if (delays) free(delays);
        if (error_out) *error_out = "stb_image failed to decode GIF";
        return false;
    }

    out.canvas_width = w;
    out.canvas_height = h;
    out.format = "gif";

    size_t frame_bytes = static_cast<size_t>(w * h * 4);
    for (int i = 0; i < frame_count; ++i) {
        double duration = 0.1;
        if (delays && delays[i] > 0) {
            duration = static_cast<double>(delays[i]) / 100.0;
        }

        std::vector<uint8_t> frame_rgba(frame_bytes);
        std::memcpy(frame_rgba.data(), raw_pixels + (i * frame_bytes), frame_bytes);

        AnimationFrame frame(w, h, std::move(frame_rgba), duration);
        frame.set_frame_index(i);
        frame.set_disposal_mode(FrameDisposalMode::DoNotDispose);
        out.frames.push_back(std::move(frame));
    }

    stbi_image_free(raw_pixels);
    if (delays) free(delays);

    return true;
}

uint64_t AnimationEngine::create_empty_animation(int width, int height, const std::string& name) {
    if (width <= 0 || height <= 0 || width > limits_.max_width || height > limits_.max_height) {
        return 0;
    }

    auto anim = std::make_shared<AnimationObject>();
    anim->canvas_width = width;
    anim->canvas_height = height;
    anim->source_path = name;
    anim->format = "raw";
    anim->current_canvas.resize(static_cast<size_t>(width * height * 4), 0);

    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t new_id = next_id_++;
    anim->id = new_id;
    animations_[new_id] = anim;
    return new_id;
}

bool AnimationEngine::add_frame(uint64_t anim_id, const AnimationFrame& frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end() || !frame.is_valid()) return false;

    if (it->second->frames.size() >= limits_.max_frames) return false;

    AnimationFrame f = frame;
    f.set_frame_index(static_cast<int>(it->second->frames.size()));
    it->second->frames.push_back(std::move(f));
    it->second->controller.set_frames(it->second->frames);

    if (it->second->frames.size() == 1) {
        render_frame_to_canvas(*it->second, 0);
    }
    return true;
}

bool AnimationEngine::set_frame_duration(uint64_t anim_id, size_t frame_index, double duration_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;

    if (frame_index < it->second->frames.size()) {
        it->second->frames[frame_index].set_duration(duration_seconds);
        it->second->controller.set_frames(it->second->frames);
        return true;
    }
    return false;
}

size_t AnimationEngine::frame_count(uint64_t anim_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    return (it != animations_.end()) ? it->second->frames.size() : 0;
}

bool AnimationEngine::play(uint64_t anim_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;
    it->second->controller.play();
    return true;
}

bool AnimationEngine::pause(uint64_t anim_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;
    it->second->controller.pause();
    return true;
}

bool AnimationEngine::stop(uint64_t anim_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;
    it->second->controller.stop();
    render_frame_to_canvas(*it->second, 0);
    return true;
}

bool AnimationEngine::seek(uint64_t anim_id, double seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;
    it->second->controller.seek_time(seconds);
    render_frame_to_canvas(*it->second, it->second->controller.current_frame_index());
    return true;
}

bool AnimationEngine::seek_frame(uint64_t anim_id, size_t frame_index) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;
    it->second->controller.seek_frame(frame_index);
    render_frame_to_canvas(*it->second, it->second->controller.current_frame_index());
    return true;
}

bool AnimationEngine::setLoopCount(uint64_t anim_id, int count) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;
    it->second->controller.set_loop_count(count);
    return true;
}

bool AnimationEngine::setFrameRate(uint64_t anim_id, double fps) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return false;
    it->second->controller.set_fps_override(fps);
    return true;
}

std::vector<uint64_t> AnimationEngine::tick(double dt) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<uint64_t> changed_ids;

    for (auto& [id, anim] : animations_) {
        if (anim->controller.tick(dt)) {
            render_frame_to_canvas(*anim, anim->controller.current_frame_index());
            changed_ids.push_back(id);
        }
    }

    return changed_ids;
}

void AnimationEngine::render_frame_to_canvas(AnimationObject& anim, size_t target_frame) {
    if (anim.frames.empty() || target_frame >= anim.frames.size()) return;

    // Direct keyframe or frame assignment
    const auto& f = anim.frames[target_frame];
    if (f.width() == anim.canvas_width && f.height() == anim.canvas_height && f.x_offset() == 0 && f.y_offset() == 0) {
        anim.current_canvas = f.rgba();
        anim.last_rendered_frame = target_frame;
        return;
    }

    // Composite partial frame into canvas
    f.blit_into(anim.current_canvas, anim.canvas_width, anim.canvas_height);
    anim.last_rendered_frame = target_frame;
}

const std::vector<uint8_t>& AnimationEngine::get_composed_canvas(uint64_t anim_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    static const std::vector<uint8_t> empty_buf;
    auto it = animations_.find(anim_id);
    if (it == animations_.end()) return empty_buf;
    return it->second->current_canvas;
}

const AnimationObject* AnimationEngine::get_animation(uint64_t anim_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = animations_.find(anim_id);
    return (it != animations_.end()) ? it->second.get() : nullptr;
}

bool AnimationEngine::remove_animation(uint64_t anim_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return animations_.erase(anim_id) > 0;
}

void AnimationEngine::clear_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    animations_.clear();
}

std::vector<uint64_t> AnimationEngine::list_active_animations() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<uint64_t> ids;
    for (const auto& [id, _] : animations_) {
        ids.push_back(id);
    }
    return ids;
}

size_t AnimationEngine::total_memory_usage() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total = 0;
    for (const auto& [_, anim] : animations_) {
        total += anim->total_memory_bytes();
    }
    return total;
}

bool AnimationEngine::enforce_memory_limit() {
    size_t total = 0;
    for (const auto& [_, anim] : animations_) {
        total += anim->total_memory_bytes();
    }

    if (total <= limits_.max_memory_bytes) return true;

    // Evict oldest stopped animations first
    for (auto it = animations_.begin(); it != animations_.end(); ) {
        if (it->second->controller.is_stopped()) {
            it = animations_.erase(it);
        } else {
            ++it;
        }
    }
    return true;
}

} // namespace meridian::graphics

