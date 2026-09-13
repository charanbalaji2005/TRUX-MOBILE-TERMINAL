// tests/test_icon_registry.cpp
#include "mini_test.hpp"
#include "../src/core/icons/icon_registry.hpp"
#include "../src/core/icons/icon_detector.hpp"
#include "../src/core/icons/animated_glyph.hpp"
#include "../src/core/icons/nerd_font_icons.hpp"
#include "../src/core/icons/fallback_icons.hpp"

using namespace meridian::icons;

MTEST(icon_registry_singleton) {
    auto& reg1 = IconRegistry::instance();
    auto& reg2 = IconRegistry::instance();
    ASSERT_EQ(&reg1, &reg2);
}

MTEST(icon_detector_tiers) {
    IconTier tier = IconDetector::detect_tier();
    ASSERT_TRUE(tier == IconTier::NERD_FONT || tier == IconTier::UNICODE || tier == IconTier::ASCII);
    ASSERT_FALSE(IconDetector::tier_name(tier).empty());
}

MTEST(icon_extension_detection_programming) {
    auto& reg = IconRegistry::instance();

    Icon cpp_ic = reg.getIconForExtension(".cpp");
    ASSERT_FALSE(cpp_ic.glyph.empty());
    ASSERT_FALSE(cpp_ic.color_escape.empty());

    Icon py_ic = reg.getIconForExtension("py");
    ASSERT_FALSE(py_ic.glyph.empty());

    Icon rs_ic = reg.getIconForExtension(".rs");
    ASSERT_FALSE(rs_ic.glyph.empty());

    Icon go_ic = reg.getIconForExtension(".go");
    ASSERT_FALSE(go_ic.glyph.empty());

    Icon ts_ic = reg.getIconForExtension(".ts");
    ASSERT_FALSE(ts_ic.glyph.empty());

    Icon java_ic = reg.getIconForExtension(".java");
    ASSERT_FALSE(java_ic.glyph.empty());

    Icon sh_ic = reg.getIconForExtension(".sh");
    ASSERT_FALSE(sh_ic.glyph.empty());
}

MTEST(icon_extension_detection_web_and_data) {
    auto& reg = IconRegistry::instance();

    Icon html_ic = reg.getIconForExtension(".html");
    ASSERT_FALSE(html_ic.glyph.empty());

    Icon css_ic = reg.getIconForExtension(".css");
    ASSERT_FALSE(css_ic.glyph.empty());

    Icon json_ic = reg.getIconForExtension(".json");
    ASSERT_FALSE(json_ic.glyph.empty());

    Icon toml_ic = reg.getIconForExtension(".toml");
    ASSERT_FALSE(toml_ic.glyph.empty());

    Icon sql_ic = reg.getIconForExtension(".sql");
    ASSERT_FALSE(sql_ic.glyph.empty());

    Icon zip_ic = reg.getIconForExtension(".zip");
    ASSERT_FALSE(zip_ic.glyph.empty());
}

MTEST(icon_filename_special_files) {
    auto& reg = IconRegistry::instance();

    FileInfo fi_readme;
    fi_readme.filename = "README.md";
    Icon ic_readme = reg.getIcon(fi_readme);
    ASSERT_FALSE(ic_readme.glyph.empty());

    FileInfo fi_docker;
    fi_docker.filename = "Dockerfile";
    Icon ic_docker = reg.getIcon(fi_docker);
    ASSERT_FALSE(ic_docker.glyph.empty());

    FileInfo fi_make;
    fi_make.filename = "Makefile";
    Icon ic_make = reg.getIcon(fi_make);
    ASSERT_FALSE(ic_make.glyph.empty());

    FileInfo fi_cargo;
    fi_cargo.filename = "Cargo.toml";
    Icon ic_cargo = reg.getIcon(fi_cargo);
    ASSERT_FALSE(ic_cargo.glyph.empty());

    FileInfo fi_npm;
    fi_npm.filename = "package.json";
    Icon ic_npm = reg.getIcon(fi_npm);
    ASSERT_FALSE(ic_npm.glyph.empty());

    FileInfo fi_git;
    fi_git.filename = ".gitignore";
    Icon ic_git = reg.getIcon(fi_git);
    ASSERT_FALSE(ic_git.glyph.empty());
}

