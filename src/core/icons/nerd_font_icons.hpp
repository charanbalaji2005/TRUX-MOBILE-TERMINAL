#pragma once
// src/core/icons/nerd_font_icons.hpp
//
// Comprehensive catalogue of Nerd Font (v3.x) glyph codepoints.
// Aggregated from Font Awesome, Devicons, Octicons, Codicons, Material Design Icons,
// Powerline, Weather Icons, and Linux / developer font logos.

#include <string>

namespace meridian::icons {

struct NerdFontGlyphs {
    // ── Powerline Symbols ─────────────────────────────────────────────────────
    static constexpr const char* POWERLINE_RIGHT_ARROW         = "\xEE\x82\xB0"; // U+E0B0 
    static constexpr const char* POWERLINE_RIGHT_ARROW_THIN    = "\xEE\x82\xB1"; // U+E0B1 
    static constexpr const char* POWERLINE_LEFT_ARROW          = "\xEE\x82\xB2"; // U+E0B2 
    static constexpr const char* POWERLINE_LEFT_ARROW_THIN     = "\xEE\x82\xB3"; // U+E0B3 
    static constexpr const char* POWERLINE_BRANCH              = "\xEE\x82\xA0"; // U+E0A0 
    static constexpr const char* POWERLINE_LINE_NUMBER         = "\xEE\x82\xA1"; // U+E0A1 
    static constexpr const char* POWERLINE_LOCK                = "\xEE\x82\xA2"; // U+E0A2 

    // ── Git & Version Control ─────────────────────────────────────────────────
    static constexpr const char* GIT_BRANCH                    = "\xF3\xB0\x98\xAC"; // U+F062C 󰘬
    static constexpr const char* GIT_BRANCH_ALT                = "\xEE\x82\xA0";     // U+E0A0 
    static constexpr const char* GIT_COMMIT                    = "\xF3\xB0\x98\xB3"; // U+F0633 󰘳
    static constexpr const char* GIT_MERGE                     = "\xF3\xB0\x99\x8E"; // U+F064E 󰙎
    static constexpr const char* GIT_PULL_REQUEST              = "\xEF\x84\x97";     // U+F117 
    static constexpr const char* GIT_REPO                      = "\xEF\x84\x93";     // U+F113 
    static constexpr const char* GIT_OCTOCAT                  = "\xEF\x81\x9B";     // U+F05B 
    static constexpr const char* GIT_DIFF                      = "\xF3\xB0\x99\x84"; // U+F0644 󰙄
    static constexpr const char* GIT_AHEAD                     = "\xE2\x86\x91";     // ↑
    static constexpr const char* GIT_BEHIND                    = "\xE2\x86\x93";     // ↓
    static constexpr const char* GIT_DIVERGED                  = "\xE2\x86\x95";     // ↕
    static constexpr const char* GIT_MODIFIED                  = "\xE2\x9C\x9A";     // ✚
    static constexpr const char* GIT_STAGED                    = "\xE2\x97\x8F";     // ●
    static constexpr const char* GIT_UNTRACKED                 = "?";
    static constexpr const char* GIT_DELETED                   = "\xE2\x9C\x96";     // ✖
    static constexpr const char* GIT_RENAMED                   = "\xE2\x86\x92";     // →
    static constexpr const char* GIT_CONFLICT                  = "!";
    static constexpr const char* GIT_CLEAN                     = "\xE2\x9C\x94";     // ✔
    static constexpr const char* GIT_STASH                     = "\xEF\x83\x8C";     // U+F0CC 

