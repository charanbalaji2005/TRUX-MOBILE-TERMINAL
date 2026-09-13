#pragma once
// meridian-terminal / dev / command_palette.hpp
//
// Command Palette engine (Ctrl+Shift+P). Fuzzy search and quick launcher
// for all terminal actions, multiplexer layouts, AI diagnostics, and dev tools.

#include <string>
#include <vector>

namespace meridian::dev {

struct PaletteAction {
    std::string id;
    std::string title;
    std::string category;
    std::string shortcut;
    int match_score = 0;
};

class CommandPalette {
public:
    CommandPalette();

    void register_action(PaletteAction action);
    std::vector<PaletteAction> search(const std::string& query) const;
    const std::vector<PaletteAction>& all_actions() const { return actions_; }

    static int fuzzy_score(const std::string& pattern, const std::string& target);

private:
    std::vector<PaletteAction> actions_;
    void load_default_actions();
};

} // namespace meridian::dev

