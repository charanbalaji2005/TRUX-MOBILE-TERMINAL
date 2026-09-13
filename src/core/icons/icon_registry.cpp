// src/core/icons/icon_registry.cpp
#include "icon_registry.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>

namespace meridian::icons {

namespace {

// Standard ANSI 24-bit TrueColor escapes for icons
namespace Colors {
    const std::string C_BLUE       = "\033[38;2;81;154;186m";  // C/C++
    const std::string PYTHON_YEL   = "\033[38;2;255;212;59m";  // Python yellow
    const std::string RUST_ORG     = "\033[38;2;222;85;37m";   // Rust orange
    const std::string GO_CYAN      = "\033[38;2;0;173;216m";   // Go cyan
    const std::string JS_YELLOW    = "\033[38;2;247;223;30m";  // JS yellow
    const std::string TS_BLUE      = "\033[38;2;49;120;198m";  // TS blue
    const std::string JAVA_RED     = "\033[38;2;234;45;46m";   // Java red
    const std::string RUBY_RED     = "\033[38;2;204;52;45m";   // Ruby red
    const std::string PHP_PURPLE   = "\033[38;2;119;123;179m"; // PHP purple
    const std::string SHELL_GREEN  = "\033[38;2;78;186;111m";  // Shell green
    const std::string LUA_BLUE     = "\033[38;2;0;0;128m";     // Lua blue
    const std::string DART_BLUE    = "\033[38;2;0;182;237m";   // Dart blue
    const std::string SWIFT_ORG    = "\033[38;2;240;81;56m";   // Swift orange
    const std::string KOTLIN_VIO   = "\033[38;2;169;123;255m"; // Kotlin violet
    const std::string ZIG_ORG      = "\033[38;2;247;164;29m";  // Zig orange

    const std::string HTML_ORG     = "\033[38;2;227;76;38m";   // HTML orange
    const std::string CSS_BLUE     = "\033[38;2;38;77;228m";   // CSS blue
    const std::string SCSS_PINK    = "\033[38;2;205;103;153m"; // Sass pink
    const std::string JSON_YEL     = "\033[38;2;250;200;80m";  // JSON yellow
    const std::string XML_BLUE     = "\033[38;2;90;180;240m";  // XML blue
    const std::string YAML_RED     = "\033[38;2;203;56;55m";   // YAML red
    const std::string TOML_ORG     = "\033[38;2;156;66;33m";   // TOML orange
    const std::string MD_BLUE      = "\033[38;2;66;165;245m";  // Markdown blue

    const std::string DIR_BLUE     = "\033[38;2;97;175;239m";  // Directory blue
    const std::string DIR_SRC      = "\033[38;2;98;218;171m";  // Source green
    const std::string DIR_BLD      = "\033[38;2;229;192;123m"; // Build yellow
    const std::string DIR_CFG      = "\033[38;2;198;120;221m"; // Config purple
    const std::string DIR_GIT      = "\033[38;2;240;80;50m";   // Git orange
    const std::string DIR_DOC      = "\033[38;2;209;154;102m"; // Docs tan
    const std::string DIR_PKG      = "\033[38;2;224;108;117m"; // Package red

    const std::string GIT_CLEAN    = "\033[38;2;72;240;152m";  // Clean green
    const std::string GIT_DIRTY    = "\033[38;2;255;185;60m";  // Dirty amber
    const std::string GIT_STAGED   = "\033[38;2;97;175;239m";  // Staged blue
    const std::string GIT_UNTRK    = "\033[38;2;224;108;117m"; // Untracked red
    const std::string GIT_SYNC     = "\033[38;2;180;140;255m"; // Sync purple

