#pragma once
// src/dev/ide_detector.hpp
//
// Meridian Terminal 2.5 — Modular IDE Detection & Adapter Subsystem:
// Automatically scans the host environment for installed IDEs and editors,
// registering Meridian Terminal and Meridian Shell as available integrated
// terminal profiles without requiring user intervention.
//
// Preserves all existing user configurations, custom profiles, and default shells.

#include <string>
#include <vector>
#include <memory>

namespace meridian::dev {

struct IdeInfo {
    std::string id;
    std::string name;
    std::string category;
    bool is_installed = false;
    bool supports_terminal = false;
    bool is_registered = false;
    std::string config_path;
    std::string message;
};

class IdeAdapter {
public:
    virtual ~IdeAdapter() = default;
    virtual std::string id() const = 0;
    virtual std::string name() const = 0;
    virtual IdeInfo detect() = 0;
    virtual bool register_profile(const std::string& shell_bin, const std::string& gui_bin) = 0;
};

class IdeDetector {
public:
    static IdeDetector& instance();

    // Auto-detects and registers Meridian across all installed IDEs
    std::vector<IdeInfo> scan_and_register(bool silent = false);

    // Checks if first-run auto-registration is needed and performs it silently
    static void ensure_first_run_registered();

    // Re-scans and prints status report
    static void print_ide_report();

private:
    IdeDetector();
    std::vector<std::unique_ptr<IdeAdapter>> adapters_;
};

} // namespace meridian::dev
