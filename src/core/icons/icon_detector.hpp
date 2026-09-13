#pragma once
// src/core/icons/icon_detector.hpp
//
// Detects terminal capabilities, font features, and active icon theme.
// Controls automatic fallback tiers (Nerd Font -> Unicode -> ASCII).

#include <string>

namespace meridian::icons {

enum class IconTier {
    NERD_FONT,  // 10,000+ Nerd Font v3.x developer glyphs
    UNICODE,    // Safe BMP Unicode emoji / symbols
    ASCII       // Clean ASCII tokens for minimal / raw environments
};

enum class IconTheme {
    NerdFont,   // Full Nerd Font icons with ANSI colors
    Unicode,    // Emoji / Unicode symbols
    Minimal,    // Minimal glyph set
    Monochrome, // Nerd Font glyphs without ANSI color escapes
    Custom      // Loaded from ~/.config/meridian/icons.toml
};

class IconDetector {
public:
    // Detect the best supported icon tier from environment and terminal capabilities
    static IconTier detect_tier();

    // Check if the current environment explicitly or reliably supports Nerd Fonts
    static bool supports_nerd_fonts();

    // Check if the environment supports UTF-8 Unicode characters
    static bool supports_unicode();

    // Get current configured or detected icon theme
    static IconTheme get_active_theme();

    // Override icon theme programmatically
    static void set_active_theme(IconTheme theme);

    // String name for an IconTier
    static std::string tier_name(IconTier tier);

    // String name for an IconTheme
    static std::string theme_name(IconTheme theme);
};

} // namespace meridian::icons

