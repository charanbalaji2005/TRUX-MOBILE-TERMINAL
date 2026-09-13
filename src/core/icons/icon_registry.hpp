#pragma once
// src/core/icons/icon_registry.hpp
//
// Central Icon Registry for Meridian Terminal.
// Maps files, extensions, directories, devops tools, git statuses, and commands
// to rich Nerd Font v3 glyphs with automatic Unicode & ASCII fallback tiers.
// Supports custom overrides via ~/.config/meridian/icons.toml.

#include "icon_detector.hpp"
#include "nerd_font_icons.hpp"
#include "fallback_icons.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace meridian::icons {

// ─── Data Structures ─────────────────────────────────────────────────────────

enum class GitStatusType {
    Branch,
    Clean,
    Modified,
    Staged,
    Untracked,
    Deleted,
    Renamed,
    Conflict,
    Ahead,
    Behind,
    Diverged
};

struct Icon {
    std::string glyph;         // Raw character / codepoint string (e.g. "")
    std::string color_escape;  // ANSI color code (e.g. "\033[38;2;81;154;186m")
    std::string category;      // "programming", "web", "directory", "git", etc.
    IconTier tier = IconTier::NERD_FONT;

    // Formatted output
    std::string to_string(bool with_color = true) const {
        if (!with_color || color_escape.empty()) {
            return glyph;
        }
        return color_escape + glyph + "\033[0m";
    }

    // Implicit conversion for convenience
    operator std::string() const { return to_string(true); }
};

struct FileInfo {
    std::string filename;
    std::string extension;
    std::string path;
    bool is_directory = false;
    bool is_executable = false;
    bool is_symlink = false;
    bool is_hidden = false;
    char git_status = ' '; // ' ' clean, 'M' modified, 'A' staged, '?' untracked, 'D' deleted
};

struct DirectoryInfo {
    std::string path;
    std::string name;
    bool is_git_repo = false;
    bool is_empty = false;
};

// ─── Icon Registry ──────────────────────────────────────────────────────────

class IconRegistry {
public:
    static IconRegistry& instance();

    IconRegistry();
    ~IconRegistry() = default;

    // Reload configuration and custom mappings (from ~/.config/meridian/icons.toml)
    void reload_custom_mappings();

    // ── Primary APIs ────────────────────────────────────────────────────────
    Icon getIcon(const FileInfo& file_info) const;
    Icon getIconForExtension(const std::string& extension) const;
    Icon getIconForCommand(const std::string& command_name) const;
    Icon getIconForGitStatus(GitStatusType status) const;
    Icon getIconForDirectory(const DirectoryInfo& dir_info) const;
    Icon getIconForApplication(const std::string& app_name) const;

    // Convenience helpers
    Icon getIconForFile(
        const std::string& filename,
        bool is_dir = false,
        bool is_executable = false,
        char git_status = ' '
    ) const;

    Icon getIconForPath(const std::string& formatted_path, bool is_git_repo = false) const;

    // Extensible dynamic custom registration
    void registerCustomExtension(const std::string& ext, const std::string& glyph, const std::string& color = "");
    void registerCustomFilename(const std::string& filename, const std::string& glyph, const std::string& color = "");
    void registerCustomDirectory(const std::string& dirname, const std::string& glyph, const std::string& color = "");
    void registerCustomCommand(const std::string& cmd, const std::string& glyph, const std::string& color = "");

    // Direct access to tier-aware icon sets
    IconTier current_tier() const { return IconDetector::detect_tier(); }

private:
    void init_default_mappings();
    void parse_toml_file(const std::string& filepath);

    // Mappings: key -> pair<NerdFontGlyph, AnsiColor>
    std::map<std::string, std::pair<std::string, std::string>> extension_map_;
    std::map<std::string, std::pair<std::string, std::string>> filename_map_;
    std::map<std::string, std::pair<std::string, std::string>> directory_map_;
    std::map<std::string, std::pair<std::string, std::string>> command_map_;
    std::map<std::string, std::pair<std::string, std::string>> app_map_;

    // Custom user overrides
    std::map<std::string, std::pair<std::string, std::string>> custom_extensions_;
    std::map<std::string, std::pair<std::string, std::string>> custom_filenames_;
    std::map<std::string, std::pair<std::string, std::string>> custom_directories_;
    std::map<std::string, std::pair<std::string, std::string>> custom_commands_;
};

} // namespace meridian::icons

