// src/dev/icon_provider.cpp
#include "icon_provider.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

namespace meridian::dev {

// ─── IconSet Implementation ───────────────────────────────────────────────────

// Directories
const char* IconSet::dir_home(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-home
        case IconTier::UNICODE:   return "🏠 ";
        default:                  return "~ ";
    }
}
const char* IconSet::dir_folder(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-folder
        case IconTier::UNICODE:   return "📁 ";
        default:                  return "/ ";
    }
}
const char* IconSet::dir_downloads(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-download
        case IconTier::UNICODE:   return "📥 ";
        default:                  return "dl/";
    }
}
const char* IconSet::dir_documents(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "󰈙 ";   // nf-md-file_document
        case IconTier::UNICODE:   return "📄 ";
        default:                  return "doc/";
    }
}
const char* IconSet::dir_desktop(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-desktop
        case IconTier::UNICODE:   return "🖥 ";
        default:                  return "dt/";
    }
}
const char* IconSet::dir_pictures(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-picture_o
        case IconTier::UNICODE:   return "🖼 ";
        default:                  return "pic/";
    }
}
const char* IconSet::dir_videos(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-film
        case IconTier::UNICODE:   return "🎬 ";
        default:                  return "vid/";
    }
}
const char* IconSet::dir_music(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-music
        case IconTier::UNICODE:   return "🎵 ";
        default:                  return "mus/";
    }
}
const char* IconSet::dir_src(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-code
        case IconTier::UNICODE:   return "📦 ";
        default:                  return "src/";
    }
}
const char* IconSet::dir_build(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-cog
        case IconTier::UNICODE:   return "⚙ ";
        default:                  return "bld/";
    }
}
const char* IconSet::dir_config(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-cog
        case IconTier::UNICODE:   return "⚙ ";
        default:                  return "cfg/";
    }
}
const char* IconSet::dir_git_repo(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-git
        case IconTier::UNICODE:   return "🔀 ";
        default:                  return "git/";
    }
}
const char* IconSet::dir_node_modules(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-nodejs
        case IconTier::UNICODE:   return "📦 ";
        default:                  return "nm/";
    }
}
const char* IconSet::dir_hidden(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-folder
        case IconTier::UNICODE:   return "📂 ";
        default:                  return "./";
    }
}

// Programming Languages
const char* IconSet::lang_cpp(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-custom-cpp
        case IconTier::UNICODE:   return "C++ ";
        default:                  return ".cpp";
    }
}
const char* IconSet::lang_c(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-custom-c
        case IconTier::UNICODE:   return "C ";
        default:                  return ".c  ";
    }
}
const char* IconSet::lang_python(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-python
        case IconTier::UNICODE:   return "🐍 ";
        default:                  return ".py ";
    }
}
const char* IconSet::lang_rust(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-rust
        case IconTier::UNICODE:   return "⚙ ";
        default:                  return ".rs ";
    }
}
const char* IconSet::lang_go(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-go
        case IconTier::UNICODE:   return "Go ";
        default:                  return ".go ";
    }
}
const char* IconSet::lang_js(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-javascript
        case IconTier::UNICODE:   return "JS ";
        default:                  return ".js ";
    }
}
const char* IconSet::lang_ts(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "󰛦 ";   // nf-md-language_typescript
        case IconTier::UNICODE:   return "TS ";
        default:                  return ".ts ";
    }
}
const char* IconSet::lang_java(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-java
        case IconTier::UNICODE:   return "☕ ";
        default:                  return ".jv ";
    }
}
const char* IconSet::lang_shell(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-terminal
        case IconTier::UNICODE:   return "$ ";
        default:                  return ".sh ";
    }
}
const char* IconSet::lang_lua(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-seti-lua
        case IconTier::UNICODE:   return "🌙 ";
        default:                  return ".lua";
    }
}
const char* IconSet::lang_ruby(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-ruby
        case IconTier::UNICODE:   return "💎 ";
        default:                  return ".rb ";
    }
}
const char* IconSet::lang_php(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-php
        case IconTier::UNICODE:   return "🐘 ";
        default:                  return ".php";
    }
}
const char* IconSet::lang_swift(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-swift
        case IconTier::UNICODE:   return "🐦 ";
        default:                  return ".swf";
    }
}
const char* IconSet::lang_kotlin(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-custom-kotlin
        case IconTier::UNICODE:   return "K ";
        default:                  return ".kt ";
    }
}
const char* IconSet::lang_zig(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-seti-zig
        case IconTier::UNICODE:   return "⚡ ";
        default:                  return ".zig";
    }
}

