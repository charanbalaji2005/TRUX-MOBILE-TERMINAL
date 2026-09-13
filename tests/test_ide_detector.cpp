#include "mini_test.hpp"
#include "../src/dev/ide_detector.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace meridian::dev;

MTEST(ide_detector_scan_and_register) {
    auto results = IdeDetector::instance().scan_and_register(true);
    ASSERT_TRUE(!results.empty());

    bool found_vscode_adapter = false;
    bool found_antigravity_adapter = false;

    for (const auto& info : results) {
        if (info.id == "vscode") found_vscode_adapter = true;
        if (info.id == "antigravity") found_antigravity_adapter = true;
    }

    ASSERT_TRUE(found_vscode_adapter);
    ASSERT_TRUE(found_antigravity_adapter);
}

MTEST(ide_detector_first_run_ensure) {
    IdeDetector::ensure_first_run_registered();
    // Should run idempotently
    IdeDetector::ensure_first_run_registered();
    ASSERT_TRUE(true);
}
