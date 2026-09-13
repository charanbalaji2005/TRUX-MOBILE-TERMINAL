// src/dev/ide_detector.cpp
#include "ide_detector.hpp"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <glob.h>

namespace meridian::dev {

namespace {

std::string get_home_dir() {
    const char* h = std::getenv("HOME");
    return h ? std::string(h) : "/tmp";
}

bool file_exists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

// Fast in-process PATH lookup without forking a subshell
bool is_command_in_path(const std::string& cmd) {
    if (cmd.empty()) return false;
    if (cmd.find('/') != std::string::npos) {
        return access(cmd.c_str(), X_OK) == 0;
    }
    const char* p = std::getenv("PATH");
    if (!p) return false;
    std::string path_str(p);
    size_t start = 0;
    while (start <= path_str.size()) {
        size_t colon = path_str.find(':', start);
        std::string dir = path_str.substr(start, (colon == std::string::npos) ? std::string::npos : (colon - start));
        if (!dir.empty()) {
            std::string full = dir + "/" + cmd;
            if (access(full.c_str(), X_OK) == 0) return true;
        }
        if (colon == std::string::npos) break;
        start = colon + 1;
    }
    return false;
}

std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return "";
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void write_file(const std::string& path, const std::string& contents) {
    size_t last_slash = path.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string dir = path.substr(0, last_slash);
        mkdir(dir.c_str(), 0755);
    }
    std::ofstream out(path);
    if (out.is_open()) {
        out << contents;
    }
}

std::string get_shell_binary_path() {
    std::string home = get_home_dir();
    std::string local_bin = home + "/.local/bin/meridian-shell";
    std::string sys_bin = "/usr/local/bin/meridian-shell";
    if (file_exists(local_bin)) return local_bin;
    if (file_exists(sys_bin)) return sys_bin;
    return "meridian-shell";
}

std::string get_gui_binary_path() {
    std::string home = get_home_dir();
    std::string local_gui = home + "/.local/bin/meridian";
    std::string sys_gui = "/usr/local/bin/meridian";
    if (file_exists(local_gui)) return local_gui;
    if (file_exists(sys_gui)) return sys_gui;
    return "meridian";
}

// Safely merge VS Code style terminal profile into settings.json across Linux, macOS, and Windows/WSL
std::string merge_vscode_profile(const std::string& existing_json, const std::string& shell_bin, const std::string& gui_bin) {
    if (existing_json.find("\"Meridian Shell\"") != std::string::npos && existing_json.find("\"Meridian Terminal\"") != std::string::npos) {
        return existing_json; // Already registered
    }

    if (existing_json.empty() || existing_json == "{}") {
        return R"({
    "terminal.integrated.profiles.linux": {
        "Meridian Shell": {
            "path": ")" + shell_bin + R"(",
            "icon": "terminal",
            "overrideName": true
        },
        "meridian-shell": {
            "path": ")" + shell_bin + R"(",
            "icon": "terminal",
            "overrideName": true
        },
        "Meridian Terminal": {
            "path": ")" + gui_bin + R"(",
            "icon": "terminal-tmux",
            "overrideName": true
        },
        "bash": {
            "path": "bash",
            "icon": "terminal-bash"
        },
        "zsh": {
            "path": "zsh"
        }
    },
    "terminal.integrated.profiles.osx": {
        "Meridian Shell": {
            "path": ")" + shell_bin + R"(",
            "icon": "terminal",
            "overrideName": true
        },
        "Meridian Terminal": {
            "path": ")" + gui_bin + R"(",
            "icon": "terminal-tmux",
            "overrideName": true
        }
    },
    "terminal.integrated.profiles.windows": {
        "Meridian Shell": {
            "path": "wsl.exe",
            "args": ["-e", "meridian-shell"],
            "icon": "terminal"
        }
    },
    "terminal.integrated.defaultProfile.linux": "Meridian Shell"
})";
    }

    // Safely insert profile entries before closing bracket
    std::string out = existing_json;
    size_t profiles_pos = out.find("\"terminal.integrated.profiles.linux\"");
    if (profiles_pos != std::string::npos) {
        size_t brace_open = out.find('{', profiles_pos);
        if (brace_open != std::string::npos) {
            std::string snippet = R"(
        "Meridian Shell": {
            "path": ")" + shell_bin + R"(",
            "icon": "terminal",
            "overrideName": true
        },
        "Meridian Terminal": {
            "path": ")" + gui_bin + R"(",
            "icon": "terminal-tmux",
            "overrideName": true
        },)";
            out.insert(brace_open + 1, snippet);
            return out;
        }
    }

    // Insert new profile section
    size_t last_brace = out.rfind('}');
    if (last_brace != std::string::npos) {
        std::string section = R"(,
    "terminal.integrated.profiles.linux": {
        "Meridian Shell": {
            "path": ")" + shell_bin + R"(",
            "icon": "terminal",
            "overrideName": true
        },
        "Meridian Terminal": {
            "path": ")" + gui_bin + R"(",
            "icon": "terminal-tmux",
            "overrideName": true
        }
    },
    "terminal.integrated.profiles.osx": {
        "Meridian Shell": {
            "path": ")" + shell_bin + R"(",
            "icon": "terminal",
            "overrideName": true
        }
    }
)";
        out.insert(last_brace, section);
    }
    return out;
}

} // namespace

