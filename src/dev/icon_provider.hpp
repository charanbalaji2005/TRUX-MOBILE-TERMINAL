// src/dev/icon_provider.hpp
#pragma once
//
// Meridian Icon Provider — Nerd Font / Unicode / ASCII tiered icon engine.
// Resolves per-file, per-language, per-directory, and per-git-status icons.
// Three tiers:
//   NERD_FONT: Full Nerd Font / Powerline icon codepoints
//   UNICODE  : Safe BMP Unicode symbols (emoji + misc)
//   ASCII    : Plain ASCII markers (no box-drawing, no replacements)
//
// Never outputs replacement boxes.

#include <string>

namespace meridian::dev {

enum class IconTier { NERD_FONT, UNICODE, ASCII };

struct IconSet {
    // Directories
    static const char* dir_home(IconTier t);
    static const char* dir_folder(IconTier t);
    static const char* dir_downloads(IconTier t);
    static const char* dir_documents(IconTier t);
    static const char* dir_desktop(IconTier t);
    static const char* dir_pictures(IconTier t);
    static const char* dir_videos(IconTier t);
    static const char* dir_music(IconTier t);
    static const char* dir_src(IconTier t);
    static const char* dir_build(IconTier t);
    static const char* dir_config(IconTier t);
    static const char* dir_git_repo(IconTier t);
    static const char* dir_node_modules(IconTier t);
    static const char* dir_hidden(IconTier t);

    // Programming Languages
    static const char* lang_cpp(IconTier t);
    static const char* lang_c(IconTier t);
    static const char* lang_python(IconTier t);
    static const char* lang_rust(IconTier t);
    static const char* lang_go(IconTier t);
    static const char* lang_js(IconTier t);
    static const char* lang_ts(IconTier t);
    static const char* lang_java(IconTier t);
    static const char* lang_shell(IconTier t);
    static const char* lang_lua(IconTier t);
    static const char* lang_ruby(IconTier t);
    static const char* lang_php(IconTier t);
    static const char* lang_swift(IconTier t);
    static const char* lang_kotlin(IconTier t);
    static const char* lang_zig(IconTier t);

    // Files by category
    static const char* file_text(IconTier t);
    static const char* file_readme(IconTier t);
    static const char* file_license(IconTier t);
    static const char* file_makefile(IconTier t);
    static const char* file_cmake(IconTier t);
    static const char* file_docker(IconTier t);
    static const char* file_git(IconTier t);
    static const char* file_env(IconTier t);
    static const char* file_json(IconTier t);
    static const char* file_yaml(IconTier t);
    static const char* file_xml(IconTier t);
    static const char* file_toml(IconTier t);
    static const char* file_ini(IconTier t);
    static const char* file_image(IconTier t);
    static const char* file_video(IconTier t);
    static const char* file_audio(IconTier t);
    static const char* file_archive(IconTier t);
    static const char* file_binary(IconTier t);
    static const char* file_executable(IconTier t);
    static const char* file_lock(IconTier t);
    static const char* file_unknown(IconTier t);

    // Git status badges
    static const char* git_modified(IconTier t);
    static const char* git_staged(IconTier t);
    static const char* git_untracked(IconTier t);
    static const char* git_deleted(IconTier t);
    static const char* git_clean(IconTier t);
    static const char* git_branch(IconTier t);
    static const char* git_ahead(IconTier t);
    static const char* git_behind(IconTier t);
    static const char* git_conflict(IconTier t);
};

class IconProvider {
public:
    // Detect the best supported tier from environment variables and terminal type.
    static IconTier detect_tier();

    // Return the icon string for a file/directory.
    // filename: the base filename (not full path)
    // is_dir: true for directories
    // is_executable: true if file has execute permission
    // git_status: ' '=clean, 'M'=modified, 'A'=staged, '?'=untracked, 'D'=deleted
    static std::string icon_for_file(
        const std::string& filename,
        bool is_dir,
        bool is_executable = false,
        char git_status = ' '
    );

    // Directory icon based on the formatted path (like "~/Downloads")
    static std::string icon_for_directory(const std::string& formatted_path, bool is_git_repo = false);

    // Colorized icon + ANSI reset; color is based on file type
    static std::string colored_icon_for_file(
        const std::string& filename,
        bool is_dir,
        bool is_executable = false,
        char git_status = ' '
    );

private:
    static std::string icon_for_extension(const std::string& ext, IconTier t);
    static std::string icon_for_known_filename(const std::string& name, IconTier t);
};

} // namespace meridian::dev

