// src/shell/line_editor.cpp
#include "line_editor.hpp"
#include "autosuggest_engine.hpp"
#include "../dev/git_intel.hpp"
#include "../dev/icon_provider.hpp"
#include "../core/icons/icon_registry.hpp"
#include "../core/icons/animated_glyph.hpp"
#include "../dev/command_palette.hpp"
#include "../dev/universal_search.hpp"
#include "../dev/rich_history.hpp"
#include "../core/vt/screen_buffer.hpp"
#include "../workspace/pane_tree.hpp"
#include "../core/art_gallery.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <poll.h>
#include <pwd.h>
#include <sstream>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace meridian::shell {

namespace {

// ─── Raw Mode ───────────────────────────────────────────────────────────────

struct RawModeScope {
    struct termios original_termios;
    bool active = false;

    RawModeScope() {
        if (isatty(STDIN_FILENO)) {
            if (tcgetattr(STDIN_FILENO, &original_termios) == 0) {
                struct termios raw = original_termios;
                raw.c_lflag &= ~(ICANON | ECHO);
                raw.c_cc[VMIN] = 1;
                raw.c_cc[VTIME] = 0;
                if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
                    active = true;
                }
            }
        }
    }

    ~RawModeScope() {
        if (active) {
            tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        }
    }
};

bool has_pending_input(int fd, int timeout_ms) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    return poll(&pfd, 1, timeout_ms) > 0 && (pfd.revents & POLLIN);
}

// ─── CWD Helpers ────────────────────────────────────────────────────────────

std::string format_cwd(const std::string& cwd) {
    std::string dir = cwd.empty() ? "~" : cwd;
    const char* home = std::getenv("HOME");
    if (home && dir.rfind(home, 0) == 0) {
        dir = "~" + dir.substr(std::strlen(home));
    }
    return dir;
}

static std::string detect_project_language(const std::string& dir) {
    std::string d = dir.empty() ? "." : dir;
    if (access((d + "/CMakeLists.txt").c_str(), F_OK) == 0 ||
        access((d + "/Makefile").c_str(), F_OK) == 0)       return " ";
    if (access((d + "/Cargo.toml").c_str(), F_OK) == 0)     return " ";
    if (access((d + "/pyproject.toml").c_str(), F_OK) == 0 ||
        access((d + "/requirements.txt").c_str(), F_OK) == 0 ||
        access((d + "/setup.py").c_str(), F_OK) == 0)       return " ";
    if (access((d + "/package.json").c_str(), F_OK) == 0 ||
        access((d + "/tsconfig.json").c_str(), F_OK) == 0)  return " ";
    if (access((d + "/go.mod").c_str(), F_OK) == 0)         return " ";
    if (access((d + "/pom.xml").c_str(), F_OK) == 0 ||
        access((d + "/build.gradle").c_str(), F_OK) == 0)   return " ";
    if (access((d + "/Dockerfile").c_str(), F_OK) == 0 ||
        access((d + "/compose.yaml").c_str(), F_OK) == 0)   return "󰡨 ";
    if (access((d + "/flake.nix").c_str(), F_OK) == 0)      return " ";
    return "";
}

// ─── Gradient / color helpers ─────────────────────────────────────────────────

static std::string fg_rgb(int r, int g, int b) {
    char buf[32];
    snprintf(buf, sizeof(buf), "\033[38;2;%d;%d;%dm", r, g, b);
    return buf;
}
static std::string bg_rgb(int r, int g, int b) {
    char buf[32];
    snprintf(buf, sizeof(buf), "\033[48;2;%d;%d;%dm", r, g, b);
    return buf;
}

