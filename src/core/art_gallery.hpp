#pragma once
// src/core/art_gallery.hpp
//
// Curated high-resolution anime, cyberpunk, synthwave & space artwork gallery
// for Meridian Terminal. Supports permanent theme selection, custom image upload, and rotation.

#include "terminal_image.hpp"
#include <string>
#include <vector>
#include <utility>

namespace meridian::core {

struct ArtworkTheme {
    std::string id;
    std::string title;
    TerminalImage image;
};

class ArtGallery {
public:
    // Returns the active artwork (checks user permanent choice or rotates randomly)
    static ArtworkTheme get_active_artwork(int width = 56, int height = 22);

    // Returns a fresh rotated/random artwork theme
    static ArtworkTheme get_next_artwork(int width = 56, int height = 22);

    // Returns a specific artwork by index
    static ArtworkTheme get_artwork_by_index(size_t index, int width = 56, int height = 22);

    // Returns a specific artwork by theme ID or custom file path
    static ArtworkTheme get_artwork_by_id_or_file(const std::string& id_or_file, int width = 56, int height = 22);

    // Total count of curated themes
    static size_t theme_count();

    // List of all theme IDs and Titles
    static std::vector<std::pair<std::string, std::string>> list_themes();

    // Gets the current permanent configuration ("random", theme ID, or custom path)
    static std::string get_configured_choice();

    // Sets the permanent configuration ("random", theme ID, or custom path)
    static bool set_permanent_choice(const std::string& choice);

    // Renders the artwork as clean, high-definition TrueColor halfblock lines (28 cols x 11 rows)
    static std::vector<std::string> render_artwork_lines(const TerminalImage& img, int target_cols = 28, int target_rows = 11);
};

} // namespace meridian::core