    // ── Directory Glyphs ──────────────────────────────────────────────────────
    static constexpr const char* DIR_FOLDER                    = "\xEF\x84\x95";     // U+F115 
    static constexpr const char* DIR_FOLDER_OPEN               = "\xEF\x84\x94";     // U+F114 
    static constexpr const char* DIR_HOME                      = "\xEF\x80\x95";     // U+F015 
    static constexpr const char* DIR_SRC                       = "\xF3\xB0\x99\xB2"; // U+F0672 󰙲
    static constexpr const char* DIR_BUILD                     = "\xEF\x8B\xAD";     // U+F2ED 
    static constexpr const char* DIR_BIN                       = "\xEF\x81\xAE";     // U+F06E 
    static constexpr const char* DIR_CONFIG                    = "\xEF\x80\x93";     // U+F013 
    static constexpr const char* DIR_DOCUMENTS                 = "\xEF\x81\x9B";     // U+F02D 
    static constexpr const char* DIR_DOWNLOADS                 = "\xEF\x80\x99";     // U+F019 
    static constexpr const char* DIR_DESKTOP                   = "\xEF\x84\x88";     // U+F108 
    static constexpr const char* DIR_PICTURES                  = "\xEF\x80\xBE";     // U+F03E 
    static constexpr const char* DIR_VIDEOS                    = "\xEF\x80\x88";     // U+F008 
    static constexpr const char* DIR_MUSIC                     = "\xEF\x80\x81";     // U+F001 
    static constexpr const char* DIR_GIT                       = "\xEE\x9C\x82";     // U+E702 
    static constexpr const char* DIR_GITHUB                    = "\xEF\x81\x9B";     // U+F05B 
    static constexpr const char* DIR_NODE_MODULES              = "\xEE\x9C\x98";     // U+E718 
    static constexpr const char* DIR_TESTS                     = "\xEF\x83\x80";     // U+F0C0 
    static constexpr const char* DIR_HIDDEN                    = "\xEF\x81\xB0";     // U+F070 
    static constexpr const char* DIR_PACKAGE                   = "\xEF\x86\xB3";     // U+F1B3 
    static constexpr const char* DIR_DATABASE                  = "\xEF\x87\x80";     // U+F1C0 
    static constexpr const char* DIR_PUBLIC                    = "\xEF\x82\xAC";     // U+F0AC 

    // ── Programming Languages ─────────────────────────────────────────────────
    static constexpr const char* LANG_C                        = "\xEE\x98\x9E";     // U+E61E 
    static constexpr const char* LANG_CPP                      = "\xEE\x98\x9D";     // U+E61D 
    static constexpr const char* LANG_HEADER                   = "\xEF\x83\x88";     // U+F0C8 
    static constexpr const char* LANG_PYTHON                   = "\xEE\x98\x86";     // U+E606 
    static constexpr const char* LANG_RUST                     = "\xEE\x9E\xA8";     // U+E7A8 
    static constexpr const char* LANG_GO                       = "\xEE\x99\x9D";     // U+E65D 
    static constexpr const char* LANG_JAVASCRIPT               = "\xEE\x98\x8C";     // U+E60C 
    static constexpr const char* LANG_TYPESCRIPT               = "\xEE\x98\xA8";     // U+E628 
    static constexpr const char* LANG_JAVA                     = "\xEE\x98\xB8";     // U+E638 
    static constexpr const char* LANG_KOTLIN                   = "\xEE\x99\x94";     // U+E654 
    static constexpr const char* LANG_SWIFT                    = "\xEE\x9C\x91";     // U+E711 
    static constexpr const char* LANG_RUBY                     = "\xEE\x98\x85";     // U+E605 
    static constexpr const char* LANG_PHP                      = "\xEE\x98\x9F";     // U+E61F 
    static constexpr const char* LANG_CSHARP                   = "\xF3\xB0\x98\xAE"; // U+F062E 󰘮
    static constexpr const char* LANG_DART                     = "\xEE\x99\x9A";     // U+E65A 
    static constexpr const char* LANG_LUA                      = "\xEE\x98\xA0";     // U+E620 
    static constexpr const char* LANG_SHELL                    = "\xEF\x84\xA0";     // U+F120 
    static constexpr const char* LANG_POWERSHELL               = "\xF3\xB0\x99\xAA"; // U+F066A 󰙪
    static constexpr const char* LANG_ZIG                      = "\xEE\x9A\xA7";     // U+E6A7 
    static constexpr const char* LANG_HASKELL                  = "\xEE\x99\x9F";     // U+E65F 
    static constexpr const char* LANG_ELIXIR                   = "\xEE\x98\xAD";     // U+E62D 
    static constexpr const char* LANG_SCALA                    = "\xEE\x98\xB7";     // U+E637 
    static constexpr const char* LANG_R                        = "\xEE\x98\x8B";     // U+E60B 
    static constexpr const char* LANG_PERL                     = "\xEE\x98\xBE";     // U+E63E 
    static constexpr const char* LANG_JULIA                    = "\xEE\x98\xA4";     // U+E624 
    static constexpr const char* LANG_NIM                      = "\xEE\x99\xA5";     // U+E665 
    static constexpr const char* LANG_OCAML                    = "\xEE\x99\x9A";     // U+E65A 
    static constexpr const char* LANG_CLOJURE                  = "\xEE\x98\xA1";     // U+E621 
    static constexpr const char* LANG_ERLANG                   = "\xEE\x99\x91";     // U+E651 
    static constexpr const char* LANG_ASSEMBLY                 = "\xEE\x98\xBB";     // U+E63B 

