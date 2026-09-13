#pragma once
// meridian-core / config.hpp
//
// A deliberately minimal flat key=value config store — NOT a full TOML
// parser, even though the spec's example configs look like TOML
// (`terminal.toml`, `keybindings.toml`, `ai.toml`). Implementing real
// TOML (nested tables, arrays, typed values, quoting rules) is real
// scope on its own; this covers exactly what a `key = "value"` line
// needs and says so honestly rather than pretending to parse a format
// it doesn't. Shared by src/ai/ and src/config/ rather than duplicated —
// both just point the same class at a different file.

#include <map>
#include <string>

namespace meridian {

class Config {
public:
    explicit Config(std::string path);

    // Loads from disk if the file exists; returns false (not an error)
    // if it doesn't yet — callers get default values from get() either way.
    bool load();
    bool save() const;

    std::string get(const std::string& key, const std::string& default_value = "") const;
    void set(const std::string& key, const std::string& value);
    bool get_bool(const std::string& key, bool default_value) const;
    void set_bool(const std::string& key, bool value);

    const std::string& path() const { return path_; }

private:
    std::string path_;
    std::map<std::string, std::string> values_;
};

} // namespace meridian
