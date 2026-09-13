// src/core/graphics/pixel_art_renderer.cpp
#include "pixel_art_renderer.hpp"

#include <cmath>
#include <algorithm>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>

namespace meridian::graphics {

namespace {

inline float rgb_to_luminance(uint8_t r, uint8_t g, uint8_t b) {
    return 0.299f * r + 0.587f * g + 0.114f * b;
}

inline float color_dist_sq(const PixelRgba& c1, const PixelRgba& c2) {
    float dr = static_cast<float>(c1.r) - c2.r;
    float dg = static_cast<float>(c1.g) - c2.g;
    float db = static_cast<float>(c1.b) - c2.b;
    // Red-mean perceptual color distance
    float rbar = (c1.r + c2.r) / 2.0f;
    return (2.0f + rbar / 256.0f) * dr * dr + 4.0f * dg * dg + (2.0f + (255.0f - rbar) / 256.0f) * db * db;
}

struct ColorBox {
    std::vector<PixelRgba> colors;
    uint8_t min_r = 255, max_r = 0;
    uint8_t min_g = 255, max_g = 0;
    uint8_t min_b = 255, max_b = 0;

    void update_bounds() {
        min_r = min_g = min_b = 255;
        max_r = max_g = max_b = 0;
        for (const auto& c : colors) {
            min_r = std::min(min_r, c.r);
            max_r = std::max(max_r, c.r);
            min_g = std::min(min_g, c.g);
            max_g = std::max(max_g, c.g);
            min_b = std::min(min_b, c.b);
            max_b = std::max(max_b, c.b);
        }
    }

    int volume() const {
        return (max_r - min_r) * (max_g - min_g) * (max_b - min_b);
    }

    int longest_axis() const {
        int r_len = max_r - min_r;
        int g_len = max_g - min_g;
        int b_len = max_b - min_b;
        if (r_len >= g_len && r_len >= b_len) return 0; // Red
        if (g_len >= r_len && g_len >= b_len) return 1; // Green
        return 2; // Blue
    }

    PixelRgba average_color() const {
        if (colors.empty()) return {0, 0, 0, 255};
        uint64_t sr = 0, sg = 0, sb = 0;
        for (const auto& c : colors) {
            sr += c.r;
            sg += c.g;
            sb += c.b;
        }
        size_t n = colors.size();
        return {
            static_cast<uint8_t>(sr / n),
            static_cast<uint8_t>(sg / n),
            static_cast<uint8_t>(sb / n),
            255
        };
    }
};

static const int bayer4x4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 }
};

} // namespace

void PixelArtRenderer::get_terminal_dimensions(int& out_cols, int& out_rows) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        out_cols = ws.ws_col;
        out_rows = ws.ws_row;
    } else {
        out_cols = 80;
        out_rows = 24;
    }
}

std::vector<PixelRgba> PixelArtRenderer::sharpen_image(const std::vector<PixelRgba>& src, int w, int h, float amount) {
    if (amount <= 0.01f || src.empty() || w < 3 || h < 3) return src;

    std::vector<PixelRgba> dst = src;
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            const PixelRgba& center = src[y * w + x];
            if (center.is_transparent()) continue;

            float r_blur = 0.0f, g_blur = 0.0f, b_blur = 0.0f;
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    const PixelRgba& n = src[(y + dy) * w + (x + dx)];
                    r_blur += n.r;
                    g_blur += n.g;
                    b_blur += n.b;
                    count++;
                }
            }
            r_blur /= count;
            g_blur /= count;
            b_blur /= count;

            float new_r = center.r + amount * (center.r - r_blur);
            float new_g = center.g + amount * (center.g - g_blur);
            float new_b = center.b + amount * (center.b - b_blur);

            dst[y * w + x].r = static_cast<uint8_t>(std::clamp(new_r, 0.0f, 255.0f));
            dst[y * w + x].g = static_cast<uint8_t>(std::clamp(new_g, 0.0f, 255.0f));
            dst[y * w + x].b = static_cast<uint8_t>(std::clamp(new_b, 0.0f, 255.0f));
        }
    }
    return dst;
}