    const std::string IMG_PURPLE   = "\033[38;2;160;100;220m"; // Image purple
    const std::string VID_RED      = "\033[38;2;230;70;70m";   // Video red
    const std::string AUD_CYAN     = "\033[38;2;40;200;200m";  // Audio cyan
    const std::string ARC_GOLD     = "\033[38;2;230;180;40m";  // Archive gold
    const std::string EXE_GREEN    = "\033[38;2;100;220;120m"; // Exec green
    const std::string DOCKER_CYAN  = "\033[38;2;36;150;237m";  // Docker cyan
}

std::string extract_extension(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    std::string filename = (slash != std::string::npos) ? path.substr(slash + 1) : path;
    size_t dot = filename.find_last_of('.');
    if (dot == std::string::npos || dot == 0) return "";
    std::string ext = filename.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string extract_filename(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    return (slash != std::string::npos) ? path.substr(slash + 1) : path;
}

} // namespace

IconRegistry& IconRegistry::instance() {
    static IconRegistry inst;
    return inst;
}

IconRegistry::IconRegistry() {
    init_default_mappings();
    reload_custom_mappings();
}

void IconRegistry::init_default_mappings() {
    // ══════════════════════════════════════════════════════════════════════════
    // 1. EXTENSION MAPPINGS
    // ══════════════════════════════════════════════════════════════════════════

    // C / C++
    extension_map_[".cpp"]  = {NerdFontGlyphs::LANG_CPP,        Colors::C_BLUE};
    extension_map_[".cc"]   = {NerdFontGlyphs::LANG_CPP,        Colors::C_BLUE};
    extension_map_[".cxx"]  = {NerdFontGlyphs::LANG_CPP,        Colors::C_BLUE};
    extension_map_[".hpp"]  = {NerdFontGlyphs::LANG_CPP,        Colors::C_BLUE};
    extension_map_[".hxx"]  = {NerdFontGlyphs::LANG_CPP,        Colors::C_BLUE};
    extension_map_[".hh"]   = {NerdFontGlyphs::LANG_CPP,        Colors::C_BLUE};
    extension_map_[".c"]    = {NerdFontGlyphs::LANG_C,          Colors::C_BLUE};
    extension_map_[".h"]    = {NerdFontGlyphs::LANG_HEADER,     Colors::C_BLUE};
    extension_map_[".inl"]  = {NerdFontGlyphs::LANG_HEADER,     Colors::C_BLUE};

    // Python
    extension_map_[".py"]   = {NerdFontGlyphs::LANG_PYTHON,     Colors::PYTHON_YEL};
    extension_map_[".pyw"]  = {NerdFontGlyphs::LANG_PYTHON,     Colors::PYTHON_YEL};
    extension_map_[".ipynb"]= {NerdFontGlyphs::LANG_PYTHON,     Colors::PYTHON_YEL};
    extension_map_[".pyi"]  = {NerdFontGlyphs::LANG_PYTHON,     Colors::PYTHON_YEL};

    // Rust
    extension_map_[".rs"]   = {NerdFontGlyphs::LANG_RUST,       Colors::RUST_ORG};

    // Go
    extension_map_[".go"]   = {NerdFontGlyphs::LANG_GO,         Colors::GO_CYAN};

    // JavaScript / TypeScript / Web Frameworks
    extension_map_[".js"]   = {NerdFontGlyphs::LANG_JAVASCRIPT, Colors::JS_YELLOW};
    extension_map_[".mjs"]  = {NerdFontGlyphs::LANG_JAVASCRIPT, Colors::JS_YELLOW};
    extension_map_[".cjs"]  = {NerdFontGlyphs::LANG_JAVASCRIPT, Colors::JS_YELLOW};
    extension_map_[".jsx"]  = {NerdFontGlyphs::WEB_REACT,       Colors::GO_CYAN};
    extension_map_[".ts"]   = {NerdFontGlyphs::LANG_TYPESCRIPT, Colors::TS_BLUE};
    extension_map_[".mts"]  = {NerdFontGlyphs::LANG_TYPESCRIPT, Colors::TS_BLUE};
    extension_map_[".cts"]  = {NerdFontGlyphs::LANG_TYPESCRIPT, Colors::TS_BLUE};
    extension_map_[".tsx"]  = {NerdFontGlyphs::WEB_REACT,       Colors::TS_BLUE};
    extension_map_[".vue"]  = {NerdFontGlyphs::WEB_VUE,         Colors::DIR_SRC};
    extension_map_[".svelte"]={NerdFontGlyphs::WEB_SVELTE,      Colors::RUST_ORG};
    extension_map_[".wasm"] = {NerdFontGlyphs::WEB_WASM,        Colors::TS_BLUE};
    extension_map_[".graphql"]={NerdFontGlyphs::WEB_GRAPHQL,    Colors::SCSS_PINK};
    extension_map_[".gql"]  = {NerdFontGlyphs::WEB_GRAPHQL,     Colors::SCSS_PINK};

    // JVM & Mobile
    extension_map_[".java"] = {NerdFontGlyphs::LANG_JAVA,       Colors::JAVA_RED};
    extension_map_[".class"]= {NerdFontGlyphs::LANG_JAVA,       Colors::JAVA_RED};
    extension_map_[".jar"]  = {NerdFontGlyphs::LANG_JAVA,       Colors::JAVA_RED};
    extension_map_[".kt"]   = {NerdFontGlyphs::LANG_KOTLIN,     Colors::KOTLIN_VIO};
    extension_map_[".kts"]  = {NerdFontGlyphs::LANG_KOTLIN,     Colors::KOTLIN_VIO};
    extension_map_[".scala"]= {NerdFontGlyphs::LANG_SCALA,      Colors::JAVA_RED};
    extension_map_[".swift"]= {NerdFontGlyphs::LANG_SWIFT,      Colors::SWIFT_ORG};
    extension_map_[".dart"] = {NerdFontGlyphs::LANG_DART,       Colors::DART_BLUE};
    extension_map_[".m"]    = {NerdFontGlyphs::LANG_C,          Colors::C_BLUE};
    extension_map_[".mm"]   = {NerdFontGlyphs::LANG_CPP,        Colors::C_BLUE};
    extension_map_[".cs"]   = {NerdFontGlyphs::LANG_CSHARP,     Colors::DIR_CFG};

    // Other System & Scripting Languages
    extension_map_[".rb"]   = {NerdFontGlyphs::LANG_RUBY,       Colors::RUBY_RED};
    extension_map_[".php"]  = {NerdFontGlyphs::LANG_PHP,        Colors::PHP_PURPLE};
    extension_map_[".lua"]  = {NerdFontGlyphs::LANG_LUA,        Colors::LUA_BLUE};
    extension_map_[".zig"]  = {NerdFontGlyphs::LANG_ZIG,        Colors::ZIG_ORG};
    extension_map_[".sh"]   = {NerdFontGlyphs::LANG_SHELL,      Colors::SHELL_GREEN};
    extension_map_[".bash"] = {NerdFontGlyphs::LANG_SHELL,      Colors::SHELL_GREEN};
    extension_map_[".zsh"]  = {NerdFontGlyphs::LANG_SHELL,      Colors::SHELL_GREEN};
    extension_map_[".fish"] = {NerdFontGlyphs::LANG_SHELL,      Colors::SHELL_GREEN};
    extension_map_[".ps1"]  = {NerdFontGlyphs::LANG_POWERSHELL, Colors::TS_BLUE};
    extension_map_[".hs"]   = {NerdFontGlyphs::LANG_HASKELL,    Colors::DIR_CFG};
    extension_map_[".ex"]   = {NerdFontGlyphs::LANG_ELIXIR,     Colors::DIR_CFG};
    extension_map_[".exs"]  = {NerdFontGlyphs::LANG_ELIXIR,     Colors::DIR_CFG};
    extension_map_[".erl"]  = {NerdFontGlyphs::LANG_ERLANG,     Colors::RUBY_RED};
    extension_map_[".r"]    = {NerdFontGlyphs::LANG_R,          Colors::TS_BLUE};
    extension_map_[".pl"]   = {NerdFontGlyphs::LANG_PERL,       Colors::TS_BLUE};
    extension_map_[".jl"]   = {NerdFontGlyphs::LANG_JULIA,      Colors::DIR_CFG};
    extension_map_[".nim"]  = {NerdFontGlyphs::LANG_NIM,        Colors::PYTHON_YEL};
    extension_map_[".clj"]  = {NerdFontGlyphs::LANG_CLOJURE,    Colors::DIR_SRC};
    extension_map_[".asm"]  = {NerdFontGlyphs::LANG_ASSEMBLY,   Colors::C_BLUE};
    extension_map_[".s"]    = {NerdFontGlyphs::LANG_ASSEMBLY,   Colors::C_BLUE};

    // Web Formats & Styles
    extension_map_[".html"] = {NerdFontGlyphs::WEB_HTML,        Colors::HTML_ORG};
    extension_map_[".htm"]  = {NerdFontGlyphs::WEB_HTML,        Colors::HTML_ORG};
    extension_map_[".css"]  = {NerdFontGlyphs::WEB_CSS,         Colors::CSS_BLUE};
    extension_map_[".scss"] = {NerdFontGlyphs::WEB_SCSS,        Colors::SCSS_PINK};
    extension_map_[".sass"] = {NerdFontGlyphs::WEB_SASS,        Colors::SCSS_PINK};
    extension_map_[".less"] = {NerdFontGlyphs::WEB_LESS,        Colors::CSS_BLUE};

    // Config & Structured Data
    extension_map_[".json"] = {NerdFontGlyphs::CONFIG_JSON,     Colors::JSON_YEL};
    extension_map_[".jsonc"]= {NerdFontGlyphs::CONFIG_JSON,     Colors::JSON_YEL};
    extension_map_[".json5"]= {NerdFontGlyphs::CONFIG_JSON,     Colors::JSON_YEL};
    extension_map_[".xml"]  = {NerdFontGlyphs::CONFIG_XML,      Colors::XML_BLUE};
    extension_map_[".svg"]  = {NerdFontGlyphs::MEDIA_VECTOR,    Colors::HTML_ORG};
    extension_map_[".toml"] = {NerdFontGlyphs::CONFIG_TOML,     Colors::TOML_ORG};
    extension_map_[".yaml"] = {NerdFontGlyphs::CONFIG_YAML,     Colors::YAML_RED};
    extension_map_[".yml"]  = {NerdFontGlyphs::CONFIG_YAML,     Colors::YAML_RED};
    extension_map_[".ini"]  = {NerdFontGlyphs::CONFIG_INI,      Colors::DIR_BLD};
    extension_map_[".conf"] = {NerdFontGlyphs::CONFIG_INI,      Colors::DIR_BLD};
    extension_map_[".cfg"]  = {NerdFontGlyphs::CONFIG_INI,      Colors::DIR_BLD};
    extension_map_[".env"]  = {NerdFontGlyphs::CONFIG_ENV,      Colors::DIR_BLD};
    extension_map_[".lock"] = {NerdFontGlyphs::CONFIG_LOCK,     Colors::DIR_BLD};

    // Documents
    extension_map_[".md"]   = {NerdFontGlyphs::DOC_MARKDOWN,    Colors::MD_BLUE};
    extension_map_[".markdown"]={NerdFontGlyphs::DOC_MARKDOWN,  Colors::MD_BLUE};
    extension_map_[".txt"]  = {NerdFontGlyphs::DOC_TEXT,        Colors::DIR_DOC};
    extension_map_[".pdf"]  = {NerdFontGlyphs::DOC_PDF,         Colors::JAVA_RED};
    extension_map_[".doc"]  = {NerdFontGlyphs::DOC_WORD,        Colors::TS_BLUE};
    extension_map_[".docx"] = {NerdFontGlyphs::DOC_WORD,        Colors::TS_BLUE};
    extension_map_[".rst"]  = {NerdFontGlyphs::DOC_TEXT,        Colors::DIR_DOC};
    extension_map_[".tex"]  = {NerdFontGlyphs::DOC_TEXT,        Colors::DIR_DOC};
    extension_map_[".org"]  = {NerdFontGlyphs::DOC_TEXT,        Colors::DIR_DOC};

    // Data Formats
    extension_map_[".csv"]    = {NerdFontGlyphs::DATA_TABLE,    Colors::DIR_SRC};
    extension_map_[".tsv"]    = {NerdFontGlyphs::DATA_TABLE,    Colors::DIR_SRC};
    extension_map_[".sql"]    = {NerdFontGlyphs::DATA_SQL,      Colors::DIR_CFG};
    extension_map_[".db"]     = {NerdFontGlyphs::DATA_DATABASE, Colors::DIR_CFG};
    extension_map_[".sqlite"] = {NerdFontGlyphs::DATA_DATABASE, Colors::DIR_CFG};
    extension_map_[".sqlite3"]= {NerdFontGlyphs::DATA_DATABASE, Colors::DIR_CFG};
    extension_map_[".parquet"]= {NerdFontGlyphs::DATA_DATABASE, Colors::DIR_CFG};

    // Media: Images
    extension_map_[".png"]  = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};
    extension_map_[".jpg"]  = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};
    extension_map_[".jpeg"] = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};
    extension_map_[".webp"] = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};
    extension_map_[".gif"]  = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};
    extension_map_[".bmp"]  = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};
    extension_map_[".tiff"] = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};
    extension_map_[".ico"]  = {NerdFontGlyphs::MEDIA_IMAGE,     Colors::IMG_PURPLE};

    // Media: Video
    extension_map_[".mp4"]  = {NerdFontGlyphs::MEDIA_VIDEO,     Colors::VID_RED};
    extension_map_[".mkv"]  = {NerdFontGlyphs::MEDIA_VIDEO,     Colors::VID_RED};
    extension_map_[".webm"] = {NerdFontGlyphs::MEDIA_VIDEO,     Colors::VID_RED};
    extension_map_[".mov"]  = {NerdFontGlyphs::MEDIA_VIDEO,     Colors::VID_RED};
    extension_map_[".avi"]  = {NerdFontGlyphs::MEDIA_VIDEO,     Colors::VID_RED};

    // Media: Audio
    extension_map_[".mp3"]  = {NerdFontGlyphs::MEDIA_AUDIO,     Colors::AUD_CYAN};
    extension_map_[".wav"]  = {NerdFontGlyphs::MEDIA_AUDIO,     Colors::AUD_CYAN};
    extension_map_[".flac"] = {NerdFontGlyphs::MEDIA_AUDIO,     Colors::AUD_CYAN};
    extension_map_[".ogg"]  = {NerdFontGlyphs::MEDIA_AUDIO,     Colors::AUD_CYAN};
    extension_map_[".m4a"]  = {NerdFontGlyphs::MEDIA_AUDIO,     Colors::AUD_CYAN};

    // Archives & Binaries
    extension_map_[".zip"]  = {NerdFontGlyphs::ARCHIVE_ZIP,     Colors::ARC_GOLD};
    extension_map_[".tar"]  = {NerdFontGlyphs::ARCHIVE_TAR,     Colors::ARC_GOLD};
    extension_map_[".gz"]   = {NerdFontGlyphs::ARCHIVE_TAR,     Colors::ARC_GOLD};
    extension_map_[".bz2"]  = {NerdFontGlyphs::ARCHIVE_TAR,     Colors::ARC_GOLD};
    extension_map_[".xz"]   = {NerdFontGlyphs::ARCHIVE_TAR,     Colors::ARC_GOLD};
    extension_map_[".7z"]   = {NerdFontGlyphs::ARCHIVE_ZIP,     Colors::ARC_GOLD};
    extension_map_[".rar"]  = {NerdFontGlyphs::ARCHIVE_ZIP,     Colors::ARC_GOLD};
    extension_map_[".tgz"]  = {NerdFontGlyphs::ARCHIVE_TAR,     Colors::ARC_GOLD};
    extension_map_[".zst"]  = {NerdFontGlyphs::ARCHIVE_TAR,     Colors::ARC_GOLD};
    extension_map_[".o"]    = {NerdFontGlyphs::BINARY_OBJECT,   Colors::DIR_DOC};
    extension_map_[".so"]   = {NerdFontGlyphs::BINARY_LIBRARY,  Colors::DIR_DOC};
    extension_map_[".dylib"]= {NerdFontGlyphs::BINARY_LIBRARY,  Colors::DIR_DOC};
    extension_map_[".dll"]  = {NerdFontGlyphs::BINARY_LIBRARY,  Colors::DIR_DOC};
    extension_map_[".a"]    = {NerdFontGlyphs::BINARY_LIBRARY,  Colors::DIR_DOC};

    // ══════════════════════════════════════════════════════════════════════════
    // 2. SPECIAL EXACT FILENAMES
    // ══════════════════════════════════════════════════════════════════════════
    filename_map_["README"]             = {NerdFontGlyphs::DOC_README,            Colors::MD_BLUE};
    filename_map_["README.md"]          = {NerdFontGlyphs::DOC_README,            Colors::MD_BLUE};
    filename_map_["LICENSE"]            = {NerdFontGlyphs::DOC_LICENSE,           Colors::DIR_BLD};
    filename_map_["LICENSE.md"]         = {NerdFontGlyphs::DOC_LICENSE,           Colors::DIR_BLD};
    filename_map_["CHANGELOG.md"]       = {NerdFontGlyphs::DOC_CHANGELOG,         Colors::DIR_SRC};
    filename_map_["CONTRIBUTING.md"]    = {NerdFontGlyphs::DOC_CHANGELOG,         Colors::DIR_SRC};
    filename_map_[".gitignore"]         = {NerdFontGlyphs::GIT_OCTOCAT,           Colors::DIR_GIT};
    filename_map_[".gitattributes"]     = {NerdFontGlyphs::GIT_OCTOCAT,           Colors::DIR_GIT};
    filename_map_[".gitmodules"]        = {NerdFontGlyphs::GIT_OCTOCAT,           Colors::DIR_GIT};
    filename_map_[".env"]               = {NerdFontGlyphs::CONFIG_ENV,            Colors::DIR_BLD};
    filename_map_[".env.example"]       = {NerdFontGlyphs::CONFIG_ENV,            Colors::DIR_BLD};
    filename_map_[".editorconfig"]      = {NerdFontGlyphs::CONFIG_SETTINGS,       Colors::DIR_DOC};
    filename_map_["Makefile"]           = {NerdFontGlyphs::DEVOPS_MAKEFILE,       Colors::DIR_BLD};
    filename_map_["CMakeLists.txt"]     = {NerdFontGlyphs::DEVOPS_CMAKE,          Colors::C_BLUE};
    filename_map_["Dockerfile"]         = {NerdFontGlyphs::DEVOPS_DOCKER,         Colors::DOCKER_CYAN};
    filename_map_["docker-compose.yml"] = {NerdFontGlyphs::DEVOPS_DOCKER_COMPOSE, Colors::DOCKER_CYAN};
    filename_map_["docker-compose.yaml"]= {NerdFontGlyphs::DEVOPS_DOCKER_COMPOSE, Colors::DOCKER_CYAN};
    filename_map_["Containerfile"]      = {NerdFontGlyphs::DEVOPS_PODMAN,         Colors::DIR_GIT};
    filename_map_["Vagrantfile"]        = {NerdFontGlyphs::DEVOPS_VAGRANT,        Colors::TS_BLUE};
    filename_map_["package.json"]       = {NerdFontGlyphs::CMD_NPM,               Colors::DIR_PKG};
    filename_map_["package-lock.json"]  = {NerdFontGlyphs::CONFIG_LOCK,           Colors::DIR_PKG};
    filename_map_["yarn.lock"]          = {NerdFontGlyphs::CMD_YARN,              Colors::TS_BLUE};
    filename_map_["pnpm-lock.yaml"]     = {NerdFontGlyphs::CONFIG_LOCK,           Colors::DIR_BLD};
    filename_map_["Cargo.toml"]         = {NerdFontGlyphs::LANG_RUST,             Colors::RUST_ORG};
    filename_map_["Cargo.lock"]         = {NerdFontGlyphs::CONFIG_LOCK,           Colors::RUST_ORG};
    filename_map_["requirements.txt"]   = {NerdFontGlyphs::LANG_PYTHON,           Colors::PYTHON_YEL};
    filename_map_["pyproject.toml"]     = {NerdFontGlyphs::LANG_PYTHON,           Colors::PYTHON_YEL};
    filename_map_["go.mod"]             = {NerdFontGlyphs::LANG_GO,               Colors::GO_CYAN};
    filename_map_["go.sum"]             = {NerdFontGlyphs::CONFIG_LOCK,           Colors::GO_CYAN};
    filename_map_["pom.xml"]            = {NerdFontGlyphs::CMD_MAVEN,             Colors::JAVA_RED};
    filename_map_["build.gradle"]       = {NerdFontGlyphs::CMD_GRADLE,            Colors::TS_BLUE};
    filename_map_["flake.nix"]          = {NerdFontGlyphs::DEVOPS_NIX,            Colors::TS_BLUE};

    // ══════════════════════════════════════════════════════════════════════════
    // 3. DIRECTORIES
    // ══════════════════════════════════════════════════════════════════════════
    directory_map_["src"]               = {NerdFontGlyphs::DIR_SRC,          Colors::DIR_SRC};
    directory_map_["source"]            = {NerdFontGlyphs::DIR_SRC,          Colors::DIR_SRC};
    directory_map_["build"]             = {NerdFontGlyphs::DIR_BUILD,        Colors::DIR_BLD};
    directory_map_["bin"]               = {NerdFontGlyphs::DIR_BIN,          Colors::EXE_GREEN};
    directory_map_["docs"]              = {NerdFontGlyphs::DIR_DOCUMENTS,    Colors::DIR_DOC};
    directory_map_["doc"]               = {NerdFontGlyphs::DIR_DOCUMENTS,    Colors::DIR_DOC};
    directory_map_["resources"]         = {NerdFontGlyphs::DIR_PICTURES,     Colors::IMG_PURPLE};
    directory_map_["res"]               = {NerdFontGlyphs::DIR_PICTURES,     Colors::IMG_PURPLE};
    directory_map_["images"]            = {NerdFontGlyphs::DIR_PICTURES,     Colors::IMG_PURPLE};
    directory_map_["img"]               = {NerdFontGlyphs::DIR_PICTURES,     Colors::IMG_PURPLE};
    directory_map_["config"]            = {NerdFontGlyphs::DIR_CONFIG,       Colors::DIR_CFG};
    directory_map_[".config"]           = {NerdFontGlyphs::DIR_CONFIG,       Colors::DIR_CFG};
    directory_map_[".git"]              = {NerdFontGlyphs::DIR_GIT,          Colors::DIR_GIT};
    directory_map_[".github"]           = {NerdFontGlyphs::DIR_GITHUB,       Colors::DIR_DOC};
    directory_map_["node_modules"]      = {NerdFontGlyphs::DIR_NODE_MODULES, Colors::DIR_PKG};
    directory_map_["tests"]             = {NerdFontGlyphs::DIR_TESTS,        Colors::DIR_SRC};
    directory_map_["test"]              = {NerdFontGlyphs::DIR_TESTS,        Colors::DIR_SRC};
    directory_map_["target"]            = {NerdFontGlyphs::DIR_BUILD,        Colors::DIR_BLD};
    directory_map_["dist"]              = {NerdFontGlyphs::DIR_BUILD,        Colors::DIR_BLD};
    directory_map_["packages"]          = {NerdFontGlyphs::DIR_PACKAGE,      Colors::DIR_PKG};
    directory_map_["pkg"]               = {NerdFontGlyphs::DIR_PACKAGE,      Colors::DIR_PKG};
    directory_map_["Downloads"]         = {NerdFontGlyphs::DIR_DOWNLOADS,    Colors::TS_BLUE};
    directory_map_["Documents"]         = {NerdFontGlyphs::DIR_DOCUMENTS,    Colors::DIR_DOC};
    directory_map_["Desktop"]           = {NerdFontGlyphs::DIR_DESKTOP,      Colors::TS_BLUE};
    directory_map_["Pictures"]          = {NerdFontGlyphs::DIR_PICTURES,     Colors::IMG_PURPLE};
    directory_map_["Videos"]            = {NerdFontGlyphs::DIR_VIDEOS,       Colors::VID_RED};
    directory_map_["Music"]             = {NerdFontGlyphs::DIR_MUSIC,        Colors::AUD_CYAN};
    directory_map_["public"]            = {NerdFontGlyphs::DIR_PUBLIC,       Colors::HTML_ORG};
    directory_map_["db"]                = {NerdFontGlyphs::DIR_DATABASE,     Colors::DIR_CFG};

    // ══════════════════════════════════════════════════════════════════════════
    // 4. COMMAND ICONS
    // ══════════════════════════════════════════════════════════════════════════
    command_map_["git"]                 = {NerdFontGlyphs::GIT_OCTOCAT,          Colors::DIR_GIT};
    command_map_["gh"]                  = {NerdFontGlyphs::GIT_OCTOCAT,          Colors::DIR_GIT};
    command_map_["ssh"]                 = {NerdFontGlyphs::CMD_SSH,              Colors::DIR_CFG};
    command_map_["docker"]              = {NerdFontGlyphs::DEVOPS_DOCKER,        Colors::DOCKER_CYAN};
    command_map_["podman"]              = {NerdFontGlyphs::DEVOPS_PODMAN,        Colors::DIR_GIT};
    command_map_["python"]              = {NerdFontGlyphs::CMD_PYTHON,           Colors::PYTHON_YEL};
    command_map_["python3"]             = {NerdFontGlyphs::CMD_PYTHON,           Colors::PYTHON_YEL};
    command_map_["node"]                = {NerdFontGlyphs::CMD_NODE,             Colors::DIR_SRC};
    command_map_["npm"]                 = {NerdFontGlyphs::CMD_NPM,              Colors::DIR_PKG};
    command_map_["yarn"]                = {NerdFontGlyphs::CMD_YARN,             Colors::TS_BLUE};
    command_map_["pnpm"]                = {NerdFontGlyphs::CMD_NPM,              Colors::DIR_BLD};
    command_map_["cargo"]               = {NerdFontGlyphs::CMD_CARGO,            Colors::RUST_ORG};
    command_map_["rustc"]               = {NerdFontGlyphs::CMD_CARGO,            Colors::RUST_ORG};
    command_map_["vim"]                 = {NerdFontGlyphs::CMD_VIM,              Colors::DIR_SRC};
    command_map_["nvim"]                = {NerdFontGlyphs::CMD_NEOVIM,           Colors::DIR_SRC};
    command_map_["emacs"]               = {NerdFontGlyphs::CMD_EMACS,            Colors::DIR_CFG};
    command_map_["tmux"]                = {NerdFontGlyphs::CMD_TMUX,             Colors::DIR_SRC};
    command_map_["make"]                = {NerdFontGlyphs::DEVOPS_MAKEFILE,      Colors::DIR_BLD};
    command_map_["cmake"]               = {NerdFontGlyphs::DEVOPS_CMAKE,         Colors::C_BLUE};
    command_map_["gcc"]                 = {NerdFontGlyphs::CMD_COMPILER,         Colors::C_BLUE};
    command_map_["g++"]                 = {NerdFontGlyphs::CMD_COMPILER,         Colors::C_BLUE};
    command_map_["clang"]               = {NerdFontGlyphs::CMD_COMPILER,         Colors::C_BLUE};
    command_map_["clang++"]             = {NerdFontGlyphs::CMD_COMPILER,         Colors::C_BLUE};
    command_map_["java"]                = {NerdFontGlyphs::CMD_JAVA,             Colors::JAVA_RED};
    command_map_["javac"]               = {NerdFontGlyphs::CMD_JAVA,             Colors::JAVA_RED};
    command_map_["gradle"]              = {NerdFontGlyphs::CMD_GRADLE,           Colors::TS_BLUE};
    command_map_["mvn"]                 = {NerdFontGlyphs::CMD_MAVEN,            Colors::JAVA_RED};
    command_map_["curl"]                = {NerdFontGlyphs::CMD_NETWORK,          Colors::DIR_SRC};
    command_map_["wget"]                = {NerdFontGlyphs::CMD_DOWNLOAD,         Colors::DIR_SRC};
    command_map_["ping"]                = {NerdFontGlyphs::CMD_NETWORK,          Colors::DIR_SRC};
    command_map_["systemctl"]           = {NerdFontGlyphs::CMD_SERVICE,          Colors::DIR_BLD};
    command_map_["journalctl"]          = {NerdFontGlyphs::CMD_SERVICE,          Colors::DIR_BLD};
    command_map_["sudo"]                = {NerdFontGlyphs::CMD_SECURITY,         Colors::JAVA_RED};
    command_map_["clear"]               = {NerdFontGlyphs::CMD_TERMINAL,         Colors::TS_BLUE};
    command_map_["cd"]                  = {NerdFontGlyphs::DIR_FOLDER,           Colors::DIR_BLUE};
    command_map_["ls"]                  = {NerdFontGlyphs::DIR_FOLDER,           Colors::DIR_BLUE};
    command_map_["cat"]                 = {NerdFontGlyphs::DOC_TEXT,             Colors::DIR_DOC};
    command_map_["grep"]                = {NerdFontGlyphs::CMD_SEARCH,           Colors::DIR_SRC};
    command_map_["ripgrep"]             = {NerdFontGlyphs::CMD_SEARCH,           Colors::DIR_SRC};
    command_map_["rg"]                  = {NerdFontGlyphs::CMD_SEARCH,           Colors::DIR_SRC};
    command_map_["find"]                = {NerdFontGlyphs::CMD_SEARCH,           Colors::DIR_SRC};
    command_map_["fd"]                  = {NerdFontGlyphs::CMD_SEARCH,           Colors::DIR_SRC};
    command_map_["top"]                 = {NerdFontGlyphs::CMD_MONITOR,          Colors::DIR_BLD};
    command_map_["htop"]                = {NerdFontGlyphs::CMD_MONITOR,          Colors::DIR_BLD};
    command_map_["btop"]                = {NerdFontGlyphs::CMD_MONITOR,          Colors::DIR_BLD};
    command_map_["pic"]                 = {NerdFontGlyphs::MEDIA_IMAGE,          Colors::IMG_PURPLE};
    command_map_["ai"]                  = {NerdFontGlyphs::CMD_AI,               Colors::DIR_CFG};
}

