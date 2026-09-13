#include "ansi_parser.hpp"

#include <algorithm>
#include <cstdlib>

namespace meridian::vt {

AnsiParser::AnsiParser(ScreenBuffer& screen) : screen_(screen) {}

void AnsiParser::feed(const std::string& data) {
    feed(data.data(), data.size());
}

void AnsiParser::feed(const char* data, std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        feed_byte(static_cast<uint8_t>(data[i]));
    }
}

void AnsiParser::feed_byte(uint8_t b) {
    switch (state_) {
        case State::Ground:            handle_ground(b); return;
        case State::Escape:            handle_escape(b); return;
        case State::CsiParam:          handle_csi(b); return;
        case State::OscString:         handle_osc(b); return;
        case State::OscDiscard:        handle_osc_discard(b); return;
        case State::ApcString:         handle_apc(b); return;
        case State::DcsString:         handle_dcs(b); return;
        case State::Utf8Continuation:  handle_utf8_continuation(b); return;
    }
}

void AnsiParser::handle_ground(uint8_t b) {
    if (b == 0x1B) { state_ = State::Escape; return; }
    if (b == '\n' || b == 0x0B || b == 0x0C) { screen_.newline(); return; }
    if (b == '\r') { screen_.carriage_return(); return; }
    if (b == '\b') { screen_.backspace(); return; }
    if (b == '\t') { screen_.tab(); return; }
    if (b < 0x20) { return; } // other C0 controls: ignore

    if (b < 0x80) {
        screen_.put_codepoint(static_cast<char32_t>(b), current_attrs_);
        return;
    }
    if ((b & 0xE0) == 0xC0) { utf8_codepoint_ = b & 0x1F; utf8_remaining_ = 1; state_ = State::Utf8Continuation; return; }
    if ((b & 0xF0) == 0xE0) { utf8_codepoint_ = b & 0x0F; utf8_remaining_ = 2; state_ = State::Utf8Continuation; return; }
    if ((b & 0xF8) == 0xF0) { utf8_codepoint_ = b & 0x07; utf8_remaining_ = 3; state_ = State::Utf8Continuation; return; }
}

void AnsiParser::handle_utf8_continuation(uint8_t b) {
    if ((b & 0xC0) == 0x80) {
        utf8_codepoint_ = (utf8_codepoint_ << 6) | (b & 0x3F);
        if (--utf8_remaining_ == 0) {
            screen_.put_codepoint(utf8_codepoint_, current_attrs_);
            state_ = State::Ground;
        }
        return;
    }
    utf8_remaining_ = 0;
    state_ = State::Ground;
    handle_ground(b);
}

void AnsiParser::handle_escape(uint8_t b) {
    if (b == '[') { reset_csi(); state_ = State::CsiParam; return; }
    if (b == ']') { reset_osc(); state_ = State::OscString; return; }
    if (b == '_') { reset_apc(); state_ = State::ApcString; return; } // APC (Kitty Graphics)
    if (b == 'P') { reset_dcs(); state_ = State::DcsString; return; } // DCS (DEC Sixel)
    if (b == '7') { screen_.save_cursor(); state_ = State::Ground; return; }
    if (b == '8') { screen_.restore_cursor(); state_ = State::Ground; return; }
    if (b == 'c') {
        screen_.erase_in_display(2);
        screen_.set_cursor(0, 0);
        current_attrs_ = Attributes{};
        state_ = State::Ground;
        return;
    }
    if (b == 'D') { screen_.newline(); state_ = State::Ground; return; }
    if (b == 'M') { screen_.move_cursor_rel(-1, 0); state_ = State::Ground; return; }
    if (b == 'E') { screen_.carriage_return(); screen_.newline(); state_ = State::Ground; return; }
    if (b == '\\') { state_ = State::Ground; return; } // String Terminator (ST)
    state_ = State::Ground;
}

