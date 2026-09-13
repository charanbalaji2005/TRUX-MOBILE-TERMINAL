// src/core/graphics/gpu_texture_manager.cpp
#include "gpu_texture_manager.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace meridian::graphics {

GpuTextureManager::GpuTextureManager() = default;

GpuTextureManager::~GpuTextureManager() {
    clear_all();
}

std::shared_ptr<GpuTexture> GpuTextureManager::get_or_create_texture(
    const std::string& key,
    const DecodedImage& image,
    size_t frame_index
) {
    if (!image.is_valid()) return nullptr;
    if (frame_index >= image.frames.size()) frame_index = 0;

    auto existing = find_texture(key);
    if (existing) {
        return existing;
    }

    const auto& frame = image.frame(frame_index);
    if (!frame.is_valid()) return nullptr;

    return create_texture_from_rgba(key, frame.rgba.data(), frame.width, frame.height);
}

std::shared_ptr<GpuTexture> GpuTextureManager::create_texture_from_rgba(
    const std::string& key,
    const uint8_t* data,
    int width,
    int height
) {
    if (!data || width <= 0 || height <= 0) return nullptr;

    size_t needed_bytes = static_cast<size_t>(width) * height * 4;
    evict_if_needed(needed_bytes);

    auto tex = std::make_shared<GpuTexture>();
    tex->id = next_id_++;
    tex->key = key;
    tex->width = width;
    tex->height = height;
    tex->handle = next_handle_++;
    tex->rgba.assign(data, data + needed_bytes);
    tex->is_uploaded = true;
    tex->generation = 1;

    // Check for alpha channel
    tex->has_alpha = false;
    for (size_t i = 3; i < needed_bytes; i += 4) {
        if (data[i] < 255) {
            tex->has_alpha = true;
            break;
        }
    }

    total_allocated_bytes_ += needed_bytes;
    textures_by_id_[tex->id] = tex;
    if (!key.empty()) {
        textures_by_key_[key] = tex;
    }

    mark_damage({0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)});
    return tex;
}

std::shared_ptr<GpuTexture> GpuTextureManager::find_texture(const std::string& key) {
    if (key.empty()) return nullptr;
    auto it = textures_by_key_.find(key);
    if (it != textures_by_key_.end()) {
        return it->second;
    }
    return nullptr;
}

bool GpuTextureManager::destroy_texture(uint64_t id) {
    auto it = textures_by_id_.find(id);
    if (it == textures_by_id_.end()) return false;

    auto tex = it->second;
    total_allocated_bytes_ -= tex->memory_bytes();
    if (!tex->key.empty()) {
        textures_by_key_.erase(tex->key);
    }
    textures_by_id_.erase(it);
    return true;
}

bool GpuTextureManager::destroy_texture_by_key(const std::string& key) {
    auto it = textures_by_key_.find(key);
    if (it == textures_by_key_.end()) return false;

    uint64_t id = it->second->id;
    return destroy_texture(id);
}

void GpuTextureManager::clear_all() {
    textures_by_key_.clear();
    textures_by_id_.clear();
    damage_rects_.clear();
    total_allocated_bytes_ = 0;
}

void GpuTextureManager::mark_damage(const DamageRect& rect) {
    damage_rects_.push_back(rect);
}

std::vector<DamageRect> GpuTextureManager::consume_damage() {
    std::vector<DamageRect> out = std::move(damage_rects_);
    damage_rects_.clear();
    return out;
}

void GpuTextureManager::evict_if_needed(size_t required_bytes) {
    if (total_allocated_bytes_ + required_bytes <= max_memory_limit_) return;

    // Evict oldest textures until sufficient memory is available
    std::vector<uint64_t> ids_to_remove;
    for (const auto& [id, tex] : textures_by_id_) {
        ids_to_remove.push_back(id);
        if (total_allocated_bytes_ + required_bytes <= max_memory_limit_) break;
    }

    for (uint64_t id : ids_to_remove) {
        destroy_texture(id);
    }
}

} // namespace meridian::graphics

