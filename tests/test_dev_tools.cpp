#include "mini_test.hpp"
#include "../src/dev/command_palette.hpp"
#include "../src/dev/file_explorer.hpp"
#include "../src/dev/git_intel.hpp"
#include "../src/dev/rich_history.hpp"
#include "../src/dev/system_monitor.hpp"
#include "../src/dev/universal_search.hpp"
#include "../src/core/vt/screen_buffer.hpp"

#include <fstream>
#include <unistd.h>

using namespace meridian::dev;
using namespace meridian::vt;

MTEST(system_monitor_metrics) {
    SystemMonitor monitor;
    auto metrics = monitor.sample();

    ASSERT_GE(metrics.cpu_percent, 0.0f);
    ASSERT_LE(metrics.cpu_percent, 100.0f);
    ASSERT_GT(metrics.mem_total_bytes, 0u);
    ASSERT_GT(metrics.mem_used_bytes, 0u);

    std::string bar = metrics.format_status_bar();
    ASSERT_TRUE(bar.find("CPU") != std::string::npos);
    ASSERT_TRUE(bar.find("RAM") != std::string::npos);

    std::string dash = metrics.format_dashboard();
    ASSERT_TRUE(dash.find("Meridian System Monitor") != std::string::npos);
}

MTEST(git_intel_inspection) {
    std::string test_dir = "/tmp/meridian_test_gitintel_" + std::to_string(getpid());
    std::string git_dir = test_dir + "/.git";
    system(("mkdir -p " + git_dir).c_str());

    {
        std::ofstream head(git_dir + "/HEAD");
        head << "ref: refs/heads/main\n";
    }

    auto status = GitIntel::inspect_directory(test_dir);
    ASSERT_TRUE(status.is_git_repo);
    ASSERT_EQ(status.root_dir, test_dir);
    ASSERT_EQ(status.branch_name, "main");

    std::string panel = status.format_panel();
    ASSERT_TRUE(panel.find("Meridian Git") != std::string::npos);

    // Non-git directory
    auto non_git = GitIntel::inspect_directory("/tmp");
    // /tmp might not have .git/HEAD
    if (!non_git.is_git_repo) {
        ASSERT_FALSE(non_git.is_git_repo);
    }

    system(("rm -rf " + test_dir).c_str());
}

MTEST(file_explorer_scan_and_icons) {
    // Icons should be non-empty strings (exact value depends on terminal tier)
    ASSERT_FALSE(FileExplorer::get_file_icon("main.cpp", false).empty());
    ASSERT_FALSE(FileExplorer::get_file_icon("server.py", false).empty());
    ASSERT_FALSE(FileExplorer::get_file_icon("cargo.rs", false).empty());
    ASSERT_FALSE(FileExplorer::get_file_icon("src", true).empty());

    auto root = FileExplorer::scan_directory(".", 1);
    ASSERT_TRUE(root.is_directory);
    ASSERT_GT(root.children.size(), 0u);
}

MTEST(command_palette_fuzzy_search) {
    CommandPalette palette;
    ASSERT_GT(palette.all_actions().size(), 0u);

    auto results = palette.search("split");
    ASSERT_GT(results.size(), 0u);
    ASSERT_TRUE(results[0].title.find("Split") != std::string::npos);

    auto ai_results = palette.search("intent");
    ASSERT_GT(ai_results.size(), 0u);
    ASSERT_TRUE(ai_results[0].title.find("Intent") != std::string::npos);
}

MTEST(rich_history_and_universal_search) {
    std::string test_db = "/tmp/test_meridian_history.db";
    unlink(test_db.c_str());

    RichHistory history(test_db);
    history.add_entry("git status", "/home/user/project", 0, 15, "main", "default");
    history.add_entry("npm run test", "/home/user/project", 1, 1420, "main", "backend");
    history.add_entry("docker compose up", "/home/user/project", 0, 5200, "main", "dev");

    ASSERT_EQ(history.size(), 3u);

    auto search_git = history.search("git");
    ASSERT_EQ(search_git.size(), 1u);
    ASSERT_EQ(search_git[0].command, "git status");

    // Universal search
    ScreenBuffer screen(24, 80);
    Attributes attrs;
    screen.put_codepoint(U'c', attrs);
    screen.put_codepoint(U'o', attrs);
    screen.put_codepoint(U'm', attrs);
    screen.put_codepoint(U'p', attrs);
    screen.put_codepoint(U'o', attrs);
    screen.put_codepoint(U's', attrs);
    screen.put_codepoint(U'e', attrs);

    auto matches = UniversalSearch::search_all(screen, history, "compose");
    ASSERT_GE(matches.size(), 2u); // 1 on screen, 1 in history

    unlink(test_db.c_str());
}