std::vector<float> PixelArtRenderer::compute_sobel_edges(const std::vector<PixelRgba>& src, int w, int h) {
    std::vector<float> edges(w * h, 0.0f);
    if (w < 3 || h < 3) return edges;

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            float p00 = rgb_to_luminance(src[(y-1)*w + (x-1)].r, src[(y-1)*w + (x-1)].g, src[(y-1)*w + (x-1)].b);
            float p01 = rgb_to_luminance(src[(y-1)*w + x].r,     src[(y-1)*w + x].g,     src[(y-1)*w + x].b);
            float p02 = rgb_to_luminance(src[(y-1)*w + (x+1)].r, src[(y-1)*w + (x+1)].g, src[(y-1)*w + (x+1)].b);
            float p10 = rgb_to_luminance(src[y*w + (x-1)].r,     src[y*w + (x-1)].g,     src[y*w + (x-1)].b);
            float p12 = rgb_to_luminance(src[y*w + (x+1)].r,     src[y*w + (x+1)].g,     src[y*w + (x+1)].b);
            float p20 = rgb_to_luminance(src[(y+1)*w + (x-1)].r, src[(y+1)*w + (x-1)].g, src[(y+1)*w + (x-1)].b);
            float p21 = rgb_to_luminance(src[(y+1)*w + x].r,     src[(y+1)*w + x].g,     src[(y+1)*w + x].b);
            float p22 = rgb_to_luminance(src[(y+1)*w + (x+1)].r, src[(y+1)*w + (x+1)].g, src[(y+1)*w + (x+1)].b);

            float gx = (p02 + 2.0f * p12 + p22) - (p00 + 2.0f * p10 + p20);
            float gy = (p20 + 2.0f * p21 + p22) - (p00 + 2.0f * p01 + p02);
            float mag = std::sqrt(gx * gx + gy * gy) / 255.0f;
            edges[y * w + x] = std::clamp(mag, 0.0f, 1.0f);
        }
    }
    return edges;
}

std::vector<PixelRgba> PixelArtRenderer::reduce_to_pixel_grid(
    const std::vector<PixelRgba>& src, int src_w, int src_h,
    int target_w, int target_h, const std::vector<float>& edges
) {
    std::vector<PixelRgba> grid(target_w * target_h);

    float scale_x = static_cast<float>(src_w) / target_w;
    float scale_y = static_cast<float>(src_h) / target_h;

    for (int ty = 0; ty < target_h; ++ty) {
        for (int tx = 0; tx < target_w; ++tx) {
            int sx_start = static_cast<int>(tx * scale_x);
            int sx_end = std::min(src_w, static_cast<int>((tx + 1) * scale_x));
            int sy_start = static_cast<int>(ty * scale_y);
            int sy_end = std::min(src_h, static_cast<int>((ty + 1) * scale_y));

            if (sx_end <= sx_start) sx_end = sx_start + 1;
            if (sy_end <= sy_start) sy_end = sy_start + 1;

            float max_edge = -1.0f;
            PixelRgba edge_color = {0, 0, 0, 0};
            uint64_t sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0;
            int total_samples = 0;

            for (int sy = sy_start; sy < sy_end; ++sy) {
                for (int sx = sx_start; sx < sx_end; ++sx) {
                    const PixelRgba& pix = src[sy * src_w + sx];
                    sum_r += pix.r;
                    sum_g += pix.g;
                    sum_b += pix.b;
                    sum_a += pix.a;
                    total_samples++;

                    if (!edges.empty()) {
                        float e = edges[sy * src_w + sx];
                        if (e > max_edge) {
                            max_edge = e;
                            edge_color = pix;
                        }
                    }
                }
            }

            if (total_samples == 0) {
                grid[ty * target_w + tx] = {0, 0, 0, 0};
                continue;
            }

            uint8_t avg_a = static_cast<uint8_t>(sum_a / total_samples);
            if (avg_a < 64) {
                grid[ty * target_w + tx] = {0, 0, 0, 0};
            } else if (max_edge > 0.35f && !edge_color.is_transparent()) {
                // Edge dominant preserve
                grid[ty * target_w + tx] = edge_color;
            } else {
                grid[ty * target_w + tx] = {
                    static_cast<uint8_t>(sum_r / total_samples),
                    static_cast<uint8_t>(sum_g / total_samples),
                    static_cast<uint8_t>(sum_b / total_samples),
                    avg_a
                };
            }
        }
    }
    return grid;
}