// Renders prompt + user_input + dim ghost suffix in one line.
// cursor_pos is the position within user_input (0-based).
static void render_prompt_with_suggestion(
    std::ostream& out,
    const std::string& prompt,
    const std::string& user_input,
    const std::string& suggestion_suffix,
    int cursor_pos
) {
    out << "\r\033[K" << prompt << "\033[0m" << user_input;

    if (!suggestion_suffix.empty()) {
        // Dim ghost color: dark grey, dim attribute
        out << "\033[38;2;85;95;115;2m" << suggestion_suffix << "\033[0m";
    }

    int input_len = static_cast<int>(user_input.size());
    int back = input_len - cursor_pos;
    if (!suggestion_suffix.empty()) back += static_cast<int>(suggestion_suffix.size());
    if (back > 0) {
        out << "\033[" << back << "D";
    }
    out.flush();
}

// ─── Tab Completion Menu ─────────────────────────────────────────────────────

static int draw_completion_menu(
    std::ostream& out,
    const std::vector<std::string>& candidates,
    int max_show = 8
) {
    if (candidates.empty()) return 0;

    int show = std::min(static_cast<int>(candidates.size()), max_show);
    int lines = 0;

    out << "\r\n";
    lines++;

    if (candidates.size() == 1) {
        out << "\033[38;2;80;90;110m  " << candidates[0] << "\033[0m\r\n";
        lines++;
    } else {
        for (int i = 0; i < show; ++i) {
            out << "  " << candidates[i];
            if (i < show - 1) out << "   ";
            if ((i + 1) % 3 == 0 && i < show - 1) {
                out << "\r\n";
                lines++;
            }
        }
        if (candidates.size() > static_cast<size_t>(max_show)) {
            out << "  \033[38;2;100;110;130m(+" << (candidates.size() - max_show)
                << " more, press Tab again)\033[0m";
        }
        out << "\r\n";
        lines++;
    }

    out << "\033[" << lines << "A\r";
    out.flush();
    return lines;
}

static void clear_completion_menu(std::ostream& out, int lines_drawn) {
    if (lines_drawn <= 0) return;
    for (int i = 0; i < lines_drawn; ++i) {
        out << "\n\033[2K\r";
    }
    out << "\033[" << lines_drawn << "A\r";
    out.flush();
}

} // namespace

// ─── LineEditor Interface ─────────────────────────────────────────────────────

bool LineEditor::is_terminal_interactive() {
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
}

