#include "graphics.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace meridian::vt {

namespace {

const std::string BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Default DEC Sixel color palette (16 classic registers)
const uint32_t DEFAULT_SIXEL_PALETTE[16] = {
    0xFF000000, // 0: Black
    0xFFFF0000, // 1: Blue (ABGR / ARGB normalized to 0xAARRGGBB) -> 0xFF0000FF in standard RGBA
    0xFF00FF00, // 2: Red
    0xFF00FFFF, // 3: Green
    0xFFFF00FF, // 4: Magenta
    0xFFFFFF00, // 5: Cyan
    0xFF00FFFF, // 6: Yellow
    0xFFFFFFFF, // 7: White
    0xFF808080, // 8: Gray
    0xFFFF8080, // 9: Light Blue
    0xFF80FF80, // 10: Light Red
    0xFF80FFFF, // 11: Light Green
    0xFFFF80FF, // 12: Light Magenta
    0xFFFFFF80, // 13: Light Cyan
    0xFF80FFFF, // 14: Light Yellow
    0xFFE0E0E0  // 15: Bright White
};

} // namespace

GraphicsEngine::GraphicsEngine() {
    clear_all();
}

std::vector<uint8_t> GraphicsEngine::decode_base64(std::string_view in) {
    std::vector<uint8_t> out;
    out.reserve((in.size() * 3) / 4);
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[static_cast<unsigned char>(BASE64_CHARS[i])] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (c == '=') break;
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t') continue;
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

void GraphicsEngine::clear_all() {
    images_.clear();
    placements_.clear();
    chunk_sessions_.clear();
    current_anonymous_session_ = ChunkSession{};
    next_image_id_ = 1;
    next_placement_id_ = 1;
}

const GraphicImage* GraphicsEngine::find_image(uint32_t id) const {
    auto it = images_.find(id);
    return it != images_.end() ? &it->second : nullptr;
}

void GraphicsEngine::send_kitty_response(uint32_t image_id, uint32_t placement_id, const std::string& msg) {
    if (!response_cb_) return;
    std::ostringstream ss;
    ss << "\033_G";
    if (image_id > 0) ss << "i=" << image_id << ";";
    if (placement_id > 0) ss << "p=" << placement_id << ";";
    ss << msg << "\033\\";
    response_cb_(ss.str());
}

bool GraphicsEngine::handle_kitty_graphics(std::string_view payload, int cursor_row, int cursor_col) {
    if (payload.empty()) return false;

    // Kitty graphics control syntax: key=val,key=val;payload_data
    auto semi = payload.find(';');
    std::string_view keys_part = (semi != std::string_view::npos) ? payload.substr(0, semi) : payload;
    std::string_view chunk_data = (semi != std::string_view::npos) ? payload.substr(semi + 1) : "";

    // Default control parameters
    KittyAction action = KittyAction::TransmitAndDisplay;
    ImageFormat fmt = ImageFormat::Rgba32;
    TransmissionType trans = TransmissionType::Direct;
    uint32_t image_id = 0;
    uint32_t placement_id = 0;
    int width = 0;
    int height = 0;
    int cols = 0;
    int rows = 0;
    int z_index = 0;
    int more_chunks = 0; // m=0 (last chunk), m=1 (more chunks)
    bool quiet = false;
    char delete_action = ' ';

    // Parse comma-delimited key=value pairs
    std::size_t pos = 0;
    while (pos < keys_part.size()) {
        auto comma = keys_part.find(',', pos);
        std::string_view token = keys_part.substr(pos, (comma == std::string_view::npos) ? std::string_view::npos : comma - pos);
        auto eq = token.find('=');
        if (eq != std::string_view::npos) {
            std::string_view k = token.substr(0, eq);
            std::string_view v = token.substr(eq + 1);

            if (k == "a") {
                if (v == "t") action = KittyAction::Transmit;
                else if (v == "T") action = KittyAction::TransmitAndDisplay;
                else if (v == "p") action = KittyAction::Put;
                else if (v == "q") action = KittyAction::Query;
                else if (v == "d" || v == "D") action = KittyAction::Delete;
            } else if (k == "f") {
                int f = std::stoi(std::string(v));
                if (f == 24) fmt = ImageFormat::Rgb24;
                else if (f == 32) fmt = ImageFormat::Rgba32;
                else if (f == 100) fmt = ImageFormat::Png;
            } else if (k == "t") {
                if (v == "d") trans = TransmissionType::Direct;
                else if (v == "f") trans = TransmissionType::File;
                else if (v == "t") trans = TransmissionType::TempFile;
                else if (v == "s") trans = TransmissionType::SharedMem;
            } else if (k == "i") {
                image_id = static_cast<uint32_t>(std::stoul(std::string(v)));
            } else if (k == "p") {
                placement_id = static_cast<uint32_t>(std::stoul(std::string(v)));
            } else if (k == "s") {
                width = std::stoi(std::string(v));
            } else if (k == "v") {
                height = std::stoi(std::string(v));
            } else if (k == "c") {
                cols = std::stoi(std::string(v));
            } else if (k == "r") {
                rows = std::stoi(std::string(v));
            } else if (k == "z") {
                z_index = std::stoi(std::string(v));
            } else if (k == "m") {
                more_chunks = std::stoi(std::string(v));
            } else if (k == "q") {
                quiet = (std::stoi(std::string(v)) > 0);
            } else if (k == "d") {
                if (!v.empty()) delete_action = v[0];
            }
        }
        if (comma == std::string_view::npos) break;
        pos = comma + 1;
    }

    // Handle Delete Action
    if (action == KittyAction::Delete) {
        if (delete_action == 'a' || delete_action == 'A') {
            clear_all();
        } else if (image_id > 0) {
            images_.erase(image_id);
            placements_.erase(
                std::remove_if(placements_.begin(), placements_.end(),
                               [image_id](const ImagePlacement& p) { return p.image_id == image_id; }),
                placements_.end());
        }
        if (!quiet) send_kitty_response(image_id, placement_id, "OK");
        return true;
    }

    // Handle Put Action (display an already loaded image)
    if (action == KittyAction::Put) {
        if (image_id > 0 && images_.find(image_id) != images_.end()) {
            const auto& img = images_[image_id];
            ImagePlacement pl;
            pl.image_id = image_id;
            pl.placement_id = (placement_id > 0) ? placement_id : next_placement_id_++;
            pl.row = cursor_row;
            pl.col = cursor_col;
            pl.cols_spanned = (cols > 0) ? cols : std::max(1, img.width / 10);
            pl.rows_spanned = (rows > 0) ? rows : std::max(1, img.height / 20);
            pl.z_index = z_index;
            placements_.push_back(pl);
            if (!quiet) send_kitty_response(image_id, pl.placement_id, "OK");
            return true;
        }
        if (!quiet) send_kitty_response(image_id, placement_id, "ENOENT");
        return false;
    }

    // Handle Transmissions (with multi-chunk buffering)
    ChunkSession* sess = nullptr;
    if (image_id > 0) {
        sess = &chunk_sessions_[image_id];
        sess->image_id = image_id;
    } else {
        sess = &current_anonymous_session_;
        if (sess->image_id == 0) {
            sess->image_id = next_image_id_++;
        }
    }

    // Update metadata if specified on first chunk
    if (width > 0) sess->width = width;
    if (height > 0) sess->height = height;
    if (cols > 0) sess->cols = cols;
    if (rows > 0) sess->rows = rows;
    sess->format = fmt;
    sess->trans_type = trans;
    sess->action = action;
    sess->z_index = z_index;
    sess->quiet = quiet;

    // Append chunk payload
    sess->accumulated_payload.append(chunk_data.data(), chunk_data.size());

    // If more chunks are expected (m=1), keep waiting
    if (more_chunks == 1) {
        return true;
    }

    // Complete transmission: process accumulated data
    process_kitty_command(*sess, cursor_row, cursor_col);

    if (image_id == 0) {
        current_anonymous_session_ = ChunkSession{};
    } else {
        chunk_sessions_.erase(image_id);
    }

    return true;
}

void GraphicsEngine::process_kitty_command(const ChunkSession& session, int cursor_row, int cursor_col) {
    std::vector<uint8_t> raw_bytes;

    if (session.trans_type == TransmissionType::File || session.trans_type == TransmissionType::TempFile) {
        // Direct file read
        std::string filepath = session.accumulated_payload;
        std::ifstream file(filepath, std::ios::binary);
        if (file.is_open()) {
            raw_bytes = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                              std::istreambuf_iterator<char>());
        }
        if (session.trans_type == TransmissionType::TempFile) {
            std::remove(filepath.c_str());
        }
    } else {
        // Base64 decoded payload
        raw_bytes = decode_base64(session.accumulated_payload);
    }

    int w = session.width > 0 ? session.width : 64;
    int h = session.height > 0 ? session.height : 64;

    GraphicImage img;
    img.id = session.image_id;
    img.width = w;
    img.height = h;
    img.format = session.format;

    if (session.format == ImageFormat::Rgb24) {
        // Convert 24-bit RGB -> 32-bit RGBA
        img.rgba_data.resize(w * h * 4, 255);
        size_t pixel_count = std::min<size_t>(w * h, raw_bytes.size() / 3);
        for (size_t i = 0; i < pixel_count; ++i) {
            img.rgba_data[i * 4 + 0] = raw_bytes[i * 3 + 0];
            img.rgba_data[i * 4 + 1] = raw_bytes[i * 3 + 1];
            img.rgba_data[i * 4 + 2] = raw_bytes[i * 3 + 2];
            img.rgba_data[i * 4 + 3] = 255;
        }
    } else {
        img.rgba_data = std::move(raw_bytes);
    }

    images_[img.id] = std::move(img);

    if (session.action == KittyAction::TransmitAndDisplay) {
        ImagePlacement pl;
        pl.image_id = session.image_id;
        pl.placement_id = next_placement_id_++;
        pl.row = cursor_row;
        pl.col = cursor_col;
        pl.cols_spanned = (session.cols > 0) ? session.cols : std::max(1, w / 10);
        pl.rows_spanned = (session.rows > 0) ? session.rows : std::max(1, h / 20);
        pl.z_index = session.z_index;
        placements_.push_back(pl);
    }

    if (!session.quiet) {
        send_kitty_response(session.image_id, 0, "OK");
    }
}

