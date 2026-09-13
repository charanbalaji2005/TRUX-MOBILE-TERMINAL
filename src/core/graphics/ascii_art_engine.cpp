// src/core/graphics/ascii_art_engine.cpp
#include "ascii_art_engine.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace meridian::graphics {

namespace {

inline float sinc(float x) {
    if (std::abs(x) < 1e-6f) return 1.0f;
    float px = static_cast<float>(M_PI) * x;
    return std::sin(px) / px;
}

inline float lanczos3_kernel(float x) {
    float ax = std::abs(x);
    if (ax >= 3.0f) return 0.0f;
    return sinc(ax) * sinc(ax / 3.0f);
}

} // namespace

void AsciiArtEngine::get_terminal_dimensions(int& cols, int& rows) {
    cols = 80;
    rows = 24;

    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0) cols = ws.ws_col;
        if (ws.ws_row > 0) rows = ws.ws_row;
    } else {
        const char* c = std::getenv("COLUMNS");
        const char* r = std::getenv("LINES");
        if (c) { try { cols = std::stoi(c); } catch (...) {} }
        if (r) { try { rows = std::stoi(r); } catch (...) {} }
    }

    if (cols <= 10) cols = 80;
    if (rows <= 5) rows = 24;
}

std::vector<uint8_t> AsciiArtEngine::resample_lanczos3(
    const std::vector<uint8_t>& src,
    int src_w, int src_h,
    int dst_w, int dst_h
) {
    if (src.empty() || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) {
        return {};
    }

    if (src_w == dst_w && src_h == dst_h) {
        return src;
    }

    // Step 1: Horizontal Pass (src_w -> dst_w, intermediate height = src_h)
    std::vector<float> temp(dst_w * src_h * 4, 0.0f);
    float scale_x = static_cast<float>(src_w) / dst_w;
    float filter_scale_x = std::max(1.0f, scale_x);
    float radius_x = 3.0f * filter_scale_x;

    for (int y = 0; y < src_h; ++y) {
        for (int dx = 0; dx < dst_w; ++dx) {
            float center_x = (dx + 0.5f) * scale_x - 0.5f;
            int min_x = std::max(0, static_cast<int>(std::floor(center_x - radius_x)));
            int max_x = std::min(src_w - 1, static_cast<int>(std::ceil(center_x + radius_x)));

            float weight_sum = 0.0f;
            float acc[4] = {0.0f, 0.0f, 0.0f, 0.0f};

            for (int sx = min_x; sx <= max_x; ++sx) {
                float dist = (sx - center_x) / filter_scale_x;
                float w = lanczos3_kernel(dist);
                if (w == 0.0f) continue;

                int src_idx = (y * src_w + sx) * 4;
                acc[0] += src[src_idx + 0] * w;
                acc[1] += src[src_idx + 1] * w;
                acc[2] += src[src_idx + 2] * w;
                acc[3] += src[src_idx + 3] * w;
                weight_sum += w;
            }

            int dst_idx = (y * dst_w + dx) * 4;
            if (weight_sum > 0.0001f) {
                temp[dst_idx + 0] = acc[0] / weight_sum;
                temp[dst_idx + 1] = acc[1] / weight_sum;
                temp[dst_idx + 2] = acc[2] / weight_sum;
                temp[dst_idx + 3] = acc[3] / weight_sum;
            }
        }
    }

    // Step 2: Vertical Pass (src_h -> dst_h, width = dst_w)
    std::vector<uint8_t> output(dst_w * dst_h * 4, 0);
    float scale_y = static_cast<float>(src_h) / dst_h;
    float filter_scale_y = std::max(1.0f, scale_y);
    float radius_y = 3.0f * filter_scale_y;

    for (int dy = 0; dy < dst_h; ++dy) {
        float center_y = (dy + 0.5f) * scale_y - 0.5f;
        int min_y = std::max(0, static_cast<int>(std::floor(center_y - radius_y)));
        int max_y = std::min(src_h - 1, static_cast<int>(std::ceil(center_y + radius_y)));

        for (int x = 0; x < dst_w; ++x) {
            float weight_sum = 0.0f;
            float acc[4] = {0.0f, 0.0f, 0.0f, 0.0f};

            for (int sy = min_y; sy <= max_y; ++sy) {
                float dist = (sy - center_y) / filter_scale_y;
                float w = lanczos3_kernel(dist);
                if (w == 0.0f) continue;

                int temp_idx = (sy * dst_w + x) * 4;
                acc[0] += temp[temp_idx + 0] * w;
                acc[1] += temp[temp_idx + 1] * w;
                acc[2] += temp[temp_idx + 2] * w;
                acc[3] += temp[temp_idx + 3] * w;
                weight_sum += w;
            }

            int out_idx = (dy * dst_w + x) * 4;
            if (weight_sum > 0.0001f) {
                output[out_idx + 0] = static_cast<uint8_t>(std::clamp(acc[0] / weight_sum, 0.0f, 255.0f));
                output[out_idx + 1] = static_cast<uint8_t>(std::clamp(acc[1] / weight_sum, 0.0f, 255.0f));
                output[out_idx + 2] = static_cast<uint8_t>(std::clamp(acc[2] / weight_sum, 0.0f, 255.0f));
                output[out_idx + 3] = static_cast<uint8_t>(std::clamp(acc[3] / weight_sum, 0.0f, 255.0f));
            }
        }
    }

    return output;
}