void IconRegistry::reload_custom_mappings() {
    custom_extensions_.clear();
    custom_filenames_.clear();
    custom_directories_.clear();
    custom_commands_.clear();

    const char* home = std::getenv("HOME");
    if (!home) return;
    std::string config_path = std::string(home) + "/.config/meridian/icons.toml";
    if (access(config_path.c_str(), R_OK) == 0) {
        parse_toml_file(config_path);
    }
}

void IconRegistry::parse_toml_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        // Strip whitespace
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos || line[first] == '#' || line[first] == ';') continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, last - first + 1);

        if (line.front() == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            std::transform(current_section.begin(), current_section.end(), current_section.begin(), ::tolower);
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        // Trim key
        size_t k_last = key.find_last_not_of(" \t\"");
        size_t k_first = key.find_first_not_of(" \t\"");
        if (k_first != std::string::npos && k_last != std::string::npos) {
            key = key.substr(k_first, k_last - k_first + 1);
        }

        // Trim val
        size_t v_first = val.find_first_not_of(" \t\"");
        size_t v_last = val.find_last_not_of(" \t\"");
        if (v_first != std::string::npos && v_last != std::string::npos) {
            val = val.substr(v_first, v_last - v_first + 1);
        }

        if (key.empty() || val.empty()) continue;

        if (current_section == "extensions") {
            registerCustomExtension(key, val);
        } else if (current_section == "filenames" || current_section == "files") {
            registerCustomFilename(key, val);
        } else if (current_section == "directories" || current_section == "dirs") {
            registerCustomDirectory(key, val);
        } else if (current_section == "commands") {
            registerCustomCommand(key, val);
        }
    }
}

