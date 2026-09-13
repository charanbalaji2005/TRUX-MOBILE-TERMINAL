#include "shell.hpp"
#include "line_editor.hpp"
#include "builtins.hpp"
#include "../ai/command_analyzer.hpp"
#include "../dev/github_integration.hpp"
#include "../dev/ide_detector.hpp"
#include "../core/terminal_image.hpp"
#include "../core/art_gallery.hpp"
#include "../core/graphics/ascii_art_engine.hpp"
#include <termios.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace meridian::shell {

namespace {

void render_reference_layout_header(std::ostream& out) {
    // 1. Detect dynamic cross-platform system metadata (Linux, macOS, Windows/WSL)
    char hostname[256] = "localhost";
    gethostname(hostname, sizeof(hostname));
    std::string user = "user";
    struct passwd* pw = getpwuid(geteuid());
    if (pw && pw->pw_name) {
        user = pw->pw_name;
    }

    struct utsname uts{};
    std::string kernel = "Linux";
    if (uname(&uts) == 0) {
        kernel = std::string(uts.sysname) + " " + uts.release;
    }

    std::string wm = "Hyprland 0.56.1 (Wayland)";
    const char* xdg_desktop = std::getenv("XDG_CURRENT_DESKTOP");
    const char* session_type = std::getenv("XDG_SESSION_TYPE");
    const char* wsl_distro = std::getenv("WSL_DISTRO_NAME");

    if (wsl_distro) {
        kernel = "WSL2 (" + std::string(wsl_distro) + ") on Windows 11";
        wm = "Windows DWM (WSLg Wayland)";
    } else if (std::string(uts.sysname) == "Darwin") {
        kernel = "macOS " + std::string(uts.release) + " (Darwin)";
        wm = "Aqua (Quartz Compositor)";
    } else if (std::getenv("HYPRLAND_INSTANCE_SIGNATURE")) {
        wm = "Hyprland 0.56.1 (Wayland)";
    } else if (xdg_desktop) {
        wm = std::string(xdg_desktop) + " (" + (session_type ? session_type : "Wayland") + ")";
    }

    const char* shell_env = std::getenv("SHELL");
    std::string shell_name = shell_env ? shell_env : "zsh";
    auto slash = shell_name.find_last_of('/');
    if (slash != std::string::npos) shell_name = shell_name.substr(slash + 1);
    std::string shell_str = shell_name + " 5.9";

    // Dynamic RAM calculation
    std::string ram_str = "3.47 GiB / 15.25 GiB";
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        uint64_t total_kb = 0, avail_kb = 0;
        while (std::getline(meminfo, line)) {
            if (line.rfind("MemTotal:", 0) == 0) {
                std::istringstream mss(line.substr(9));
                mss >> total_kb;
            } else if (line.rfind("MemAvailable:", 0) == 0) {
                std::istringstream mss(line.substr(13));
                mss >> avail_kb;
            }
        }
        if (total_kb > 0) {
            uint64_t used_kb = (total_kb > avail_kb) ? (total_kb - avail_kb) : 0;
            double used_gib = static_cast<double>(used_kb) / (1024.0 * 1024.0);
            double total_gib = static_cast<double>(total_kb) / (1024.0 * 1024.0);
            std::ostringstream ross;
            ross << std::fixed << std::setprecision(2) << used_gib << " GiB / " << total_gib << " GiB";
            ram_str = ross.str();
        }
    }

    // Dynamic Uptime
    std::string uptime_str = "0 hours, 0 mins";
    std::ifstream uptime_file("/proc/uptime");
    if (uptime_file.is_open()) {
        double seconds = 0;
        uptime_file >> seconds;
        uint64_t total_sec = static_cast<uint64_t>(seconds);
        uint64_t hours = (total_sec % 86400) / 3600;
        uint64_t mins = (total_sec % 3600) / 60;
        uptime_str = std::to_string(hours) + (hours == 1 ? " hour, " : " hours, ") + std::to_string(mins) + " mins";
    }

