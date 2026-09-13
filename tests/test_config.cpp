// tests / test_config.cpp
#include "mini_test.hpp"
#include "../src/config/keybindings.hpp"
#include "../src/config/terminal_config.hpp"

#include <cstdio>
#include <unistd.h>

using namespace meridian::config;

MTEST(terminal_config_defaults_without_a_file) {
    TerminalConfig cfg("/tmp/meridian_definitely_missing_terminal_" + std::to_string(getpid()) + ".toml");
    cfg.load(); // file doesn't exist -> defaults apply, no crash
    auto s = cfg.get();
    ASSERT_EQ(s.scrollback_lines, 10000);
    ASSERT_EQ(s.default_shell, std::string("/bin/bash"));
    ASSERT_TRUE(s.cursor_blink);
}

MTEST(terminal_config_round_trips_custom_values) {
    std::string path = "/tmp/meridian_test_terminal_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    {
        TerminalConfig cfg(path);
        auto s = cfg.get();
        s.scrollback_lines = 50000;
        s.default_shell = "/usr/bin/zsh";
        s.font_size = 16;
        s.cursor_blink = false;
        cfg.set(s);
        ASSERT_TRUE(cfg.save());
    }
    {
        TerminalConfig cfg(path);
        cfg.load();
        auto s = cfg.get();
        ASSERT_EQ(s.scrollback_lines, 50000);
        ASSERT_EQ(s.default_shell, std::string("/usr/bin/zsh"));
        ASSERT_EQ(s.font_size, 16);
        ASSERT_FALSE(s.cursor_blink);
    }
    std::remove(path.c_str());
}

MTEST(keybindings_returns_spec_defaults_when_unset) {
    Keybindings kb("/tmp/meridian_definitely_missing_keybindings_" + std::to_string(getpid()) + ".toml");
    ASSERT_EQ(kb.get("ai_popup"), std::string("Ctrl+Space"));
    ASSERT_EQ(kb.get("search"), std::string("Ctrl+Shift+F"));
}

MTEST(keybindings_override_persists) {
    std::string path = "/tmp/meridian_test_keybindings_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    {
        Keybindings kb(path);
        kb.set("ai_popup", "Alt+A");
        ASSERT_TRUE(kb.save());
    }
    {
        Keybindings kb(path);
        kb.load();
        ASSERT_EQ(kb.get("ai_popup"), std::string("Alt+A"));
        // Unrelated actions still fall back to their defaults.
        ASSERT_EQ(kb.get("search"), std::string("Ctrl+Shift+F"));
    }
    std::remove(path.c_str());
}

MTEST(keybindings_all_lists_every_default_action) {
    Keybindings kb("/tmp/meridian_definitely_missing_kb2_" + std::to_string(getpid()) + ".toml");
    auto all = kb.all();
    ASSERT_TRUE(all.count("ai_popup") == 1);
    ASSERT_TRUE(all.count("new_tab") == 1);
    ASSERT_TRUE(all.size() >= 10);
}
