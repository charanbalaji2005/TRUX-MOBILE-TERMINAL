// meridian-config / keybindings.cpp
#include "keybindings.hpp"

namespace meridian::config {

Keybindings::Keybindings(std::string path) : config_(std::move(path)) {
    // Defaults straight from the spec's own examples: §42 (Ctrl+Space
    // for the AI popup), §23 (Ctrl+Shift+F search), §18/§19 (tabs/panes).
    defaults_ = {
        {"ai_popup", "Ctrl+Space"},
        {"search", "Ctrl+Shift+F"},
        {"new_tab", "Ctrl+T"},
        {"close_tab", "Ctrl+W"},
        {"next_tab", "Ctrl+Tab"},
        {"prev_tab", "Ctrl+Shift+Tab"},
        {"split_horizontal", "Ctrl+Shift+H"},
        {"split_vertical", "Ctrl+Shift+V"},
        {"focus_next_pane", "Ctrl+Alt+Right"},
        {"focus_prev_pane", "Ctrl+Alt+Left"},
        {"close_pane", "Ctrl+Shift+W"},
        {"copy", "Ctrl+Shift+C"},
        {"paste", "Ctrl+Shift+V"},
        {"increase_font_size", "Ctrl+Plus"},
        {"decrease_font_size", "Ctrl+Minus"},
    };
}

bool Keybindings::load() { return config_.load(); }
bool Keybindings::save() { return config_.save(); }

std::string Keybindings::get(const std::string& action) const {
    std::string default_combo;
    auto it = defaults_.find(action);
    if (it != defaults_.end()) default_combo = it->second;
    return config_.get("keybind." + action, default_combo);
}

void Keybindings::set(const std::string& action, const std::string& combo) {
    config_.set("keybind." + action, combo);
}

std::map<std::string, std::string> Keybindings::all() const {
    std::map<std::string, std::string> result;
    for (const auto& [action, default_combo] : defaults_) {
        result[action] = config_.get("keybind." + action, default_combo);
    }
    return result;
}

} // namespace meridian::config