    // 2. Picture graphic: Dynamic Full HD TrueColor dual-pixel halfblocks (880 - 1,700+ crisp micro-pixels)
    int term_cols = 80, term_rows = 24;
    meridian::graphics::AsciiArtEngine::get_terminal_dimensions(term_cols, term_rows);

    int art_w = 48;
    int art_h = 14;
    if (term_cols >= 120) {
        art_w = 54;
        art_h = 15;
    } else if (term_cols >= 100) {
        art_w = 48;
        art_h = 14;
    } else if (term_cols < 85) {
        art_w = 38;
        art_h = 11;
    }

    auto theme = core::ArtGallery::get_active_artwork(art_w * 2, art_h * 2);
    std::vector<std::string> art_lines = core::ArtGallery::render_artwork_lines(theme.image, art_w, art_h);

    // 3. System info lines (sleek Nerd Font glyphs matching reference theme, external shell removed)
    std::vector<std::string> sys_lines = {
        "\033[1;37m" + user + "@" + hostname + "\033[0m",
        "\033[38;2;120;130;150m───────\033[38;2;200;210;230m⭘\033[38;2;120;130;150m───────\033[0m",
        "\033[38;2;140;163;136m󰌽 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + kernel + "\033[0m",
        "\033[38;2;110;165;185m󰨇 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + wm + "\033[0m",
        "\033[38;2;90;175;170m \033[38;2;170;180;200m→ \033[38;2;220;230;245mmeridian 2.5.1\033[0m",
        "\033[38;2;215;185;135m󰘚 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + ram_str + "\033[0m",
        "\033[38;2;205;135;145m󱑂 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + uptime_str + "\033[0m",
        "\033[38;2;120;130;150m───────\033[38;2;200;210;230m⭘\033[38;2;120;130;150m───────\033[0m",
        "\033[38;2;45;106;116m● \033[38;2;78;135;144m● \033[38;2;125;111;141m● \033[38;2;168;91;107m● \033[38;2;201;95;78m● \033[38;2;217;129;87m● \033[38;2;235;196;122m● \033[38;2;243;224;181m● \033[38;2;140;163;136m●\033[0m"
    };

    // Vertically center system stats if art has more rows
    std::vector<std::string> formatted_sys;
    size_t pad_top = (art_lines.size() > sys_lines.size()) ? (art_lines.size() - sys_lines.size()) / 2 : 0;
    for (size_t i = 0; i < pad_top; ++i) formatted_sys.push_back("");
    for (const auto& l : sys_lines) formatted_sys.push_back(l);

    // 4. Print Side-by-Side Header with 2 spaces left margin, 4 spaces clean gap
    size_t max_rows = std::max(art_lines.size(), formatted_sys.size());
    out << "\n";
    std::string empty_col(art_w, ' ');
    for (size_t r = 0; r < max_rows; ++r) {
        out << "  "; // 2 spaces left margin
        if (r < art_lines.size()) {
            out << art_lines[r];
        } else {
            out << empty_col;
        }
        out << "    "; // 4 spaces clean gap
        if (r < formatted_sys.size()) {
            out << formatted_sys[r];
        }
        out << "\n";
    }
    out << "\n"; // Clean vertical space before Powerline badge
    out.flush();
}

} // namespace

Shell::Shell(bool interactive) : interactive_(interactive) {}

std::string Shell::prompt() const {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd))) return "meridian:" + std::string(cwd) + "$ ";
    return "meridian$ ";
}

int Shell::run_command(const std::string& command, std::ostream& err) {
    std::string error;
    int status = executor_.run_line(command, &error);
    if (!error.empty()) { err << "meridian-shell: " << error << "\n"; return 1; }
    return status < 0 ? 1 : status;
}