void AnsiParser::handle_apc(uint8_t b) {
    // APC string terminator: ESC \ or BEL
    if (b == 0x07) {
        if (apc_buffer_.rfind("G", 0) == 0) {
            screen_.graphics().handle_kitty_graphics(std::string_view(apc_buffer_).substr(1),
                                                    screen_.cursor_row(), screen_.cursor_col());
        }
        state_ = State::Ground;
        return;
    }
    if (b == 0x1B) {
        // Look for string terminator (ST) '\'
        if (apc_buffer_.rfind("G", 0) == 0) {
            screen_.graphics().handle_kitty_graphics(std::string_view(apc_buffer_).substr(1),
                                                    screen_.cursor_row(), screen_.cursor_col());
        }
        state_ = State::Escape;
        return;
    }
    if (apc_buffer_.size() < kMaxApcLen) {
        apc_buffer_.push_back(static_cast<char>(b));
    }
}

void AnsiParser::handle_dcs(uint8_t b) {
    // DCS string terminator: ESC \ or BEL
    if (b == 0x07) {
        if (dcs_buffer_.rfind("q", 0) == 0) {
            screen_.graphics().handle_sixel(std::string_view(dcs_buffer_).substr(1),
                                          screen_.cursor_row(), screen_.cursor_col());
        }
        state_ = State::Ground;
        return;
    }
    if (b == 0x1B) {
        if (dcs_buffer_.rfind("q", 0) == 0) {
            screen_.graphics().handle_sixel(std::string_view(dcs_buffer_).substr(1),
                                          screen_.cursor_row(), screen_.cursor_col());
        }
        state_ = State::Escape;
        return;
    }
    if (dcs_buffer_.size() < kMaxDcsLen) {
        dcs_buffer_.push_back(static_cast<char>(b));
    }
}

void AnsiParser::handle_csi(uint8_t b) {
    if (b == 0x1B) { reset_csi(); state_ = State::Escape; return; }
    if ((b == '?' || b == '>' || b == '<' || b == '=') && csi_params_.empty() && csi_prefix_ == 0) {
        csi_prefix_ = static_cast<char>(b);
        if (b == '?') csi_private_ = true;
        return;
    }
    if ((b >= '0' && b <= '9') || b == ';' || b == ':') {
        if (csi_params_.size() < kMaxCsiLen) csi_params_.push_back(static_cast<char>(b));
        return;
    }
    if (b >= 0x40 && b <= 0x7E) {
        dispatch_csi(b);
        state_ = State::Ground;
        return;
    }
}