std::string LineEditor::build_date_badge(const std::string& cwd) {
    std::string dir = format_cwd(cwd);

    // ── Time ──
    std::time_t now_t = std::time(nullptr);
    std::tm* local_tm = std::localtime(&now_t);
    char time_buf[64];
    std::strftime(time_buf, sizeof(time_buf), "%a %d %b  -  %H:%M", local_tm);

    // ── Context ──
    auto git_status = dev::GitIntel::inspect_directory(cwd.empty() ? "." : cwd);
    std::string dir_icon = dev::IconProvider::icon_for_directory(dir, git_status.is_git_repo);
    std::string lang_icon = detect_project_language(cwd);

    // ── Animated spinner (Braille, time-based) ──
    icons::Spinner spin(icons::SpinnerStyle::Braille, 8.0);
    std::string spin_str = spin.frame();

    std::ostringstream ss;
    ss << "\033[0m";

    // ══════════════════════════════════════════════════════════════════
    // SEGMENT 1 ── Clock / SSH  ──  Deep Space Blue  #101830
    // ══════════════════════════════════════════════════════════════════
    const char* ssh_conn   = std::getenv("SSH_CONNECTION");
    const char* ssh_client = std::getenv("SSH_CLIENT");
    bool is_ssh = (ssh_conn || ssh_client);

    if (is_ssh) {
        char hname[256] = "remote";
        gethostname(hname, sizeof(hname));
        std::string u = "user";
        struct passwd* pw = getpwuid(geteuid());
        if (pw && pw->pw_name) u = pw->pw_name;
        // SSH: vivid magenta badge
        ss << bg_rgb(140, 30, 180) << "\033[38;2;255;230;255;1m"
           << "  " << u << "@" << hname << " "
           << "\033[0m"
           << bg_rgb(13, 42, 74) << fg_rgb(140, 30, 180) << "\uE0B0"
           << "\033[0m";
    } else {
        // Clock badge: deep navy BG, sky-blue clock icon + animated spinner
        ss << bg_rgb(16, 24, 56) << "\033[38;2;160;200;255;1m"
           << " \uE381 "          // nf-mdi-clock_outline
           << "\033[38;2;80;180;255m" << spin_str << " "   // animated spinner cyan
           << "\033[38;2;200;225;255;1m" << time_buf << " "
           << "\033[0m";
        // Powerline right arrow → into directory segment
        ss << bg_rgb(13, 42, 74) << fg_rgb(16, 24, 56) << "\uE0B0" << "\033[0m";
    }

    // ══════════════════════════════════════════════════════════════════
    // SEGMENT 2 ── Directory  ──  Ocean Navy  #0d2a4a
    // ══════════════════════════════════════════════════════════════════
    ss << bg_rgb(13, 42, 74) << "\033[38;2;100;210;255;1m"
       << " " << dir_icon;

    // Gradient on path chars: teal #5ae0d0 → sky #a8f0ff
    {
        int n = static_cast<int>(dir.size());
        for (int i = 0; i < n; ++i) {
            float t = (n > 1) ? static_cast<float>(i) / (n - 1) : 0.0f;
            int r = 90  + static_cast<int>((168 - 90)  * t);
            int g = 224 + static_cast<int>((240 - 224) * t);
            int b = 208 + static_cast<int>((255 - 208) * t);
            ss << fg_rgb(r, g, b) << dir[i];
        }
    }

    // Language icon pill (dim amber)
    if (!lang_icon.empty()) {
        ss << "  \033[38;2;255;200;80;2m" << lang_icon << "\033[0m"
           << bg_rgb(13, 42, 74);
    }
    ss << " ";

    // ══════════════════════════════════════════════════════════════════
    // SEGMENT 3 ── Git  ──  State-colour BG
    // ══════════════════════════════════════════════════════════════════
    if (git_status.is_git_repo && !git_status.branch_name.empty()) {
        std::string branch = git_status.branch_name;
        bool is_dirty = (git_status.unstaged_count > 0 ||
                         git_status.staged_count   > 0 ||
                         git_status.untracked_count > 0);
        bool is_sync  = (git_status.ahead_count > 0 || git_status.behind_count > 0);

        int bg_r, bg_g, bg_b, fg_r, fg_g, fg_b;
        const char* state_badge;
        if (is_dirty) {
            bg_r=72; bg_g=38; bg_b=0;      // amber-dark
            fg_r=255; fg_g=185; fg_b=60;   // amber-bright
            state_badge = "✦";
        } else if (is_sync) {
            bg_r=40; bg_g=20; bg_b=88;     // violet-dark
            fg_r=180; fg_g=140; fg_b=255;  // violet-bright
            state_badge = "\uE0A0";        // nf-powerline-branch_alt
        } else {
            bg_r=8; bg_g=52; bg_b=32;      // forest-dark
            fg_r=72; fg_g=240; fg_b=152;   // emerald
            state_badge = "✔";
        }

        // Arrow into git segment
        ss << bg_rgb(bg_r, bg_g, bg_b) << fg_rgb(13, 42, 74) << "\uE0B0" << "\033[0m";

        // Branch icon + gradient branch name
        ss << bg_rgb(bg_r, bg_g, bg_b) << "\033[1m"
           << fg_rgb(fg_r, fg_g, fg_b)
           << "  ";  // nf-dev-git_branch

        int n = static_cast<int>(branch.size());
        for (int i = 0; i < n; ++i) {
            float t = (n > 1) ? static_cast<float>(i) / (n - 1) : 0.5f;
            int r = fg_r - static_cast<int>(50 * (1.0f - t));
            int g = fg_g - static_cast<int>(50 * (1.0f - t));
            int b = fg_b - static_cast<int>(50 * (1.0f - t));
            ss << fg_rgb(std::max(0,r), std::max(0,g), std::max(0,b)) << branch[i];
        }

        // Status counts
        ss << " " << fg_rgb(fg_r, fg_g, fg_b);
        if (git_status.ahead_count  > 0) ss << " ↑" << git_status.ahead_count;
        if (git_status.behind_count > 0) ss << " ↓" << git_status.behind_count;
        if (git_status.staged_count > 0) ss << " ●" << git_status.staged_count;
        if (git_status.unstaged_count>0) ss << " ✚" << git_status.unstaged_count;
        if (git_status.untracked_count>0)ss << " ?" << git_status.untracked_count;

        ss << " " << state_badge << " "
           << "\033[0m"
           << fg_rgb(bg_r, bg_g, bg_b) << "\uE0B0" << "\033[0m";
    } else {
        // No git: close directory arrow
        ss << "\033[0m" << fg_rgb(13, 42, 74) << "\uE0B0" << "\033[0m";
    }

    return ss.str();
}

