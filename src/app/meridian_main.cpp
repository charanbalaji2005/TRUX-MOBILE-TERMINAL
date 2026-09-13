// src/app/meridian_main.cpp
//
// Meridian 2.0 Unified Developer Platform & AI CLI.
// Exposes the full suite of terminal capabilities:
//  - Local AI: intent engine, error diagnostics, autonomous agent, typo/risk analysis
//  - Workspace & Session multiplexer management
//  - Developer Intelligence: Git status, System/Network monitor, File explorer,
//    Universal search, Command palette.

#include "../ai/ai_agent.hpp"
#include "../ai/ai_controller.hpp"
#include "../ai/error_diagnostics.hpp"
#include "../ai/intent_engine.hpp"
#include "../dev/platform_manager.hpp"
#include "../dev/github_integration.hpp"
#include "../dev/ide_detector.hpp"
#include "../dev/command_palette.hpp"
#include "../dev/file_explorer.hpp"
#include "../dev/git_intel.hpp"
#include "../dev/rich_history.hpp"
#include "../dev/ssh_manager.hpp"
#include "../dev/system_monitor.hpp"
#include "../dev/universal_search.hpp"
#include "../plugins/plugin_manager.hpp"
#include "../core/renderer/telemetry_profiler.hpp"
#include "../shell/builtins.hpp"
#include "../shell/shell.hpp"
#include "../workspace/session_recorder.hpp"
#include "../core/pty/pty_session.hpp"
#include "meridian_gui.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace meridian;