// Files
const char* IconSet::file_text(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-file_text
        case IconTier::UNICODE:   return "📝 ";
        default:                  return "txt ";
    }
}
const char* IconSet::file_readme(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-book
        case IconTier::UNICODE:   return "📖 ";
        default:                  return "doc ";
    }
}
const char* IconSet::file_license(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-balance_scale
        case IconTier::UNICODE:   return "⚖ ";
        default:                  return "lic ";
    }
}
const char* IconSet::file_makefile(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-gnu
        case IconTier::UNICODE:   return "🔧 ";
        default:                  return "mk  ";
    }
}
const char* IconSet::file_cmake(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-custom-cmake
        case IconTier::UNICODE:   return "🔧 ";
        default:                  return "mk  ";
    }
}
const char* IconSet::file_docker(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "󰡨 ";   // nf-md-docker
        case IconTier::UNICODE:   return "🐋 ";
        default:                  return "dok ";
    }
}
const char* IconSet::file_git(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-git
        case IconTier::UNICODE:   return "🔀 ";
        default:                  return "git ";
    }
}
const char* IconSet::file_env(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-lock
        case IconTier::UNICODE:   return "🔒 ";
        default:                  return "env ";
    }
}
const char* IconSet::file_json(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-seti-json
        case IconTier::UNICODE:   return "{ } ";
        default:                  return "json";
    }
}
const char* IconSet::file_yaml(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-seti-yaml
        case IconTier::UNICODE:   return "📋 ";
        default:                  return "yaml";
    }
}
const char* IconSet::file_xml(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "󰗀 ";   // nf-md-xml
        case IconTier::UNICODE:   return "📄 ";
        default:                  return "xml ";
    }
}
const char* IconSet::file_toml(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-seti-config
        case IconTier::UNICODE:   return "⚙ ";
        default:                  return "toml";
    }
}
const char* IconSet::file_ini(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-seti-config
        case IconTier::UNICODE:   return "⚙ ";
        default:                  return "ini ";
    }
}
const char* IconSet::file_image(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-image
        case IconTier::UNICODE:   return "🖼 ";
        default:                  return "img ";
    }
}
const char* IconSet::file_video(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-film
        case IconTier::UNICODE:   return "🎬 ";
        default:                  return "vid ";
    }
}
const char* IconSet::file_audio(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-music
        case IconTier::UNICODE:   return "🎵 ";
        default:                  return "aud ";
    }
}
const char* IconSet::file_archive(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-file_zip_o
        case IconTier::UNICODE:   return "📦 ";
        default:                  return "zip ";
    }
}
const char* IconSet::file_binary(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-oct-file_binary
        case IconTier::UNICODE:   return "⬛ ";
        default:                  return "bin ";
    }
}
const char* IconSet::file_executable(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-terminal
        case IconTier::UNICODE:   return "⚡ ";
        default:                  return "exe ";
    }
}
const char* IconSet::file_lock(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-lock
        case IconTier::UNICODE:   return "🔒 ";
        default:                  return "lck ";
    }
}
const char* IconSet::file_unknown(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-fa-file_o
        case IconTier::UNICODE:   return "📄 ";
        default:                  return "    ";
    }
}

