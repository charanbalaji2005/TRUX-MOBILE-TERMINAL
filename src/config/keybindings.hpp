#pragma once
// meridian-config / keybindings.hpp
//
// Spec §75/§20's keybindings.toml: a named-action -> key-combo map with
// sensible defaults (matching the specific shortcuts named elsewhere in
// the spec: Ctrl+Space for the AI popup, Ctrl+Shift+F for search, etc).
// This is real, persisted, testable data even with no GUI attached yet
// to actually intercept these key combos — a future input layer reads
// from here rather than hardcoding shortcuts.

#include "../core/config.hpp"
#include <map>
#include <string>

namespace meridian::config {

class Keybindings {
public:
    explicit Keybindings(std::string path);

    bool load();
    bool save();

    // Returns the bound combo for `action`, or the built-in default if
    // never customized (defaults are seeded in the constructor, so this
    // never returns empty for a known action name).
    std::string get(const std::string& action) const;
    void set(const std::string& action, const std::string& combo);

    // All action -> combo pairs currently in effect (defaults plus any
    // overrides), for listing/display purposes.
    std::map<std::string, std::string> all() const;

private:
    Config config_;
    std::map<std::string, std::string> defaults_;
};

} // namespace meridian::config