namespace {

std::string default_config_path() {
    if (const char* override_dir = std::getenv("MERIDIAN_CONFIG_HOME")) {
        return std::string(override_dir) + "/ai.toml";
    }
    const char* home = std::getenv("HOME");
    std::string base = home ? home : ".";
    return base + "/.config/meridian/ai.toml";
}

void print_usage() {
    std::cout <<
        "Meridian 2.5 — Terminal + Developer Environment + AI Platform\n\n"
        "Usage: meridian [subcommand] [args]\n\n"
        "Terminal & System Lifecycle:\n"
        "  (no args)                   launch interactive Meridian Terminal & Shell\n"
        "  -c \"<cmd>\"                  execute command in Meridian shell engine and exit\n"
        "  update [--check|--yes]      check and update Meridian to the latest release\n"
        "  stats [--year YYYY|--growth]view anonymous usage counts & yearly statistics\n"
        "  telemetry [status|on|off]   manage opt-in anonymous metrics and privacy\n"
        "  uninstall [--purge]         uninstall Meridian binaries, launchers, and configs\n"
        "  pic <file>                  render direct full-color inline raster image\n"
        "  pic set <0-13|name|path>    set startup anime artwork permanently or random\n"
        "  --version, -v               display version and build information\n\n"
        "AI Commands:\n"
        "  ask \"<intent>\"              translate natural language intent to safe shell command\n"
        "  diag \"<error_text>\"          diagnose compiler/runtime/database errors with suggested fixes\n"
        "  agent \"<goal>\"              launch autonomous coding & repair agent loop\n"
        "  ai <status|on|off|...>      manage local AI engine settings, risk classification, redaction\n\n"
        "Workspace & Session Multiplexing:\n"
        "  workspace list              list saved persistent workspaces\n"
        "  workspace save <name>       save current environment layout & state\n"
        "  workspace open <name>       inspect/restore a saved development workspace\n"
        "  workspace rm <name>         delete a saved workspace\n"
        "  session save <file>         save recorded session trace to file\n\n"
        "Developer Tooling:\n"
        "  auth github                 1-click GitHub SSH key setup & live authentication\n"
        "  ssh [alias]                 manage & connect to SSH remote workspaces (~/.ssh/config)\n"
        "  plugins                     list active extensible plugins & hooks (~/.config/meridian/plugins/)\n"
        "  --performance, perf         display live GPU framerate, PTY latency & telemetry profiler\n"
        "  monitor                     display real-time CPU, RAM, Disk, Network & Process metrics\n"
        "  gitintel                    inspect Git branch divergence, staged/unstaged changes\n"
        "  files [dir]                 view tree file explorer with git badges\n"
        "  search \"<query>\"             search across screen buffers, command history & files\n"
        "  palette [query]             open / fuzzy-search Command Palette actions (Ctrl+Shift+P)\n"
        "  history                     view recent rich command history with durations & exit codes\n";
}

int join_argv(int argc, char** argv, int start, std::string* out) {
    std::string s;
    for (int i = start; i < argc; ++i) {
        if (i > start) s += " ";
        s += argv[i];
    }
    *out = s;
    return 0;
}

int handle_ai_subcommand(int argc, char** argv) {
    if (argc < 3) {
        std::cout <<
            "Usage: meridian ai <command> [args]\n\n"
            "  on | off                    enable/disable Meridian AI\n"
            "  status                      show current AI state\n"
            "  detect on | detect off      toggle local command analysis\n"
            "  privacy on | privacy off    toggle privacy mode\n"
            "  providers                   list configured providers\n"
            "  use <provider>              set the active provider name\n"
            "  test                        test provider connectivity\n"
            "  analyze \"<command>\"         local typo + risk analysis (no network)\n"
            "  explain \"<command>\"         explain a command\n"
            "  redact \"<text>\"             show text with likely secrets redacted\n";
        return 1;
    }

    ai::AIController controller(default_config_path());
    controller.load();

    std::string cmd = argv[2];

    if (cmd == "on") { controller.set_enabled(true); controller.save(); std::cout << "Meridian AI enabled.\n"; return 0; }
    if (cmd == "off") { controller.set_enabled(false); controller.save(); std::cout << "Meridian AI disabled.\n"; return 0; }
    if (cmd == "status") { std::cout << controller.status_report(); return 0; }

    if (cmd == "detect" && argc >= 4) {
        std::string sub = argv[3];
        if (sub == "on") { controller.set_detection_enabled(true); controller.save(); std::cout << "Detection enabled.\n"; return 0; }
        if (sub == "off") { controller.set_detection_enabled(false); controller.save(); std::cout << "Detection disabled.\n"; return 0; }
    }

    if (cmd == "privacy" && argc >= 4) {
        std::string sub = argv[3];
        if (sub == "on") { controller.set_privacy_mode(true); controller.save(); std::cout << "Privacy mode enabled.\n"; return 0; }
        if (sub == "off") { controller.set_privacy_mode(false); controller.save(); std::cout << "Privacy mode disabled.\n"; return 0; }
    }

    if (cmd == "providers") {
        std::cout << "Configured providers:\n  (none — local AI is active)\nActive provider setting: " << controller.provider() << "\n";
        return 0;
    }

    if (cmd == "use" && argc >= 4) {
        controller.set_provider(argv[3]);
        controller.save();
        std::cout << "Provider preference set to '" << argv[3] << "'.\n";
        return 0;
    }

    if (cmd == "test") { std::cout << controller.test_providers(); return 0; }

    if (cmd == "analyze" && argc >= 4) {
        std::string line;
        join_argv(argc, argv, 3, &line);
        if (!controller.enabled()) { std::cout << "Meridian AI is off (`meridian ai on` to enable).\n"; return 0; }
        if (!controller.detection_enabled()) { std::cout << "Detection is off (`meridian ai detect on` to enable).\n"; return 0; }
        std::string report = controller.analyze_command(line);
        std::cout << (report.empty() ? "No issues detected.\n" : report);
        return 0;
    }

    if (cmd == "explain" && argc >= 4) {
        std::string line;
        join_argv(argc, argv, 3, &line);
        std::cout << controller.explain_command(line) << "\n";
        return 0;
    }

    if (cmd == "redact" && argc >= 4) {
        std::string line;
        join_argv(argc, argv, 3, &line);
        std::cout << controller.redact(line) << "\n";
        return 0;
    }

    return 1;
}

static bool launch_in_terminal_window() {
    std::string bin = "meridian-shell";
    const char* home = std::getenv("HOME");
    std::string local_bin = std::string(home ? home : "") + "/.local/bin/meridian-shell";
    if (access(local_bin.c_str(), X_OK) == 0) bin = local_bin;
    else if (access("/usr/local/bin/meridian-shell", X_OK) == 0) bin = "/usr/local/bin/meridian-shell";

#ifdef __APPLE__
    if (system(("open -a Terminal \"" + bin + "\"").c_str()) == 0) return true;
#endif

    if (std::getenv("WSL_DISTRO_NAME")) {
        if (system(("cmd.exe /c start wt.exe -w 0 wsl.exe -e " + bin + " 2>/dev/null").c_str()) == 0) return true;
    }

    std::vector<std::string> launchers = {
        "x-terminal-emulator -e \"" + bin + "\"",
        "gnome-terminal -- \"" + bin + "\"",
        "kitty \"" + bin + "\"",
        "alacritty -e \"" + bin + "\"",
        "wezterm start \"" + bin + "\"",
        "konsole -e \"" + bin + "\"",
        "xfce4-terminal -e \"" + bin + "\"",
        "foot \"" + bin + "\"",
        "xterm -e \"" + bin + "\""
    };

    for (const auto& cmd : launchers) {
        std::string full_cmd = "which " + cmd.substr(0, cmd.find(' ')) + " >/dev/null 2>&1 && (" + cmd + " &)";
        if (system(full_cmd.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace

int main(int argc, char** argv) {
    // Ensure automatic background IDE integration across Linux, macOS, Windows/WSL
    dev::IdeDetector::ensure_first_run_registered();

    // Direct command execution flag (-c "command")
    if (argc >= 3 && std::string(argv[1]) == "-c") {
        shell::Shell sh(false);
        return sh.run_command(argv[2], std::cerr);
    }

    if (argc < 2 || (argc >= 2 && (std::string(argv[1]) == "shell" || std::string(argv[1]) == "run"))) {
        // If double-clicked in desktop GUI without a TTY, launch inside graphical terminal window
        if (!isatty(STDIN_FILENO) && !isatty(STDOUT_FILENO)) {
            if (launch_in_terminal_window()) {
                return 0;
            }
        }
        shell::Shell sh(true);
        return sh.run_interactive(std::cin, std::cout, std::cerr);
    }

    std::string sub = argv[1];

    if (sub == "--help" || sub == "-h" || sub == "help") {
        print_usage();
        return 0;
    }

    if (sub == "--version" || sub == "-v" || sub == "version") {
        std::cout << "Meridian Terminal 2.5.0 (x86_64-linux)\n"
                  << "Copyright (c) 2025-2026 Charan Balaji and Meridian Contributors.\n";
        return 0;
    }

    if (sub == "vscode" || sub == "ide") {
        dev::IdeDetector::print_ide_report();
        return 0;
    }

    if (sub == "update" || sub == "upgrade") {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
        return dev::PlatformManager::handle_update(args);
    }

    if (sub == "stats" || sub == "stat") {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
        return dev::PlatformManager::handle_stats(args);
    }

    if (sub == "telemetry") {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
        return dev::PlatformManager::handle_telemetry(args);
    }

    if (sub == "gh" || sub == "github") {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
        return dev::GitHubIntegration::handle_gh_command(args);
    }

    if (sub == "auth") {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
        shell::Executor ctx;
        return shell::run_builtin("auth", args, ctx);
    }

    if (sub == "ai") {
        return handle_ai_subcommand(argc, argv);
    }

    if (sub == "install") {
        std::cout << "-> Installing Meridian Terminal 2.5 to system...\n";
        int res = system("curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash");
        return res;
    }

    if (sub == "uninstall" || sub == "remove") {
        std::string flags;
        for (int i = 2; i < argc; ++i) {
            flags += " ";
            flags += argv[i];
        }
        std::cout << "-> Running Meridian Terminal uninstaller...\n";
        int res = system(("curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/uninstall.sh | bash -s --" + flags).c_str());
        return res;
    }

    // meridian pic [file]
    if (sub == "pic") {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);
        shell::Executor ctx;
        return shell::run_builtin("pic", args, ctx);
    }

    // 1. meridian ask "<intent>"
    if (sub == "ask" && argc >= 3) {
        std::string query;
        join_argv(argc, argv, 2, &query);
        ai::IntentEngine engine;
        auto result = engine.translate(query);
        std::cout << ai::IntentEngine::format_card(result);
        return 0;
    }

    // 2. meridian diag "<error text>"
    if (sub == "diag" && argc >= 3) {
        std::string error_text;
        join_argv(argc, argv, 2, &error_text);
        ai::ErrorDiagnostics diagnostics;
        auto card = diagnostics.analyze(error_text);
        std::cout << card.format();
        return 0;
    }

    // 3. meridian agent "<goal>"
    if (sub == "agent") {
        std::string goal_text = "Diagnose repository and run test suite";
        if (argc >= 3) {
            join_argv(argc, argv, 2, &goal_text);
        }
        ai::AiAgent agent;
        ai::AgentGoal goal;
        goal.description = goal_text;
        agent.set_goal(goal);

        std::cout << "Starting Meridian AI Agent for goal: \"" << goal_text << "\"\n\n";
        while (agent.status() == ai::AgentStatus::Executing) {
            agent.run_next_step();
        }
        std::cout << agent.format_progress();
        return 0;
    }

    // 4. meridian workspace [list|save|open|rm]
    if (sub == "workspace") {
        workspace::WorkspaceManager wm;
        if (argc < 3 || std::string(argv[2]) == "list") {
            auto list = wm.list_workspaces();
            std::cout << "Saved Meridian Workspaces (" << list.size() << "):\n";
            for (const auto& name : list) {
                std::cout << "  • " << name << "\n";
            }
            return 0;
        }
        std::string action = argv[2];
        if (action == "save" && argc >= 4) {
            std::string name = argv[3];
            workspace::PaneTree tree;
            auto ws = workspace::WorkspaceManager::capture_live_workspace(name, ".", "main", tree);
            if (wm.save_workspace(ws)) {
                std::cout << "Workspace '" << name << "' saved successfully to " << wm.storage_dir() << "\n";
            } else {
                std::cerr << "Failed to save workspace '" << name << "'.\n";
            }
            return 0;
        }
        if (action == "open" && argc >= 4) {
            std::string name = argv[3];
            auto ws = wm.load_workspace(name);
            if (ws.has_value()) {
                std::cout << "Workspace: " << ws->name << "\n"
                          << "Root Directory: " << ws->root_dir << "\n"
                          << "Panes: " << ws->panes.size() << "\n";
            } else {
                std::cerr << "Workspace '" << name << "' not found.\n";
            }
            return 0;
        }
        if (action == "rm" && argc >= 4) {
            std::string name = argv[3];
            if (wm.delete_workspace(name)) {
                std::cout << "Workspace '" << name << "' removed.\n";
            } else {
                std::cerr << "Could not delete workspace '" << name << "'.\n";
            }
            return 0;
        }
    }

    // 5. meridian monitor
    if (sub == "monitor") {
        dev::SystemMonitor mon;
        auto metrics = mon.sample();
        std::cout << metrics.format_dashboard();
        return 0;
    }

    // 6. meridian git
    if (sub == "git") {
        auto status = dev::GitIntel::inspect_directory(".");
        std::cout << status.format_panel();
        return 0;
    }

    // 7. meridian files [dir]
    if (sub == "files") {
        std::string target = (argc >= 3) ? argv[2] : ".";
        auto root = dev::FileExplorer::scan_directory(target, 2);
        std::cout << dev::FileExplorer::format_tree(root);
        return 0;
    }

    // 8. meridian search "<query>"
    if (sub == "search" && argc >= 3) {
        std::string q;
        join_argv(argc, argv, 2, &q);
        vt::ScreenBuffer screen;
        dev::RichHistory history;
        auto matches = dev::UniversalSearch::search_all(screen, history, q);
        std::cout << "Universal Search results for \"" << q << "\" (" << matches.size() << " matches):\n";
        for (const auto& m : matches) {
            std::cout << "  [" << m.source_label << "] " << m.line_content << "\n";
        }
        return 0;
    }

    // 9. meridian palette [query]
    if (sub == "palette") {
        std::string q = (argc >= 3) ? argv[2] : "";
        dev::CommandPalette palette;
        auto results = palette.search(q);
        std::cout << "┌─── Meridian Command Palette (Ctrl+Shift+P) ────────────\n";
        for (const auto& a : results) {
            std::cout << "│ [" << a.category << "] " << a.title << " (" << a.shortcut << ")\n";
        }
        std::cout << "└─────────────────────────────────────────────────────────\n";
        return 0;
    }

    // 10. meridian history
    if (sub == "history") {
        dev::RichHistory history;
        auto recent = history.recent(15);
        std::cout << "Recent Rich Command History (" << recent.size() << " entries):\n";
        for (const auto& r : recent) {
            std::cout << "  #" << r.id << " [exit=" << r.exit_code << ", " << r.duration_ms << "ms] (" << r.working_dir << ") " << r.command << "\n";
        }
        return 0;
    }

    // 11. meridian ssh [alias]
    if (sub == "ssh") {
        dev::SSHManager ssh_mgr;
        if (argc >= 3) {
            std::string alias = argv[2];
            auto host = ssh_mgr.find_host(alias);
            if (host) {
                std::cout << "\033[1;38;2;0;229;255mConnecting to " << host->alias << " (" << host->hostname << ")...\033[0m\n";
                std::string cmd = host->command_line();
                return std::system(cmd.c_str());
            } else {
                std::cerr << "SSH host '" << alias << "' not found in ~/.ssh/config. Showing workspace overview:\n\n";
            }
        }
        std::cout << ssh_mgr.format_overview();
        return 0;
    }

    // 12. meridian plugins / plugin list
    if (sub == "plugins" || sub == "plugin") {
        std::cout << plugins::PluginManager::instance().format_plugin_list();
        return 0;
    }

    // 13. meridian --performance / perf
    if (sub == "--performance" || sub == "perf" || sub == "performance") {
        std::cout << renderer::TelemetryProfiler::instance().format_report();
        return 0;
    }

    // 14. meridian gui
    if (sub == "gui") {
        gui::MeridianGui gui;
        return gui.run();
    }

    print_usage();
    return 1;
}
