#pragma once
// src/core/graphics/graphics_manager.hpp
//
// Central Native GPU Graphics Subsystem for Meridian Terminal.
// Unifies ImageDecoder, GpuTextureManager, TerminalImageCompositor,
// GraphicsProtocolParser, and AnimationEngine.

#include "image_decoder.hpp"
#include "image_object.hpp"
#include "gpu_texture_manager.hpp"
#include "terminal_image_compositor.hpp"
#include "graphics_protocol_parser.hpp"
#include "animation_engine.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace meridian::graphics {

struct ImageDebugReport {
    std::string format;
    int original_width = 0;
    int original_height = 0;
    std::string decoded_format = "RGBA8";
    int texture_width = 0;
    int texture_height = 0;
    float position_x = 0.0f;
    float position_y = 0.0f;
    float display_width = 0.0f;
    float display_height = 0.0f;
    bool gpu_enabled = true;
};

class GraphicsManager {
public:
    static GraphicsManager& instance();

    GraphicsManager();
    ~GraphicsManager();

    // Adds and loads an image from disk into GPU memory as an ImageObject
    uint64_t add_image_file(
        const std::string& filepath,
        const ImageObject& config,
        std::string* error_out = nullptr
    );

    // Adds and loads raw image bytes into GPU memory
    uint64_t add_image_memory(
        const uint8_t* data,
        size_t size,
        const std::string& hint,
        const ImageObject& config,
        std::string* error_out = nullptr
    );

    // Image Object Management
    bool remove_image(uint64_t id);
    void clear_all_images();
    const ImageObject* get_image(uint64_t id) const;
    std::vector<ImageObject> list_images() const;
    size_t image_count() const;

    // Subsystem Accessors
    GpuTextureManager& texture_manager() { return texture_manager_; }
    const GpuTextureManager& texture_manager() const { return texture_manager_; }

    TerminalImageCompositor& compositor() { return compositor_; }
    const TerminalImageCompositor& compositor() const { return compositor_; }

    GraphicsProtocolParser& protocol_parser() { return protocol_parser_; }
    AnimationEngine& animation_engine() { return AnimationEngine::instance(); }

    // Lifecycle, Scroll, & Resize events
    void tick(double dt);
    void on_scroll(int lines);
    void on_resize(int cols, int rows, float cell_w, float cell_h);

    // Hardware capability check
    bool is_gpu_available() const;
    void set_gpu_available(bool available);

    // Debug metadata reporting
    bool generate_debug_report(const std::string& filepath, ImageDebugReport& report);

private:
    mutable std::mutex mutex_;
    uint64_t next_image_id_ = 1;
    bool gpu_available_ = true;

    GpuTextureManager texture_manager_;
    TerminalImageCompositor compositor_;
    GraphicsProtocolParser protocol_parser_;
    std::vector<ImageObject> images_;
};

} // namespace meridian::graphics
