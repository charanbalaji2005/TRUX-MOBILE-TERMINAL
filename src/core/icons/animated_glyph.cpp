// src/core/icons/animated_glyph.cpp
#include "animated_glyph.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <sstream>

namespace meridian::icons {

namespace {
double current_time_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<double>(ts.tv_sec) + ts.tv_nsec / 1e9;
}
} // namespace

std::vector<std::string> Spinner::get_frames_for_style(SpinnerStyle style) {
    switch (style) {
        case SpinnerStyle::Dots:
            return {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
        case SpinnerStyle::Circles:
            return {"◌", "◔", "◑", "◕", "●"};
        case SpinnerStyle::Braille:
            return {"⣾", "⣽", "⣻", "⢿", "⡿", "⣟", "⣯", "⣷"};
        case SpinnerStyle::Lines:
            return {"|", "/", "-", "\\"};
        case SpinnerStyle::Pulse:
            return {"░", "▒", "▓", "█", "▓", "▒", "░"};
        case SpinnerStyle::Arrows:
            return {"←", "↖", "↑", "↗", "→", "↘", "↓", "↙"};
        case SpinnerStyle::Grow:
            return {" ", "▃", "▄", "▅", "▆", "▇", "█", "▇", "▆", "▅", "▄", "▃"};
    }
    return {"⣾", "⣽", "⣻", "⢿", "⡿", "⣟", "⣯", "⣷"};
}

Spinner::Spinner(SpinnerStyle style, double fps)
    : fps_(fps) {
    set_style(style);
}

void Spinner::set_style(SpinnerStyle style) {
    frames_ = get_frames_for_style(style);
}

std::string Spinner::frame() const {
    return frame(current_time_seconds());
}

std::string Spinner::frame(double elapsed_seconds) const {
    if (frames_.empty()) return "";
    size_t index = static_cast<size_t>(std::floor(elapsed_seconds * fps_)) % frames_.size();
    return frames_[index];
}

std::string Spinner::frame_at_index(size_t index) const {
    if (frames_.empty()) return "";
    return frames_[index % frames_.size()];
}

AnimatedIcon::AnimatedIcon(AnimatedIconType type)
    : type_(type), spinner_(SpinnerStyle::Braille) {
    set_type(type);
}

void AnimatedIcon::set_type(AnimatedIconType type) {
    type_ = type;
    switch (type) {
        case AnimatedIconType::Connecting:
            spinner_.set_style(SpinnerStyle::Circles);
            spinner_.set_fps(4.0);
            break;
        case AnimatedIconType::Loading:
            spinner_.set_style(SpinnerStyle::Dots);
            spinner_.set_fps(10.0);
            break;
        case AnimatedIconType::Building:
            spinner_.set_style(SpinnerStyle::Braille);
            spinner_.set_fps(8.0);
            break;
        case AnimatedIconType::GitSync:
            spinner_.set_style(SpinnerStyle::Arrows);
            spinner_.set_fps(6.0);
            break;
        case AnimatedIconType::Success:
        case AnimatedIconType::Error:
            spinner_.set_style(SpinnerStyle::Dots);
            break;
    }
}

std::string AnimatedIcon::frame() const {
    if (type_ == AnimatedIconType::Success) return "✓";
    if (type_ == AnimatedIconType::Error) return "✖";
    return spinner_.frame();
}

std::string AnimatedIcon::frame(double elapsed_seconds) const {
    if (type_ == AnimatedIconType::Success) return "✓";
    if (type_ == AnimatedIconType::Error) return "✖";
    return spinner_.frame(elapsed_seconds);
}

std::string ProgressIndicator::render_bar(
    double progress,
    int width,
    const std::string& filled_color,
    const std::string& empty_color
) {
    if (width <= 0) return "";
    progress = std::clamp(progress, 0.0, 1.0);

    // 8 sub-cell blocks: '▏', '▎', '▍', '▌', '▋', '▊', '▉', '█'
    static const char* kSubBlocks[] = {"", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};

    double total_units = progress * width;
    int full_blocks = static_cast<int>(std::floor(total_units));
    int sub_index = static_cast<int>(std::floor((total_units - full_blocks) * 8.0));
    sub_index = std::clamp(sub_index, 0, 8);

    std::ostringstream ss;
    ss << filled_color;
    for (int i = 0; i < full_blocks && i < width; ++i) {
        ss << "█";
    }

    if (full_blocks < width && sub_index > 0) {
        ss << kSubBlocks[sub_index];
        full_blocks++;
    }

    ss << empty_color;
    for (int i = full_blocks; i < width; ++i) {
        ss << "░";
    }
    ss << "\033[0m";

    return ss.str();
}

} // namespace meridian::icons