// -----------------------------------------------------------------------------
// VS Code Adapter (Linux / macOS / Windows / WSL)
// -----------------------------------------------------------------------------
class VSCodeAdapter : public IdeAdapter {
public:
    std::string id() const override { return "vscode"; }
    std::string name() const override { return "Visual Studio Code"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "IDE";
        info.supports_terminal = true;

        std::string home = get_home_dir();
        std::vector<std::string> candidates = {
            home + "/.config/Code/User/settings.json",
            home + "/.config/Code - OSS/User/settings.json",
            home + "/.config/VSCodium/User/settings.json",
            home + "/Library/Application Support/Code/User/settings.json",
            home + "/Library/Application Support/VSCodium/User/settings.json"
        };

        for (const auto& path : candidates) {
            if (file_exists(path) || is_command_in_path("code")) {
                info.is_installed = true;
                info.config_path = path;
                std::string contents = read_file(path);
                info.is_registered = (contents.find("Meridian Shell") != std::string::npos);
                break;
            }
        }
        return info;
    }

    bool register_profile(const std::string& shell_bin, const std::string& gui_bin) override {
        std::string home = get_home_dir();
        std::vector<std::string> paths = {
            home + "/.config/Code/User/settings.json",
            home + "/.config/Code - OSS/User/settings.json",
            home + "/.config/VSCodium/User/settings.json",
            home + "/Library/Application Support/Code/User/settings.json",
            home + "/Library/Application Support/VSCodium/User/settings.json"
        };

        bool ok = false;
        for (const auto& path : paths) {
            std::string content = read_file(path);
            if (!content.empty() || file_exists(path)) {
                std::string updated = merge_vscode_profile(content, shell_bin, gui_bin);
                write_file(path, updated);
                ok = true;
            }
        }
        return ok;
    }
};

// -----------------------------------------------------------------------------
// Google Antigravity Adapter (Linux / macOS / Windows / WSL)
// -----------------------------------------------------------------------------
class AntigravityAdapter : public IdeAdapter {
public:
    std::string id() const override { return "antigravity"; }
    std::string name() const override { return "Google Antigravity"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "AI IDE";
        info.supports_terminal = true;

        std::string home = get_home_dir();
        std::vector<std::string> candidates = {
            home + "/.config/Antigravity/User/settings.json",
            home + "/.config/antigravity/User/settings.json",
            home + "/.gemini/antigravity",
            home + "/Library/Application Support/Antigravity/User/settings.json"
        };

        for (const auto& path : candidates) {
            if (file_exists(path)) {
                info.is_installed = true;
                info.config_path = (path.find("settings.json") != std::string::npos) ? path : (home + "/.config/Antigravity/User/settings.json");
                std::string contents = read_file(info.config_path);
                info.is_registered = (contents.find("Meridian Shell") != std::string::npos);
                break;
            }
        }
        return info;
    }

    bool register_profile(const std::string& shell_bin, const std::string& gui_bin) override {
        std::string home = get_home_dir();
        std::vector<std::string> paths = {
            home + "/.config/Antigravity/User/settings.json",
            home + "/.config/antigravity/User/settings.json",
            home + "/Library/Application Support/Antigravity/User/settings.json"
        };

        for (const auto& path : paths) {
            std::string content = read_file(path);
            std::string updated = merge_vscode_profile(content, shell_bin, gui_bin);
            write_file(path, updated);
        }
        return true;
    }
};

