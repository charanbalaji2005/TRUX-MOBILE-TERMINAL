#include "terminal_image.hpp"
#include "graphics/image_decoder.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace meridian::core {

TerminalImage::TerminalImage() = default;

TerminalImage::TerminalImage(int width, int height)
    : width_(width)
    , height_(height)
    , pixels_(width * height, RgbColor{0, 0, 0, 255})
{
}

void TerminalImage::set_pixel(int x, int y, RgbColor color) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        pixels_[y * width_ + x] = color;
    }
}

RgbColor TerminalImage::get_pixel(int x, int y) const {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        return pixels_[y * width_ + x];
    }
    return RgbColor{0, 0, 0, 0};
}

RgbColor TerminalImage::sample_bilinear(float u, float v) const {
    if (width_ <= 0 || height_ <= 0) return RgbColor{0, 0, 0, 255};

    float px = u * (width_ - 1);
    float py = v * (height_ - 1);
    int x0 = std::clamp(static_cast<int>(std::floor(px)), 0, width_ - 1);
    int y0 = std::clamp(static_cast<int>(std::floor(py)), 0, height_ - 1);
    int x1 = std::clamp(x0 + 1, 0, width_ - 1);
    int y1 = std::clamp(y0 + 1, 0, height_ - 1);

    float fx = px - x0;
    float fy = py - y0;

    auto c00 = get_pixel(x0, y0);
    auto c10 = get_pixel(x1, y0);
    auto c01 = get_pixel(x0, y1);
    auto c11 = get_pixel(x1, y1);

    auto interp = [fx, fy](uint8_t v00, uint8_t v10, uint8_t v01, uint8_t v11) -> uint8_t {
        float top = v00 * (1.0f - fx) + v10 * fx;
        float bot = v01 * (1.0f - fx) + v11 * fx;
        return static_cast<uint8_t>(std::clamp(top * (1.0f - fy) + bot * fy, 0.0f, 255.0f));
    };

    return RgbColor{
        interp(c00.r, c10.r, c01.r, c11.r),
        interp(c00.g, c10.g, c01.g, c11.g),
        interp(c00.b, c10.b, c01.b, c11.b),
        interp(c00.a, c10.a, c01.a, c11.a)
    };
}

char TerminalImage::char_for_luminance(float lum, const std::string& charset) const {
    if (charset.empty()) return ' ';
    int idx = static_cast<int>(lum * (charset.size() - 1));
    idx = std::clamp(idx, 0, static_cast<int>(charset.size() - 1));
    return charset[idx];
}

bool TerminalImage::load_file(const std::string& path) {
    if (path.empty()) return false;

    // Decode original image (PNG, JPEG, WebP, GIF, BMP, PPM) into full 32-bit RGBA pixels
    auto decoded = graphics::ImageDecoder::decode_file(path);
    if (decoded.is_valid()) {
        const auto& frame = decoded.frame(0);
        width_ = frame.width;
        height_ = frame.height;
        pixels_.resize(width_ * height_);
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                size_t idx = (y * width_ + x) * 4;
                pixels_[y * width_ + x] = RgbColor{
                    frame.rgba[idx],
                    frame.rgba[idx + 1],
                    frame.rgba[idx + 2],
                    frame.rgba[idx + 3]
                };
            }
        }
        return true;
    }

    return false;
}

#include "chainsaw_man_data.inl"

