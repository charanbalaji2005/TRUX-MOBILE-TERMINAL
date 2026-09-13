#pragma once
// src/core/icons/animated_glyph.hpp
//
// Lightweight terminal glyph animations for status indicators, loading states,
// Git sync operations, and progress indicators.
// NOTE: These are terminal text glyph animations, NOT raster images.

#include <string>
#include <vector>
#include <cstdint>

namespace meridian::icons {

enum class SpinnerStyle {
    Dots,         // ⠋ ⠙ ⠹ ⠸ ⠼ ⠴ ⠦ ⠧ ⠇ ⠏
    Circles,      // ◌ ◔ ◑ ◕ ●
    Braille,      // ⣾ ⣽ ⣻ ⢿ ⡿ ⣟ ⣯ ⣷
    Lines,        // | / - |
    Pulse,        // ░ ▒ ▓ █ ▓ ▒ ░
    Arrows,       // ← ↖ ↑ ↗ → ↘ ↓ ↙
    Grow          //   ▃ ▄ ▅ ▆ ▇ █ ▇ ▆ ▅ ▄ ▃
};

class Spinner {
public:
    explicit Spinner(SpinnerStyle style = SpinnerStyle::Braille, double fps = 10.0);

    // Get the current frame at the given timestamp or wall-clock time
    std::string frame() const;
    std::string frame(double elapsed_seconds) const;
    std::string frame_at_index(size_t index) const;

    size_t frame_count() const { return frames_.size(); }
    void set_style(SpinnerStyle style);
    void set_fps(double fps) { fps_ = fps; }

    static std::vector<std::string> get_frames_for_style(SpinnerStyle style);

private:
    std::vector<std::string> frames_;
    double fps_ = 10.0;
};

enum class AnimatedIconType {
    Connecting,   // ◌ ◔ ◑ ◕ ●
    Loading,      // ⠋ ⠙ ⠹ ⠸ ⠼ ⠴ ⠦ ⠧ ⠇ ⠏
    Building,     // 🔨 ⚙ 🔧 ⚙
    GitSync,      // ↑ ↗ → ↘ ↓ ↙ ← ↖
    Success,      // ✓ (static or sparkle)
    Error         // ✖
};

class AnimatedIcon {
public:
    explicit AnimatedIcon(AnimatedIconType type = AnimatedIconType::Loading);

    std::string frame() const;
    std::string frame(double elapsed_seconds) const;

    void set_type(AnimatedIconType type);

private:
    AnimatedIconType type_;
    Spinner spinner_;
};

class ProgressIndicator {
public:
    // Render smooth sub-character resolution progress bar
    // progress: 0.0 to 1.0
    // width: number of terminal character cells
    static std::string render_bar(
        double progress,
        int width = 20,
        const std::string& filled_color = "\033[38;2;34;197;94m",
        const std::string& empty_color = "\033[38;2;60;70;90m"
    );
};

} // namespace meridian::icons