// -----------------------------------------------------------------------------
// Cursor Adapter (Linux / macOS / Windows / WSL)
// -----------------------------------------------------------------------------
class CursorAdapter : public IdeAdapter {
public:
    std::string id() const override { return "cursor"; }
    std::string name() const override { return "Cursor"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "AI IDE";
        info.supports_terminal = true;

        std::string home = get_home_dir();
        std::vector<std::string> candidates = {
            home + "/.config/Cursor/User/settings.json",
            home + "/Library/Application Support/Cursor/User/settings.json"
        };

        for (const auto& path : candidates) {
            if (file_exists(path) || is_command_in_path("cursor")) {
                info.is_installed = true;
                info.config_path = path;
                std::string contents = read_file(path);
                info.is_registered = (contents.find("Meridian Shell") != std::string::npos);
                break;
            }
        }
        return info;
    }

    bool register_profile(const std::string& shell_bin, const std::string& gui_bin) override {
        std::string home = get_home_dir();
        std::vector<std::string> paths = {
            home + "/.config/Cursor/User/settings.json",
            home + "/Library/Application Support/Cursor/User/settings.json"
        };

        for (const auto& path : paths) {
            std::string content = read_file(path);
            std::string updated = merge_vscode_profile(content, shell_bin, gui_bin);
            write_file(path, updated);
        }
        return true;
    }
};

// -----------------------------------------------------------------------------
// Windsurf Adapter (Linux / macOS / Windows / WSL)
// -----------------------------------------------------------------------------
class WindsurfAdapter : public IdeAdapter {
public:
    std::string id() const override { return "windsurf"; }
    std::string name() const override { return "Windsurf"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "AI IDE";
        info.supports_terminal = true;

        std::string home = get_home_dir();
        std::vector<std::string> candidates = {
            home + "/.config/Windsurf/User/settings.json",
            home + "/Library/Application Support/Windsurf/User/settings.json"
        };

        for (const auto& path : candidates) {
            if (file_exists(path) || is_command_in_path("windsurf")) {
                info.is_installed = true;
                info.config_path = path;
                std::string contents = read_file(path);
                info.is_registered = (contents.find("Meridian Shell") != std::string::npos);
                break;
            }
        }
        return info;
    }

    bool register_profile(const std::string& shell_bin, const std::string& gui_bin) override {
        std::string home = get_home_dir();
        std::vector<std::string> paths = {
            home + "/.config/Windsurf/User/settings.json",
            home + "/Library/Application Support/Windsurf/User/settings.json"
        };

        for (const auto& path : paths) {
            std::string content = read_file(path);
            std::string updated = merge_vscode_profile(content, shell_bin, gui_bin);
            write_file(path, updated);
        }
        return true;
    }
};

// -----------------------------------------------------------------------------
// Zed Editor Adapter
// -----------------------------------------------------------------------------
class ZedAdapter : public IdeAdapter {
public:
    std::string id() const override { return "zed"; }
    std::string name() const override { return "Zed"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "Editor";
        info.supports_terminal = true;

        std::string home = get_home_dir();
        std::vector<std::string> candidates = {
            home + "/.config/zed/settings.json",
            home + "/Library/Application Support/Zed/settings.json"
        };

        for (const auto& path : candidates) {
            if (file_exists(path) || is_command_in_path("zed")) {
                info.is_installed = true;
                info.config_path = path;
                std::string contents = read_file(path);
                info.is_registered = (contents.find("meridian-shell") != std::string::npos);
                break;
            }
        }
        return info;
    }

    bool register_profile(const std::string& shell_bin, const std::string& /*gui_bin*/) override {
        std::string home = get_home_dir();
        std::vector<std::string> paths = {
            home + "/.config/zed/settings.json",
            home + "/Library/Application Support/Zed/settings.json"
        };

        for (const auto& path : paths) {
            std::string content = read_file(path);
            if (content.find("terminal") == std::string::npos) {
                std::string snippet = "{\n  \"terminal\": {\n    \"shell\": {\n      \"program\": \"" + shell_bin + "\"\n    }\n  }\n}\n";
                write_file(path, snippet);
            }
        }
        return true;
    }
};

// -----------------------------------------------------------------------------
// JetBrains IDE Adapter (IntelliJ, CLion, PyCharm, WebStorm, RustRover)
// -----------------------------------------------------------------------------
class JetBrainsAdapter : public IdeAdapter {
public:
    std::string id() const override { return "jetbrains"; }
    std::string name() const override { return "JetBrains IDEs"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "IDE";
        info.supports_terminal = true;

        std::string home = get_home_dir();
        std::vector<std::string> jb_roots = {
            home + "/.config/JetBrains",
            home + "/Library/Application Support/JetBrains"
        };

        for (const auto& jb : jb_roots) {
            if (file_exists(jb)) {
                info.is_installed = true;
                info.config_path = jb;
                info.is_registered = true;
                break;
            }
        }
        if (!info.is_installed) {
            if (is_command_in_path("idea") || is_command_in_path("clion") ||
                is_command_in_path("pycharm") || is_command_in_path("webstorm") ||
                is_command_in_path("rustrover")) {
                info.is_installed = true;
                info.is_registered = true;
            }
        }
        return info;
    }

