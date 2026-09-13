#include "mini_test.hpp"
#include "../src/core/graphics/pixel_art_renderer.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

using namespace meridian::graphics;

MTEST(pixel_art_sobel_edge_detection) {
    int w = 10, h = 10;
    std::vector<PixelRgba> pixels(w * h, {0, 0, 0, 255}); // Black background

    // Draw a bright white vertical line in the middle
    for (int y = 0; y < h; ++y) {
        pixels[y * w + 5] = {255, 255, 255, 255};
    }

    auto edges = PixelArtRenderer::compute_sobel_edges(pixels, w, h);
    ASSERT_EQ(edges.size(), static_cast<size_t>(w * h));

    // Edges around column 4, 5, 6 should have high gradient magnitude
    float edge_mag = edges[5 * w + 4];
    ASSERT_TRUE(edge_mag > 0.3f);
}

MTEST(pixel_art_color_quantization_median_cut) {
    int w = 8, h = 8;
    std::vector<PixelRgba> pixels(w * h);

    // Create 64 distinct subtle variations of red and blue
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (y < 4) {
                pixels[y * w + x] = {static_cast<uint8_t>(200 + x * 5), 10, 10, 255};
            } else {
                pixels[y * w + x] = {10, 10, static_cast<uint8_t>(200 + x * 5), 255};
            }
        }
    }

    // Quantize down to 4 discrete colors
    auto quantized = PixelArtRenderer::quantize_colors(pixels, 4, false, w, h);
    ASSERT_EQ(quantized.size(), static_cast<size_t>(w * h));

    // Count unique colors in quantized output
    std::vector<PixelRgba> unique_colors;
    for (const auto& p : quantized) {
        if (std::find(unique_colors.begin(), unique_colors.end(), p) == unique_colors.end()) {
            unique_colors.push_back(p);
        }
    }

    ASSERT_TRUE(unique_colors.size() <= 4);
}

MTEST(pixel_art_halfblock_ansi_emission) {
    int w = 4, h = 4;
    std::vector<PixelRgba> pixels(w * h, {255, 0, 0, 255}); // Red top
    for (int x = 0; x < w; ++x) {
        pixels[1 * w + x] = {0, 255, 0, 255}; // Green bottom
    }

    std::string ansi = PixelArtRenderer::emit_ansi_halfblocks(pixels, w, h, true);
    ASSERT_TRUE(!ansi.empty());
    ASSERT_TRUE(ansi.find("▀") != std::string::npos);
    ASSERT_TRUE(ansi.find("\033[38;2;") != std::string::npos); // TrueColor fg
    ASSERT_TRUE(ansi.find("\033[48;2;") != std::string::npos); // TrueColor bg
}

MTEST(pixel_art_ascii_and_braille_emission) {
    int w = 8, h = 8;
    std::vector<PixelRgba> pixels(w * h, {200, 200, 200, 255});

    std::string ascii = PixelArtRenderer::emit_ascii(pixels, w, h, false, " .:-=+*#%@");
    ASSERT_TRUE(!ascii.empty());
    ASSERT_TRUE(ascii.find("\n") != std::string::npos);

    std::string braille = PixelArtRenderer::emit_braille(pixels, w, h);
    ASSERT_TRUE(!braille.empty());
}

MTEST(pixel_art_full_pipeline_options) {
    int w = 16, h = 16;
    std::vector<PixelRgba> pixels(w * h, {120, 150, 200, 255});

    PixelArtOptions opts;
    opts.style = PixelRenderStyle::PixelArt;
    opts.num_colors = 16;
    opts.scale = 2;
    opts.sharpness = 0.5f;
    opts.dither = false;
    opts.max_width_cols = 16;
    opts.max_height_rows = 8;

    std::string result = PixelArtRenderer::render_pixels(pixels, w, h, opts);
    ASSERT_TRUE(!result.empty());
    ASSERT_TRUE(result.find("▀") != std::string::npos);
}
