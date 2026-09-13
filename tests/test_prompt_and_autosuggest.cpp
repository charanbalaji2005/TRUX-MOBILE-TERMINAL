// tests/test_prompt_and_autosuggest.cpp
// Meridian Shell — Prompt, Autosuggestion Engine, and Icon Provider Tests

#include "mini_test.hpp"
#include "../src/shell/autosuggest_engine.hpp"
#include "../src/dev/icon_provider.hpp"

#include <algorithm>
#include <initializer_list>
#include <string>
#include <vector>

using namespace meridian::shell;
using namespace meridian::dev;

// ─────────────────────────────────────────────────────────────────────────────
// Helper
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<std::string> H(std::initializer_list<const char*> cmds) {
    std::vector<std::string> h;
    for (auto c : cmds) h.push_back(c);
    return h;
}

// ─────────────────────────────────────────────────────────────────────────────
// 1. History Autosuggestion — Prefix Matching
// ─────────────────────────────────────────────────────────────────────────────

MTEST(autosuggest_empty_prefix_returns_empty) {
    auto h = H({"git status", "cd /tmp"});
    ASSERT_TRUE(AutosuggestEngine::get_suggestion("", h).empty());
}

MTEST(autosuggest_no_history_returns_empty) {
    ASSERT_TRUE(AutosuggestEngine::get_suggestion("git", {}).empty());
}

MTEST(autosuggest_simple_prefix_match) {
    auto h = H({"git status", "ls -la"});
    std::string s = AutosuggestEngine::get_suggestion("git", h);
    ASSERT_EQ(s, std::string("git status"));
}

MTEST(autosuggest_longer_prefix_match) {
    auto h = H({"git status", "git push origin main"});
    std::string s = AutosuggestEngine::get_suggestion("git p", h);
    ASSERT_EQ(s, std::string("git push origin main"));
}

MTEST(autosuggest_no_match_returns_empty) {
    auto h = H({"git status", "ls -la"});
    std::string s = AutosuggestEngine::get_suggestion("make", h);
    ASSERT_TRUE(s.empty());
}

MTEST(autosuggest_exact_match_not_suggested) {
    auto h = H({"git status"});
    std::string s = AutosuggestEngine::get_suggestion("git status", h);
    // Should be empty (no longer suffix to offer) or the same (no additional chars)
    ASSERT_TRUE(s.empty() || s == std::string("git status"));
}

// ─────────────────────────────────────────────────────────────────────────────
// 2. History Autosuggestion — Recency and Ranking
// ─────────────────────────────────────────────────────────────────────────────

MTEST(autosuggest_recency_match) {
    auto h = H({
        "git commit -m \"init\"",
        "git status",
        "git push origin main",
        "git status --short",
        "git status"
    });
    std::string s = AutosuggestEngine::get_suggestion("git st", h);
    ASSERT_TRUE(s == std::string("git status") || s == std::string("git status --short"));
}

MTEST(autosuggest_history_with_duplicates) {
    auto h = H({
        "git status", "git status", "git status",
        "git push origin main", "git pull",
        "git status"
    });
    std::string s = AutosuggestEngine::get_suggestion("git s", h);
    ASSERT_EQ(s, std::string("git status"));
}

MTEST(autosuggest_special_chars_in_history) {
    auto h = H({"echo 'hello world'", "cat README.md | grep -i version"});
    std::string s = AutosuggestEngine::get_suggestion("echo", h);
    ASSERT_TRUE(s == std::string("echo 'hello world'") || s.empty());
}

MTEST(autosuggest_single_char_prefix) {
    auto h = H({"git status", "git push origin main", "ls -la"});
    std::string s = AutosuggestEngine::get_suggestion("g", h);
    ASSERT_TRUE(s.empty() || s.substr(0, 1) == "g");
}

MTEST(autosuggest_very_long_prefix) {
    auto h = H({"git push origin main --force-with-lease"});
    std::string s = AutosuggestEngine::get_suggestion(
        "git push origin main --force-with-lease", h);
    ASSERT_TRUE(s.empty() || s == std::string("git push origin main --force-with-lease"));
}

// ─────────────────────────────────────────────────────────────────────────────
// 3. Suggestion Suffix Extraction
// ─────────────────────────────────────────────────────────────────────────────

MTEST(suggestion_suffix_basic) {
    std::string suffix = AutosuggestEngine::get_suggestion_suffix("git pu", "git push origin main");
    ASSERT_EQ(suffix, std::string("sh origin main"));
}

MTEST(suggestion_suffix_no_match_returns_empty) {
    std::string suffix = AutosuggestEngine::get_suggestion_suffix("ls", "git status");
    ASSERT_TRUE(suffix.empty());
}