// Git status badges
const char* IconSet::git_modified(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "✸";
        case IconTier::UNICODE:   return "✸";
        default:                  return "M";
    }
}
const char* IconSet::git_staged(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "●";
        case IconTier::UNICODE:   return "●";
        default:                  return "S";
    }
}
const char* IconSet::git_untracked(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "?";
        case IconTier::UNICODE:   return "?";
        default:                  return "?";
    }
}
const char* IconSet::git_deleted(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "✖";
        case IconTier::UNICODE:   return "✖";
        default:                  return "D";
    }
}
const char* IconSet::git_clean(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "✔";
        case IconTier::UNICODE:   return "✔";
        default:                  return "OK";
    }
}
const char* IconSet::git_branch(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return " ";    // nf-dev-git_branch
        case IconTier::UNICODE:   return "⎇ ";
        default:                  return "br:";
    }
}
const char* IconSet::git_ahead(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "↑";
        case IconTier::UNICODE:   return "↑";
        default:                  return "^";
    }
}
const char* IconSet::git_behind(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "↓";
        case IconTier::UNICODE:   return "↓";
        default:                  return "v";
    }
}
const char* IconSet::git_conflict(IconTier t) {
    switch (t) {
        case IconTier::NERD_FONT: return "═";
        case IconTier::UNICODE:   return "⚡";
        default:                  return "!";
    }
}

// ─── IconProvider Implementation ─────────────────────────────────────────────

IconTier IconProvider::detect_tier() {
    // Check explicit override
    const char* override_env = std::getenv("MERIDIAN_ICON_TIER");
    if (override_env) {
        std::string val = override_env;
        if (val == "NERD_FONT" || val == "nerd") return IconTier::NERD_FONT;
        if (val == "UNICODE" || val == "unicode") return IconTier::UNICODE;
        if (val == "ASCII" || val == "ascii") return IconTier::ASCII;
    }

    // Nerd Font-capable terminal detection
    const char* term_prog = std::getenv("TERM_PROGRAM");
    const char* term_var = std::getenv("TERM");
    const char* nerd_font = std::getenv("NERD_FONTS_VERSION");
    const char* kitty_env = std::getenv("KITTY_WINDOW_ID");
    const char* wez_env = std::getenv("WEZTERM_EXECUTABLE");
    const char* alacritty_env = std::getenv("ALACRITTY_LOG");

    if (nerd_font) return IconTier::NERD_FONT;
    if (kitty_env || wez_env) return IconTier::NERD_FONT;
    if (alacritty_env) return IconTier::NERD_FONT;
    if (term_prog) {
        std::string tp = term_prog;
        for (auto& c : tp) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (tp.find("kitty") != std::string::npos ||
            tp.find("iterm") != std::string::npos ||
            tp.find("wezterm") != std::string::npos ||
            tp.find("hyper") != std::string::npos) return IconTier::NERD_FONT;
    }
    if (term_var) {
        std::string tv = term_var;
        for (auto& c : tv) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (tv.find("kitty") != std::string::npos ||
            tv.find("xterm-256color") != std::string::npos) {
            // Could be Nerd Font; assume UNICODE as safe minimum
            return IconTier::UNICODE;
        }
    }

    // Check locale for UTF-8 support
    const char* lang = std::getenv("LANG");
    const char* lc_all = std::getenv("LC_ALL");
    const char* lc_ctype = std::getenv("LC_CTYPE");
    auto has_utf8 = [](const char* s) {
        if (!s) return false;
        std::string v = s;
        for (auto& c : v) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return v.find("utf-8") != std::string::npos || v.find("utf8") != std::string::npos;
    };
    if (has_utf8(lang) || has_utf8(lc_all) || has_utf8(lc_ctype)) {
        return IconTier::UNICODE;
    }

    // Not a TTY or no unicode support
    if (!isatty(STDOUT_FILENO)) return IconTier::ASCII;
    return IconTier::ASCII;
}

