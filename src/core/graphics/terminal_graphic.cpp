#include "terminal_graphic.hpp"
#include "../chainsaw_man_data.inl"

#include <algorithm>
#include <fstream>

namespace meridian::graphics {

GraphicManager::GraphicManager() = default;

void GraphicManager::add_graphic(TerminalGraphic graphic) {
    if (graphic.id == 0) {
        graphic.id = next_id_++;
    }
    // Remove if already exists with same ID
    remove_graphic(graphic.id);
    graphics_.push_back(std::move(graphic));
}

TerminalGraphic* GraphicManager::find_graphic(uint64_t id) {
    for (auto& g : graphics_) {
        if (g.id == id) return &g;
    }
    return nullptr;
}

bool GraphicManager::remove_graphic(uint64_t id) {
    auto it = std::remove_if(graphics_.begin(), graphics_.end(), [id](const TerminalGraphic& g) {
        return g.id == id;
    });
    if (it != graphics_.end()) {
        graphics_.erase(it, graphics_.end());
        return true;
    }
    return false;
}

void GraphicManager::clear() {
    graphics_.clear();
}

void GraphicManager::tick(double dt) {
    for (auto& g : graphics_) {
        if (g.visible && g.is_animated()) {
            g.advance_animation(dt);
        }
    }
}

TerminalGraphic GraphicManager::create_startup_artwork_graphic(float x, float y, float w, float h) {
    TerminalGraphic g;
    g.id = 1001;
    g.type = GraphicType::Image;
    g.x = x;
    g.y = y;
    g.width = w;
    g.height = h;
    g.row = 1;
    g.column = 2;
    g.cols_spanned = 24;
    g.rows_spanned = 10;
    g.z_index = -1; // Under text by default
    g.opacity = 1.0f;
    g.visible = true;
    g.scroll_with_terminal = false;
    g.fit_mode = ImageFitMode::Contain;
    g.filter_mode = ImageScaleFilter::Smooth;

    // 1. Try decoding real file if present
    std::string candidate_paths[] = {
        "resources/images/artwork.jpg",
        "resources/images/artwork_thumb.ppm",
        "resources/images/artwork.png"
    };

    bool loaded = false;
    for (const auto& path : candidate_paths) {
        DecodedImage dec = ImageDecoder::decode_file(path);
        if (dec.is_valid()) {
            g.frames = std::move(dec.frames);
            loaded = true;
            break;
        }
    }

    // 2. If no file, use compiled Chainsaw Man pixel buffer
    if (!loaded) {
        ImageFrame frame;
        frame.width = 48;
        frame.height = 44;
        frame.duration = 0.1;
        frame.rgba.resize(48 * 44 * 4);
        for (int i = 0; i < 48 * 44; ++i) {
            frame.rgba[i * 4 + 0] = kChainsawManPixels[i * 3 + 0];
            frame.rgba[i * 4 + 1] = kChainsawManPixels[i * 3 + 1];
            frame.rgba[i * 4 + 2] = kChainsawManPixels[i * 3 + 2];
            frame.rgba[i * 4 + 3] = 255;
        }
        g.frames.push_back(std::move(frame));
    }

    return g;
}

} // namespace meridian::graphics