void IconRegistry::registerCustomExtension(const std::string& ext, const std::string& glyph, const std::string& color) {
    std::string k = (ext.front() != '.') ? ("." + ext) : ext;
    std::transform(k.begin(), k.end(), k.begin(), ::tolower);
    custom_extensions_[k] = {glyph, color};
}

void IconRegistry::registerCustomFilename(const std::string& filename, const std::string& glyph, const std::string& color) {
    custom_filenames_[filename] = {glyph, color};
}

void IconRegistry::registerCustomDirectory(const std::string& dirname, const std::string& glyph, const std::string& color) {
    custom_directories_[dirname] = {glyph, color};
}

void IconRegistry::registerCustomCommand(const std::string& cmd, const std::string& glyph, const std::string& color) {
    custom_commands_[cmd] = {glyph, color};
}

Icon IconRegistry::getIcon(const FileInfo& file_info) const {
    IconTier tier = IconDetector::detect_tier();

    if (file_info.is_directory) {
        DirectoryInfo dinfo;
        dinfo.path = file_info.path;
        dinfo.name = file_info.filename;
        dinfo.is_git_repo = (file_info.filename == ".git" || file_info.git_status != ' ');
        return getIconForDirectory(dinfo);
    }

    // 1. Custom filename overrides
    auto it_cfile = custom_filenames_.find(file_info.filename);
    if (it_cfile != custom_filenames_.end()) {
        return Icon{it_cfile->second.first, it_cfile->second.second, "custom", tier};
    }

    // 2. Exact filename match
    auto it_fname = filename_map_.find(file_info.filename);
    if (it_fname != filename_map_.end()) {
        if (tier == IconTier::NERD_FONT) {
            return Icon{it_fname->second.first, it_fname->second.second, "special", tier};
        } else if (tier == IconTier::UNICODE) {
            return Icon{UnicodeFallbackGlyphs::DOC_TEXT, it_fname->second.second, "special", tier};
        } else {
            return Icon{AsciiFallbackGlyphs::DOC_TEXT, "", "special", tier};
        }
    }

    // 3. Custom extension overrides
    std::string ext = file_info.extension.empty() ? extract_extension(file_info.filename) : file_info.extension;
    auto it_cext = custom_extensions_.find(ext);
    if (it_cext != custom_extensions_.end()) {
        return Icon{it_cext->second.first, it_cext->second.second, "custom", tier};
    }

    // 4. Extension match
    if (!ext.empty()) {
        return getIconForExtension(ext);
    }

    // 5. Executable check
    if (file_info.is_executable) {
        if (tier == IconTier::NERD_FONT) {
            return Icon{NerdFontGlyphs::BINARY_EXEC, Colors::EXE_GREEN, "binary", tier};
        } else if (tier == IconTier::UNICODE) {
            return Icon{UnicodeFallbackGlyphs::BINARY_EXEC, Colors::EXE_GREEN, "binary", tier};
        } else {
            return Icon{AsciiFallbackGlyphs::BINARY_EXEC, "", "binary", tier};
        }
    }

    // 6. Generic File Fallback
    if (tier == IconTier::NERD_FONT) {
        return Icon{NerdFontGlyphs::FILE_GENERIC, Colors::DIR_DOC, "generic", tier};
    } else if (tier == IconTier::UNICODE) {
        return Icon{UnicodeFallbackGlyphs::FILE_GENERIC, Colors::DIR_DOC, "generic", tier};
    } else {
        return Icon{AsciiFallbackGlyphs::FILE_GENERIC, "", "generic", tier};
    }
}