static std::string base64_decode_string(const std::string& in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; ++i) {
        T[static_cast<uint8_t>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i])] = i;
    }
    int val = 0, valb = -8;
    for (uint8_t c : in) {
        if (c == '=') break;
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

void AnsiParser::dispatch_osc() {
    auto semi = osc_buffer_.find(';');
    if (semi == std::string::npos) return;
    std::string code = osc_buffer_.substr(0, semi);
    std::string payload = osc_buffer_.substr(semi + 1);

    if (code == "0" || code == "2") {
        window_title_ = payload;
    } else if (code == "7") {
        // OSC 7: file://hostname/path
        std::string path = payload;
        auto file_pos = path.find("file://");
        if (file_pos != std::string::npos) {
            path = path.substr(file_pos + 7);
            auto slash_pos = path.find('/');
            if (slash_pos != std::string::npos) {
                path = path.substr(slash_pos);
            }
        }
        screen_.set_working_directory(path);
    } else if (code == "8") {
        // OSC 8: 8;params;url
        auto next_semi = payload.find(';');
        if (next_semi != std::string::npos) {
            std::string params = payload.substr(0, next_semi);
            std::string url = payload.substr(next_semi + 1);
            if (url.empty()) {
                current_attrs_.hyperlink_id = 0;
            } else {
                current_attrs_.hyperlink_id = screen_.register_hyperlink(url, params);
            }
        } else {
            current_attrs_.hyperlink_id = 0;
        }
    } else if (code == "52") {
        // OSC 52: 52;c;base64 or 52;p;base64
        auto next_semi = payload.find(';');
        if (next_semi != std::string::npos) {
            std::string b64 = payload.substr(next_semi + 1);
            screen_.set_clipboard(base64_decode_string(b64));
        }
    } else if (code == "133") {
        // OSC 133 Shell Integration / Semantic Prompts
        if (!payload.empty()) {
            char type = payload[0];
            if (type == 'A') {
                screen_.set_semantic_state(ScreenBuffer::SemanticPromptState::Prompt);
            } else if (type == 'B') {
                screen_.set_semantic_state(ScreenBuffer::SemanticPromptState::CommandInput);
            } else if (type == 'C') {
                screen_.set_semantic_state(ScreenBuffer::SemanticPromptState::CommandOutput);
            } else if (type == 'D') {
                int exit_code = 0;
                auto code_semi = payload.find(';');
                if (code_semi != std::string::npos) {
                    try { exit_code = std::stoi(payload.substr(code_semi + 1)); } catch (...) {}
                }
                screen_.set_semantic_state(ScreenBuffer::SemanticPromptState::CommandFinished, exit_code);
            }
        }
    }
}

void AnsiParser::handle_osc(uint8_t b) {
    if (b == 0x07) {
        dispatch_osc();
        state_ = State::Ground;
        return;
    }
    if (b == 0x1B) {
        dispatch_osc();
        state_ = State::Escape;
        return;
    }
    if (osc_buffer_.size() < kMaxOscLen) {
        osc_buffer_.push_back(static_cast<char>(b));
    } else {
        state_ = State::OscDiscard;
    }
}

void AnsiParser::handle_osc_discard(uint8_t b) {
    if (b == 0x07 || b == 0x1B) state_ = State::Ground;
}

void AnsiParser::reset_csi() {
    csi_params_.clear();
    csi_private_ = false;
    csi_prefix_ = 0;
}

void AnsiParser::reset_osc() {
    osc_buffer_.clear();
}

void AnsiParser::reset_apc() {
    apc_buffer_.clear();
}

void AnsiParser::reset_dcs() {
    dcs_buffer_.clear();
}

std::vector<long> AnsiParser::parse_params() const {
    std::vector<long> out;
    if (csi_params_.empty()) return out;
    std::size_t start = 0;
    while (start <= csi_params_.size()) {
        std::size_t end = csi_params_.find_first_of(";:", start);
        if (end == std::string::npos) end = csi_params_.size();
        if (end == start) {
            out.push_back(0);
        } else {
            char* endp = nullptr;
            long v = std::strtol(csi_params_.c_str() + start, &endp, 10);
            out.push_back(v);
        }
        start = end + 1;
    }
    return out;
}

void AnsiParser::dispatch_csi(uint8_t final_byte) {
    auto p = parse_params();
    auto p0 = !p.empty() ? p[0] : 0;
    auto p1 = p.size() > 1 ? p[1] : 0;

    // Kitty Keyboard Protocol (CSI > flags u, CSI < flags u, CSI = flags u, CSI ? u)
    if (final_byte == 'u' && (csi_prefix_ == '>' || csi_prefix_ == '<' || csi_prefix_ == '=' || csi_prefix_ == '?')) {
        dispatch_kitty_keyboard(csi_prefix_, p);
        return;
    }

    if (csi_private_) {
        if (final_byte == 'h') dispatch_decset(p, true);
        else if (final_byte == 'l') dispatch_decset(p, false);
        return;
    }

    switch (final_byte) {
        case 'm': dispatch_sgr(p); break;
        case 'A': screen_.move_cursor_rel(-std::max(1L, p0), 0); break;
        case 'B': screen_.move_cursor_rel(std::max(1L, p0), 0); break;
        case 'C': screen_.move_cursor_rel(0, std::max(1L, p0)); break;
        case 'D': screen_.move_cursor_rel(0, -std::max(1L, p0)); break;
        case 'H':
        case 'f':
            screen_.set_cursor(static_cast<int>(p0 > 0 ? p0 - 1 : 0),
                              static_cast<int>(p1 > 0 ? p1 - 1 : 0));
            break;
        case 'J': screen_.erase_in_display(static_cast<int>(p0)); break;
        case 'K': screen_.erase_in_line(static_cast<int>(p0)); break;
        case 'L': screen_.insert_lines(static_cast<int>(std::max(1L, p0))); break;
        case 'M': screen_.delete_lines(static_cast<int>(std::max(1L, p0))); break;
        case 'P': screen_.delete_chars(static_cast<int>(std::max(1L, p0))); break;
        case '@': screen_.insert_chars(static_cast<int>(std::max(1L, p0))); break;
        case 'r':
            screen_.set_scroll_region(static_cast<int>(p0), static_cast<int>(p1));
            break;
        case 'S': screen_.scroll_up(static_cast<int>(std::max(1L, p0))); break;
        case 'T': screen_.scroll_down(static_cast<int>(std::max(1L, p0))); break;
        case 's': screen_.save_cursor(); break;
        case 'u': screen_.restore_cursor(); break;
        default: break;
    }
}

void AnsiParser::dispatch_kitty_keyboard(char prefix, const std::vector<long>& params) {
    if (params.empty()) return;
    if (prefix == '>') {
        // Push keyboard mode flags
        kitty_keyboard_flags_ |= static_cast<uint32_t>(params[0]);
    } else if (prefix == '<') {
        // Pop / clear keyboard mode flags
        kitty_keyboard_flags_ &= ~static_cast<uint32_t>(params[0]);
    } else if (prefix == '=') {
        // Set exact keyboard mode flags
        kitty_keyboard_flags_ = static_cast<uint32_t>(params[0]);
    }
}

void AnsiParser::dispatch_decset(const std::vector<long>& params, bool set) {
    for (auto code : params) {
        if (code == 1049 || code == 47 || code == 1047) {
            if (set) screen_.enter_alt_screen();
            else screen_.exit_alt_screen();
        }
    }
}

void AnsiParser::dispatch_sgr(const std::vector<long>& params) {
    if (params.empty()) {
        current_attrs_ = Attributes{};
        return;
    }

    std::size_t i = 0;
    while (i < params.size()) {
        long code = params[i++];
        switch (code) {
            case 0: current_attrs_ = Attributes{}; break;
            case 1: current_attrs_.bold = true; break;
            case 2: current_attrs_.dim = true; break;
            case 3: current_attrs_.italic = true; break;
            case 4: current_attrs_.underline = true; break;
            case 7: current_attrs_.reverse = true; break;
            case 9: current_attrs_.strikethrough = true; break;
            case 22: current_attrs_.bold = false; current_attrs_.dim = false; break;
            case 23: current_attrs_.italic = false; break;
            case 24: current_attrs_.underline = false; break;
            case 27: current_attrs_.reverse = false; break;
            case 29: current_attrs_.strikethrough = false; break;
            case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
                current_attrs_.fg = Color::indexed(static_cast<uint8_t>(code - 30));
                break;
            case 39: current_attrs_.fg = Color::default_color(); break;
            case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
                current_attrs_.bg = Color::indexed(static_cast<uint8_t>(code - 40));
                break;
            case 49: current_attrs_.bg = Color::default_color(); break;
            case 90: case 91: case 92: case 93: case 94: case 95: case 96: case 97:
                current_attrs_.fg = Color::indexed(static_cast<uint8_t>(code - 90 + 8));
                break;
            case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107:
                current_attrs_.bg = Color::indexed(static_cast<uint8_t>(code - 100 + 8));
                break;
            case 38:
            case 48: {
                bool is_fg = (code == 38);
                if (i < params.size()) {
                    long mode = params[i++];
                    if (mode == 5 && i < params.size()) {
                        uint8_t idx = static_cast<uint8_t>(params[i++]);
                        if (is_fg) current_attrs_.fg = Color::indexed(idx);
                        else current_attrs_.bg = Color::indexed(idx);
                    } else if (mode == 2 && i + 2 < params.size()) {
                        uint8_t r = static_cast<uint8_t>(params[i++]);
                        uint8_t g = static_cast<uint8_t>(params[i++]);
                        uint8_t b = static_cast<uint8_t>(params[i++]);
                        if (is_fg) current_attrs_.fg = Color::rgb(r, g, b);
                        else current_attrs_.bg = Color::rgb(r, g, b);
                    }
                }
                break;
            }
            default: break;
        }
    }
}

} // namespace meridian::vt