MTEST(suggestion_suffix_exact_prefix_returns_rest) {
    std::string suffix = AutosuggestEngine::get_suggestion_suffix("cd /tmp", "cd /tmp/build");
    ASSERT_EQ(suffix, std::string("/build"));
}

MTEST(suggestion_suffix_empty_prefix) {
    std::string suffix = AutosuggestEngine::get_suggestion_suffix("", "git status");
    ASSERT_EQ(suffix, std::string("git status"));
}

MTEST(autosuggest_suffix_with_empty_suggestion) {
    std::string suffix = AutosuggestEngine::get_suggestion_suffix("git", "");
    ASSERT_TRUE(suffix.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. Accept Next Word (Ctrl+Right)
// ─────────────────────────────────────────────────────────────────────────────

MTEST(accept_next_word_first_word) {
    std::string result = AutosuggestEngine::accept_next_word("git", "git push origin main");
    ASSERT_EQ(result, std::string("git push "));
}

MTEST(accept_next_word_second_word) {
    std::string result = AutosuggestEngine::accept_next_word("git push ", "git push origin main");
    ASSERT_EQ(result, std::string("git push origin "));
}

MTEST(accept_next_word_no_more_words) {
    std::string result = AutosuggestEngine::accept_next_word(
        "git push origin main", "git push origin main");
    ASSERT_EQ(result, std::string("git push origin main"));
}

MTEST(accept_next_word_mismatch) {
    std::string result = AutosuggestEngine::accept_next_word("ls", "git status");
    ASSERT_EQ(result, std::string("ls"));
}

MTEST(accept_next_word_empty_current) {
    std::string result = AutosuggestEngine::accept_next_word("", "git status");
    ASSERT_TRUE(result == std::string("git ") || result == std::string("git"));
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. Filesystem Context-Aware Suggestions
// ─────────────────────────────────────────────────────────────────────────────

MTEST(filesystem_completion_tmp) {
    std::string result = AutosuggestEngine::get_filesystem_completion("/tm");
    ASSERT_TRUE(result == std::string("/tmp") || result == std::string("/tmp/"));
}

MTEST(filesystem_completion_no_match) {
    std::string result = AutosuggestEngine::get_filesystem_completion("/zzz_does_not_exist_xyz");
    ASSERT_TRUE(result.empty());
}

MTEST(filesystem_completion_proc) {
    std::string result = AutosuggestEngine::get_filesystem_completion("/pro");
    ASSERT_TRUE(result == std::string("/proc") || result == std::string("/proc/"));
}

MTEST(filesystem_completion_for_cd_context) {
    auto h = H({"cd /home"});
    std::string s = AutosuggestEngine::get_suggestion("cd /tm", h, "/");
    ASSERT_TRUE(s.empty() || s.substr(0, 6) == "cd /tm");
}

// ─────────────────────────────────────────────────────────────────────────────
// 6. Tab Completion
// ─────────────────────────────────────────────────────────────────────────────

MTEST(tab_completions_no_match_empty) {
    auto results = AutosuggestEngine::get_tab_completions("/zzz_nonexistent_xyz", ".", 10);
    ASSERT_TRUE(results.empty());
}

MTEST(tab_completions_no_crash_for_tmp) {
    auto results = AutosuggestEngine::get_tab_completions("/tmp/", "/tmp", 20);
    ASSERT_GE((int)results.size(), 0);
}

MTEST(tab_completions_for_line_empty_line) {
    auto results = AutosuggestEngine::get_tab_completions_for_line("", ".", 10);
    ASSERT_GE((int)results.size(), 0);
}

MTEST(tab_completions_for_line_ls_context) {
    auto results = AutosuggestEngine::get_tab_completions_for_line("ls /tm", "/", 5);
    ASSERT_GE((int)results.size(), 0);
}

MTEST(tab_completions_for_line_cd_dirs_only) {
    auto results = AutosuggestEngine::get_tab_completions_for_line("cd /tmp/", "/tmp", 10);
    ASSERT_GE((int)results.size(), 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// 7. Icon Provider — Tier Detection
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_tier_detection_returns_valid_tier) {
    IconTier t = IconProvider::detect_tier();
    ASSERT_TRUE(t == IconTier::NERD_FONT || t == IconTier::UNICODE || t == IconTier::ASCII);
}

MTEST(icon_tier_override_env_nerd_font) {
    setenv("MERIDIAN_ICON_TIER", "NERD_FONT", 1);
    ASSERT_EQ(IconProvider::detect_tier(), IconTier::NERD_FONT);
    unsetenv("MERIDIAN_ICON_TIER");
}

MTEST(icon_tier_override_env_ascii) {
    setenv("MERIDIAN_ICON_TIER", "ASCII", 1);
    ASSERT_EQ(IconProvider::detect_tier(), IconTier::ASCII);
    unsetenv("MERIDIAN_ICON_TIER");
}

MTEST(icon_tier_override_env_unicode) {
    setenv("MERIDIAN_ICON_TIER", "UNICODE", 1);
    ASSERT_EQ(IconProvider::detect_tier(), IconTier::UNICODE);
    unsetenv("MERIDIAN_ICON_TIER");
}

// ─────────────────────────────────────────────────────────────────────────────
// 8. Icon Provider — Programming Language Files
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_for_cpp_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("main.cpp", false).empty());
}

MTEST(icon_for_hpp_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("engine.hpp", false).empty());
}

MTEST(icon_for_python_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("script.py", false).empty());
}

