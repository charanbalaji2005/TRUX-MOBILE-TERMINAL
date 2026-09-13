// src/core/graphics/graphics_manager.cpp
#include "graphics_manager.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace meridian::graphics {

GraphicsManager& GraphicsManager::instance() {
    static GraphicsManager mgr;
    return mgr;
}

GraphicsManager::GraphicsManager() {
    gpu_available_ = true;
}

GraphicsManager::~GraphicsManager() {
    clear_all_images();
}

uint64_t GraphicsManager::add_image_file(
    const std::string& filepath,
    const ImageObject& config,
    std::string* error_out
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!gpu_available_) {
        if (error_out) *error_out = "Meridian: native graphics rendering is unavailable.";
        return 0;
    }

    auto decoded = ImageDecoder::decode_file(filepath);
    if (!decoded.is_valid()) {
        if (error_out) *error_out = "meridian: failed to decode image or unsupported format: " + filepath;
        return 0;
    }

    auto tex = texture_manager_.get_or_create_texture(filepath, decoded);
    if (!tex) {
        if (error_out) *error_out = "meridian: failed to allocate GPU texture for " + filepath;
        return 0;
    }

    ImageObject obj = config;
    obj.id = next_image_id_++;
    obj.gpu_texture_id = tex->id;
    obj.source_path = filepath;
    obj.format = decoded.format.empty() ? "RGBA8" : decoded.format;
    obj.original_width = decoded.original_width;
    obj.original_height = decoded.original_height;
    obj.has_alpha = tex->has_alpha;
    obj.is_animated = decoded.is_animated;

    if (obj.display_width <= 0.0f && obj.width_percentage <= 0.0f && obj.cols_spanned <= 0) {
        obj.display_width = static_cast<float>(obj.original_width);
    }
    if (obj.display_height <= 0.0f && obj.height_percentage <= 0.0f && obj.rows_spanned <= 0) {
        obj.display_height = static_cast<float>(obj.original_height);
    }

    images_.push_back(obj);
    return obj.id;
}

uint64_t GraphicsManager::add_image_memory(
    const uint8_t* data,
    size_t size,
    const std::string& hint,
    const ImageObject& config,
    std::string* error_out
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!gpu_available_) {
        if (error_out) *error_out = "Meridian: native graphics rendering is unavailable.";
        return 0;
    }

    auto decoded = ImageDecoder::decode_memory(data, size, hint);
    if (!decoded.is_valid()) {
        if (error_out) *error_out = "meridian: failed to decode in-memory image";
        return 0;
    }

    std::string cache_key = hint.empty() ? ("mem_" + std::to_string(next_image_id_)) : hint;
    auto tex = texture_manager_.get_or_create_texture(cache_key, decoded);
    if (!tex) {
        if (error_out) *error_out = "meridian: failed to allocate GPU texture for in-memory image";
        return 0;
    }

    ImageObject obj = config;
    obj.id = next_image_id_++;
    obj.gpu_texture_id = tex->id;
    obj.source_path = cache_key;
    obj.format = decoded.format;
    obj.original_width = decoded.original_width;
    obj.original_height = decoded.original_height;
    obj.has_alpha = tex->has_alpha;
    obj.is_animated = decoded.is_animated;

    images_.push_back(obj);
    return obj.id;
}

bool GraphicsManager::remove_image(uint64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(images_.begin(), images_.end(), [id](const ImageObject& o) {
        return o.id == id;
    });
    if (it != images_.end()) {
        images_.erase(it, images_.end());
        return true;
    }
    return false;
}

void GraphicsManager::clear_all_images() {
    std::lock_guard<std::mutex> lock(mutex_);
    images_.clear();
    texture_manager_.clear_all();
}

const ImageObject* GraphicsManager::get_image(uint64_t id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& img : images_) {
        if (img.id == id) return &img;
    }
    return nullptr;
}

std::vector<ImageObject> GraphicsManager::list_images() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return images_;
}

size_t GraphicsManager::image_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return images_.size();
}

void GraphicsManager::tick(double dt) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& img : images_) {
        if (img.is_animated) {
            img.frame_elapsed += dt;
        }
    }
    AnimationEngine::instance().tick(dt);
}

void GraphicsManager::on_scroll(int lines) {
    std::lock_guard<std::mutex> lock(mutex_);
    compositor_.on_terminal_scroll(lines, images_);
}

void GraphicsManager::on_resize(int cols, int rows, float cell_w, float cell_h) {
    std::lock_guard<std::mutex> lock(mutex_);
    compositor_.handle_resize(cols, rows, cell_w, cell_h);
}

bool GraphicsManager::is_gpu_available() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return gpu_available_;
}

void GraphicsManager::set_gpu_available(bool available) {
    std::lock_guard<std::mutex> lock(mutex_);
    gpu_available_ = available;
    texture_manager_.set_gpu_available(available);
}

bool GraphicsManager::generate_debug_report(const std::string& filepath, ImageDebugReport& report) {
    auto decoded = ImageDecoder::decode_file(filepath);
    if (!decoded.is_valid()) return false;

    std::string ext;
    size_t dot = filepath.find_last_of('.');
    if (dot != std::string::npos) ext = filepath.substr(dot + 1);
    if (ext.empty() && !decoded.format.empty()) {
        size_t d = decoded.format.find_last_of('.');
        ext = (d != std::string::npos) ? decoded.format.substr(d + 1) : decoded.format;
    }
    if (ext.empty()) ext = "PNG";
    std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
    if (ext == "JPG") ext = "JPEG";
    report.format = ext;
    report.original_width = decoded.original_width;
    report.original_height = decoded.original_height;
    report.decoded_format = "RGBA8";
    report.texture_width = decoded.original_width;
    report.texture_height = decoded.original_height;
    report.position_x = 0.0f;
    report.position_y = 0.0f;
    report.display_width = decoded.original_width > 0 ? (decoded.original_width / 2.0f) : 0.0f;
    report.display_height = decoded.original_height > 0 ? (decoded.original_height / 2.0f) : 0.0f;
    report.gpu_enabled = gpu_available_;
    return true;
}

} // namespace meridian::graphics