    // ── Web & Frontend ────────────────────────────────────────────────────────
    static constexpr const char* WEB_HTML                      = "\xEE\x9C\xB6";     // U+E736 
    static constexpr const char* WEB_CSS                       = "\xEE\x9C\x80";     // U+E700 
    static constexpr const char* WEB_SCSS                      = "\xEE\x98\x83";     // U+E603 
    static constexpr const char* WEB_SASS                      = "\xEE\x98\x83";     // U+E603 
    static constexpr const char* WEB_LESS                      = "\xEE\x98\x92";     // U+E612 
    static constexpr const char* WEB_VUE                       = "\xF3\xB0\x99\x82"; // U+F0642 󰙂
    static constexpr const char* WEB_REACT                     = "\xEE\x98\xBA";     // U+E63A 
    static constexpr const char* WEB_SVELTE                    = "\xEE\x99\x93";     // U+E653 
    static constexpr const char* WEB_ANGULAR                   = "\xEE\x99\x93";     // U+E653 
    static constexpr const char* WEB_WASM                      = "\xEE\x99\x84";     // U+E644 
    static constexpr const char* WEB_GRAPHQL                   = "\xEE\x99\x9F";     // U+E65F 
    static constexpr const char* WEB_TAILWIND                  = "\xF3\xB0\x99\xB3"; // U+F0673 󰙳

    // ── Configuration & Data Formats ──────────────────────────────────────────
    static constexpr const char* CONFIG_TOML                   = "\xEE\x98\x94";     // U+E614 
    static constexpr const char* CONFIG_YAML                   = "\xEE\x98\x95";     // U+E615 
    static constexpr const char* CONFIG_JSON                   = "\xEE\x98\x8B";     // U+E60B 
    static constexpr const char* CONFIG_XML                    = "\xEE\x98\x99";     // U+E619 
    static constexpr const char* CONFIG_INI                    = "\xEF\x80\x93";     // U+F013 
    static constexpr const char* CONFIG_ENV                    = "\xEF\x81\xA9";     // U+F069 
    static constexpr const char* CONFIG_LOCK                   = "\xEF\x80\xA3";     // U+F023 
    static constexpr const char* CONFIG_SETTINGS               = "\xEF\x80\x93";     // U+F013 