MTEST(icon_directory_detection) {
    auto& reg = IconRegistry::instance();

    DirectoryInfo di_src;
    di_src.name = "src";
    Icon ic_src = reg.getIconForDirectory(di_src);
    ASSERT_FALSE(ic_src.glyph.empty());

    DirectoryInfo di_bld;
    di_bld.name = "build";
    Icon ic_bld = reg.getIconForDirectory(di_bld);
    ASSERT_FALSE(ic_bld.glyph.empty());

    DirectoryInfo di_docs;
    di_docs.name = "docs";
    Icon ic_docs = reg.getIconForDirectory(di_docs);
    ASSERT_FALSE(ic_docs.glyph.empty());

    DirectoryInfo di_home;
    di_home.name = "~";
    Icon ic_home = reg.getIconForDirectory(di_home);
    ASSERT_FALSE(ic_home.glyph.empty());

    DirectoryInfo di_git;
    di_git.name = "my_repo";
    di_git.is_git_repo = true;
    Icon ic_git = reg.getIconForDirectory(di_git);
    ASSERT_FALSE(ic_git.glyph.empty());
}

MTEST(icon_command_detection) {
    auto& reg = IconRegistry::instance();

    Icon cmd_git = reg.getIconForCommand("git status");
    ASSERT_FALSE(cmd_git.glyph.empty());

    Icon cmd_ssh = reg.getIconForCommand("ssh user@host");
    ASSERT_FALSE(cmd_ssh.glyph.empty());

    Icon cmd_docker = reg.getIconForCommand("docker ps");
    ASSERT_FALSE(cmd_docker.glyph.empty());

    Icon cmd_py = reg.getIconForCommand("python3 script.py");
    ASSERT_FALSE(cmd_py.glyph.empty());

    Icon cmd_cargo = reg.getIconForCommand("cargo build");
    ASSERT_FALSE(cmd_cargo.glyph.empty());

    Icon cmd_make = reg.getIconForCommand("make -j4");
    ASSERT_FALSE(cmd_make.glyph.empty());
}

MTEST(icon_git_status_badges) {
    auto& reg = IconRegistry::instance();

    Icon st_clean = reg.getIconForGitStatus(GitStatusType::Clean);
    ASSERT_FALSE(st_clean.glyph.empty());

    Icon st_mod = reg.getIconForGitStatus(GitStatusType::Modified);
    ASSERT_FALSE(st_mod.glyph.empty());

    Icon st_stage = reg.getIconForGitStatus(GitStatusType::Staged);
    ASSERT_FALSE(st_stage.glyph.empty());

    Icon st_untrk = reg.getIconForGitStatus(GitStatusType::Untracked);
    ASSERT_FALSE(st_untrk.glyph.empty());

    Icon st_ahead = reg.getIconForGitStatus(GitStatusType::Ahead);
    ASSERT_FALSE(st_ahead.glyph.empty());

    Icon st_behind = reg.getIconForGitStatus(GitStatusType::Behind);
    ASSERT_FALSE(st_behind.glyph.empty());
}

MTEST(icon_custom_registration) {
    auto& reg = IconRegistry::instance();

    reg.registerCustomExtension(".mycustomext", "★", "\033[38;2;255;255;0m");
    Icon custom_ic = reg.getIconForExtension(".mycustomext");
    ASSERT_EQ(custom_ic.glyph, "★");

    reg.registerCustomFilename("my_secret_file", "🔥", "");
    FileInfo fi;
    fi.filename = "my_secret_file";
    Icon custom_f = reg.getIcon(fi);
    ASSERT_EQ(custom_f.glyph, "🔥");
}

MTEST(animated_glyph_spinner) {
    Spinner spin_dots(SpinnerStyle::Dots, 10.0);
    ASSERT_EQ(spin_dots.frame_count(), 10u);
    ASSERT_FALSE(spin_dots.frame(0.0).empty());
    ASSERT_FALSE(spin_dots.frame(0.15).empty());

    Spinner spin_braille(SpinnerStyle::Braille, 8.0);
    ASSERT_EQ(spin_braille.frame_count(), 8u);
    ASSERT_NE(spin_braille.frame(0.0), spin_braille.frame(0.2));

    Spinner spin_circles(SpinnerStyle::Circles, 4.0);
    ASSERT_EQ(spin_circles.frame_count(), 5u);
}

MTEST(animated_glyph_progress_bar) {
    std::string bar0 = ProgressIndicator::render_bar(0.0, 10);
    ASSERT_FALSE(bar0.empty());

    std::string bar50 = ProgressIndicator::render_bar(0.5, 10);
    ASSERT_FALSE(bar50.empty());

    std::string bar100 = ProgressIndicator::render_bar(1.0, 10);
    ASSERT_FALSE(bar100.empty());
    ASSERT_NE(bar0, bar100);
}

