#pragma once
// src/dev/platform_manager.hpp
//
// Meridian 2.5 Platform Integration & Lifecycle Subsystems:
//  - VS Code Terminal Auto-Detection & Profile Management
//  - Prebuilt Release Update Engine (GitHub Releases & SemVer)
//  - Anonymous Usage Statistics & Yearly Growth Analytics
//  - Opt-in Privacy-Preserving Telemetry Management

#include <string>
#include <vector>

namespace meridian::dev {

class PlatformManager {
public:
    static int handle_vscode(const std::vector<std::string>& argv);
    static int handle_update(const std::vector<std::string>& argv);
    static int handle_stats(const std::vector<std::string>& argv);
    static int handle_telemetry(const std::vector<std::string>& argv);
};

} // namespace meridian::dev

