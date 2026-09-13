#include "meridian_gui.hpp"
#include "../core/terminal_image.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>

namespace meridian::gui {

MeridianGui::MeridianGui() {
    tabs_ = {"1: main", "2: server", "3: agent"};
    active_tab_ = 0;
}

std::string MeridianGui::render_frame(int width, int height) {
    if (width < 70) width = 80;
    if (height < 22) height = 26;

    std::ostringstream ss;

    // Detect system info
    char hostname[256] = "fedora";
    gethostname(hostname, sizeof(hostname));
    std::string user = "charanbalaji";
    struct passwd* pw = getpwuid(geteuid());
    if (pw && pw->pw_name) {
        user = pw->pw_name;
    }

    struct utsname uts{};
    std::string kernel = "Linux 7.1.5-201.fc44.x86_64";
    if (uname(&uts) == 0) {
        kernel = std::string(uts.sysname) + " " + uts.release;
    }

    const char* xdg_desktop = std::getenv("XDG_CURRENT_DESKTOP");
    const char* session_type = std::getenv("XDG_SESSION_TYPE");
    std::string wm = (std::getenv("HYPRLAND_INSTANCE_SIGNATURE")) ? "Hyprland 0.56.1 (Wayland)" :
                     (xdg_desktop ? (std::string(xdg_desktop) + " (" + (session_type ? session_type : "Wayland") + ")") : "Hyprland 0.56.1 (Wayland)");

    const char* shell_env = std::getenv("SHELL");
    std::string shell_name = shell_env ? shell_env : "zsh";
    auto slash = shell_name.find_last_of('/');
    if (slash != std::string::npos) shell_name = shell_name.substr(slash + 1);
    std::string shell_str = shell_name + " 5.9";

    // Memory info
    std::string ram_str = "7.66 GiB / 15.25 GiB";
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

    // Uptime
    std::string uptime_str = "1 hour, 11 mins";
    std::ifstream uptime_file("/proc/uptime");
    if (uptime_file.is_open()) {
        double seconds = 0;
        uptime_file >> seconds;
        uint64_t total_sec = static_cast<uint64_t>(seconds);
        uint64_t hours = (total_sec % 86400) / 3600;
        uint64_t mins = (total_sec % 3600) / 60;
        uptime_str = std::to_string(hours) + (hours == 1 ? " hour, " : " hours, ") + std::to_string(mins) + " mins";
    }

    // Time
    std::time_t now = std::time(nullptr);
    std::tm* local_tm = std::localtime(&now);
    char time_buf[64];
    std::strftime(time_buf, sizeof(time_buf), "%a %d %b  -  %H:%M", local_tm);

    // System info lines (sleek Nerd Font glyphs matching reference theme)
    std::vector<std::string> sys_lines = {
        "\033[1;37m" + user + "@" + hostname + "\033[0m",
        "\033[38;2;120;130;150m───────\033[38;2;200;210;230m⭘\033[38;2;120;130;150m───────\033[0m",
        "\033[38;2;34;197;94m󰌽 \033[38;2;170;180;200m→ \033[38;2;245;247;250m" + kernel + "\033[0m",
        "\033[38;2;0;229;255m󰨇 \033[38;2;170;180;200m→ \033[38;2;245;247;250m" + wm + "\033[0m",
        "\033[38;2;168;85;247m󰞷 \033[38;2;170;180;200m→ \033[38;2;245;247;250m" + shell_str + "\033[0m",
        "\033[38;2;59;130;246m \033[38;2;170;180;200m→ \033[38;2;245;247;250mmeridian 2.0\033[0m",
        "\033[38;2;245;158;11m󰘚 \033[38;2;170;180;200m→ \033[38;2;245;247;250m" + ram_str + "\033[0m",
        "\033[38;2;244;63;94m󱑂 \033[38;2;170;180;200m→ \033[38;2;245;247;250m" + uptime_str + "\033[0m",
        "\033[38;2;120;130;150m───────\033[38;2;200;210;230m⭘\033[38;2;120;130;150m───────\033[0m",
        "\033[38;2;0;229;255m● \033[38;2;59;130;246m● \033[38;2;168;85;247m● \033[38;2;244;63;94m● \033[38;2;34;197;94m● \033[38;2;245;158;11m● \033[38;2;253;224;71m● \033[38;2;251;146;60m● \033[38;2;239;68;68m●\033[0m"
    };

    // Render Top Box: Artwork (Left via Kitty Graphics) + System Info (Right)
    ss << "\n";
    ss << core::TerminalImage::render_kitty_graphics_artwork(2, 1, 24, 10);
    for (size_t i = 0; i < sys_lines.size(); ++i) {
        ss << "  ";
        ss << "                        "; // 24 spaces reserved for raster graphics
        ss << "    ";
        ss << sys_lines[i];
        ss << "\n";
    }

    // Render Vivid Powerline Badges
    char cwd_buf[1024];
    std::string cwd_str = getcwd(cwd_buf, sizeof(cwd_buf)) ? cwd_buf : "~";
    const char* home_env = std::getenv("HOME");
    if (home_env && cwd_str.find(home_env) == 0) {
        cwd_str = "~" + cwd_str.substr(strlen(home_env));
    }

    ss << "\n";
    // Row 1: Cobalt Blue Date + Ocean Cyan Working Directory + Crimson Red Git Status
    ss << "  \033[48;2;26;108;218;38;2;255;255;255;1m " << time_buf << " \033[48;2;24;156;184;38;2;26;108;218m\033[48;2;24;156;184;38;2;255;255;255;1m " << cwd_str << " \033[48;2;201;59;59;38;2;24;156;184m\033[48;2;201;59;59;38;2;255;255;255;1m  origin  main 6✸ 8● \033[0;38;2;201;59;59m\033[0m\n";
    // Row 2: Golden Yellow Username
    ss << "  \033[48;2;212;180;27;38;2;45;24;50;1m @" << user << " \033[0;38;2;212;180;27m\033[0m \033[1;37m❯\033[0m \n";

    return ss.str();
}

int MeridianGui::run() {
    // Enter alternate screen buffer
    std::cout << "\033[?1049h\033[2J\033[H";
    std::cout.flush();

    struct winsize ws{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    int cols = ws.ws_col > 0 ? ws.ws_col : 80;
    int rows = ws.ws_row > 0 ? ws.ws_row : 24;

    std::cout << render_frame(cols, rows);
    std::cout.flush();

    // Check if interactive
    if (isatty(STDIN_FILENO)) {
        struct termios raw{}, orig{};
        tcgetattr(STDIN_FILENO, &orig);
        raw = orig;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        char c = 0;
        while (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'q' || c == 27 || c == 3) {
                break;
            }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    }

    // Exit alternate screen buffer
    std::cout << "\033[?1049l";
    std::cout.flush();
    return 0;
}

} // namespace meridian::gui