std::vector<PixelRgba> PixelArtRenderer::quantize_colors(
    const std::vector<PixelRgba>& src, int num_colors, bool dither, int w, int h
) {
    if (num_colors <= 0 || src.empty()) return src;

    // Collect non-transparent colors
    std::vector<PixelRgba> valid_colors;
    valid_colors.reserve(src.size());
    for (const auto& c : src) {
        if (!c.is_transparent()) {
            valid_colors.push_back(c);
        }
    }

    if (valid_colors.empty()) return src;

    // Median-Cut Palette Generation
    std::vector<ColorBox> boxes;
    ColorBox initial_box;
    initial_box.colors = std::move(valid_colors);
    initial_box.update_bounds();
    boxes.push_back(std::move(initial_box));

    while (static_cast<int>(boxes.size()) < num_colors) {
        // Find box with highest volume
        auto split_it = std::max_element(boxes.begin(), boxes.end(), [](const ColorBox& a, const ColorBox& b) {
            return a.volume() < b.volume();
        });

        if (split_it == boxes.end() || split_it->colors.size() <= 1) break;

        int axis = split_it->longest_axis();
        if (axis == 0) {
            std::sort(split_it->colors.begin(), split_it->colors.end(), [](const PixelRgba& a, const PixelRgba& b) {
                return a.r < b.r;
            });
        } else if (axis == 1) {
            std::sort(split_it->colors.begin(), split_it->colors.end(), [](const PixelRgba& a, const PixelRgba& b) {
                return a.g < b.g;
            });
        } else {
            std::sort(split_it->colors.begin(), split_it->colors.end(), [](const PixelRgba& a, const PixelRgba& b) {
                return a.b < b.b;
            });
        }

        size_t mid = split_it->colors.size() / 2;
        ColorBox new_box;
        new_box.colors.assign(split_it->colors.begin() + mid, split_it->colors.end());
        new_box.update_bounds();

        split_it->colors.erase(split_it->colors.begin() + mid, split_it->colors.end());
        split_it->update_bounds();

        boxes.push_back(std::move(new_box));
    }

    std::vector<PixelRgba> palette;
    palette.reserve(boxes.size());
    for (const auto& box : boxes) {
        palette.push_back(box.average_color());
    }

    // Remap source grid to quantized palette
    std::vector<PixelRgba> dst = src;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            PixelRgba pix = src[y * w + x];
            if (pix.is_transparent()) {
                dst[y * w + x] = {0, 0, 0, 0};
                continue;
            }

            if (dither) {
                float dither_val = (bayer4x4[y % 4][x % 4] / 16.0f - 0.5f) * 32.0f;
                pix.r = static_cast<uint8_t>(std::clamp(pix.r + dither_val, 0.0f, 255.0f));
                pix.g = static_cast<uint8_t>(std::clamp(pix.g + dither_val, 0.0f, 255.0f));
                pix.b = static_cast<uint8_t>(std::clamp(pix.b + dither_val, 0.0f, 255.0f));
            }

            float min_dist = 1e9f;
            PixelRgba best = palette[0];
            for (const auto& p : palette) {
                float d = color_dist_sq(pix, p);
                if (d < min_dist) {
                    min_dist = d;
                    best = p;
                }
            }
            dst[y * w + x] = best;
        }
    }

    return dst;
}