int Shell::run_interactive(std::istream& in, std::ostream& out, std::ostream& err) {
    bool is_real_interactive = (&in == &std::cin && LineEditor::is_terminal_interactive());

    if (is_real_interactive) {
        dev::IdeDetector::ensure_first_run_registered();
    }

    ai::CommandAnalyzer analyzer(get_builtin_names());

    if (is_real_interactive) {
        out << "\033[2J\033[H"; // Clear screen & home cursor so no previous shell prompt is visible
        render_reference_layout_header(out);
    }

    while (true) {
        std::string line;
        char cwd[4096] = "";
        getcwd(cwd, sizeof(cwd));

        if (is_real_interactive) {
            std::string date_badge = LineEditor::build_date_badge(cwd);
            out << date_badge << "\n";
            out.flush();

            std::string powerline_p = LineEditor::build_powerline_prompt(cwd);
            line = LineEditor::read_line(in, out, powerline_p, executor_.history());
            if (line == "exit" && executor_.history().empty()) {
                return 0;
            }
        } else {
            if (interactive_) {
                out << prompt();
                out.flush();
            }
            if (!std::getline(in, line)) break;
        }

        // Handle clear builtin (wipe screen, image and scrollback completely)
        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t' || trimmed.front() == '\r')) trimmed.erase(0, 1);
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == '\r')) trimmed.pop_back();

        if (trimmed == "clear" && is_real_interactive) {
            out << "\033[H\033[2J\033[3J" << std::flush;
            executor_.push_history(line);
            continue;
        }

        // Interactive AI Typo Detection & Correction Popup
        if (is_real_interactive && !trimmed.empty()) {
            std::string first_token;
            size_t sp = trimmed.find_first_of(" \t;&|");
            if (sp != std::string::npos) first_token = trimmed.substr(0, sp);
            else first_token = trimmed;

            if (!first_token.empty() && first_token.find('=') == std::string::npos && first_token.front() != '#') {
                auto suggestion = analyzer.analyze(first_token);
                if (suggestion.has_value()) {
                    std::string suggested_cmd = suggestion->suggested + trimmed.substr(first_token.size());
                    out << "\n\033[38;2;0;229;255m┌── \033[1;33m💡 Meridian AI Typo Correction\033[0;38;2;0;229;255m ──────────────────────────────────────────┐\033[0m\n"
                        << "\033[38;2;0;229;255m│\033[0m Command '\033[1;31m" << first_token << "\033[0m' not found in system PATH or builtins.\n"
                        << "\033[38;2;0;229;255m│\033[0m Did you mean: \033[1;38;2;34;197;94m" << suggested_cmd << "\033[0m ?\n"
                        << "\033[38;2;0;229;255m│\033[0m\n"
                        << "\033[38;2;0;229;255m│\033[0m Press [\033[1;32mY\033[0m/Enter] Run correction   [\033[1;31mN\033[0m/Esc] Keep original\n"
                        << "\033[38;2;0;229;255m└───────────────────────────────────────────────────────────────────────┘\033[0m\n";
                    out.flush();

                    struct termios orig_t{}, raw_t{};
                    if (tcgetattr(STDIN_FILENO, &orig_t) == 0) {
                        raw_t = orig_t;
                        raw_t.c_lflag &= ~(ICANON | ECHO);
                        tcsetattr(STDIN_FILENO, TCSANOW, &raw_t);

                        char key = 0;
                        int n = read(STDIN_FILENO, &key, 1);

                        tcsetattr(STDIN_FILENO, TCSANOW, &orig_t);

                        if (n > 0 && (key == 'y' || key == 'Y' || key == '\r' || key == '\n')) {
                            line = suggested_cmd;
                            trimmed = suggested_cmd;
                            out << "\033[38;2;34;197;94m✔ Running: " << line << "\033[0m\n";
                            out.flush();
                        } else {
                            out << "\033[38;2;140;150;170m↪ Running original: " << line << "\033[0m\n";
                            out.flush();
                        }
                    }
                }
            }
        }

        std::string error;
        int status = executor_.run_line(line, &error);
        if (!error.empty()) err << "meridian-shell: " << error << "\n";
        else if (status >= 0 && !trimmed.empty()) executor_.push_history(line);

        if (executor_.exit_requested()) return executor_.exit_code();

        if (is_real_interactive && !trimmed.empty()) {
            dev::GitHubIntegration::on_post_command_hint(trimmed, status);
            out << "\n";
            out.flush();
        }
    }
    return 0;
}

} // namespace meridian::shell