std::string LineEditor::build_powerline_prompt(const std::string& /*cwd*/) {
    std::string user = "user";
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw && pw->pw_name) user = pw->pw_name;

    // Time-based animation phase
    struct timespec ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_now);
    double phase4 = std::fmod(static_cast<double>(ts_now.tv_sec) + ts_now.tv_nsec / 1e9, 4.0) / 4.0;
    double beat   = (phase4 < 0.5) ? phase4 * 2.0 : (1.0 - phase4) * 2.0;

    // Arrow hue cycles: Cyan → Violet → Gold → Cyan
    double arrow_phase = std::fmod(static_cast<double>(ts_now.tv_sec) * 0.5 + ts_now.tv_nsec / 2e9, 3.0);
    int ar, ag, ab;
    if (arrow_phase < 1.0) {
        float t = static_cast<float>(arrow_phase);
        ar = static_cast<int>( 20 + (160 -  20) * t);
        ag = static_cast<int>(200 + ( 40 - 200) * t);
        ab = 255;
    } else if (arrow_phase < 2.0) {
        float t = static_cast<float>(arrow_phase - 1.0);
        ar = static_cast<int>(160 + (255 - 160) * t);
        ag = static_cast<int>( 40 + (215 -  40) * t);
        ab = static_cast<int>(255 + (  0 - 255) * t);
    } else {
        float t = static_cast<float>(arrow_phase - 2.0);
        ar = static_cast<int>(255 + ( 20 - 255) * t);
        ag = static_cast<int>(215 + (200 - 215) * t);
        ab = static_cast<int>(  0 + (255 -   0) * t);
    }
    ar = std::max(0, std::min(255, ar));
    ag = std::max(0, std::min(255, ag));
    ab = std::max(0, std::min(255, ab));

    std::ostringstream ss;
    ss << "\033[0m";

    if (uid == 0) {
        // Root: electric crimson
        ss << bg_rgb(160, 16, 16) << "\033[38;2;255;210;210;1m"
           << " ⚡ root "
           << "\033[0m" << fg_rgb(160, 16, 16) << "\uE0B0"
           << "\033[0m"
           << " \033[1m" << fg_rgb(255, 80, 80) << "❯\033[0m ";
    } else {
        // User: animated deep purple → violet badge
        int bg_r = 80  + static_cast<int>(32  * beat);
        int bg_g = 18  + static_cast<int>(12  * beat);
        int bg_b = 155 + static_cast<int>(65  * beat);

        ss << bg_rgb(bg_r, bg_g, bg_b)
           << "\033[38;2;255;235;200;1m"
           << "  ";       // nf-fa-user_circle

        // Username: Gold → Amber gradient
        int n = static_cast<int>(user.size());
        for (int i = 0; i < n; ++i) {
            float t = (n > 1) ? static_cast<float>(i) / (n - 1) : 0.5f;
            int r = 255;
            int g = 215 - static_cast<int>(70 * t);
            int b =   0 + static_cast<int>(30 * t);
            ss << fg_rgb(r, g, b) << user[i];
        }

        ss << " "
           << "\033[0m" << fg_rgb(bg_r, bg_g, bg_b) << "\uE0B0"
           << "\033[0m"
           << " \033[1m" << fg_rgb(ar, ag, ab) << "❯\033[0m ";
    }

    return ss.str();
}