Icon IconRegistry::getIconForExtension(const std::string& extension) const {
    IconTier tier = IconDetector::detect_tier();
    std::string ext = extension;
    if (!ext.empty() && ext.front() != '.') ext = "." + ext;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Custom override check
    auto it_c = custom_extensions_.find(ext);
    if (it_c != custom_extensions_.end()) {
        return Icon{it_c->second.first, it_c->second.second, "custom", tier};
    }

    auto it = extension_map_.find(ext);
    if (it != extension_map_.end()) {
        if (tier == IconTier::NERD_FONT) {
            return Icon{it->second.first, it->second.second, "extension", tier};
        } else if (tier == IconTier::UNICODE) {
            return Icon{UnicodeFallbackGlyphs::DOC_TEXT, it->second.second, "extension", tier};
        } else {
            return Icon{AsciiFallbackGlyphs::DOC_TEXT, "", "extension", tier};
        }
    }

    // Default generic extension fallback
    if (tier == IconTier::NERD_FONT) {
        return Icon{NerdFontGlyphs::FILE_GENERIC, Colors::DIR_DOC, "extension", tier};
    } else if (tier == IconTier::UNICODE) {
        return Icon{UnicodeFallbackGlyphs::FILE_GENERIC, Colors::DIR_DOC, "extension", tier};
    } else {
        return Icon{AsciiFallbackGlyphs::FILE_GENERIC, "", "extension", tier};
    }
}

