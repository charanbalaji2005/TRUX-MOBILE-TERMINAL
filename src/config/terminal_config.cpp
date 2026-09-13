// meridian-config / terminal_config.cpp
#include "terminal_config.hpp"
#include <cstdlib>

namespace meridian::config {

ColorPalette ColorPalette::vivid_neon() {
    ColorPalette p;
    p.background = "#090B10";
    p.foreground = "#F5F7FA";
    p.neon_cyan = "#00E5FF";
    p.electric_blue = "#3B82F6";
    p.neon_purple = "#A855F7";
    p.hot_pink = "#F43F5E";
    p.neon_green = "#22C55E";
    p.amber = "#F59E0B";
    p.yellow = "#FDE047";
    p.orange = "#FB923C";
    return p;
}

ColorPalette ColorPalette::dark_nord() {
    ColorPalette p;
    p.background = "#2e3440";
    p.foreground = "#d8dee9";
    p.neon_cyan = "#88c0d0";
    p.electric_blue = "#81a1c1";
    p.neon_purple = "#b48ead";
    p.hot_pink = "#bf616a";
    p.neon_green = "#a3be8c";
    p.amber = "#ebcb8b";
    p.yellow = "#ebcb8b";
    p.orange = "#d08770";
    return p;
}

TerminalConfig::TerminalConfig(std::string path) : config_(std::move(path)) {}

void TerminalConfig::settings_from_config() {
    settings_.scrollback_lines = std::atoi(config_.get("scrollback_lines", "10000").c_str());
    settings_.default_shell = config_.get("default_shell", "/bin/bash");
    settings_.startup_directory = config_.get("startup_directory", "");
    settings_.font_family = config_.get("font_family", "monospace");
    settings_.font_size = std::atoi(config_.get("font_size", "13").c_str());
    settings_.theme_name = config_.get("theme_name", "vivid-neon");
    settings_.cursor_style = config_.get("cursor_style", "block");
    settings_.cursor_blink = config_.get_bool("cursor_blink", true);
    settings_.tab_bar_visible = config_.get_bool("tab_bar_visible", true);
    settings_.padding = std::atoi(config_.get("padding", "4").c_str());

    if (settings_.theme_name == "dark-nord") {
        settings_.palette = ColorPalette::dark_nord();
    } else {
        settings_.palette = ColorPalette::vivid_neon();
    }
}

void TerminalConfig::settings_to_config() {
    config_.set("scrollback_lines", std::to_string(settings_.scrollback_lines));
    config_.set("default_shell", settings_.default_shell);
    config_.set("startup_directory", settings_.startup_directory);
    config_.set("font_family", settings_.font_family);
    config_.set("font_size", std::to_string(settings_.font_size));
    config_.set("theme_name", settings_.theme_name);
    config_.set("cursor_style", settings_.cursor_style);
    config_.set_bool("cursor_blink", settings_.cursor_blink);
    config_.set_bool("tab_bar_visible", settings_.tab_bar_visible);
    config_.set("padding", std::to_string(settings_.padding));
}

bool TerminalConfig::load() {
    bool existed = config_.load();
    settings_from_config();
    return existed;
}

bool TerminalConfig::save() {
    settings_to_config();
    return config_.save();
}

} // namespace meridian::config
