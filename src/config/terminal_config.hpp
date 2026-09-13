#pragma once
// meridian-config / terminal_config.hpp
//
// Spec §75's terminal.toml settings with vivid neon color palette definitions,
// scrollback size, default shell, font, cursor style, and theme management.

#include "../core/config.hpp"
#include <string>

namespace meridian::config {

struct ColorPalette {
    std::string background = "#090B10";
    std::string foreground = "#F5F7FA";
    std::string neon_cyan = "#00E5FF";
    std::string electric_blue = "#3B82F6";
    std::string neon_purple = "#A855F7";
    std::string hot_pink = "#F43F5E";
    std::string neon_green = "#22C55E";
    std::string amber = "#F59E0B";
    std::string yellow = "#FDE047";
    std::string orange = "#FB923C";

    static ColorPalette vivid_neon();
    static ColorPalette dark_nord();
};

struct TerminalSettings {
    // --- Meaningful right now, independent of any GUI ---
    int scrollback_lines = 10000;      // used directly by ScreenBuffer's constructor
    std::string default_shell = "/bin/bash"; // what PtyManager spawns when none is specified
    std::string startup_directory;      // empty = inherit cwd

    // --- Recorded now; only take effect once a renderer exists ---
    std::string font_family = "monospace";
    int font_size = 13;
    std::string theme_name = "vivid-neon";
    std::string cursor_style = "block"; // block | underline | bar
    bool cursor_blink = true;
    bool tab_bar_visible = true;
    int padding = 4;

    ColorPalette palette = ColorPalette::vivid_neon();
};

class TerminalConfig {
public:
    explicit TerminalConfig(std::string path);

    bool load();
    bool save();

    TerminalSettings get() const { return settings_; }
    void set(const TerminalSettings& s) { settings_ = s; }

    const std::string& path() const { return config_.path(); }

private:
    Config config_;
    TerminalSettings settings_;

    void settings_from_config();
    void settings_to_config();
};

} // namespace meridian::config