Icon IconRegistry::getIconForDirectory(const DirectoryInfo& dir_info) const {
    IconTier tier = IconDetector::detect_tier();
    std::string name = dir_info.name.empty() ? extract_filename(dir_info.path) : dir_info.name;

    // 1. Home directory
    if (name == "~" || dir_info.path == "~" || name.empty()) {
        if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::DIR_HOME, Colors::DIR_BLUE, "directory", tier};
        if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::DIR_HOME, Colors::DIR_BLUE, "directory", tier};
        return Icon{AsciiFallbackGlyphs::DIR_HOME, "", "directory", tier};
    }

    // 2. Custom directory override
    auto it_cd = custom_directories_.find(name);
    if (it_cd != custom_directories_.end()) {
        return Icon{it_cd->second.first, it_cd->second.second, "custom", tier};
    }

    // 3. Known directory name
    auto it = directory_map_.find(name);
    if (it != directory_map_.end()) {
        if (tier == IconTier::NERD_FONT) {
            return Icon{it->second.first, it->second.second, "directory", tier};
        } else if (tier == IconTier::UNICODE) {
            return Icon{UnicodeFallbackGlyphs::DIR_FOLDER, it->second.second, "directory", tier};
        } else {
            return Icon{AsciiFallbackGlyphs::DIR_FOLDER, "", "directory", tier};
        }
    }

    // 4. Git Repo Directory
    if (dir_info.is_git_repo) {
        if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::DIR_GIT, Colors::DIR_GIT, "directory", tier};
        if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::DIR_GIT, Colors::DIR_GIT, "directory", tier};
        return Icon{AsciiFallbackGlyphs::DIR_GIT, "", "directory", tier};
    }

    // 5. Hidden directory
    if (!name.empty() && name[0] == '.') {
        if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::DIR_HIDDEN, Colors::DIR_CFG, "directory", tier};
        if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::DIR_HIDDEN, Colors::DIR_CFG, "directory", tier};
        return Icon{AsciiFallbackGlyphs::DIR_HIDDEN, "", "directory", tier};
    }

    // 6. Generic Folder
    if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::DIR_FOLDER, Colors::DIR_BLUE, "directory", tier};
    if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::DIR_FOLDER, Colors::DIR_BLUE, "directory", tier};
    return Icon{AsciiFallbackGlyphs::DIR_FOLDER, "", "directory", tier};
}