void AsciiArtEngine::apply_color_adjustments(
    std::vector<uint8_t>& rgba,
    int width, int height,
    float brightness, float contrast, float gamma, float saturation
) {
    if (rgba.empty() || width <= 0 || height <= 0) return;

    bool do_brightness = std::abs(brightness - 1.0f) > 0.001f;
    bool do_contrast = std::abs(contrast - 1.0f) > 0.001f;
    bool do_gamma = std::abs(gamma - 1.0f) > 0.001f;
    bool do_saturation = std::abs(saturation - 1.0f) > 0.001f;

    if (!do_brightness && !do_contrast && !do_gamma && !do_saturation) return;

    float inv_gamma = (gamma > 0.001f) ? (1.0f / gamma) : 1.0f;

    for (size_t i = 0; i < rgba.size(); i += 4) {
        float r = rgba[i + 0];
        float g = rgba[i + 1];
        float b = rgba[i + 2];

        // 1. Saturation adjustment in RGB space
        if (do_saturation) {
            float lum = 0.299f * r + 0.587f * g + 0.114f * b;
            r = lum + (r - lum) * saturation;
            g = lum + (g - lum) * saturation;
            b = lum + (b - lum) * saturation;
        }

        // 2. Contrast adjustment around middle gray (128)
        if (do_contrast) {
            r = 128.0f + (r - 128.0f) * contrast;
            g = 128.0f + (g - 128.0f) * contrast;
            b = 128.0f + (b - 128.0f) * contrast;
        }

        // 3. Brightness adjustment
        if (do_brightness) {
            r *= brightness;
            g *= brightness;
            b *= brightness;
        }

        // 4. Gamma correction
        if (do_gamma) {
            r = 255.0f * std::pow(std::clamp(r / 255.0f, 0.0f, 1.0f), inv_gamma);
            g = 255.0f * std::pow(std::clamp(g / 255.0f, 0.0f, 1.0f), inv_gamma);
            b = 255.0f * std::pow(std::clamp(b / 255.0f, 0.0f, 1.0f), inv_gamma);
        }

        rgba[i + 0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
        rgba[i + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
        rgba[i + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
    }
}

void AsciiArtEngine::apply_edge_enhancement(
    std::vector<uint8_t>& rgba,
    int width, int height,
    float strength
) {
    if (rgba.empty() || width <= 2 || height <= 2 || strength <= 0.01f) return;

    std::vector<uint8_t> copy = rgba;
    float s = std::clamp(strength, 0.0f, 1.0f);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int center_idx = (y * width + x) * 4;

            for (int ch = 0; ch < 3; ++ch) {
                float center = copy[center_idx + ch];
                float top = copy[((y - 1) * width + x) * 4 + ch];
                float bot = copy[((y + 1) * width + x) * 4 + ch];
                float left = copy[(y * width + (x - 1)) * 4 + ch];
                float right = copy[(y * width + (x + 1)) * 4 + ch];

                // Laplacian high-pass
                float lap = (4.0f * center) - top - bot - left - right;
                float enhanced = center + lap * s * 0.5f;

                rgba[center_idx + ch] = static_cast<uint8_t>(std::clamp(enhanced, 0.0f, 255.0f));
            }
        }
    }
}

std::string AsciiArtEngine::emit_truecolor_halfblocks(
    const std::vector<uint8_t>& rgba,
    int width, int height,
    bool preserve_alpha
) {
    std::stringstream ss;
    int last_fg_r = -1, last_fg_g = -1, last_fg_b = -1;
    int last_bg_r = -1, last_bg_g = -1, last_bg_b = -1;

    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; ++x) {
            int top_idx = (y * width + x) * 4;
            int bot_idx = ((y + 1 < height ? y + 1 : y) * width + x) * 4;

            uint8_t tr = rgba[top_idx + 0], tg = rgba[top_idx + 1], tb = rgba[top_idx + 2], ta = rgba[top_idx + 3];
            uint8_t br = rgba[bot_idx + 0], bg = rgba[bot_idx + 1], bb = rgba[bot_idx + 2], ba = (y + 1 < height) ? rgba[bot_idx + 3] : 0;

            bool top_trans = preserve_alpha && (ta < 64);
            bool bot_trans = preserve_alpha && (ba < 64);

            if (top_trans && bot_trans) {
                if (last_fg_r != -1 || last_bg_r != -1) {
                    ss << "\033[0m";
                    last_fg_r = last_fg_g = last_fg_b = -1;
                    last_bg_r = last_bg_g = last_bg_b = -1;
                }
                ss << " ";
                continue;
            }

            if (top_trans && !bot_trans) {
                // Lower halfblock '▄'
                if (last_fg_r != br || last_fg_g != bg || last_fg_b != bb) {
                    ss << "\033[38;2;" << (int)br << ";" << (int)bg << ";" << (int)bb << "m";
                    last_fg_r = br; last_fg_g = bg; last_fg_b = bb;
                }
                if (last_bg_r != -1) {
                    ss << "\033[49m";
                    last_bg_r = last_bg_g = last_bg_b = -1;
                }
                ss << "▄";
                continue;
            }

            if (!top_trans && bot_trans) {
                // Upper halfblock '▀'
                if (last_fg_r != tr || last_fg_g != tg || last_fg_b != tb) {
                    ss << "\033[38;2;" << (int)tr << ";" << (int)tg << ";" << (int)tb << "m";
                    last_fg_r = tr; last_fg_g = tg; last_fg_b = tb;
                }
                if (last_bg_r != -1) {
                    ss << "\033[49m";
                    last_bg_r = last_bg_g = last_bg_b = -1;
                }
                ss << "▀";
                continue;
            }

            // Both colored -> Upper halfblock '▀' with FG (top) & BG (bot)
            if (last_fg_r != tr || last_fg_g != tg || last_fg_b != tb) {
                ss << "\033[38;2;" << (int)tr << ";" << (int)tg << ";" << (int)tb << "m";
                last_fg_r = tr; last_fg_g = tg; last_fg_b = tb;
            }
            if (last_bg_r != br || last_bg_g != bg || last_bg_b != bb) {
                ss << "\033[48;2;" << (int)br << ";" << (int)bg << ";" << (int)bb << "m";
                last_bg_r = br; last_bg_g = bg; last_bg_b = bb;
            }
            ss << "▀";
        }

        if (last_fg_r != -1 || last_bg_r != -1) {
            ss << "\033[0m";
            last_fg_r = last_fg_g = last_fg_b = -1;
            last_bg_r = last_bg_g = last_bg_b = -1;
        }
        ss << "\n";
    }

    return ss.str();
}

