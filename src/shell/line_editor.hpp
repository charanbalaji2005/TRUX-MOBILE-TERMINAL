// src/shell/line_editor.hpp
#pragma once
// meridian-shell / line_editor.hpp
//
// Interactive line editor with:
//   - Fish/Zsh-style real-time dim ghost autosuggestions
//   - Lanczos-powered Powerline prompt with icons and rich Git status
//   - Tab completion with file-type icons
//   - History navigation (Up/Down)
//   - Ctrl+L and clear viewport support
//   - Ctrl+R reverse search stub
//   - Arrow keys, Home/End, Ctrl+Arrow word-jump

#include <iostream>
#include <string>
#include <vector>

namespace meridian::shell {

class LineEditor {
public:
    static bool is_terminal_interactive();

    // Builds the Date/Time status badge line (printed once above prompt)
    static std::string build_date_badge(const std::string& cwd);

    // Builds the single-line powerline prompt (without newlines for smooth in-place line editing)
    static std::string build_powerline_prompt(const std::string& cwd);

    // Reads an interactive line from stdin with:
    //   - Real-time dim ghost autosuggestions
    //   - Tab completion with multi-item icon menu
    //   - History navigation (Up/Down arrows)
    //   - Right Arrow / End: accept suggestion
    //   - Ctrl+Right / Alt+Right: accept next word
    //   - Esc: dismiss suggestion
    //   - Ctrl+L: clear viewport
    // Falls back to std::getline if not a TTY.
    static std::string read_line(
        std::istream& in,
        std::ostream& out,
        const std::string& prompt,
        const std::vector<std::string>& history
    );

private:
    static void draw_history_preview(
        std::ostream& out,
        const std::vector<std::string>& history,
        int selected_idx,
        int visible_count
    );

    static void clear_history_preview(std::ostream& out, int lines_drawn);
};

} // namespace meridian::shell
