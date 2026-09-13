// tests/test_advanced_protocols.cpp
//
// Automated unit and regression test suite for:
//  - OSC 8 Hyperlinks
//  - OSC 52 Clipboard Protocol
//  - OSC 7 Working Directory Tracking
//  - OSC 133 Shell Integration & Semantic Prompts
//  - Native SSH Workspace Manager
//  - Extensible Plugin Architecture
//  - GPU & Performance Telemetry Profiler

#include "mini_test.hpp"
#include "../src/core/vt/ansi_parser.hpp"
#include "../src/core/vt/screen_buffer.hpp"
#include "../src/dev/ssh_manager.hpp"
#include "../src/plugins/plugin_manager.hpp"
#include "../src/core/renderer/telemetry_profiler.hpp"

using namespace meridian;

MTEST(protocol_osc8_hyperlinks_attached_to_cells) {
    vt::ScreenBuffer screen(10, 40);
    vt::AnsiParser parser(screen);

    // Write text with OSC 8 hyperlink: "\033]8;id=gh;https://github.com\033\\GitHub\033]8;;\033\\"
    parser.feed("\033]8;id=gh;https://github.com\033\\GitHub\033]8;;\033\\");

    // Verify cell characters
    ASSERT_EQ(screen.cell_at(0, 0).codepoint, U'G');
    ASSERT_EQ(screen.cell_at(0, 1).codepoint, U'i');
    ASSERT_EQ(screen.cell_at(0, 5).codepoint, U'b');

    // Verify hyperlink ID is non-zero
    uint32_t link_id = screen.cell_at(0, 0).attrs.hyperlink_id;
    ASSERT_TRUE(link_id > 0);
    ASSERT_EQ(screen.get_hyperlink(link_id), "https://github.com");

    // Verify closed hyperlink returns 0 for subsequent cells
    parser.feed(" Plain");
    ASSERT_EQ(screen.cell_at(0, 6).attrs.hyperlink_id, 0u);
}

MTEST(protocol_osc52_remote_clipboard_decoding) {
    vt::ScreenBuffer screen(10, 40);
    vt::AnsiParser parser(screen);

    // "TWVyaWRpYW4=" is base64 for "Meridian"
    parser.feed("\033]52;c;TWVyaWRpYW4=\033\\");
    ASSERT_EQ(screen.clipboard(), "Meridian");

    // "SGVsbG8gV29ybGQ=" is base64 for "Hello World"
    parser.feed("\033]52;c;SGVsbG8gV29ybGQ=\007");
    ASSERT_EQ(screen.clipboard(), "Hello World");
}

MTEST(protocol_osc7_working_directory_tracking) {
    vt::ScreenBuffer screen(10, 40);
    vt::AnsiParser parser(screen);

    parser.feed("\033]7;file://fedora-laptop/home/charanbalaji/Downloads/meridian\033\\");
    ASSERT_EQ(screen.working_directory(), "/home/charanbalaji/Downloads/meridian");

    parser.feed("\033]7;file://localhost/var/log\007");
    ASSERT_EQ(screen.working_directory(), "/var/log");
}

MTEST(protocol_osc133_shell_integration_and_exit_codes) {
    vt::ScreenBuffer screen(10, 40);
    vt::AnsiParser parser(screen);

    // Prompt start (A)
    parser.feed("\033]133;A\033\\");
    ASSERT_EQ(static_cast<int>(screen.semantic_state()), static_cast<int>(vt::ScreenBuffer::SemanticPromptState::Prompt));

    // Command input start (B)
    parser.feed("\033]133;B\033\\");
    ASSERT_EQ(static_cast<int>(screen.semantic_state()), static_cast<int>(vt::ScreenBuffer::SemanticPromptState::CommandInput));

    // Command output start (C)
    parser.feed("\033]133;C\033\\");
    ASSERT_EQ(static_cast<int>(screen.semantic_state()), static_cast<int>(vt::ScreenBuffer::SemanticPromptState::CommandOutput));

    // Command finished with non-zero exit code (D;127)
    parser.feed("\033]133;D;127\033\\");
    ASSERT_EQ(static_cast<int>(screen.semantic_state()), static_cast<int>(vt::ScreenBuffer::SemanticPromptState::CommandFinished));
    ASSERT_EQ(screen.last_command_exit_code(), 127);
}

MTEST(dev_ssh_manager_host_lookup_and_command_generation) {
    dev::SSHManager mgr("/nonexistent/ssh/config"); // will load default test hosts
    auto hosts = mgr.list_hosts();
    ASSERT_TRUE(!hosts.empty());

    auto prod = mgr.find_host("production");
    ASSERT_TRUE(prod.has_value());
    ASSERT_EQ(prod->hostname, "prod.meridian.internal");
    ASSERT_EQ(prod->user, "deploy");
    ASSERT_EQ(prod->port, 22);
    ASSERT_TRUE(prod->command_line().find("deploy@prod.meridian.internal") != std::string::npos);

    auto custom_port = mgr.find_host("development");
    ASSERT_TRUE(custom_port.has_value());
    ASSERT_EQ(custom_port->port, 2222);
    ASSERT_TRUE(custom_port->command_line().find("-p 2222") != std::string::npos);
}

MTEST(plugin_architecture_registration_and_dispatch) {
    auto& mgr = plugins::PluginManager::instance();
    auto list = mgr.plugins();
    ASSERT_TRUE(list.size() >= 3);

    auto git_plugin = mgr.find_plugin("git-intel");
    ASSERT_TRUE(git_plugin != nullptr);
    ASSERT_EQ(git_plugin->name(), "git-intel");

    // Test hook notification
    mgr.notify_command_pre_exec("git status");
    mgr.notify_command_post_exec("git status", 0);
    mgr.notify_directory_changed("/home/user/project");

    ASSERT_TRUE(!mgr.format_plugin_list().empty());
}

MTEST(telemetry_profiler_metrics_and_hud) {
    auto& profiler = renderer::TelemetryProfiler::instance();
    profiler.record_frame_start();
    profiler.record_frame_end(150, 2);
    profiler.record_pty_latency(0.4);

    auto m = profiler.get_metrics();
    ASSERT_TRUE(m.fps > 0.0);
    ASSERT_TRUE(m.frame_time_ms > 0.0);
    ASSERT_EQ(m.dirty_rects_count, 2u);

    std::string report = profiler.format_report();
    ASSERT_TRUE(report.find("MERIDIAN SHELL — GPU RENDERER") != std::string::npos);

    std::string hud = profiler.format_live_hud();
    ASSERT_TRUE(hud.find("FPS:") != std::string::npos);
}
