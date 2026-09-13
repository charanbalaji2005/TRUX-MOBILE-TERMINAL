#include "command_palette.hpp"

#include <algorithm>
#include <cctype>

namespace meridian::dev {

namespace {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

} // namespace

int CommandPalette::fuzzy_score(const std::string& pattern, const std::string& target) {
    if (pattern.empty()) return 100;
    std::string p = to_lower(pattern);
    std::string t = to_lower(target);

    // Exact substring bonus
    if (t.find(p) != std::string::npos) return 500 - static_cast<int>(t.size());

    // Character sequence match
    std::size_t p_idx = 0;
    int score = 0;
    for (std::size_t i = 0; i < t.size() && p_idx < p.size(); ++i) {
        if (t[i] == p[p_idx]) {
            score += 10;
            p_idx++;
        }
    }

    return (p_idx == p.size()) ? score : 0;
}

CommandPalette::CommandPalette() {
    load_default_actions();
}

void CommandPalette::load_default_actions() {
    register_action({"pane.split_h", "Split Pane Horizontally", "Pane", "Ctrl+Shift+D", 0});
    register_action({"pane.split_v", "Split Pane Vertically", "Pane", "Ctrl+Shift+E", 0});
    register_action({"pane.zoom", "Toggle Pane Zoom", "Pane", "Ctrl+Shift+Z", 0});
    register_action({"pane.close", "Close Active Pane", "Pane", "Ctrl+Shift+W", 0});
    register_action({"ai.intent", "Generate Command from English Intent", "Meridian AI", "Ctrl+Shift+I", 0});
    register_action({"ai.diagnostics", "Diagnose Last Terminal Error", "Meridian AI", "Ctrl+Shift+X", 0});
    register_action({"ai.agent", "Launch Autonomous Coding Agent", "Meridian AI", "Ctrl+Shift+A", 0});
    register_action({"dev.git", "Toggle Git Intelligence Panel", "Developer", "Ctrl+Shift+G", 0});
    register_action({"dev.monitor", "Toggle System & Network Monitor", "Developer", "Ctrl+Shift+M", 0});
    register_action({"dev.files", "Toggle File Explorer Sidecar", "Developer", "Ctrl+Shift+B", 0});
    register_action({"search.universal", "Universal Search (Output & History)", "Search", "Ctrl+Shift+F", 0});
    register_action({"workspace.save", "Save Current Workspace Layout", "Workspace", "Ctrl+Shift+S", 0});
    register_action({"workspace.open", "Open Saved Workspace", "Workspace", "Ctrl+Shift+O", 0});
}

void CommandPalette::register_action(PaletteAction action) {
    actions_.push_back(std::move(action));
}

std::vector<PaletteAction> CommandPalette::search(const std::string& query) const {
    std::vector<PaletteAction> results;
    for (const auto& a : actions_) {
        int s1 = fuzzy_score(query, a.title);
        int s2 = fuzzy_score(query, a.category);
        int best = std::max(s1, s2);
        if (best > 0) {
            PaletteAction res = a;
            res.match_score = best;
            results.push_back(res);
        }
    }

    std::sort(results.begin(), results.end(), [](const PaletteAction& a, const PaletteAction& b) {
        return a.match_score > b.match_score;
    });

    return results;
}

} // namespace meridian::dev

