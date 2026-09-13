#include "image_decoder.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace meridian::graphics {

namespace {

// Helper: Decode binary PPM (P6)
bool decode_ppm(const uint8_t* data, size_t size, DecodedImage& out) {
    if (size < 10 || data[0] != 'P') return false;
    char type = static_cast<char>(data[1]);
    if (type != '6' && type != '3') return false;

    const char* ptr = reinterpret_cast<const char*>(data + 2);
    const char* end = reinterpret_cast<const char*>(data + size);

    auto skip_ws_and_comments = [&]() {
        while (ptr < end) {
            if (*ptr == '#') {
                while (ptr < end && *ptr != '\n') ++ptr;
            } else if (std::isspace(static_cast<unsigned char>(*ptr))) {
                ++ptr;
            } else {
                break;
            }
        }
    };

    skip_ws_and_comments();
    if (ptr >= end) return false;

    // Read width, height, maxval
    int w = 0, h = 0, maxval = 255;
    w = static_cast<int>(std::strtol(ptr, const_cast<char**>(&ptr), 10));
    skip_ws_and_comments();
    h = static_cast<int>(std::strtol(ptr, const_cast<char**>(&ptr), 10));
    skip_ws_and_comments();
    maxval = static_cast<int>(std::strtol(ptr, const_cast<char**>(&ptr), 10));

    if (w <= 0 || h <= 0 || maxval <= 0) return false;

    if (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) {
        ++ptr; // single whitespace after maxval in P6
    }

    ImageFrame frame;
    frame.width = w;
    frame.height = h;
    frame.duration = 0.1;
    frame.rgba.resize(w * h * 4);

    if (type == '6') { // Binary RGB
        const uint8_t* raw_pixels = reinterpret_cast<const uint8_t*>(ptr);
        size_t available = (data + size) - raw_pixels;
        size_t needed = static_cast<size_t>(w * h * 3);
        if (available < needed) return false;

        for (int i = 0; i < w * h; ++i) {
            frame.rgba[i * 4 + 0] = raw_pixels[i * 3 + 0];
            frame.rgba[i * 4 + 1] = raw_pixels[i * 3 + 1];
            frame.rgba[i * 4 + 2] = raw_pixels[i * 3 + 2];
            frame.rgba[i * 4 + 3] = 255;
        }
    } else { // P3 Plain Text
        for (int i = 0; i < w * h && ptr < end; ++i) {
            skip_ws_and_comments();
            int r = static_cast<int>(std::strtol(ptr, const_cast<char**>(&ptr), 10));
            skip_ws_and_comments();
            int g = static_cast<int>(std::strtol(ptr, const_cast<char**>(&ptr), 10));
            skip_ws_and_comments();
            int b = static_cast<int>(std::strtol(ptr, const_cast<char**>(&ptr), 10));

            frame.rgba[i * 4 + 0] = static_cast<uint8_t>(std::clamp(r * 255 / maxval, 0, 255));
            frame.rgba[i * 4 + 1] = static_cast<uint8_t>(std::clamp(g * 255 / maxval, 0, 255));
            frame.rgba[i * 4 + 2] = static_cast<uint8_t>(std::clamp(b * 255 / maxval, 0, 255));
            frame.rgba[i * 4 + 3] = 255;
        }
    }

    out.original_width = w;
    out.original_height = h;
    out.format = "ppm";
    out.is_animated = false;
    out.frames.push_back(std::move(frame));
    return true;
}

// Helper: Decode uncompressed Windows BMP
bool decode_bmp(const uint8_t* data, size_t size, DecodedImage& out) {
    if (size < 54 || data[0] != 'B' || data[1] != 'M') return false;

    uint32_t data_offset = *reinterpret_cast<const uint32_t*>(data + 10);
    uint32_t header_size = *reinterpret_cast<const uint32_t*>(data + 14);
    int32_t w = *reinterpret_cast<const int32_t*>(data + 18);
    int32_t h = *reinterpret_cast<const int32_t*>(data + 22);
    uint16_t bpp = *reinterpret_cast<const uint16_t*>(data + 28);
    uint32_t compression = *reinterpret_cast<const uint32_t*>(data + 30);

    if (header_size < 40 || w <= 0 || h == 0 || (bpp != 24 && bpp != 32) || compression != 0) return false;
    if (data_offset >= size) return false;

    bool flip_y = (h > 0);
    int abs_h = std::abs(h);

    ImageFrame frame;
    frame.width = w;
    frame.height = abs_h;
    frame.duration = 0.1;
    frame.rgba.resize(w * abs_h * 4);

    int row_stride = ((w * (bpp / 8) + 3) / 4) * 4;
    const uint8_t* pixel_src = data + data_offset;

    for (int y = 0; y < abs_h; ++y) {
        int dst_y = flip_y ? (abs_h - 1 - y) : y;
        const uint8_t* row_ptr = pixel_src + y * row_stride;
        if (row_ptr + w * (bpp / 8) > data + size) break;

        for (int x = 0; x < w; ++x) {
            int dst_idx = (dst_y * w + x) * 4;
            if (bpp == 24) {
                frame.rgba[dst_idx + 0] = row_ptr[x * 3 + 2]; // R
                frame.rgba[dst_idx + 1] = row_ptr[x * 3 + 1]; // G
                frame.rgba[dst_idx + 2] = row_ptr[x * 3 + 0]; // B
                frame.rgba[dst_idx + 3] = 255;                // A
            } else if (bpp == 32) {
                frame.rgba[dst_idx + 0] = row_ptr[x * 4 + 2]; // R
                frame.rgba[dst_idx + 1] = row_ptr[x * 4 + 1]; // G
                frame.rgba[dst_idx + 2] = row_ptr[x * 4 + 0]; // B
                frame.rgba[dst_idx + 3] = row_ptr[x * 4 + 3]; // A
            }
        }
    }

    out.original_width = w;
    out.original_height = abs_h;
    out.format = "bmp";
    out.is_animated = false;
    out.frames.push_back(std::move(frame));
    return true;
}

} // namespace