TerminalImage TerminalImage::create_default_reference_artwork() {
    // 1. Try loading user custom artwork or bundled artwork from file
    const char* env_art = std::getenv("MERIDIAN_ARTWORK");
    if (env_art && access(env_art, R_OK) == 0) {
        TerminalImage img;
        if (img.load_file(env_art)) return img;
    }

    const char* home = std::getenv("HOME");
    if (home) {
        std::string p1 = std::string(home) + "/.config/meridian/artwork.jpg";
        std::string p2 = std::string(home) + "/.config/meridian/artwork.ppm";
        TerminalImage img;
        if (img.load_file(p1) || img.load_file(p2)) return img;
    }

    std::string candidate_paths[] = {
        "resources/images/artwork.jpg",
        "resources/images/artwork_thumb.ppm",
        "resources/images/artwork.png"
    };
    for (const auto& cp : candidate_paths) {
        TerminalImage img;
        if (img.load_file(cp)) return img;
    }

    // 2. Fallback to embedded Chainsaw Man pixel buffer
    int w = 48;
    int h = 44;
    TerminalImage img(w, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 3;
            uint8_t r = kChainsawManPixels[idx];
            uint8_t g = kChainsawManPixels[idx + 1];
            uint8_t b = kChainsawManPixels[idx + 2];
            img.set_pixel(x, y, RgbColor{r, g, b, 255});
        }
    }

    return img;
}

std::vector<std::string> TerminalImage::render_reference_artwork_lines(int max_rows) {
    auto art = create_default_reference_artwork();
    ImageOptions opts;
    opts.mode = ImageRenderMode::HalfBlock;
    opts.target_width = 24;
    opts.target_height = max_rows > 0 ? max_rows : 11;

    std::vector<std::string> lines;
    int target_w = opts.target_width;
    int target_h = opts.target_height;

    for (int r = 0; r < target_h; ++r) {
        std::ostringstream ss;
        for (int c = 0; c < target_w; ++c) {
            float u = static_cast<float>(c) / (target_w - 1);
            float v_top = static_cast<float>(r * 2) / (target_h * 2 - 1);
            float v_bot = static_cast<float>(r * 2 + 1) / (target_h * 2 - 1);

            auto c_top = art.sample_bilinear(u, v_top);
            auto c_bot = art.sample_bilinear(u, v_bot);

            // \033[38;2;R;G;Bm for top, \033[48;2;R;G;Bm for bottom, ▀
            ss << "\033[38;2;" << static_cast<int>(c_top.r) << ";" << static_cast<int>(c_top.g) << ";" << static_cast<int>(c_top.b)
               << ";48;2;" << static_cast<int>(c_bot.r) << ";" << static_cast<int>(c_bot.g) << ";" << static_cast<int>(c_bot.b)
               << "m▀";
        }
        ss << "\033[0m";
        lines.push_back(ss.str());
    }

    return lines;
}

static std::string to_base64(const uint8_t* data, size_t len) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t b = (data[i] << 16) | ((i + 1 < len ? data[i + 1] : 0) << 8) | (i + 2 < len ? data[i + 2] : 0);
        out.push_back(table[(b >> 18) & 0x3F]);
        out.push_back(table[(b >> 12) & 0x3F]);
        out.push_back((i + 1 < len) ? table[(b >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < len) ? table[b & 0x3F] : '=');
    }
    return out;
}

