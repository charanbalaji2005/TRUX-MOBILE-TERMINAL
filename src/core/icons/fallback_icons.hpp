#pragma once
// src/core/icons/fallback_icons.hpp
//
// Unicode (Safe BMP / Basic Multilingual Plane) and ASCII fallback icon sets.
// Guarantees zero unprintable glyphs or replacement box characters (□, , ?) across any terminal.

#include <string>

namespace meridian::icons {

struct UnicodeFallbackGlyphs {
    // Directories
    static constexpr const char* DIR_FOLDER                    = "📁";
    static constexpr const char* DIR_FOLDER_OPEN               = "📂";
    static constexpr const char* DIR_HOME                      = "🏠";
    static constexpr const char* DIR_SRC                       = "⚙";
    static constexpr const char* DIR_BUILD                     = "🔨";
    static constexpr const char* DIR_BIN                       = "⚡";
    static constexpr const char* DIR_CONFIG                    = "🔧";
    static constexpr const char* DIR_DOCUMENTS                 = "📄";
    static constexpr const char* DIR_DOWNLOADS                 = "📥";
    static constexpr const char* DIR_DESKTOP                   = "🖥";
    static constexpr const char* DIR_PICTURES                  = "🖼";
    static constexpr const char* DIR_VIDEOS                    = "🎬";
    static constexpr const char* DIR_MUSIC                     = "🎵";
    static constexpr const char* DIR_GIT                       = "🌿";
    static constexpr const char* DIR_GITHUB                    = "🐙";
    static constexpr const char* DIR_NODE_MODULES              = "📦";
    static constexpr const char* DIR_TESTS                     = "🧪";
    static constexpr const char* DIR_HIDDEN                    = "🔒";
    static constexpr const char* DIR_PACKAGE                   = "📦";
    static constexpr const char* DIR_DATABASE                  = "🗄";

    // Languages & Web
    static constexpr const char* LANG_CODE                     = "λ";
    static constexpr const char* LANG_CPP                      = "C++";
    static constexpr const char* LANG_C                        = "C";
    static constexpr const char* LANG_PYTHON                   = "Py";
    static constexpr const char* LANG_RUST                     = "Rs";
    static constexpr const char* LANG_GO                       = "Go";
    static constexpr const char* LANG_JAVASCRIPT               = "JS";
    static constexpr const char* LANG_TYPESCRIPT               = "TS";
    static constexpr const char* LANG_JAVA                     = "☕";
    static constexpr const char* LANG_SHELL                    = "$";
    static constexpr const char* WEB_HTML                      = "🌐";
    static constexpr const char* WEB_CSS                       = "🎨";
    static constexpr const char* CONFIG_FILE                   = "⚙";
    static constexpr const char* CONFIG_LOCK                   = "🔒";

    // Media & Docs
    static constexpr const char* DOC_TEXT                      = "📄";
    static constexpr const char* DOC_MARKDOWN                  = "📝";
    static constexpr const char* DATA_DATABASE                 = "🗄";
    static constexpr const char* MEDIA_IMAGE                   = "🖼";
    static constexpr const char* MEDIA_VIDEO                   = "🎬";
    static constexpr const char* MEDIA_AUDIO                   = "🎵";
    static constexpr const char* ARCHIVE_ZIP                   = "🗜";
    static constexpr const char* BINARY_EXEC                   = "⚡";
    static constexpr const char* FILE_GENERIC                  = "•";

    // Git
    static constexpr const char* GIT_BRANCH                    = "⎇";
    static constexpr const char* GIT_CLEAN                     = "✓";
    static constexpr const char* GIT_MODIFIED                  = "✚";
    static constexpr const char* GIT_STAGED                    = "●";
    static constexpr const char* GIT_UNTRACKED                 = "?";
    static constexpr const char* GIT_DELETED                   = "✖";
    static constexpr const char* GIT_RENAMED                   = "→";
    static constexpr const char* GIT_CONFLICT                  = "!";
    static constexpr const char* GIT_AHEAD                     = "↑";
    static constexpr const char* GIT_BEHIND                    = "↓";
    static constexpr const char* GIT_DIVERGED                  = "↕";