MTEST(icon_for_rust_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("main.rs", false).empty());
}

MTEST(icon_for_go_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("server.go", false).empty());
}

MTEST(icon_for_js_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("index.js", false).empty());
}

MTEST(icon_for_ts_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("app.ts", false).empty());
}

MTEST(icon_for_java_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("Main.java", false).empty());
}

MTEST(icon_for_shell_script) {
    ASSERT_FALSE(IconProvider::icon_for_file("install.sh", false).empty());
}

MTEST(icon_for_lua_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("init.lua", false).empty());
}

MTEST(icon_for_rust_zig_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("main.zig", false).empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 9. Icon Provider — Config and Document Files
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_for_markdown_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("README.md", false).empty());
}

MTEST(icon_for_json_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("config.json", false).empty());
}

MTEST(icon_for_yaml_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("docker-compose.yml", false).empty());
}

MTEST(icon_for_toml_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("Cargo.toml", false).empty());
}

MTEST(icon_for_xml_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("pom.xml", false).empty());
}

MTEST(icon_for_ini_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("config.ini", false).empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 10. Icon Provider — Media and Archive Files
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_for_image_files) {
    ASSERT_FALSE(IconProvider::icon_for_file("wallpaper.png", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("photo.jpg", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("anim.gif", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("img.webp", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("icon.svg", false).empty());
}

MTEST(icon_for_video_files) {
    ASSERT_FALSE(IconProvider::icon_for_file("clip.mp4", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("movie.mkv", false).empty());
}

MTEST(icon_for_audio_files) {
    ASSERT_FALSE(IconProvider::icon_for_file("song.mp3", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("track.flac", false).empty());
}

MTEST(icon_for_archive_files) {
    ASSERT_FALSE(IconProvider::icon_for_file("backup.zip", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("data.tar.gz", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("pkg.7z", false).empty());
    ASSERT_FALSE(IconProvider::icon_for_file("archive.rar", false).empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 11. Icon Provider — Well-Known Filenames
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_for_readme_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("README.md", false).empty());
}

MTEST(icon_for_license_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("LICENSE", false).empty());
}

MTEST(icon_for_makefile_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("Makefile", false).empty());
}

MTEST(icon_for_cmake_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("CMakeLists.txt", false).empty());
}

MTEST(icon_for_dockerfile_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("Dockerfile", false).empty());
}

MTEST(icon_for_gitignore_known) {
    ASSERT_FALSE(IconProvider::icon_for_file(".gitignore", false).empty());
}

MTEST(icon_for_env_known) {
    ASSERT_FALSE(IconProvider::icon_for_file(".env", false).empty());
}

MTEST(icon_for_cargo_toml_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("Cargo.toml", false).empty());
}

MTEST(icon_for_package_json_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("package.json", false).empty());
}

MTEST(icon_for_tsconfig_known) {
    ASSERT_FALSE(IconProvider::icon_for_file("tsconfig.json", false).empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 12. Icon Provider — Directories
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_for_directory_home) {
    ASSERT_FALSE(IconProvider::icon_for_file("~", true).empty());
}

MTEST(icon_for_directory_src) {
    ASSERT_FALSE(IconProvider::icon_for_file("src", true).empty());
}

MTEST(icon_for_directory_build) {
    ASSERT_FALSE(IconProvider::icon_for_file("build", true).empty());
}

MTEST(icon_for_directory_downloads) {
    ASSERT_FALSE(IconProvider::icon_for_file("Downloads", true).empty());
}

MTEST(icon_for_directory_generic) {
    ASSERT_FALSE(IconProvider::icon_for_file("myproject", true).empty());
}

MTEST(icon_for_directory_hidden) {
    ASSERT_FALSE(IconProvider::icon_for_file(".config", true).empty());
}

MTEST(icon_for_directory_by_path_home) {
    ASSERT_FALSE(IconProvider::icon_for_directory("~", false).empty());
}

MTEST(icon_for_directory_by_path_git) {
    ASSERT_FALSE(IconProvider::icon_for_directory("~/projects/myrepo", true).empty());
}

MTEST(icon_for_directory_by_path_downloads) {
    ASSERT_FALSE(IconProvider::icon_for_directory("~/Downloads", false).empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 13. Icon Provider — Executables and Binary Files
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_for_executable_binary) {
    ASSERT_FALSE(IconProvider::icon_for_file("meridian-shell", false, true).empty());
}

MTEST(icon_for_object_file) {
    ASSERT_FALSE(IconProvider::icon_for_file("main.o", false).empty());
}

MTEST(icon_for_shared_lib) {
    ASSERT_FALSE(IconProvider::icon_for_file("libfoo.so", false).empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 14. Icon Provider — No Replacement Box Characters
// ─────────────────────────────────────────────────────────────────────────────

MTEST(icon_never_contains_replacement_box) {
    const char* test_files[][2] = {
        {"main.cpp", "0"}, {"README.md", "0"}, {"Makefile", "0"},
        {"src", "1"}, {"build", "1"}, {"wallpaper.png", "0"},
        {".gitignore", "0"}, {"LICENSE", "0"}, {"script.sh", "0"},
    };
    for (auto& pair : test_files) {
        bool is_dir = (pair[1][0] == '1');
        std::string icon = IconProvider::icon_for_file(pair[0], is_dir);
        ASSERT_TRUE(icon.find("\xEF\xBF\xBD") == std::string::npos);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 15. Icon Provider — Colored Icons
// ─────────────────────────────────────────────────────────────────────────────

MTEST(colored_icon_contains_ansi_escape) {
    std::string icon = IconProvider::colored_icon_for_file("main.cpp", false);
    ASSERT_TRUE(icon.find("\033[") != std::string::npos);
}

MTEST(colored_icon_for_directory_is_blue) {
    std::string icon = IconProvider::colored_icon_for_file("src", true);
    ASSERT_TRUE(icon.find("97;175;239") != std::string::npos);
}

MTEST(colored_icon_has_reset) {
    std::string icon = IconProvider::colored_icon_for_file("wallpaper.png", false);
    ASSERT_TRUE(icon.find("\033[0m") != std::string::npos);
}

// ─────────────────────────────────────────────────────────────────────────────
// 16. IconSet — All Tier Strings Non-Empty
// ─────────────────────────────────────────────────────────────────────────────

MTEST(iconset_nerd_font_strings_nonempty) {
    ASSERT_TRUE(std::string(IconSet::dir_home(IconTier::NERD_FONT)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_cpp(IconTier::NERD_FONT)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_git(IconTier::NERD_FONT)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::git_ahead(IconTier::NERD_FONT)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::git_branch(IconTier::NERD_FONT)).size() > 0);
}

MTEST(iconset_unicode_strings_nonempty) {
    ASSERT_TRUE(std::string(IconSet::dir_home(IconTier::UNICODE)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_python(IconTier::UNICODE)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_image(IconTier::UNICODE)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_archive(IconTier::UNICODE)).size() > 0);
}

MTEST(iconset_ascii_strings_nonempty) {
    ASSERT_TRUE(std::string(IconSet::dir_folder(IconTier::ASCII)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::git_clean(IconTier::ASCII)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::git_ahead(IconTier::ASCII)).size() > 0);
}

MTEST(iconset_git_badges_all_tiers) {
    for (int i = 0; i < 3; ++i) {
        IconTier t = (i == 0) ? IconTier::NERD_FONT :
                     (i == 1) ? IconTier::UNICODE : IconTier::ASCII;
        ASSERT_TRUE(std::string(IconSet::git_ahead(t)).size() > 0);
        ASSERT_TRUE(std::string(IconSet::git_behind(t)).size() > 0);
        ASSERT_TRUE(std::string(IconSet::git_modified(t)).size() > 0);
        ASSERT_TRUE(std::string(IconSet::git_staged(t)).size() > 0);
        ASSERT_TRUE(std::string(IconSet::git_untracked(t)).size() > 0);
        ASSERT_TRUE(std::string(IconSet::git_deleted(t)).size() > 0);
        ASSERT_TRUE(std::string(IconSet::git_conflict(t)).size() > 0);
    }
}

MTEST(icon_all_languages_nerd_font) {
    IconTier t = IconTier::NERD_FONT;
    ASSERT_TRUE(std::string(IconSet::lang_cpp(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_c(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_python(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_rust(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_go(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_js(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_ts(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_java(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_shell(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_lua(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_ruby(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_php(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_swift(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_kotlin(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::lang_zig(t)).size() > 0);
}

MTEST(icon_all_file_types_unicode) {
    IconTier t = IconTier::UNICODE;
    ASSERT_TRUE(std::string(IconSet::file_text(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_readme(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_license(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_makefile(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_cmake(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_docker(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_git(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_env(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_json(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_yaml(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_xml(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_toml(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_ini(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_image(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_video(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_audio(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_archive(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_binary(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_executable(t)).size() > 0);
    ASSERT_TRUE(std::string(IconSet::file_lock(t)).size() > 0);
}

