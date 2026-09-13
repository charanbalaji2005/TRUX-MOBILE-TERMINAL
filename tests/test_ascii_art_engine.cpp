// tests/test_ascii_art_engine.cpp
#include "mini_test.hpp"
#include "../src/core/graphics/ascii_art_engine.hpp"
#include "../src/core/graphics/image_decoder.hpp"
#include "../src/shell/shell.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

using namespace meridian::graphics;

MTEST(ascii_art_lanczos3_resampling_dimensions_and_channels) {
    int src_w = 100, src_h = 100;
    std::vector<uint8_t> src(src_w * src_h * 4, 128); // Solid gray

    // Resample down to 50x25
    auto dst = AsciiArtEngine::resample_lanczos3(src, src_w, src_h, 50, 25);
    ASSERT_EQ(dst.size(), static_cast<size_t>(50 * 25 * 4));

    // Values should remain uniform gray (~128)
    for (size_t i = 0; i < dst.size(); i += 4) {
        ASSERT_TRUE(std::abs(static_cast<int>(dst[i]) - 128) <= 2);
    }
}

MTEST(ascii_art_color_adjustments_contrast_and_brightness) {
    int w = 10, h = 10;
    std::vector<uint8_t> pixels(w * h * 4, 100);

    // Increase brightness by 1.5x
    AsciiArtEngine::apply_color_adjustments(pixels, w, h, 1.5f, 1.0f, 1.0f, 1.0f);
    ASSERT_TRUE(pixels[0] > 140);

    // Increase contrast
    AsciiArtEngine::apply_color_adjustments(pixels, w, h, 1.0f, 2.0f, 1.0f, 1.0f);
    // Value > 128 should expand higher
    ASSERT_TRUE(pixels[0] > 160);
}

MTEST(ascii_art_edge_enhancement_kernel) {
    int w = 10, h = 10;
    std::vector<uint8_t> pixels(w * h * 4, 0); // Black

    // Draw vertical white bar
    for (int y = 0; y < h; ++y) {
        for (int ch = 0; ch < 3; ++ch) {
            pixels[(y * w + 5) * 4 + ch] = 255;
        }
    }

    AsciiArtEngine::apply_edge_enhancement(pixels, w, h, 0.8f);
    // The edge boundary at column 5 should be sharp
    ASSERT_EQ(static_cast<int>(pixels[(5 * w + 5) * 4]), 255);
}

MTEST(ascii_art_dual_pixel_halfblock_truecolor_emission) {
    int w = 4, h = 4;
    std::vector<uint8_t> pixels(w * h * 4, 0);

    // Top 2 rows red, bottom 2 rows blue
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 4;
            pixels[idx + 0] = 255; // R
            pixels[idx + 3] = 255; // A
        }
    }
    for (int y = 2; y < 4; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 4;
            pixels[idx + 2] = 255; // B
            pixels[idx + 3] = 255; // A
        }
    }

    std::string ansi = AsciiArtEngine::emit_truecolor_halfblocks(pixels, w, h, true);
    ASSERT_FALSE(ansi.empty());
    // Must contain 24-bit TrueColor escape sequence "\033[38;2;" and Unicode halfblock "▀"
    ASSERT_TRUE(ansi.find("\033[38;2;") != std::string::npos);
    ASSERT_TRUE(ansi.find("▀") != std::string::npos);
}

MTEST(ascii_art_70_char_ramp_mapping) {
    int w = 10, h = 2;
    std::vector<uint8_t> pixels(w * h * 4, 0);

    // Gradient from 0 to 255
    for (int x = 0; x < w; ++x) {
        uint8_t val = static_cast<uint8_t>(x * 255 / (w - 1));
        for (int y = 0; y < h; ++y) {
            int idx = (y * w + x) * 4;
            pixels[idx + 0] = val;
            pixels[idx + 1] = val;
            pixels[idx + 2] = val;
            pixels[idx + 3] = 255;
        }
    }

    AsciiArtOptions opts;
    opts.mode = AsciiRenderMode::Ascii;
    std::string out = AsciiArtEngine::emit_ascii_art(
        pixels, w, h,
        opts.mode, false, opts.character_ramp, true
    );

    ASSERT_FALSE(out.empty());
    // Start of line should be dim character (' ' or '.'), end should be dense ('$' or '@')
    ASSERT_TRUE(out.front() == ' ' || out.front() == '.');
}

MTEST(ascii_art_floyd_steinberg_dithering) {
    int w = 8, h = 8;
    std::vector<uint8_t> pixels(w * h * 4, 128); // 50% gray
    for (size_t i = 3; i < pixels.size(); i += 4) pixels[i] = 255;

    std::string ramp = " @";
    std::string dithered = AsciiArtEngine::emit_ascii_art(
        pixels, w, h,
        AsciiRenderMode::Ascii, true, ramp, true
    );

    ASSERT_FALSE(dithered.empty());
    // Both ' ' and '@' should be present in dithered pattern
    ASSERT_TRUE(dithered.find(' ') != std::string::npos);
    ASSERT_TRUE(dithered.find('@') != std::string::npos);
}

MTEST(ascii_art_render_file_gallery_images) {
    AsciiArtOptions opts;
    opts.target_width = 60;
    opts.mode = AsciiRenderMode::TrueColor;
    opts.quality = AsciiQuality::Ultra;

    std::string output = AsciiArtEngine::render_file("resources/images/gallery/sharingan_eye.png", opts);
    ASSERT_FALSE(output.empty());
    ASSERT_TRUE(output.find("▀") != std::string::npos);
}

MTEST(ascii_art_cli_pic_command_modes) {
    meridian::shell::Shell shell(false);
    std::ostringstream err;

    int rc1 = shell.run_command("pic eye --width 60 --mode truecolor", err);
    ASSERT_EQ(rc1, 0);

    int rc2 = shell.run_command("pic eye --width 60 --mode ascii --dither", err);
    ASSERT_EQ(rc2, 0);
}