    // ── Documents & Notes ─────────────────────────────────────────────────────
    static constexpr const char* DOC_MARKDOWN                  = "\xEE\x98\x89";     // U+E609 
    static constexpr const char* DOC_TEXT                      = "\xEF\x85\x9C";     // U+F15C 
    static constexpr const char* DOC_PDF                       = "\xEF\x87\x81";     // U+F1C1 
    static constexpr const char* DOC_WORD                      = "\xEF\x87\x82";     // U+F1C2 
    static constexpr const char* DOC_EXCEL                     = "\xEF\x87\x83";     // U+F1C3 
    static constexpr const char* DOC_POWERPOINT                = "\xEF\x87\x84";     // U+F1C4 
    static constexpr const char* DOC_README                    = "\xEF\x84\x9C";     // U+F11C 
    static constexpr const char* DOC_LICENSE                   = "\xEF\x81\xB1";     // U+F071 
    static constexpr const char* DOC_CHANGELOG                 = "\xEF\x83\x8E";     // U+F0CE 

    // ── Data & Database ───────────────────────────────────────────────────────
    static constexpr const char* DATA_DATABASE                 = "\xEF\x87\x80";     // U+F1C0 
    static constexpr const char* DATA_SQL                      = "\xEF\x87\x80";     // U+F1C0 
    static constexpr const char* DATA_TABLE                    = "\xEF\x83\x8E";     // U+F0CE 
    static constexpr const char* DATA_GRAPH                    = "\xEF\x88\x81";     // U+F201 

    // ── Media: Images, Videos, Audio ──────────────────────────────────────────
    static constexpr const char* MEDIA_IMAGE                   = "\xEF\x87\x85";     // U+F1C5 
    static constexpr const char* MEDIA_VIDEO                   = "\xEF\x87\x88";     // U+F1C8 
    static constexpr const char* MEDIA_AUDIO                   = "\xEF\x87\x87";     // U+F1C7 
    static constexpr const char* MEDIA_VECTOR                  = "\xEF\x87\x85";     // U+F1C5 
    static constexpr const char* MEDIA_FONT                    = "\xEF\x80\xB1";     // U+F031 

    // ── Archives & Binary ─────────────────────────────────────────────────────
    static constexpr const char* ARCHIVE_ZIP                   = "\xEF\x87\x86";     // U+F1C6 
    static constexpr const char* ARCHIVE_TAR                   = "\xEF\x87\x86";     // U+F1C6 
    static constexpr const char* BINARY_EXEC                   = "\xEF\x81\xAE";     // U+F06E 
    static constexpr const char* BINARY_OBJECT                 = "\xEF\x83\x8B";     // U+F0CB 
    static constexpr const char* BINARY_LIBRARY                = "\xEF\x83\x8B";     // U+F0CB 
    static constexpr const char* FILE_GENERIC                  = "\xEF\x85\x9B";     // U+F15B 

    // ── DevOps, Containers, Cloud ─────────────────────────────────────────────
    static constexpr const char* DEVOPS_DOCKER                 = "\xF3\xB0\xA1\xA8"; // U+F0868 󰡨
    static constexpr const char* DEVOPS_DOCKER_COMPOSE         = "\xF3\xB0\xA1\xA8"; // U+F0868 󰡨
    static constexpr const char* DEVOPS_PODMAN                 = "\xF3\xB0\xA1\xA8"; // U+F0868 󰡨
    static constexpr const char* DEVOPS_KUBERNETES             = "\xF3\xB0\xA9\xB3"; // U+F0A73 󰩳
    static constexpr const char* DEVOPS_TERRAFORM              = "\xF3\xB0\xA9\xB2"; // U+F0A72 󰩲
    static constexpr const char* DEVOPS_MAKEFILE               = "\xEF\x85\x8E";     // U+F14E 
    static constexpr const char* DEVOPS_CMAKE                  = "\xEF\x85\x8E";     // U+F14E 
    static constexpr const char* DEVOPS_VAGRANT                = "\xF3\xB0\x98\xB8"; // U+F0638 󰘸
    static constexpr const char* DEVOPS_ANSIBLE                = "\xF3\xB0\x98\xBE"; // U+F063E 󰘾
    static constexpr const char* DEVOPS_NIX                    = "\xEF\x8C\xAE";     // U+F32E 
    static constexpr const char* DEVOPS_JENKINS                = "\xEF\x8D\xAE";     // U+F36E 