std::string PixelArtRenderer::emit_ansi_halfblocks(const std::vector<PixelRgba>& grid, int grid_w, int grid_h, bool preserve_alpha) {
    std::stringstream ss;

    int last_fg_r = -1, last_fg_g = -1, last_fg_b = -1;
    int last_bg_r = -1, last_bg_g = -1, last_bg_b = -1;

    for (int y = 0; y < grid_h; y += 2) {
        for (int x = 0; x < grid_w; ++x) {
            PixelRgba top = grid[y * grid_w + x];
            PixelRgba bot = (y + 1 < grid_h) ? grid[(y + 1) * grid_w + x] : PixelRgba{0, 0, 0, 0};

            bool top_trans = preserve_alpha && top.is_transparent();
            bool bot_trans = preserve_alpha && bot.is_transparent();

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
                // Bottom only -> use lower halfblock '▄'
                if (last_fg_r != bot.r || last_fg_g != bot.g || last_fg_b != bot.b) {
                    ss << "\033[38;2;" << (int)bot.r << ";" << (int)bot.g << ";" << (int)bot.b << "m";
                    last_fg_r = bot.r; last_fg_g = bot.g; last_fg_b = bot.b;
                }
                if (last_bg_r != -1) {
                    ss << "\033[49m";
                    last_bg_r = last_bg_g = last_bg_b = -1;
                }
                ss << "▄";
                continue;
            }

            if (!top_trans && bot_trans) {
                // Top only -> use upper halfblock '▀'
                if (last_fg_r != top.r || last_fg_g != top.g || last_fg_b != top.b) {
                    ss << "\033[38;2;" << (int)top.r << ";" << (int)top.g << ";" << (int)top.b << "m";
                    last_fg_r = top.r; last_fg_g = top.g; last_fg_b = top.b;
                }
                if (last_bg_r != -1) {
                    ss << "\033[49m";
                    last_bg_r = last_bg_g = last_bg_b = -1;
                }
                ss << "▀";
                continue;
            }

            // Both colored -> Upper halfblock '▀' with foreground (top) and background (bot)
            if (last_fg_r != top.r || last_fg_g != top.g || last_fg_b != top.b) {
                ss << "\033[38;2;" << (int)top.r << ";" << (int)top.g << ";" << (int)top.b << "m";
                last_fg_r = top.r; last_fg_g = top.g; last_fg_b = top.b;
            }
            if (last_bg_r != bot.r || last_bg_g != bot.g || last_bg_b != bot.b) {
                ss << "\033[48;2;" << (int)bot.r << ";" << (int)bot.g << ";" << (int)bot.b << "m";
                last_bg_r = bot.r; last_bg_g = bot.g; last_bg_b = bot.b;
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

std::string PixelArtRenderer::emit_ascii(const std::vector<PixelRgba>& grid, int grid_w, int grid_h, bool colored, const std::string& charset) {
    std::stringstream ss;
    int last_r = -1, last_g = -1, last_b = -1;

    for (int y = 0; y < grid_h; ++y) {
        for (int x = 0; x < grid_w; ++x) {
            const PixelRgba& pix = grid[y * grid_w + x];
            if (pix.is_transparent()) {
                if (last_r != -1) {
                    ss << "\033[0m";
                    last_r = last_g = last_b = -1;
                }
                ss << " ";
                continue;
            }

            float lum = rgb_to_luminance(pix.r, pix.g, pix.b) / 255.0f;
            int char_idx = static_cast<int>(std::clamp(lum * charset.size(), 0.0f, static_cast<float>(charset.size() - 1)));
            char ch = charset[char_idx];

            if (colored) {
                if (last_r != pix.r || last_g != pix.g || last_b != pix.b) {
                    ss << "\033[38;2;" << (int)pix.r << ";" << (int)pix.g << ";" << (int)pix.b << "m";
                    last_r = pix.r; last_g = pix.g; last_b = pix.b;
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

std::string PixelArtRenderer::emit_braille(const std::vector<PixelRgba>& grid, int grid_w, int grid_h) {
    std::stringstream ss;
    // Braille matrix: 2 columns wide by 4 rows high
    static const int braille_dots[4][2] = {
        { 0x01, 0x08 },
        { 0x02, 0x10 },
        { 0x04, 0x20 },
        { 0x40, 0x80 }
    };

    for (int y = 0; y < grid_h; y += 4) {
        for (int x = 0; x < grid_w; x += 2) {
            int code = 0x2800; // Blank braille character
            for (int dy = 0; dy < 4; ++dy) {
                for (int dx = 0; dx < 2; ++dx) {
                    int gy = y + dy;
                    int gx = x + dx;
                    if (gy < grid_h && gx < grid_w) {
                        const PixelRgba& pix = grid[gy * grid_w + gx];
                        if (!pix.is_transparent() && rgb_to_luminance(pix.r, pix.g, pix.b) > 100.0f) {
                            code |= braille_dots[dy][dx];
                        }
                    }
                }
            }
            // UTF-8 encode 3-byte unicode character
            ss << static_cast<char>(0xE0 | ((code >> 12) & 0x0F))
               << static_cast<char>(0x80 | ((code >> 6) & 0x3F))
               << static_cast<char>(0x80 | (code & 0x3F));
        }
        ss << "\n";
    }
    return ss.str();
}

std::string PixelArtRenderer::render_pixels(const std::vector<PixelRgba>& pixels, int src_w, int src_h, const PixelArtOptions& opts) {
    if (src_w <= 0 || src_h <= 0 || pixels.empty()) return "";

    int term_cols = 80, term_rows = 24;
    get_terminal_dimensions(term_cols, term_rows);

    int target_cols = (opts.max_width_cols > 0) ? opts.max_width_cols : std::min(term_cols - 2, 78);
    if (target_cols <= 4) target_cols = 64;

    float img_aspect = static_cast<float>(src_w) / src_h;
    int grid_w = target_cols;
    int grid_h = std::max(2, static_cast<int>(grid_w / img_aspect));
    if (grid_h % 2 != 0) grid_h++;

    // For ASCII / AsciiColor mode (character cells are ~1:2)
    if (opts.style == PixelRenderStyle::Ascii || opts.style == PixelRenderStyle::AsciiColor) {
        grid_w = target_cols;
        grid_h = std::max(1, static_cast<int>(grid_w / (img_aspect * 2.0f)));
    }

    int low_w = grid_w;
    int low_h = grid_h;
    if (opts.style == PixelRenderStyle::PixelArt && opts.scale > 1) {
        low_w = std::max(4, grid_w / opts.scale);
        low_h = std::max(4, grid_h / opts.scale);
        if (low_h % 2 != 0) low_h++;
    }

    // 1. Sharpening (if sharpness > 0)
    std::vector<PixelRgba> processed = (opts.sharpness > 0.01f)
        ? sharpen_image(pixels, src_w, src_h, opts.sharpness)
        : pixels;

    // 2. Edge Detection (for PixelArt mode)
    std::vector<float> edges;
    if (opts.style == PixelRenderStyle::PixelArt) {
        edges = compute_sobel_edges(processed, src_w, src_h);
    }

    // 3. High-Quality Pixel Grid Reduction (Area Averaging)
    auto reduced = reduce_to_pixel_grid(processed, src_w, src_h, low_w, low_h, edges);

    // 4. Color Quantization (Only for PixelArt mode)
    if (opts.style == PixelRenderStyle::PixelArt) {
        reduced = quantize_colors(reduced, opts.num_colors, opts.dither, low_w, low_h);
    }

    // 5. Expand pixel blocks if scale > 1 and in PixelArt mode
    int final_w = low_w;
    int final_h = low_h;
    std::vector<PixelRgba> final_grid = reduced;

    if (opts.style == PixelRenderStyle::PixelArt && opts.scale > 1) {
        final_w = low_w * opts.scale;
        final_h = low_h * opts.scale;
        if (final_h % 2 != 0) final_h++;
        final_grid.assign(final_w * final_h, {0, 0, 0, 0});

        for (int ly = 0; ly < low_h; ++ly) {
            for (int lx = 0; lx < low_w; ++lx) {
                const auto& pix = reduced[ly * low_w + lx];
                for (int dy = 0; dy < opts.scale; ++dy) {
                    int fy = ly * opts.scale + dy;
                    if (fy >= final_h) continue;
                    for (int dx = 0; dx < opts.scale; ++dx) {
                        int fx = lx * opts.scale + dx;
                        if (fx >= final_w) continue;
                        final_grid[fy * final_w + fx] = pix;
                    }
                }
            }
        }
    }

    // 6. Output Emission
    if (opts.style == PixelRenderStyle::Ascii) {
        return emit_ascii(final_grid, final_w, final_h, false, opts.charset);
    } else if (opts.style == PixelRenderStyle::AsciiColor) {
        return emit_ascii(final_grid, final_w, final_h, true, opts.charset);
    } else if (opts.style == PixelRenderStyle::Braille) {
        return emit_braille(final_grid, final_w, final_h);
    } else {
        return emit_ansi_halfblocks(final_grid, final_w, final_h, opts.preserve_alpha);
    }
}

std::string PixelArtRenderer::render_image(const DecodedImage& image, const PixelArtOptions& opts) {
    if (!image.is_valid()) return "";
    const auto& frame = image.frame(0);
    std::vector<PixelRgba> pixels(frame.width * frame.height);
    for (size_t i = 0; i < pixels.size(); ++i) {
        pixels[i].r = frame.rgba[i * 4 + 0];
        pixels[i].g = frame.rgba[i * 4 + 1];
        pixels[i].b = frame.rgba[i * 4 + 2];
        pixels[i].a = frame.rgba[i * 4 + 3];
    }
    return render_pixels(pixels, frame.width, frame.height, opts);
}

std::string PixelArtRenderer::render_file(const std::string& filepath, const PixelArtOptions& opts) {
    auto decoded = ImageDecoder::decode_file(filepath);
    if (!decoded.is_valid()) {
        return "meridian: failed to decode image '" + filepath + "'\n";
    }
    return render_image(decoded, opts);
}

} // namespace meridian::graphics