DecodedImage ImageDecoder::decode_memory(const uint8_t* data, size_t size, const std::string& hint) {
    DecodedImage out;
    if (!data || size < 4) return out;

    // 1. Try high-performance stb_image for PNG, JPEG, BMP, GIF, PSD, TGA
    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &channels, 4);
    if (pixels && w > 0 && h > 0) {
        // Enforce safety limit: max 16384 x 16384
        if (w > 16384 || h > 16384 || static_cast<uint64_t>(w) * h > 67108864ULL) {
            stbi_image_free(pixels);
            return out;
        }

        ImageFrame frame;
        frame.width = w;
        frame.height = h;
        frame.duration = 0.1;
        frame.rgba.assign(pixels, pixels + (static_cast<size_t>(w) * h * 4));
        stbi_image_free(pixels);

        out.original_width = w;
        out.original_height = h;
        out.format = hint.empty() ? "image" : hint;
        out.is_animated = false;
        out.frames.push_back(std::move(frame));
        return out;
    }

    // 2. Try PPM fallback
    if (decode_ppm(data, size, out)) return out;

    // 3. Try BMP fallback
    if (decode_bmp(data, size, out)) return out;

    return out;
}

DecodedImage ImageDecoder::decode_file(const std::string& filepath) {
    DecodedImage out;
    if (filepath.empty()) return out;

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

    std::ifstream file(actual_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return out;

    std::streamsize file_size = file.tellg();
    if (file_size <= 0) return out;
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(file_size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) return out;

    return decode_memory(buffer.data(), buffer.size(), actual_path);
}

ImageDimensions ImageDecoder::calculate_fit(
    int src_w, int src_h,
    float target_box_w, float target_box_h,
    ImageFitMode mode,
    float origin_x, float origin_y
) {
    ImageDimensions dims;
    if (src_w <= 0 || src_h <= 0 || target_box_w <= 0 || target_box_h <= 0) {
        return dims;
    }

    float aspect_src = static_cast<float>(src_w) / static_cast<float>(src_h);
    float aspect_target = target_box_w / target_box_h;

    switch (mode) {
        case ImageFitMode::Contain: {
            if (aspect_src > aspect_target) {
                // Width bound
                dims.width = target_box_w;
                dims.height = target_box_w / aspect_src;
                dims.x = origin_x;
                dims.y = origin_y + (target_box_h - dims.height) * 0.5f;
            } else {
                // Height bound
                dims.height = target_box_h;
                dims.width = target_box_h * aspect_src;
                dims.x = origin_x + (target_box_w - dims.width) * 0.5f;
                dims.y = origin_y;
            }
            break;
        }
        case ImageFitMode::Cover: {
            if (aspect_src > aspect_target) {
                dims.height = target_box_h;
                dims.width = target_box_h * aspect_src;
                dims.x = origin_x - (dims.width - target_box_w) * 0.5f;
                dims.y = origin_y;
            } else {
                dims.width = target_box_w;
                dims.height = target_box_w / aspect_src;
                dims.x = origin_x;
                dims.y = origin_y - (dims.height - target_box_h) * 0.5f;
            }
            break;
        }
        case ImageFitMode::Stretch: {
            dims.x = origin_x;
            dims.y = origin_y;
            dims.width = target_box_w;
            dims.height = target_box_h;
            break;
        }
    }

    return dims;
}

ImageFrame ImageDecoder::rescale_frame(
    const ImageFrame& src,
    int target_w, int target_h,
    ImageScaleFilter filter
) {
    ImageFrame dst;
    if (!src.is_valid() || target_w <= 0 || target_h <= 0) return dst;

    dst.width = target_w;
    dst.height = target_h;
    dst.duration = src.duration;
    dst.rgba.resize(target_w * target_h * 4);

    if (filter == ImageScaleFilter::Pixel) {
        // Nearest Neighbor
        for (int y = 0; y < target_h; ++y) {
            int src_y = std::clamp(y * src.height / target_h, 0, src.height - 1);
            for (int x = 0; x < target_w; ++x) {
                int src_x = std::clamp(x * src.width / target_w, 0, src.width - 1);
                int src_idx = (src_y * src.width + src_x) * 4;
                int dst_idx = (y * target_w + x) * 4;
                std::memcpy(&dst.rgba[dst_idx], &src.rgba[src_idx], 4);
            }
        }
    } else {
        // Bilinear Filtering
        for (int y = 0; y < target_h; ++y) {
            float v = static_cast<float>(y) / (target_h - 1);
            float py = v * (src.height - 1);
            int y0 = std::clamp(static_cast<int>(std::floor(py)), 0, src.height - 1);
            int y1 = std::clamp(y0 + 1, 0, src.height - 1);
            float fy = py - y0;

            for (int x = 0; x < target_w; ++x) {
                float u = static_cast<float>(x) / (target_w - 1);
                float px = u * (src.width - 1);
                int x0 = std::clamp(static_cast<int>(std::floor(px)), 0, src.width - 1);
                int x1 = std::clamp(x0 + 1, 0, src.width - 1);
                float fx = px - x0;

                int i00 = (y0 * src.width + x0) * 4;
                int i10 = (y0 * src.width + x1) * 4;
                int i01 = (y1 * src.width + x0) * 4;
                int i11 = (y1 * src.width + x1) * 4;

                int dst_idx = (y * target_w + x) * 4;
                for (int ch = 0; ch < 4; ++ch) {
                    float top = src.rgba[i00 + ch] * (1.0f - fx) + src.rgba[i10 + ch] * fx;
                    float bot = src.rgba[i01 + ch] * (1.0f - fx) + src.rgba[i11 + ch] * fx;
                    float val = top * (1.0f - fy) + bot * fy;
                    dst.rgba[dst_idx + ch] = static_cast<uint8_t>(std::clamp(val, 0.0f, 255.0f));
                }
            }
        }
    }

    return dst;
}

} // namespace meridian::graphics