std::string IconProvider::icon_for_extension(const std::string& ext, IconTier t) {
    if (ext.empty()) return "";

    // Images
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "webp" ||
        ext == "gif" || ext == "bmp" || ext == "svg" || ext == "ico" || ext == "tiff")
        return IconSet::file_image(t);

    // Videos
    if (ext == "mp4" || ext == "mkv" || ext == "avi" || ext == "mov" ||
        ext == "webm" || ext == "flv")
        return IconSet::file_video(t);

    // Audio
    if (ext == "mp3" || ext == "ogg" || ext == "flac" || ext == "wav" ||
        ext == "aac" || ext == "m4a")
        return IconSet::file_audio(t);

    // Archives
    if (ext == "zip" || ext == "tar" || ext == "gz" || ext == "bz2" ||
        ext == "xz" || ext == "7z" || ext == "rar" || ext == "zst")
        return IconSet::file_archive(t);

    // C/C++
    if (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "c++")
        return IconSet::lang_cpp(t);
    if (ext == "hpp" || ext == "hxx" || ext == "h")
        return IconSet::lang_cpp(t);
    if (ext == "c")
        return IconSet::lang_c(t);

    // Python
    if (ext == "py" || ext == "pyw" || ext == "pyi")
        return IconSet::lang_python(t);

    // Rust
    if (ext == "rs") return IconSet::lang_rust(t);

    // Go
    if (ext == "go") return IconSet::lang_go(t);

    // JavaScript / TypeScript
    if (ext == "js" || ext == "mjs" || ext == "cjs")
        return IconSet::lang_js(t);
    if (ext == "ts" || ext == "tsx" || ext == "mts")
        return IconSet::lang_ts(t);
    if (ext == "jsx") return IconSet::lang_js(t);

    // Java / Kotlin
    if (ext == "java") return IconSet::lang_java(t);
    if (ext == "kt" || ext == "kts") return IconSet::lang_kotlin(t);

    // Shell scripts
    if (ext == "sh" || ext == "bash" || ext == "zsh" || ext == "fish" || ext == "ksh")
        return IconSet::lang_shell(t);

    // Lua
    if (ext == "lua") return IconSet::lang_lua(t);

    // Ruby
    if (ext == "rb" || ext == "erb") return IconSet::lang_ruby(t);

    // PHP
    if (ext == "php") return IconSet::lang_php(t);

    // Swift
    if (ext == "swift") return IconSet::lang_swift(t);

    // Zig
    if (ext == "zig") return IconSet::lang_zig(t);

    // Config formats
    if (ext == "json" || ext == "jsonc") return IconSet::file_json(t);
    if (ext == "yaml" || ext == "yml") return IconSet::file_yaml(t);
    if (ext == "toml") return IconSet::file_toml(t);
    if (ext == "xml") return IconSet::file_xml(t);
    if (ext == "ini" || ext == "conf" || ext == "cfg") return IconSet::file_ini(t);
    if (ext == "env") return IconSet::file_env(t);

    // Documents
    if (ext == "md" || ext == "markdown") return IconSet::file_readme(t);
    if (ext == "txt" || ext == "rst" || ext == "adoc") return IconSet::file_text(t);
    if (ext == "pdf") return IconSet::file_text(t);

    // Build files
    if (ext == "cmake") return IconSet::file_cmake(t);
    if (ext == "mk") return IconSet::file_makefile(t);
    if (ext == "lock") return IconSet::file_lock(t);

    // Binary/object
    if (ext == "o" || ext == "a" || ext == "so" || ext == "dylib" || ext == "dll")
        return IconSet::file_binary(t);

    return "";
}

std::string IconProvider::icon_for_known_filename(const std::string& name, IconTier t) {
    // Case-insensitive compare helper
    auto lc = [](const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    };
    std::string n = lc(name);

    // Well-known filenames
    if (n == "readme" || n == "readme.md" || n == "readme.txt" || n == "readme.rst")
        return IconSet::file_readme(t);
    if (n == "license" || n == "licence" || n == "license.md" || n == "license.txt")
        return IconSet::file_license(t);
    if (n == "makefile" || n == "gnumakefile") return IconSet::file_makefile(t);
    if (n == "cmakelists.txt") return IconSet::file_cmake(t);
    if (n == "dockerfile" || n == "containerfile") return IconSet::file_docker(t);
    if (n == "docker-compose.yml" || n == "docker-compose.yaml" || n == "compose.yaml")
        return IconSet::file_docker(t);
    if (n == ".gitignore" || n == ".gitattributes" || n == ".gitmodules")
        return IconSet::file_git(t);
    if (n == ".env" || n == ".env.local" || n == ".env.development" || n == ".env.production")
        return IconSet::file_env(t);
    if (n == "cargo.toml") return IconSet::lang_rust(t);
    if (n == "cargo.lock") return IconSet::file_lock(t);
    if (n == "package.json" || n == "package-lock.json" || n == "yarn.lock")
        return IconSet::lang_js(t);
    if (n == "tsconfig.json") return IconSet::lang_ts(t);
    if (n == "go.mod" || n == "go.sum") return IconSet::lang_go(t);
    if (n == "requirements.txt" || n == "setup.py" || n == "pyproject.toml" || n == "poetry.lock")
        return IconSet::lang_python(t);
    if (n == "gemfile" || n == "gemfile.lock") return IconSet::lang_ruby(t);
    if (n == "pom.xml" || n == "build.gradle" || n == "settings.gradle")
        return IconSet::lang_java(t);
    if (n == "flake.nix" || n == "default.nix" || n == "shell.nix")
        return IconSet::lang_zig(t); // No Nix-specific icon; reuse zig

    return "";
}