std::string AsciiArtEngine::emit_ascii_art(
    const std::vector<uint8_t>& rgba,
    int width, int height,
    AsciiRenderMode mode,
    bool dither,
    const std::string& ramp,
    bool preserve_alpha
) {
    if (rgba.empty() || width <= 0 || height <= 0 || ramp.empty()) return "";

    std::stringstream ss;
    std::vector<float> lum_grid(width * height, 0.0f);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            float lum = (0.299f * rgba[idx + 0] + 0.587f * rgba[idx + 1] + 0.114f * rgba[idx + 2]) / 255.0f;
            lum_grid[y * width + x] = std::clamp(lum, 0.0f, 1.0f);
        }
    }

    // Optional 2D Floyd-Steinberg error diffusion dithering
    if (dither) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float old_val = lum_grid[y * width + x];
                int ramp_idx = static_cast<int>(std::round(old_val * (ramp.size() - 1)));
                ramp_idx = std::clamp(ramp_idx, 0, static_cast<int>(ramp.size() - 1));
                float new_val = static_cast<float>(ramp_idx) / (ramp.size() - 1);
                float err = old_val - new_val;

                if (x + 1 < width) lum_grid[y * width + (x + 1)] += err * (7.0f / 16.0f);
                if (x - 1 >= 0 && y + 1 < height) lum_grid[(y + 1) * width + (x - 1)] += err * (3.0f / 16.0f);
                if (y + 1 < height) lum_grid[(y + 1) * width + x] += err * (5.0f / 16.0f);
                if (x + 1 < width && y + 1 < height) lum_grid[(y + 1) * width + (x + 1)] += err * (1.0f / 16.0f);
            }
        }
    }

    int last_r = -1, last_g = -1, last_b = -1;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            uint8_t a = rgba[idx + 3];

            if (preserve_alpha && a < 64) {
                if (last_r != -1) {
                    ss << "\033[0m";
                    last_r = last_g = last_b = -1;
                }
                ss << " ";
                continue;
            }

            float lum = std::clamp(lum_grid[y * width + x], 0.0f, 1.0f);
            int ramp_idx = static_cast<int>(std::round(lum * (ramp.size() - 1)));
            ramp_idx = std::clamp(ramp_idx, 0, static_cast<int>(ramp.size() - 1));
            char ch = ramp[ramp_idx];

            if (mode == AsciiRenderMode::Ansi || mode == AsciiRenderMode::TrueColor) {
                uint8_t r = rgba[idx + 0], g = rgba[idx + 1], b = rgba[idx + 2];
                if (last_r != r || last_g != g || last_b != b) {
                    ss << "\033[38;2;" << (int)r << ";" << (int)g << ";" << (int)b << "m";
                    last_r = r; last_g = g; last_b = b;
                }
            }

            ss << ch;
        }

        if (last_r != -1) {
            ss << "\033[0m";
            last_r = last_g = last_b = -1;
        }
        ss << "\n";
    }

    return ss.str();
}