void LineEditor::draw_history_preview(
    std::ostream& out,
    const std::vector<std::string>& history,
    int selected_idx,
    int visible_count
) {
    if (history.empty()) return;

    out << "\n\033[2K\r\033[1;36m┌─── Command History Preview (↑/↓ to navigate, Enter to run, Esc to close) ───┐\033[0m";
    int total = static_cast<int>(history.size());
    int start = std::max(0, total - visible_count);

    for (int i = start; i < total; ++i) {
        bool is_selected = (i == selected_idx);
        std::string cmd = history[i];
        if (cmd.size() > 56) cmd = cmd.substr(0, 53) + "...";

        out << "\n\033[2K\r";
        if (is_selected) {
            out << "\033[1;33m  ▶ \033[48;2;40;44;52m\033[1;37m " << std::setw(2) << (i + 1)
                << "  " << cmd << " \033[0m";
        } else {
            out << "\033[38;2;120;130;150m    " << std::setw(2) << (i + 1)
                << "  " << cmd << "\033[0m";
        }
    }
    out << "\n\033[2K\r\033[1;36m└─────────────────────────────────────────────────────────────────────────────┘\033[0m";
    out.flush();
}

void LineEditor::clear_history_preview(std::ostream& out, int lines_drawn) {
    if (lines_drawn <= 0) return;
    for (int i = 0; i < lines_drawn; ++i) {
        out << "\n\033[2K\r";
    }
    out << "\033[" << lines_drawn << "A\r";
    out.flush();
}