std::string IconProvider::icon_for_file(
    const std::string& filename,
    bool is_dir,
    bool is_executable,
    char git_status
) {
    IconTier t = detect_tier();

    if (is_dir) {
        // Use specialised directory icons based on name
        std::string n = filename;
        for (auto& c : n) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (n == "~" || n == "home") return IconSet::dir_home(t);
        if (n == "downloads" || n == "download") return IconSet::dir_downloads(t);
        if (n == "documents" || n == "document" || n == "docs" || n == "doc")
            return IconSet::dir_documents(t);
        if (n == "desktop") return IconSet::dir_desktop(t);
        if (n == "pictures" || n == "photos" || n == "images" || n == "wallpapers")
            return IconSet::dir_pictures(t);
        if (n == "videos" || n == "movies") return IconSet::dir_videos(t);
        if (n == "music" || n == "audio") return IconSet::dir_music(t);
        if (n == "src" || n == "source" || n == "projects" || n == "repos" || n == "workspace")
            return IconSet::dir_src(t);
        if (n == "build" || n == "target" || n == "dist" || n == "out" || n == "output")
            return IconSet::dir_build(t);
        if (n == ".config" || n == "config" || n == ".local")
            return IconSet::dir_config(t);
        if (n == "node_modules" || n == ".node_modules")
            return IconSet::dir_node_modules(t);
        if (!filename.empty() && filename[0] == '.')
            return IconSet::dir_hidden(t);
        return IconSet::dir_folder(t);
    }

    // Known filename (exact match)
    std::string known = icon_for_known_filename(filename, t);
    if (!known.empty()) return known;

    // Executable files (no extension or well-known)
    if (is_executable) {
        auto dot = filename.rfind('.');
        if (dot == std::string::npos) {
            // No extension + executable = binary
            return IconSet::file_executable(t);
        }
    }

    // Extension-based lookup
    auto dot = filename.rfind('.');
    if (dot != std::string::npos) {
        std::string ext = filename.substr(dot + 1);
        for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        std::string ext_icon = icon_for_extension(ext, t);
        if (!ext_icon.empty()) return ext_icon;
    }

    // Executable files with extension (like scripts)
    if (is_executable) return IconSet::file_executable(t);

    return IconSet::file_unknown(t);
}

