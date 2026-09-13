// src/core/graphics/graphics_protocol_parser.cpp
#include "graphics_protocol_parser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace meridian::graphics {

GraphicsProtocolParser::GraphicsProtocolParser() = default;

GraphicsProtocolType GraphicsProtocolParser::detect_protocol(const std::string& seq) {
    if (seq.rfind("\033_G", 0) == 0 || seq.rfind("\x1b_G", 0) == 0) {
        return GraphicsProtocolType::Kitty;
    }
    if (seq.rfind("\033]1337;", 0) == 0 || seq.rfind("\x1b]1337;", 0) == 0) {
        return GraphicsProtocolType::ITerm2;
    }
    if (seq.rfind("\033Pq", 0) == 0 || seq.rfind("\x1bPq", 0) == 0 ||
        seq.rfind("\033P0;0;q", 0) == 0) {
        return GraphicsProtocolType::Sixel;
    }
    return GraphicsProtocolType::None;
}

bool GraphicsProtocolParser::decode_base64(const std::string& input, std::vector<uint8_t>& output) {
    output.clear();
    if (input.empty()) return true;

    static const int tbl[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1, 0,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };

    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;
        if (c == '=') break;
        int d = tbl[c];
        if (d == -1) continue;
        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0) {
            output.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return true;
}

std::string GraphicsProtocolParser::encode_base64(const uint8_t* data, size_t length) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    if (!data || length == 0) return out;

    out.reserve(((length + 2) / 3) * 4);
    for (size_t i = 0; i < length; i += 3) {
        uint32_t b = (data[i] << 16) |
                     ((i + 1 < length ? data[i + 1] : 0) << 8) |
                     (i + 2 < length ? data[i + 2] : 0);
        out.push_back(tbl[(b >> 18) & 0x3F]);
        out.push_back(tbl[(b >> 12) & 0x3F]);
        out.push_back((i + 1 < length) ? tbl[(b >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < length) ? tbl[b & 0x3F] : '=');
    }
    return out;
}

bool GraphicsProtocolParser::parse_kitty_sequence(const std::string& seq, ParsedGraphicsCommand& out) {
    out = ParsedGraphicsCommand{};
    out.protocol = GraphicsProtocolType::Kitty;

    // Sequence format: ESC _ G control-keys ; payload ESC ST
    size_t start_pos = seq.find("\033_G");
    if (start_pos == std::string::npos) start_pos = seq.find("\x1b_G");
    if (start_pos == std::string::npos) return false;
    start_pos += 3;

    size_t semi = seq.find(';', start_pos);
    size_t end = seq.find("\033\\", start_pos);
    if (end == std::string::npos) end = seq.find("\x1b\\", start_pos);
    if (end == std::string::npos) end = seq.size();

    std::string control = (semi != std::string::npos && semi < end)
        ? seq.substr(start_pos, semi - start_pos)
        : seq.substr(start_pos, end - start_pos);

    std::string b64_data = (semi != std::string::npos && semi + 1 < end)
        ? seq.substr(semi + 1, end - (semi + 1))
        : "";

    // Parse control keys: e.g. a=T,f=100,s=800,v=600,c=40,r=20,m=0
    std::stringstream ss(control);
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t eq = token.find('=');
        if (eq == std::string::npos) continue;
        char key = token[0];
        std::string val = token.substr(eq + 1);

        switch (key) {
            case 'a':
                if (val == "T" || val == "t") out.action = "transmit";
                else if (val == "p" || val == "P") out.action = "display";
                else if (val == "d" || val == "D") out.action = "delete";
                else if (val == "q" || val == "Q") out.action = "query";
                else if (val == "a" || val == "A") {
                    out.action = "animate";
                    out.is_animation = true;
                }
                break;
            case 'f':
                try { out.format = std::stoi(val); } catch (...) {}
                break;
            case 's':
                try { out.width = std::stoi(val); } catch (...) {}
                break;
            case 'v':
                try { out.height = std::stoi(val); } catch (...) {}
                break;
            case 'c':
                try { out.cols = std::stoi(val); } catch (...) {}
                try { out.frame_index = std::stoi(val); } catch (...) {}
                break;
            case 'r':
                try { out.rows = std::stoi(val); } catch (...) {}
                try { out.loop_count = std::stoi(val); } catch (...) {}
                break;
            case 'i':
                try { out.image_id = static_cast<uint32_t>(std::stoul(val)); } catch (...) {}
                break;
            case 'p':
                try { out.placement_id = static_cast<uint32_t>(std::stoul(val)); } catch (...) {}
                break;
            case 'z':
                try { out.z_index = std::stoi(val); } catch (...) {}
                try { out.frame_delay_ms = std::stoi(val); } catch (...) {}
                break;
            case 'm':
                out.more_chunks = (val == "1");
                break;
            default:
                break;
        }
    }

    decode_base64(b64_data, out.payload);
    return true;
}

bool GraphicsProtocolParser::parse_iterm2_sequence(const std::string& seq, ParsedGraphicsCommand& out) {
    out = ParsedGraphicsCommand{};
    out.protocol = GraphicsProtocolType::ITerm2;

    // Sequence format: \033]1337;File=[args]:<base64>\007
    size_t start = seq.find("1337;File=");
    if (start == std::string::npos) return false;
    start += 10;

    size_t colon = seq.find(':', start);
    size_t end = seq.find('\007', start);
    if (end == std::string::npos) end = seq.find("\033\\", start);
    if (end == std::string::npos) end = seq.size();

    std::string args = (colon != std::string::npos && colon < end)
        ? seq.substr(start, colon - start)
        : "";

    std::string b64_data = (colon != std::string::npos && colon + 1 < end)
        ? seq.substr(colon + 1, end - (colon + 1))
        : "";

    std::stringstream ss(args);
    std::string token;
    while (std::getline(ss, token, ';')) {
        size_t eq = token.find('=');
        if (eq == std::string::npos) continue;
        std::string key = token.substr(0, eq);
        std::string val = token.substr(eq + 1);

        if (key == "name") {
            std::vector<uint8_t> name_dec;
            decode_base64(val, name_dec);
            out.filename.assign(name_dec.begin(), name_dec.end());
        } else if (key == "width") {
            if (!val.empty() && val != "auto") {
                try { out.width = std::stoi(val); } catch (...) {}
            }
        } else if (key == "height") {
            if (!val.empty() && val != "auto") {
                try { out.height = std::stoi(val); } catch (...) {}
            }
        } else if (key == "preserveAspectRatio") {
            out.preserve_aspect = (val != "0");
        } else if (key == "inline") {
            out.is_inline = (val == "1");
        }
    }

    out.format = 100; // Raw file container (PNG, JPEG, etc.)
    out.action = "display";
    decode_base64(b64_data, out.payload);
    return true;
}

bool GraphicsProtocolParser::parse_sixel_sequence(const std::string& seq, ParsedGraphicsCommand& out) {
    out = ParsedGraphicsCommand{};
    out.protocol = GraphicsProtocolType::Sixel;
    out.action = "display";
    out.format = 100;

    size_t start = seq.find('q');
    if (start == std::string::npos) return false;
    start++;

    size_t end = seq.find("\033\\", start);
    if (end == std::string::npos) end = seq.find("\x1b\\", start);
    if (end == std::string::npos) end = seq.size();

    std::string payload_str = seq.substr(start, end - start);
    out.payload.assign(payload_str.begin(), payload_str.end());
    return true;
}

} // namespace meridian::graphics
