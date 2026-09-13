#include "mini_test.hpp"
#include "../src/dev/platform_manager.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace meridian::dev;

MTEST(platform_manager_vscode_integration) {
    // Redirect std::cout during test
    std::streambuf* orig_buf = std::cout.rdbuf();
    std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    int r1 = PlatformManager::handle_vscode({"vscode", "status"});
    ASSERT_EQ(r1, 0);
    ASSERT_TRUE(capture.str().find("Meridian IDE & VS Code Integration") != std::string::npos || capture.str().find("Meridian") != std::string::npos);

    capture.str("");
    int r2 = PlatformManager::handle_vscode({"vscode", "enable"});
    ASSERT_EQ(r2, 0);
    ASSERT_TRUE(capture.str().find("Meridian Terminal successfully configured") != std::string::npos);

    capture.str("");
    int r3 = PlatformManager::handle_vscode({"vscode", "disable"});
    ASSERT_EQ(r3, 0);
    ASSERT_TRUE(capture.str().find("reset to standard bash") != std::string::npos);

    // Leave Meridian enabled for the user
    PlatformManager::handle_vscode({"vscode", "enable"});

    std::cout.rdbuf(orig_buf);
}

MTEST(platform_manager_stats_reporting) {
    std::streambuf* orig_buf = std::cout.rdbuf();
    std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    int r1 = PlatformManager::handle_stats({"stats"});
    ASSERT_EQ(r1, 0);
    ASSERT_TRUE(capture.str().find("GitHub Live Release Downloads") != std::string::npos || capture.str().find("Meridian") != std::string::npos);
    ASSERT_TRUE(capture.str().find("Linux Packages") != std::string::npos || capture.str().find("Total Release Downloads") != std::string::npos);

    capture.str("");
    int r2 = PlatformManager::handle_stats({"stats", "--year", "2026"});
    ASSERT_EQ(r2, 0);
    ASSERT_TRUE(capture.str().find("Meridian Usage — 2026") != std::string::npos);
    ASSERT_TRUE(capture.str().find("Linux") != std::string::npos);

    capture.str("");
    int r3 = PlatformManager::handle_stats({"stats", "--growth"});
    ASSERT_EQ(r3, 0);
    ASSERT_TRUE(capture.str().find("Meridian Growth & Adoption") != std::string::npos);
    ASSERT_TRUE(capture.str().find("+384%") != std::string::npos);

    std::cout.rdbuf(orig_buf);
}

MTEST(platform_manager_telemetry_and_updates) {
    std::streambuf* orig_buf = std::cout.rdbuf();
    std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    int r1 = PlatformManager::handle_telemetry({"telemetry", "status"});
    ASSERT_EQ(r1, 0);
    ASSERT_TRUE(capture.str().find("Meridian Telemetry & Privacy Center") != std::string::npos);

    capture.str("");
    int r2 = PlatformManager::handle_telemetry({"telemetry", "enable"});
    ASSERT_EQ(r2, 0);
    ASSERT_TRUE(capture.str().find("Anonymous telemetry enabled") != std::string::npos);

    capture.str("");
    int r3 = PlatformManager::handle_telemetry({"telemetry", "disable"});
    ASSERT_EQ(r3, 0);
    ASSERT_TRUE(capture.str().find("Telemetry completely disabled") != std::string::npos);

    capture.str("");
    int r4 = PlatformManager::handle_update({"update", "--check"});
    ASSERT_EQ(r4, 0);
    ASSERT_TRUE(capture.str().find("Meridian Update Engine") != std::string::npos);
    ASSERT_TRUE(capture.str().find("2.5") != std::string::npos);

    std::cout.rdbuf(orig_buf);
}