std::string LineEditor::read_line(
    std::istream& in,
    std::ostream& out,
    const std::string& prompt,
    const std::vector<std::string>& history
) {
    if (!is_terminal_interactive()) {
        std::string line;
        if (std::getline(in, line)) {
            return line;
        }
        return "";
    }

    char cwd_buf[4096] = "";
    getcwd(cwd_buf, sizeof(cwd_buf));
    std::string cwd = cwd_buf;

    RawModeScope raw;
    std::string current_line;
    std::string original_line;
    std::string current_suggestion;
    int cursor_pos = 0;
    int history_idx = static_cast<int>(history.size());
    int completion_lines_drawn = 0;

    auto update_suggestion = [&]() {
        if (current_line.empty()) {
            current_suggestion = "";
            return;
        }
        current_suggestion = AutosuggestEngine::get_suggestion(current_line, history, cwd);
    };

    auto redraw = [&]() {
        if (completion_lines_drawn > 0) {
            clear_completion_menu(out, completion_lines_drawn);
            completion_lines_drawn = 0;
        }
        std::string suffix = AutosuggestEngine::get_suggestion_suffix(current_line, current_suggestion);
        render_prompt_with_suggestion(out, prompt, current_line, suffix, cursor_pos);
    };

    redraw();

    while (true) {
        char c = 0;
        if (read(STDIN_FILENO, &c, 1) <= 0) {
            break;
        }

        // Enter / Return
        if (c == '\n' || c == '\r') {
            out << "\r\033[K" << prompt << "\033[0m" << current_line << "\r\n";
            out.flush();
            return current_line;
        }

        // Ctrl+D (EOF on empty line)
        if (c == 4) {
            if (current_line.empty()) {
                out << "\r\n";
                out.flush();
                return "exit";
            }
            continue;
        }

        // Ctrl+C (Cancel)
        if (c == 3) {
            out << "^C\r\n";
            out.flush();
            return "";
        }

        // Ctrl+L (Clear screen)
        if (c == 12) {
            out << "\033[H\033[2J\033[3J";
            out.flush();
            redraw();
            continue;
        }

        // Tab (Completion)
        if (c == '\t') {
            auto comps = AutosuggestEngine::get_tab_completions_for_line(current_line, cwd);
            if (!comps.empty()) {
                if (comps.size() == 1) {
                    current_line = comps[0];
                    cursor_pos = static_cast<int>(current_line.size());
                    update_suggestion();
                    redraw();
                } else {
                    std::vector<std::string> formatted;
                    for (const auto& item : comps) {
                        bool is_dir = (!item.empty() && item.back() == '/');
                        std::string icon = dev::IconProvider::colored_icon_for_file(item, is_dir, false, ' ');
                        formatted.push_back(icon + item);
                    }
                    completion_lines_drawn = draw_completion_menu(out, formatted, 9);
                }
            }
            continue;
        }

        // Backspace / Delete
        if (c == 127 || c == 8) {
            if (cursor_pos > 0 && !current_line.empty()) {
                current_line.erase(cursor_pos - 1, 1);
                cursor_pos--;
                update_suggestion();
                redraw();
            }
            continue;
        }

        // Escape sequence (Arrow keys, Home, End, Ctrl+Arrow)
        if (c == 27) {
            if (!has_pending_input(STDIN_FILENO, 20)) {
                // Standalone Esc -> dismiss suggestion
                current_suggestion = "";
                redraw();
                continue;
            }

            char seq[5] = {0};
            if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;

            if (seq[0] == '[') {
                if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;

                // Up Arrow
                if (seq[1] == 'A') {
                    if (history_idx == static_cast<int>(history.size())) {
                        original_line = current_line;
                    }
                    if (history_idx > 0) {
                        history_idx--;
                        current_line = history[history_idx];
                        cursor_pos = static_cast<int>(current_line.size());
                        update_suggestion();
                        redraw();
                    }
                    continue;
                }

                // Down Arrow
                if (seq[1] == 'B') {
                    if (history_idx < static_cast<int>(history.size()) - 1) {
                        history_idx++;
                        current_line = history[history_idx];
                        cursor_pos = static_cast<int>(current_line.size());
                        update_suggestion();
                        redraw();
                    } else if (history_idx == static_cast<int>(history.size()) - 1) {
                        history_idx = static_cast<int>(history.size());
                        current_line = original_line;
                        cursor_pos = static_cast<int>(current_line.size());
                        update_suggestion();
                        redraw();
                    }
                    continue;
                }

                // Right Arrow -> Accept suggestion or move cursor
                if (seq[1] == 'C') {
                    if (cursor_pos == static_cast<int>(current_line.size()) && !current_suggestion.empty()) {
                        current_line = current_suggestion;
                        cursor_pos = static_cast<int>(current_line.size());
                        update_suggestion();
                        redraw();
                    } else if (cursor_pos < static_cast<int>(current_line.size())) {
                        cursor_pos++;
                        redraw();
                    }
                    continue;
                }

                // Left Arrow
                if (seq[1] == 'D') {
                    if (cursor_pos > 0) {
                        cursor_pos--;
                        redraw();
                    }
                    continue;
                }

                // Home
                if (seq[1] == 'H' || seq[1] == '1') {
                    cursor_pos = 0;
                    redraw();
                    continue;
                }

                // End -> Accept suggestion
                if (seq[1] == 'F' || seq[1] == '4') {
                    if (!current_suggestion.empty()) {
                        current_line = current_suggestion;
                    }
                    cursor_pos = static_cast<int>(current_line.size());
                    update_suggestion();
                    redraw();
                    continue;
                }

                // Ctrl+Right: \033[1;5C
                if (seq[1] == '1') {
                    char extra[3] = {0};
                    if (read(STDIN_FILENO, &extra[0], 1) > 0 && extra[0] == ';' &&
                        read(STDIN_FILENO, &extra[1], 1) > 0 && extra[1] == '5' &&
                        read(STDIN_FILENO, &extra[2], 1) > 0 && extra[2] == 'C') {
                        if (!current_suggestion.empty()) {
                            current_line = AutosuggestEngine::accept_next_word(current_line, current_suggestion);
                            cursor_pos = static_cast<int>(current_line.size());
                            update_suggestion();
                            redraw();
                        }
                    }
                    continue;
                }
            }
            continue;
        }

        // Printable Character
        if (static_cast<unsigned char>(c) >= 32) {
            current_line.insert(cursor_pos, 1, c);
            cursor_pos++;
            update_suggestion();
            redraw();
        }
    }

    return current_line;
}

} // namespace meridian::shell
