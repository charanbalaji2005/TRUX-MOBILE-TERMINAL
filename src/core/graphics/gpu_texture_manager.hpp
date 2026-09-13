#pragma once
// src/core/graphics/gpu_texture_manager.hpp
//
// GPU Texture Lifecycle & Cache Manager for Meridian Terminal.
// Uploads, manages, and caches 32-bit RGBA8 texture buffers for native GPU rendering.

#include "image_decoder.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace meridian::graphics {

struct GpuTexture {
    uint64_t id = 0;
    std::string key;             // Filepath or hash key for caching
    int width = 0;               // Texture width in pixels
    int height = 0;              // Texture height in pixels
    bool has_alpha = true;
    uint32_t handle = 0;         // Native GPU texture handle (GL/Vulkan/Metal ID)
    std::vector<uint8_t> rgba;   // 32-bit RGBA8888 buffer
    bool is_uploaded = false;
    uint64_t generation = 1;

    size_t memory_bytes() const {
        return rgba.size();
    }
};

struct DamageRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

class GpuTextureManager {
public:
    GpuTextureManager();
    ~GpuTextureManager();

    // Creates or retrieves a cached GPU texture from decoded image data
    std::shared_ptr<GpuTexture> get_or_create_texture(
        const std::string& key,
        const DecodedImage& image,
        size_t frame_index = 0
    );

    // Creates a GPU texture directly from raw RGBA8 buffer
    std::shared_ptr<GpuTexture> create_texture_from_rgba(
        const std::string& key,
        const uint8_t* data,
        int width,
        int height
    );

    // Finds an existing cached texture
    std::shared_ptr<GpuTexture> find_texture(const std::string& key);

    // Deletes a texture from cache and GPU
    bool destroy_texture(uint64_t id);
    bool destroy_texture_by_key(const std::string& key);

    // Clears all texture cache
    void clear_all();

    // Damage / dirty-region tracking
    void mark_damage(const DamageRect& rect);
    std::vector<DamageRect> consume_damage();
    bool has_damage() const { return !damage_rects_.empty(); }

    // Memory stats
    size_t total_cached_textures() const { return textures_by_id_.size(); }
    size_t total_memory_bytes() const { return total_allocated_bytes_; }
    size_t max_memory_bytes() const { return max_memory_limit_; }
    void set_max_memory_limit(size_t bytes) { max_memory_limit_ = bytes; }

    // GPU device availability
    bool is_gpu_available() const { return gpu_available_; }
    void set_gpu_available(bool available) { gpu_available_ = available; }

private:
    uint64_t next_id_ = 1001;
    uint32_t next_handle_ = 1;
    bool gpu_available_ = true;
    size_t total_allocated_bytes_ = 0;
    size_t max_memory_limit_ = 256 * 1024 * 1024; // 256 MB cache limit

    std::unordered_map<std::string, std::shared_ptr<GpuTexture>> textures_by_key_;
    std::unordered_map<uint64_t, std::shared_ptr<GpuTexture>> textures_by_id_;
    std::vector<DamageRect> damage_rects_;

    void evict_if_needed(size_t required_bytes);
};

} // namespace meridian::graphics

