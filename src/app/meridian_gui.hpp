#pragma once
// src/app/meridian_gui.hpp
//
// Meridian 2.0 Graphical Terminal Interface & Dashboard.
// Provides a complete visual workstation layout with tabs, split panes,
// live system telemetry, file browser, and command history preview.

#include "../workspace/pane_tree.hpp"
#include "../workspace/workspace_manager.hpp"
#include "../dev/system_monitor.hpp"
#include "../dev/git_intel.hpp"
#include "../dev/file_explorer.hpp"
#include "../dev/command_palette.hpp"
#include "../ai/intent_engine.hpp"

#include <string>
#include <vector>

namespace meridian::gui {

class MeridianGui {
public:
    MeridianGui();

    // Renders one graphical dashboard frame
    std::string render_frame(int width, int height);

    // Runs the interactive graphical interface loop
    int run();

private:
    workspace::PaneTree pane_tree_;
    dev::SystemMonitor sys_monitor_;
    dev::CommandPalette palette_;
    std::vector<std::string> tabs_;
    int active_tab_ = 0;
    bool show_monitor_ = true;
    bool show_explorer_ = true;
    std::string status_message_ = "Meridian 2.0 Ready • Press Ctrl+Shift+P for Command Palette";
};

} // namespace meridian::gui