std::string TerminalImage::render(const ImageOptions& opts) const {
    if (!is_valid()) return "";

    std::ostringstream ss;
    int target_w = std::max(4, opts.target_width);
    int target_h = std::max(2, opts.target_height);

    if (opts.mode == ImageRenderMode::RealRaster) {
        std::vector<uint8_t> raw_rgba(width_ * height_ * 4);
        for (int i = 0; i < width_ * height_; ++i) {
            raw_rgba[i * 4 + 0] = pixels_[i].r;
            raw_rgba[i * 4 + 1] = pixels_[i].g;
            raw_rgba[i * 4 + 2] = pixels_[i].b;
            raw_rgba[i * 4 + 3] = pixels_[i].a;
        }
        std::string b64 = to_base64(raw_rgba.data(), raw_rgba.size());
        ss << "\033_Ga=T,f=32,s=" << width_ << ",v=" << height_
           << ",c=" << target_w << ",r=" << target_h << ";" << b64 << "\033\\";
        return ss.str();
    } else if (opts.mode == ImageRenderMode::Pixel) {
        // Nearest-neighbor pixel art mode
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                int px = std::clamp(c * width_ / target_w, 0, width_ - 1);
                int py_top = std::clamp((r * 2) * height_ / (target_h * 2), 0, height_ - 1);
                int py_bot = std::clamp((r * 2 + 1) * height_ / (target_h * 2), 0, height_ - 1);

                auto c_top = get_pixel(px, py_top);
                auto c_bot = get_pixel(px, py_bot);

                ss << "\033[38;2;" << static_cast<int>(c_top.r) << ";" << static_cast<int>(c_top.g) << ";" << static_cast<int>(c_top.b)
                   << ";48;2;" << static_cast<int>(c_bot.r) << ";" << static_cast<int>(c_bot.g) << ";" << static_cast<int>(c_bot.b)
                   << "m▀";
            }
            ss << "\033[0m\n";
        }
    } else if (opts.mode == ImageRenderMode::HalfBlock) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v_top = static_cast<float>(r * 2) / (target_h * 2 - 1);
                float v_bot = static_cast<float>(r * 2 + 1) / (target_h * 2 - 1);

                auto c_top = sample_bilinear(u, v_top);
                auto c_bot = sample_bilinear(u, v_bot);

                ss << "\033[38;2;" << static_cast<int>(c_top.r) << ";" << static_cast<int>(c_top.g) << ";" << static_cast<int>(c_top.b)
                   << ";48;2;" << static_cast<int>(c_bot.r) << ";" << static_cast<int>(c_bot.g) << ";" << static_cast<int>(c_bot.b)
                   << "m▀";
            }
            ss << "\033[0m\n";
        }
    } else if (opts.mode == ImageRenderMode::Ascii) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v = static_cast<float>(r) / (target_h - 1);
                auto col = sample_bilinear(u, v);
                float lum = (0.299f * col.r + 0.587f * col.g + 0.114f * col.b) / 255.0f;
                char ch = char_for_luminance(lum, opts.custom_charset);
                ss << ch;
            }
            ss << "\n";
        }
    } else if (opts.mode == ImageRenderMode::ColorAscii) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v = static_cast<float>(r) / (target_h - 1);
                auto col = sample_bilinear(u, v);
                float lum = (0.299f * col.r + 0.587f * col.g + 0.114f * col.b) / 255.0f;
                char ch = char_for_luminance(lum, opts.custom_charset);
                ss << "\033[38;2;" << static_cast<int>(col.r) << ";" << static_cast<int>(col.g) << ";" << static_cast<int>(col.b) << "m" << ch;
            }
            ss << "\033[0m\n";
        }
    } else if (opts.mode == ImageRenderMode::Hybrid) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v = static_cast<float>(r) / (target_h - 1);
                auto col = sample_bilinear(u, v);
                float lum = (0.299f * col.r + 0.587f * col.g + 0.114f * col.b) / 255.0f;
                char ch = char_for_luminance(lum, opts.custom_charset);

                ss << "\033[38;2;" << static_cast<int>(col.r) << ";" << static_cast<int>(col.g) << ";" << static_cast<int>(col.b)
                   << ";48;2;" << static_cast<int>(col.r / 3) << ";" << static_cast<int>(col.g / 3) << ";" << static_cast<int>(col.b / 3)
                   << "m" << ch;
            }
            ss << "\033[0m\n";
        }
    }

    return ss.str();
}

namespace {

std::string emit_chunked_kitty_payload(const std::string& b64, int cols, int rows, int format, int width = 0, int height = 0) {
    std::ostringstream ss;
    const size_t CHUNK_SIZE = 2048; // Safe chunk size (well under 4096 bytes)
    size_t total = b64.size();
    size_t offset = 0;
    bool first = true;

    while (offset < total) {
        size_t len = std::min(CHUNK_SIZE, total - offset);
        std::string chunk = b64.substr(offset, len);
        bool has_more = (offset + len < total);

        ss << "\033_G";
        if (first) {
            ss << "a=T,f=" << format << ",t=d";
            if (cols > 0) ss << ",c=" << cols;
            if (rows > 0) ss << ",r=" << rows;
            if (width > 0) ss << ",s=" << width;
            if (height > 0) ss << ",v=" << height;
            first = false;
        }
        ss << ",m=" << (has_more ? "1" : "0") << ";" << chunk << "\033\\";
        offset += len;
    }
    return ss.str();
}

} // namespace