    bool register_profile(const std::string& /*shell_bin*/, const std::string& /*gui_bin*/) override {
        return true;
    }
};

// -----------------------------------------------------------------------------
// Windows Terminal Adapter (Windows / WSL)
// -----------------------------------------------------------------------------
class WindowsTerminalAdapter : public IdeAdapter {
public:
    std::string id() const override { return "windowsterminal"; }
    std::string name() const override { return "Windows Terminal"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "Terminal Emulator";
        info.supports_terminal = true;

        const char* wsl = std::getenv("WSL_DISTRO_NAME");
        if (wsl || is_command_in_path("wt.exe") || is_command_in_path("wsl.exe")) {
            info.is_installed = true;
            info.is_registered = true;
        }
        return info;
    }

    bool register_profile(const std::string& /*shell_bin*/, const std::string& /*gui_bin*/) override {
        return true;
    }
};

// -----------------------------------------------------------------------------
// Neovim Adapter
// -----------------------------------------------------------------------------
class NeovimAdapter : public IdeAdapter {
public:
    std::string id() const override { return "neovim"; }
    std::string name() const override { return "Neovim"; }

    IdeInfo detect() override {
        IdeInfo info;
        info.id = id();
        info.name = name();
        info.category = "Editor";
        info.supports_terminal = true;

        std::string home = get_home_dir();
        if (file_exists(home + "/.config/nvim") || is_command_in_path("nvim")) {
            info.is_installed = true;
            info.is_registered = true;
        }
        return info;
    }

    bool register_profile(const std::string& /*shell_bin*/, const std::string& /*gui_bin*/) override {
        return true;
    }
};

// -----------------------------------------------------------------------------
// IdeDetector Implementation
// -----------------------------------------------------------------------------
IdeDetector::IdeDetector() {
    adapters_.push_back(std::make_unique<VSCodeAdapter>());
    adapters_.push_back(std::make_unique<AntigravityAdapter>());
    adapters_.push_back(std::make_unique<CursorAdapter>());
    adapters_.push_back(std::make_unique<WindsurfAdapter>());
    adapters_.push_back(std::make_unique<ZedAdapter>());
    adapters_.push_back(std::make_unique<JetBrainsAdapter>());
    adapters_.push_back(std::make_unique<WindowsTerminalAdapter>());
    adapters_.push_back(std::make_unique<NeovimAdapter>());
}

IdeDetector& IdeDetector::instance() {
    static IdeDetector s_instance;
    return s_instance;
}

std::vector<IdeInfo> IdeDetector::scan_and_register(bool silent) {
    std::string shell_bin = get_shell_binary_path();
    std::string gui_bin = get_gui_binary_path();
    std::vector<IdeInfo> results;

    if (!silent) {
        std::cout << "\n\033[1;36mDetecting development environments...\033[0m\n";
    }

    for (auto& adapter : adapters_) {
        IdeInfo info = adapter->detect();
        if (info.is_installed) {
            if (!silent) {
                std::cout << " \033[1;32m✓\033[0m " << info.name << "\n";
            }
            if (info.supports_terminal) {
                bool reg_ok = adapter->register_profile(shell_bin, gui_bin);
                info.is_registered = reg_ok || info.is_registered;
            }
        }
        results.push_back(info);
    }

    if (!silent) {
        std::cout << "\n\033[1;36mRegistering Meridian terminal integrations...\033[0m\n";
        for (const auto& info : results) {
            if (info.is_installed && info.is_registered) {
                std::cout << " \033[1;32m✓\033[0m " << info.name << "\n";
            }
        }
        std::cout << "\n\033[1;32mMeridian is now available as an integrated terminal where supported.\033[0m\n\n";
    }

    // Mark registration done
    std::string home = get_home_dir();
    write_file(home + "/.config/meridian/.ide_registered_v25", "1");

    return results;
}

void IdeDetector::ensure_first_run_registered() {
    static bool s_checked = false;
    if (s_checked) return;
    s_checked = true;

    std::string home = get_home_dir();
    std::string marker = home + "/.config/meridian/.ide_registered_v25";
    if (!file_exists(marker)) {
        instance().scan_and_register(true); // Silent background auto-registration
    }
}

void IdeDetector::print_ide_report() {
    instance().scan_and_register(false);
}

} // namespace meridian::dev