    // ── Applications & Commands ───────────────────────────────────────────────
    static constexpr const char* CMD_TERMINAL                  = "\xEF\x84\xA0";     // U+F120 
    static constexpr const char* CMD_SSH                       = "\xF3\xB0\x9A\xA3"; // U+F06A3 󰚣
    static constexpr const char* CMD_PYTHON                    = "\xEE\x98\x86";     // U+E606 
    static constexpr const char* CMD_NODE                      = "\xEE\x9C\x98";     // U+E718 
    static constexpr const char* CMD_NPM                       = "\xEE\x9C\x9E";     // U+E71E 
    static constexpr const char* CMD_YARN                      = "\xEE\x98\xAF";     // U+E62F 
    static constexpr const char* CMD_CARGO                     = "\xEE\x9E\xA8";     // U+E7A8 
    static constexpr const char* CMD_VIM                       = "\xEE\x98\xAB";     // U+E62B 
    static constexpr const char* CMD_NEOVIM                    = "\xEF\x8D\xAB";     // U+F36B 
    static constexpr const char* CMD_EMACS                     = "\xEE\x99\xAA";     // U+E66A 
    static constexpr const char* CMD_TMUX                      = "\xEF\x84\xA0";     // U+F120 
    static constexpr const char* CMD_BUILD                     = "\xEF\x8B\xAD";     // U+F2ED 
    static constexpr const char* CMD_COMPILER                  = "\xEE\x98\x9D";     // U+E61D 
    static constexpr const char* CMD_JAVA                      = "\xEE\x98\xB8";     // U+E638 
    static constexpr const char* CMD_GRADLE                    = "\xEE\x99\x8E";     // U+E64E 
    static constexpr const char* CMD_MAVEN                     = "\xEE\x98\x95";     // U+E615 
    static constexpr const char* CMD_NETWORK                   = "\xF3\xB0\x9E\x8B"; // U+F078B 󰞋
    static constexpr const char* CMD_DOWNLOAD                  = "\xEF\x80\x99";     // U+F019 
    static constexpr const char* CMD_SERVICE                   = "\xEF\x80\x93";     // U+F013 
    static constexpr const char* CMD_SECURITY                  = "\xEF\x84\xBA";     // U+F132 
    static constexpr const char* CMD_SEARCH                    = "\xEF\x80\x82";     // U+F002 
    static constexpr const char* CMD_MONITOR                   = "\xEF\x82\x80";     // U+F080 
    static constexpr const char* CMD_AI                        = "\xF3\xB0\xA0\x9F"; // U+F081F 󰠟
    static constexpr const char* CMD_CLOCK                     = "\xEE\x8E\x81";     // U+E381 
    static constexpr const char* CMD_USER                      = "\xEF\x8A\xBD";     // U+F2BD 

    // ── Status & Notification Badges ──────────────────────────────────────────
    static constexpr const char* STATUS_SUCCESS                = "\xEF\x80\x8C";     // U+F00C 
    static constexpr const char* STATUS_ERROR                  = "\xEF\x80\x8D";     // U+F00D 
    static constexpr const char* STATUS_WARNING                = "\xEF\x81\xB1";     // U+F071 
    static constexpr const char* STATUS_INFO                   = "\xEF\x81\x9A";     // U+F05A 
    static constexpr const char* STATUS_QUESTION               = "\xEF\x81\x99";     // U+F059 
    static constexpr const char* STATUS_SPARKLE                = "\xE2\x9C\xA8";     // ✨
    static constexpr const char* STATUS_LIGHTNING              = "\xE2\x9A\xA1";     // ⚡
    static constexpr const char* STATUS_FIRE                   = "\xF3\xB0\x9B\x9C"; // U+F06DC 󰛜
};

} // namespace meridian::icons