std::string AsciiArtEngine::render_pixels(
    const std::vector<uint8_t>& src_rgba,
    int src_w, int src_h,
    const AsciiArtOptions& options
) {
    if (src_rgba.empty() || src_w <= 0 || src_h <= 0) return "";

    int term_cols = 80, term_rows = 24;
    get_terminal_dimensions(term_cols, term_rows);

    int target_cols = (options.target_width > 0) ? options.target_width : std::min(term_cols - 2, 100);
    if (target_cols <= 10) target_cols = 80;

    float img_aspect = static_cast<float>(src_w) / src_h;

    // Calculate grid dimensions
    int grid_w = target_cols;
    int grid_h = 0;

    if (options.mode == AsciiRenderMode::TrueColor || options.mode == AsciiRenderMode::HalfBlock) {
        // Halfblock: 1 char width = 1 pixel, 1 char height = 2 pixels
        // So pixel aspect is 1:1, terminal character row = 2 vertical pixels
        grid_w = target_cols;
        grid_h = std::max(2, static_cast<int>(std::round(grid_w / img_aspect)));
        if (grid_h % 2 != 0) grid_h++;
    } else {
        // Standard ASCII: 1 char is ~1:2 aspect
        grid_w = target_cols;
        grid_h = std::max(1, static_cast<int>(std::round(grid_w / (img_aspect * 2.0f))));
    }

    if (options.target_height > 0) {
        if (options.mode == AsciiRenderMode::TrueColor || options.mode == AsciiRenderMode::HalfBlock) {
            grid_h = options.target_height * 2;
        } else {
            grid_h = options.target_height;
        }
    }

    // Step 1: Ultra-High-Quality Lanczos-3 Sinc Resampling
    auto resampled = resample_lanczos3(src_rgba, src_w, src_h, grid_w, grid_h);

    // Step 2: Color, Contrast, Gamma, & Saturation Processing
    apply_color_adjustments(
        resampled, grid_w, grid_h,
        options.brightness, options.contrast, options.gamma, options.saturation
    );

    // Step 3: Edge-Aware Feature & Outline Enhancement
    if (options.quality == AsciiQuality::Ultra || options.edge_enhancement > 0.01f) {
        apply_edge_enhancement(resampled, grid_w, grid_h, options.edge_enhancement);
    }

    // Step 4: Emission
    if (options.mode == AsciiRenderMode::TrueColor || options.mode == AsciiRenderMode::HalfBlock) {
        return emit_truecolor_halfblocks(resampled, grid_w, grid_h, options.preserve_alpha);
    } else {
        return emit_ascii_art(
            resampled, grid_w, grid_h,
            options.mode, options.dither, options.character_ramp, options.preserve_alpha
        );
    }
}

std::string AsciiArtEngine::render_image(
    const DecodedImage& image,
    const AsciiArtOptions& options
) {
    if (!image.is_valid()) return "";
    const auto& frame = image.frame(0);
    return render_pixels(frame.rgba, frame.width, frame.height, options);
}

std::string AsciiArtEngine::render_file(
    const std::string& filepath,
    const AsciiArtOptions& options
) {
    auto decoded = ImageDecoder::decode_file(filepath);
    if (!decoded.is_valid()) {
        return "meridian: failed to decode image '" + filepath + "'\n";
    }
    return render_image(decoded, options);
}

} // namespace meridian::graphics

