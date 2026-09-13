#pragma once
// src/core/graphics/terminal_graphic.hpp
//
// Core terminal graphic representation. Represents an authentic raster image,
// animated GIF, Kitty graphic, or Sixel graphic object on the terminal canvas.

#include "image_decoder.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace meridian::graphics {

enum class GraphicType {
    Image,
    Animation,
    KittyGraphics,
    Sixel,
    Ascii
};

struct TerminalGraphic {
    uint64_t id = 0;
    GraphicType type = GraphicType::Image;

    // Coordinate & Box Geometry
    float x = 0.0f;          // Canvas pixel X (or relative)
    float y = 0.0f;          // Canvas pixel Y (or relative)
    float width = 0.0f;      // Display pixel width
    float height = 0.0f;     // Display pixel height

    int row = 0;             // Terminal grid cell row
    int column = 0;          // Terminal grid cell column
    int cols_spanned = 0;    // Width in character cells
    int rows_spanned = 0;    // Height in character cells

    int z_index = 0;         // < 0: under text cells, >= 0: over text cells
    float opacity = 1.0f;    // 0.0f (transparent) to 1.0f (opaque)
    bool visible = true;
    bool scroll_with_terminal = false;

    ImageFitMode fit_mode = ImageFitMode::Contain;
    ImageScaleFilter filter_mode = ImageScaleFilter::Smooth;

    // Frame Buffers (for static images or animated GIFs)
    std::vector<ImageFrame> frames;
    size_t current_frame = 0;
    double frame_elapsed = 0.0;

    bool is_animated() const {
        return frames.size() > 1;
    }

    const ImageFrame& active_frame() const {
        if (frames.empty()) {
            static ImageFrame empty;
            return empty;
        }
        return frames[current_frame % frames.size()];
    }

    void advance_animation(double dt) {
        if (!is_animated()) return;
        frame_elapsed += dt;
        double cur_duration = frames[current_frame % frames.size()].duration;
        if (cur_duration <= 0.0) cur_duration = 0.1;

        if (frame_elapsed >= cur_duration) {
            frame_elapsed -= cur_duration;
            current_frame = (current_frame + 1) % frames.size();
        }
    }
};

class GraphicManager {
public:
    GraphicManager();

    // Add or replace a graphic
    void add_graphic(TerminalGraphic graphic);
    TerminalGraphic* find_graphic(uint64_t id);
    bool remove_graphic(uint64_t id);
    void clear();

    std::vector<TerminalGraphic>& graphics() { return graphics_; }
    const std::vector<TerminalGraphic>& graphics() const { return graphics_; }

    // Advance animation timers
    void tick(double dt);

    // Get default startup artwork graphic
    static TerminalGraphic create_startup_artwork_graphic(
        float x = 28.0f, float y = 32.0f,
        float w = 220.0f, float h = 170.0f
    );

private:
    std::vector<TerminalGraphic> graphics_;
    uint64_t next_id_ = 1;
};

} // namespace meridian::graphics