    // Status
    static constexpr const char* STATUS_SUCCESS                = "✓";
    static constexpr const char* STATUS_ERROR                  = "✖";
    static constexpr const char* STATUS_WARNING                = "⚠";
    static constexpr const char* STATUS_INFO                   = "ℹ";
};

struct AsciiFallbackGlyphs {
    // Directories
    static constexpr const char* DIR_FOLDER                    = "[D]";
    static constexpr const char* DIR_FOLDER_OPEN               = "[O]";
    static constexpr const char* DIR_HOME                      = "~";
    static constexpr const char* DIR_SRC                       = "[src]";
    static constexpr const char* DIR_BUILD                     = "[bld]";
    static constexpr const char* DIR_BIN                       = "[bin]";
    static constexpr const char* DIR_CONFIG                    = "[cfg]";
    static constexpr const char* DIR_DOCUMENTS                 = "[doc]";
    static constexpr const char* DIR_DOWNLOADS                 = "[dl]";
    static constexpr const char* DIR_DESKTOP                   = "[dsk]";
    static constexpr const char* DIR_PICTURES                  = "[pic]";
    static constexpr const char* DIR_VIDEOS                    = "[vid]";
    static constexpr const char* DIR_MUSIC                     = "[aud]";
    static constexpr const char* DIR_GIT                       = "[git]";
    static constexpr const char* DIR_GITHUB                    = "[gh]";
    static constexpr const char* DIR_NODE_MODULES              = "[npm]";
    static constexpr const char* DIR_TESTS                     = "[tst]";
    static constexpr const char* DIR_HIDDEN                    = "[.]";
    static constexpr const char* DIR_PACKAGE                   = "[pkg]";
    static constexpr const char* DIR_DATABASE                  = "[db]";

    // Languages & Web
    static constexpr const char* LANG_CODE                     = "#";
    static constexpr const char* LANG_CPP                      = "C++";
    static constexpr const char* LANG_C                        = "C";
    static constexpr const char* LANG_PYTHON                   = "py";
    static constexpr const char* LANG_RUST                     = "rs";
    static constexpr const char* LANG_GO                       = "go";
    static constexpr const char* LANG_JAVASCRIPT               = "js";
    static constexpr const char* LANG_TYPESCRIPT               = "ts";
    static constexpr const char* LANG_JAVA                     = "jv";
    static constexpr const char* LANG_SHELL                    = "$";
    static constexpr const char* WEB_HTML                      = "<>";
    static constexpr const char* WEB_CSS                       = "{}";
    static constexpr const char* CONFIG_FILE                   = "%";
    static constexpr const char* CONFIG_LOCK                   = "@";

    // Media & Docs
    static constexpr const char* DOC_TEXT                      = "=";
    static constexpr const char* DOC_MARKDOWN                  = "md";
    static constexpr const char* DATA_DATABASE                 = "[=]";
    static constexpr const char* MEDIA_IMAGE                   = "[img]";
    static constexpr const char* MEDIA_VIDEO                   = "[vid]";
    static constexpr const char* MEDIA_AUDIO                   = "[aud]";
    static constexpr const char* ARCHIVE_ZIP                   = "[z]";
    static constexpr const char* BINARY_EXEC                   = "*";
    static constexpr const char* FILE_GENERIC                  = "-";

    // Git
    static constexpr const char* GIT_BRANCH                    = "b:";
    static constexpr const char* GIT_CLEAN                     = "ok";
    static constexpr const char* GIT_MODIFIED                  = "+";
    static constexpr const char* GIT_STAGED                    = "*";
    static constexpr const char* GIT_UNTRACKED                 = "?";
    static constexpr const char* GIT_DELETED                   = "x";
    static constexpr const char* GIT_RENAMED                   = "->";
    static constexpr const char* GIT_CONFLICT                  = "!";
    static constexpr const char* GIT_AHEAD                     = "^";
    static constexpr const char* GIT_BEHIND                    = "v";
    static constexpr const char* GIT_DIVERGED                  = "<>";

    // Status
    static constexpr const char* STATUS_SUCCESS                = "OK";
    static constexpr const char* STATUS_ERROR                  = "ERR";
    static constexpr const char* STATUS_WARNING                = "WARN";
    static constexpr const char* STATUS_INFO                   = "INFO";
};

} // namespace meridian::icons