Icon IconRegistry::getIconForCommand(const std::string& command_name) const {
    IconTier tier = IconDetector::detect_tier();
    std::string cmd = command_name;
    size_t space = cmd.find(' ');
    if (space != std::string::npos) cmd = cmd.substr(0, space);
    size_t slash = cmd.find_last_of('/');
    if (slash != std::string::npos) cmd = cmd.substr(slash + 1);

    auto it_c = custom_commands_.find(cmd);
    if (it_c != custom_commands_.end()) {
        return Icon{it_c->second.first, it_c->second.second, "custom", tier};
    }

    auto it = command_map_.find(cmd);
    if (it != command_map_.end()) {
        if (tier == IconTier::NERD_FONT) {
            return Icon{it->second.first, it->second.second, "command", tier};
        } else if (tier == IconTier::UNICODE) {
            return Icon{UnicodeFallbackGlyphs::LANG_CODE, it->second.second, "command", tier};
        } else {
            return Icon{AsciiFallbackGlyphs::LANG_CODE, "", "command", tier};
        }
    }

    // Generic terminal command
    if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::CMD_TERMINAL, Colors::DIR_SRC, "command", tier};
    if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::LANG_SHELL, Colors::DIR_SRC, "command", tier};
    return Icon{AsciiFallbackGlyphs::LANG_SHELL, "", "command", tier};
}

