// src/core/icons/icon_detector.cpp
#include "icon_detector.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

namespace meridian::icons {

namespace {
static IconTheme s_active_theme = IconTheme::NerdFont;
static bool s_theme_overridden = false;
} // namespace

IconTier IconDetector::detect_tier() {
    // 1. Explicit override via environment variable
    const char* env_override = std::getenv("MERIDIAN_ICON_TIER");
    if (env_override) {
        std::string o = env_override;
        std::transform(o.begin(), o.end(), o.begin(), ::toupper);
        if (o == "NERD_FONT" || o == "NERDFONT" || o == "NF") return IconTier::NERD_FONT;
        if (o == "UNICODE" || o == "EMOJI") return IconTier::UNICODE;
        if (o == "ASCII" || o == "PLAIN" || o == "OFF" || o == "NONE") return IconTier::ASCII;
    }

    // 2. Explicit theme setting
    if (s_theme_overridden) {
        switch (s_active_theme) {
            case IconTheme::NerdFont:
            case IconTheme::Monochrome:
            case IconTheme::Custom:
                return IconTier::NERD_FONT;
            case IconTheme::Unicode:
                return IconTier::UNICODE;
            case IconTheme::Minimal:
                return IconTier::ASCII;
        }
    }

    // 3. Known Nerd-Font capable terminals and environments
    if (supports_nerd_fonts()) {
        return IconTier::NERD_FONT;
    }

    // 4. Unicode fallback if UTF-8 locale is present
    if (supports_unicode()) {
        return IconTier::UNICODE;
    }

    // 5. Ultimate ASCII fallback
    return IconTier::ASCII;
}

bool IconDetector::supports_nerd_fonts() {
    // Check explicit Nerd Font indicator variable
    if (std::getenv("NERD_FONTS_VERSION")) return true;
    if (std::getenv("MERIDIAN_NERD_FONT")) return true;

    // Kitty supports graphics and modern Nerd Fonts
    if (std::getenv("KITTY_WINDOW_ID") || std::getenv("KITTY_PID")) return true;

    // WezTerm, Alacritty, Ghostty, Foot, Rio
    if (std::getenv("WEZTERM_EXECUTABLE") || std::getenv("WEZTERM_PANE")) return true;
    if (std::getenv("ALACRITTY_LOG") || std::getenv("ALACRITTY_WINDOW_ID")) return true;
    if (std::getenv("GHOSTTY_RESOURCES_DIR")) return true;
    if (std::getenv("FOOT_SERVER_PATH")) return true;

    // Modern Terminal programs
    const char* term_prog = std::getenv("TERM_PROGRAM");
    if (term_prog) {
        std::string tp = term_prog;
        if (tp == "WezTerm" || tp == "ghostty" || tp == "Alacritty" ||
            tp == "vscode" || tp == "iTerm.app" || tp == "Hyper" ||
            tp == "warp" || tp == "rio" || tp == "Rio" || tp == "Tabby") {
            return true;
        }
    }

    // Modern Wayland compositors and developer terminals
    if (std::getenv("HYPRLAND_INSTANCE_SIGNATURE")) return true;
    if (std::getenv("SWAYSOCK")) return true;
    if (std::getenv("WAYLAND_DISPLAY")) return true;

    // Rich modern TERM definitions
    const char* term = std::getenv("TERM");
    if (term) {
        std::string t = term;
        if (t.find("xterm-kitty") != std::string::npos ||
            t.find("xterm-ghostty") != std::string::npos ||
            t.find("wezterm") != std::string::npos ||
            t.find("alacritty") != std::string::npos ||
            t.find("foot") != std::string::npos ||
            t.find("256color") != std::string::npos ||
            t.find("xterm-direct") != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool IconDetector::supports_unicode() {
    const char* lang = std::getenv("LANG");
    const char* lc_all = std::getenv("LC_ALL");
    const char* lc_ctype = std::getenv("LC_CTYPE");

    auto is_utf8 = [](const char* s) -> bool {
        if (!s) return false;
        std::string str = s;
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str.find("utf-8") != std::string::npos || str.find("utf8") != std::string::npos;
    };

    if (is_utf8(lc_all) || is_utf8(lang) || is_utf8(lc_ctype)) return true;

    // Default POSIX / Linux environment with modern display
    if (std::getenv("DISPLAY") || std::getenv("WAYLAND_DISPLAY") || std::getenv("WSL_DISTRO_NAME")) {
        return true;
    }

    return false;
}

IconTheme IconDetector::get_active_theme() {
    return s_active_theme;
}

void IconDetector::set_active_theme(IconTheme theme) {
    s_active_theme = theme;
    s_theme_overridden = true;
}

std::string IconDetector::tier_name(IconTier tier) {
    switch (tier) {
        case IconTier::NERD_FONT: return "NERD_FONT";
        case IconTier::UNICODE:   return "UNICODE";
        case IconTier::ASCII:     return "ASCII";
    }
    return "UNKNOWN";
}

std::string IconDetector::theme_name(IconTheme theme) {
    switch (theme) {
        case IconTheme::NerdFont:   return "nerd-font";
        case IconTheme::Unicode:    return "unicode";
        case IconTheme::Minimal:    return "minimal";
        case IconTheme::Monochrome: return "monochrome";
        case IconTheme::Custom:     return "custom";
    }
    return "unknown";
}

} // namespace meridian::icons