std::string TerminalImage::render_kitty_graphics_artwork(int col, int row, int cols_spanned, int rows_spanned) {
    (void)col; (void)row;
    return emit_chunked_kitty_payload(kChainsawManKittyBase64, cols_spanned, rows_spanned, 100);
}

std::string TerminalImage::render_hardware_image_escape(const std::string& filepath, int target_width_cols) {
    if (filepath.empty()) return "";
    std::string actual_path = filepath;
    if (access(actual_path.c_str(), R_OK) != 0) {
        std::string alias = filepath;
        if (filepath == "itachi" || filepath == "sharingan") alias = "itachi_sharingan";
        else if (filepath == "swordsman" || filepath == "shadow") alias = "shadow_swordsman";
        else if (filepath == "ribbon") alias = "ribbon_girl";
        else if (filepath == "gojo" || filepath == "six_eyes") alias = "gojo_six_eyes";
        else if (filepath == "awakening" || filepath == "honored_one") alias = "gojo_awakening";
        else if (filepath == "sunset" || filepath == "sunset_girl" || filepath == "city") alias = "sunset_girl";
        else if (filepath == "eye" || filepath == "sasuke") alias = "sharingan_eye";
        else if (filepath == "sakura") alias = "sakura_girl";
        else if (filepath == "fan") alias = "fan_girl";

        const char* home = std::getenv("HOME");
        std::string home_str = home ? home : "";
        std::vector<std::string> candidates = {
            home_str + "/.config/meridian/gallery/" + alias + ".png",
            home_str + "/.config/meridian/gallery/" + alias + ".jpg",
            home_str + "/.config/meridian/gallery/" + alias + ".webp",
            home_str + "/.config/meridian/gallery/" + alias,
            "resources/images/gallery/" + alias + ".png",
            "resources/images/gallery/" + alias + ".jpg",
            "resources/images/gallery/" + alias + ".webp",
            "resources/images/gallery/" + alias,
            home_str + "/.config/meridian/gallery/" + filepath + ".png",
            home_str + "/.config/meridian/gallery/" + filepath + ".jpg",
            home_str + "/.config/meridian/gallery/" + filepath + ".webp",
            home_str + "/.config/meridian/gallery/" + filepath,
            "resources/images/gallery/" + filepath + ".png",
            "resources/images/gallery/" + filepath + ".jpg",
            "resources/images/gallery/" + filepath + ".webp",
            "resources/images/gallery/" + filepath,
            "resources/images/" + filepath,
            "resources/images/" + filepath + ".png"
        };
        for (const auto& c : candidates) {
            if (access(c.c_str(), R_OK) == 0) {
                actual_path = c;
                break;
            }
        }
    }

    std::ifstream f(actual_path, std::ios::binary);
    if (!f.is_open()) return "";
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (buffer.empty()) return "";

    static const char b64_tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string b64;
    b64.reserve(((buffer.size() + 2) / 3) * 4);
    for (size_t k = 0; k < buffer.size(); k += 3) {
        uint32_t b = (buffer[k] << 16) | ((k + 1 < buffer.size() ? buffer[k + 1] : 0) << 8) | (k + 2 < buffer.size() ? buffer[k + 2] : 0);
        b64.push_back(b64_tbl[(b >> 18) & 0x3F]);
        b64.push_back(b64_tbl[(b >> 12) & 0x3F]);
        b64.push_back((k + 1 < buffer.size()) ? b64_tbl[(b >> 6) & 0x3F] : '=');
        b64.push_back((k + 2 < buffer.size()) ? b64_tbl[b & 0x3F] : '=');
    }

    std::ostringstream ss;
    // 1. iTerm2 inline image sequence (standard in VS Code terminal, Antigravity IDE, iTerm2, WezTerm)
    ss << "\033]1337;File=inline=1;size=" << buffer.size();
    if (target_width_cols > 0) {
        ss << ";width=" << target_width_cols;
    } else {
        ss << ";width=auto";
    }
    ss << ";preserveAspectRatio=1:" << b64 << "\007\n";

    // 2. Kitty Graphics protocol sequence (for Kitty, Ghostty, WezTerm)
    int cols = (target_width_cols > 0) ? target_width_cols : 60;
    ss << emit_chunked_kitty_payload(b64, cols, 0, 100);

    return ss.str();
}