std::string IconProvider::icon_for_directory(const std::string& formatted_path, bool is_git_repo) {
    IconTier t = detect_tier();
    std::string lower = formatted_path;
    for (auto& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (formatted_path == "~" || formatted_path == "/root") return IconSet::dir_home(t);
    if (lower.find("download") != std::string::npos) return IconSet::dir_downloads(t);
    if (lower.find("document") != std::string::npos) return IconSet::dir_documents(t);
    if (lower.find("desktop") != std::string::npos) return IconSet::dir_desktop(t);
    if (lower.find("picture") != std::string::npos || lower.find("photo") != std::string::npos ||
        lower.find("wallpaper") != std::string::npos || lower.find("image") != std::string::npos)
        return IconSet::dir_pictures(t);
    if (lower.find("video") != std::string::npos || lower.find("movie") != std::string::npos)
        return IconSet::dir_videos(t);
    if (lower.find("music") != std::string::npos || lower.find("audio") != std::string::npos)
        return IconSet::dir_music(t);
    if (lower.find("project") != std::string::npos || lower.find("workspace") != std::string::npos ||
        lower.find("/src") != std::string::npos || lower.find("repo") != std::string::npos)
        return IconSet::dir_src(t);
    if (lower.find("build") != std::string::npos || lower.find("target") != std::string::npos ||
        lower.find("/out") != std::string::npos || lower.find("/dist") != std::string::npos)
        return IconSet::dir_build(t);
    if (lower.find(".config") != std::string::npos || lower.find("/etc") != std::string::npos)
        return IconSet::dir_config(t);
    if (is_git_repo) return IconSet::dir_git_repo(t);
    return IconSet::dir_folder(t);
}

std::string IconProvider::colored_icon_for_file(
    const std::string& filename,
    bool is_dir,
    bool is_executable,
    char git_status
) {
    std::string icon = icon_for_file(filename, is_dir, is_executable, git_status);

    // ANSI color per type
    if (is_dir) return "\033[38;2;97;175;239m" + icon + "\033[0m";  // blue for directories

    // Extension-based color
    auto dot = filename.rfind('.');
    if (dot != std::string::npos) {
        std::string ext = filename.substr(dot + 1);
        for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (ext == "cpp" || ext == "hpp" || ext == "c" || ext == "h" || ext == "cc" || ext == "cxx")
            return "\033[38;2;97;130;207m" + icon + "\033[0m";  // blue-ish for C/C++
        if (ext == "py" || ext == "pyw")
            return "\033[38;2;255;212;83m" + icon + "\033[0m";  // golden for Python
        if (ext == "rs")
            return "\033[38;2;244;130;95m" + icon + "\033[0m";  // orange for Rust
        if (ext == "go")
            return "\033[38;2;0;173;216m" + icon + "\033[0m";   // cyan for Go
        if (ext == "js" || ext == "mjs" || ext == "jsx")
            return "\033[38;2;240;219;79m" + icon + "\033[0m";  // yellow for JS
        if (ext == "ts" || ext == "tsx")
            return "\033[38;2;49;120;198m" + icon + "\033[0m";  // blue for TS
        if (ext == "sh" || ext == "bash" || ext == "zsh" || ext == "fish")
            return "\033[38;2;76;175;80m" + icon + "\033[0m";   // green for shell
        if (ext == "md" || ext == "markdown")
            return "\033[38;2;132;187;255m" + icon + "\033[0m"; // soft blue for markdown
        if (ext == "json" || ext == "yaml" || ext == "yml" || ext == "toml")
            return "\033[38;2;229;192;123m" + icon + "\033[0m"; // amber for configs
        if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "webp" ||
            ext == "gif" || ext == "bmp" || ext == "svg")
            return "\033[38;2;187;143;206m" + icon + "\033[0m"; // purple for images
    }

    // Known filenames
    std::string n = filename;
    for (auto& c : n) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (n == "makefile" || n == "cmakelists.txt")
        return "\033[38;2;229;192;123m" + icon + "\033[0m";
    if (n.find(".git") == 0)
        return "\033[38;2;240;100;100m" + icon + "\033[0m";
    if (n == "license" || n == "licence")
        return "\033[38;2;150;200;150m" + icon + "\033[0m";

    if (is_executable) return "\033[38;2;76;175;80m" + icon + "\033[0m"; // green for exec

    // Git status coloring
    if (git_status == 'M' || git_status == 'm')
        return "\033[38;2;255;155;55m" + icon + "\033[0m";  // orange for modified
    if (git_status == 'A')
        return "\033[38;2;76;175;80m" + icon + "\033[0m";   // green for staged
    if (git_status == '?')
        return "\033[38;2;140;150;170m" + icon + "\033[0m"; // grey for untracked
    if (git_status == 'D')
        return "\033[38;2;240;100;100m" + icon + "\033[0m"; // red for deleted

    return "\033[38;2;200;210;230m" + icon + "\033[0m";
}

} // namespace meridian::dev