// DEC Sixel Protocol Parser
bool GraphicsEngine::handle_sixel(std::string_view sixel_stream, int cursor_row, int cursor_col,
                                  int cell_width, int cell_height) {
    if (sixel_stream.empty()) return false;

    // Palette table (up to 256 colors)
    std::vector<uint32_t> palette(256);
    for (int i = 0; i < 16; ++i) {
        palette[i] = DEFAULT_SIXEL_PALETTE[i];
    }

    int cur_color = 0;
    int x = 0;
    int y = 0;
    int max_x = 0;
    int max_y = 0;

    // Dynamic pixel canvas (32-bit RGBA)
    std::vector<uint32_t> canvas;
    int canvas_width = 256;
    int canvas_height = 256;
    canvas.resize(canvas_width * canvas_height, 0x00000000); // transparent background

    auto ensure_size = [&](int req_w, int req_h) {
        if (req_w <= canvas_width && req_h <= canvas_height) return;
        int new_w = std::max(canvas_width, req_w);
        int new_h = std::max(canvas_height, req_h);
        if (new_w > canvas_width || new_h > canvas_height) {
            new_w = std::max(new_w, canvas_width * 2);
            new_h = std::max(new_h, canvas_height * 2);
            std::vector<uint32_t> new_canvas(new_w * new_h, 0x00000000);
            for (int r = 0; r < canvas_height; ++r) {
                for (int c = 0; c < canvas_width; ++c) {
                    new_canvas[r * new_w + c] = canvas[r * canvas_width + c];
                }
            }
            canvas = std::move(new_canvas);
            canvas_width = new_w;
            canvas_height = new_h;
        }
    };

    auto plot_sixel_char = [&](char c, int repeat) {
        if (c < '?' || c > '~') return;
        uint8_t sixel_bits = static_cast<uint8_t>(c - '?');
        uint32_t color = palette[cur_color & 0xFF];

        ensure_size(x + repeat, y + 6);

        for (int dx = 0; dx < repeat; ++dx) {
            for (int b = 0; b < 6; ++b) {
                if ((sixel_bits >> b) & 1) {
                    canvas[(y + b) * canvas_width + (x + dx)] = color;
                }
            }
        }
        x += repeat;
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y + 6);
    };

    size_t i = 0;
    while (i < sixel_stream.size()) {
        char ch = sixel_stream[i];

        if (ch == '#') {
            // Palette introduction: #<id> or #<id>;2;<r_pct>;<g_pct>;<b_pct>
            ++i;
            int reg_id = 0;
            while (i < sixel_stream.size() && sixel_stream[i] >= '0' && sixel_stream[i] <= '9') {
                reg_id = reg_id * 10 + (sixel_stream[i] - '0');
                ++i;
            }
            cur_color = reg_id;

            if (i < sixel_stream.size() && sixel_stream[i] == ';') {
                ++i;
                int format = 0;
                while (i < sixel_stream.size() && sixel_stream[i] >= '0' && sixel_stream[i] <= '9') {
                    format = format * 10 + (sixel_stream[i] - '0');
                    ++i;
                }
                if (format == 2 && i < sixel_stream.size() && sixel_stream[i] == ';') {
                    // RGB percentages (0-100)
                    ++i;
                    int r_pct = 0, g_pct = 0, b_pct = 0;
                    while (i < sixel_stream.size() && sixel_stream[i] >= '0' && sixel_stream[i] <= '9') {
                        r_pct = r_pct * 10 + (sixel_stream[i] - '0');
                        ++i;
                    }
                    if (i < sixel_stream.size() && sixel_stream[i] == ';') ++i;
                    while (i < sixel_stream.size() && sixel_stream[i] >= '0' && sixel_stream[i] <= '9') {
                        g_pct = g_pct * 10 + (sixel_stream[i] - '0');
                        ++i;
                    }
                    if (i < sixel_stream.size() && sixel_stream[i] == ';') ++i;
                    while (i < sixel_stream.size() && sixel_stream[i] >= '0' && sixel_stream[i] <= '9') {
                        b_pct = b_pct * 10 + (sixel_stream[i] - '0');
                        ++i;
                    }
                    uint8_t r = static_cast<uint8_t>(std::clamp(r_pct * 255 / 100, 0, 255));
                    uint8_t g = static_cast<uint8_t>(std::clamp(g_pct * 255 / 100, 0, 255));
                    uint8_t b = static_cast<uint8_t>(std::clamp(b_pct * 255 / 100, 0, 255));
                    palette[reg_id & 0xFF] = (0xFF000000 | (r << 16) | (g << 8) | b);
                }
            }
            continue;
        }

        if (ch == '!') {
            // Repeat count: !<count><sixel_char>
            ++i;
            int count = 0;
            while (i < sixel_stream.size() && sixel_stream[i] >= '0' && sixel_stream[i] <= '9') {
                count = count * 10 + (sixel_stream[i] - '0');
                ++i;
            }
            if (i < sixel_stream.size()) {
                plot_sixel_char(sixel_stream[i], std::max(1, count));
                ++i;
            }
            continue;
        }

        if (ch == '$') {
            // Graphics carriage return
            x = 0;
            ++i;
            continue;
        }

        if (ch == '-') {
            // Graphics line feed (down 6 pixels, return left)
            x = 0;
            y += 6;
            ++i;
            continue;
        }

        if (ch >= '?' && ch <= '~') {
            plot_sixel_char(ch, 1);
            ++i;
            continue;
        }

        ++i;
    }

    if (max_x <= 0 || max_y <= 0) return false;

    // Build final tight RGBA GraphicImage
    uint32_t img_id = next_image_id_++;
    GraphicImage img;
    img.id = img_id;
    img.width = max_x;
    img.height = max_y;
    img.format = ImageFormat::Sixel;
    img.rgba_data.resize(max_x * max_y * 4);

    for (int r = 0; r < max_y; ++r) {
        for (int c = 0; c < max_x; ++c) {
            uint32_t color = canvas[r * canvas_width + c];
            size_t out_idx = (r * max_x + c) * 4;
            img.rgba_data[out_idx + 0] = (color >> 16) & 0xFF; // R
            img.rgba_data[out_idx + 1] = (color >> 8) & 0xFF;  // G
            img.rgba_data[out_idx + 2] = color & 0xFF;         // B
            img.rgba_data[out_idx + 3] = (color >> 24) & 0xFF; // A
        }
    }

    images_[img_id] = std::move(img);

    ImagePlacement pl;
    pl.image_id = img_id;
    pl.placement_id = next_placement_id_++;
    pl.row = cursor_row;
    pl.col = cursor_col;
    pl.cols_spanned = std::max(1, static_cast<int>(std::ceil(static_cast<double>(max_x) / cell_width)));
    pl.rows_spanned = std::max(1, static_cast<int>(std::ceil(static_cast<double>(max_y) / cell_height)));
    pl.z_index = 0;
    placements_.push_back(pl);

    return true;
}

} // namespace meridian::vt