Icon IconRegistry::getIconForGitStatus(GitStatusType status) const {
    IconTier tier = IconDetector::detect_tier();

    switch (status) {
        case GitStatusType::Branch:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_BRANCH, Colors::DIR_GIT, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_BRANCH, Colors::DIR_GIT, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_BRANCH, "", "git", tier};

        case GitStatusType::Clean:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_CLEAN, Colors::GIT_CLEAN, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_CLEAN, Colors::GIT_CLEAN, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_CLEAN, "", "git", tier};

        case GitStatusType::Modified:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_MODIFIED, Colors::GIT_DIRTY, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_MODIFIED, Colors::GIT_DIRTY, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_MODIFIED, "", "git", tier};

        case GitStatusType::Staged:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_STAGED, Colors::GIT_STAGED, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_STAGED, Colors::GIT_STAGED, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_STAGED, "", "git", tier};

        case GitStatusType::Untracked:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_UNTRACKED, Colors::GIT_UNTRK, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_UNTRACKED, Colors::GIT_UNTRK, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_UNTRACKED, "", "git", tier};

        case GitStatusType::Deleted:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_DELETED, Colors::GIT_UNTRK, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_DELETED, Colors::GIT_UNTRK, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_DELETED, "", "git", tier};

        case GitStatusType::Renamed:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_RENAMED, Colors::GIT_STAGED, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_RENAMED, Colors::GIT_STAGED, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_RENAMED, "", "git", tier};

        case GitStatusType::Conflict:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_CONFLICT, Colors::JAVA_RED, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_CONFLICT, Colors::JAVA_RED, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_CONFLICT, "", "git", tier};

        case GitStatusType::Ahead:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_AHEAD, Colors::GIT_SYNC, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_AHEAD, Colors::GIT_SYNC, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_AHEAD, "", "git", tier};

        case GitStatusType::Behind:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_BEHIND, Colors::GIT_SYNC, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_BEHIND, Colors::GIT_SYNC, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_BEHIND, "", "git", tier};

        case GitStatusType::Diverged:
            if (tier == IconTier::NERD_FONT) return Icon{NerdFontGlyphs::GIT_DIVERGED, Colors::GIT_SYNC, "git", tier};
            if (tier == IconTier::UNICODE)   return Icon{UnicodeFallbackGlyphs::GIT_DIVERGED, Colors::GIT_SYNC, "git", tier};
            return Icon{AsciiFallbackGlyphs::GIT_DIVERGED, "", "git", tier};
    }

    return Icon{NerdFontGlyphs::GIT_BRANCH, Colors::DIR_GIT, "git", tier};
}

Icon IconRegistry::getIconForApplication(const std::string& app_name) const {
    return getIconForCommand(app_name);
}

Icon IconRegistry::getIconForFile(
    const std::string& filename,
    bool is_dir,
    bool is_executable,
    char git_status
) const {
    FileInfo fi;
    fi.filename = filename;
    fi.is_directory = is_dir;
    fi.is_executable = is_executable;
    fi.git_status = git_status;
    return getIcon(fi);
}

Icon IconRegistry::getIconForPath(const std::string& formatted_path, bool is_git_repo) const {
    DirectoryInfo di;
    di.path = formatted_path;
    di.is_git_repo = is_git_repo;
    return getIconForDirectory(di);
}

} // namespace meridian::icons