std::string TerminalImage::render_file_raster_escape(const std::string& filepath, int x, int y, int max_w, int max_h) {
    (void)x; (void)y;
    if (filepath.empty()) return "";
    auto decoded = graphics::ImageDecoder::decode_file(filepath);
    if (!decoded.is_valid()) return "";

    const auto& frame = decoded.frame(0);
    int src_w = frame.width;
    int src_h = frame.height;
    int disp_w = max_w > 0 ? max_w : 220;
    int disp_h = max_h > 0 ? max_h : 220;

    if (src_w > 0 && src_h > 0) {
        float scale = std::min(static_cast<float>(disp_w) / src_w, static_cast<float>(disp_h) / src_h);
        disp_w = static_cast<int>(std::round(src_w * scale));
        disp_h = static_cast<int>(std::round(src_h * scale));
    }
    if (disp_w <= 0) disp_w = 1;
    if (disp_h <= 0) disp_h = 1;

    std::ifstream f(filepath, std::ios::binary);
    if (!f.is_open()) return "";
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (buffer.empty()) return "";

    static const char b64_tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string b64;
    b64.reserve(((buffer.size() + 2) / 3) * 4);
    for (size_t k = 0; k < buffer.size(); k += 3) {
        uint32_t b = (buffer[k] << 16) | ((k + 1 < buffer.size() ? buffer[k + 1] : 0) << 8) | (k + 2 < buffer.size() ? buffer[k + 2] : 0);
        b64.push_back(b64_tbl[(b >> 18) & 0x3F]);
        b64.push_back(b64_tbl[(b >> 12) & 0x3F]);
        b64.push_back((k + 1 < buffer.size()) ? b64_tbl[(b >> 6) & 0x3F] : '=');
        b64.push_back((k + 2 < buffer.size()) ? b64_tbl[b & 0x3F] : '=');
    }

    std::ostringstream ss;
    ss << "\033_Ga=d,d=a\033\\" << emit_chunked_kitty_payload(b64, disp_w / 8, disp_h / 16, 100);
    return ss.str();
}

std::string TerminalImage::to_kitty_graphics_escape(int x, int y, int cols_spanned, int rows_spanned) const {
    (void)x; (void)y;
    if (!is_valid()) return "";

    std::vector<uint8_t> raw_rgba(width_ * height_ * 4);
    for (int i = 0; i < width_ * height_; ++i) {
        raw_rgba[i * 4 + 0] = pixels_[i].r;
        raw_rgba[i * 4 + 1] = pixels_[i].g;
        raw_rgba[i * 4 + 2] = pixels_[i].b;
        raw_rgba[i * 4 + 3] = pixels_[i].a;
    }

    static const char b64_tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string b64;
    b64.reserve(((raw_rgba.size() + 2) / 3) * 4);
    for (size_t k = 0; k < raw_rgba.size(); k += 3) {
        uint32_t b = (raw_rgba[k] << 16) | ((k + 1 < raw_rgba.size() ? raw_rgba[k + 1] : 0) << 8) | (k + 2 < raw_rgba.size() ? raw_rgba[k + 2] : 0);
        b64.push_back(b64_tbl[(b >> 18) & 0x3F]);
        b64.push_back(b64_tbl[(b >> 12) & 0x3F]);
        b64.push_back((k + 1 < raw_rgba.size()) ? b64_tbl[(b >> 6) & 0x3F] : '=');
        b64.push_back((k + 2 < raw_rgba.size()) ? b64_tbl[b & 0x3F] : '=');
    }

    std::ostringstream ss;
    ss << "\033_Ga=d,d=a\033\\" << emit_chunked_kitty_payload(b64, cols_spanned, rows_spanned, 32, width_, height_);
    return ss.str();
}

} // namespace meridian::core

