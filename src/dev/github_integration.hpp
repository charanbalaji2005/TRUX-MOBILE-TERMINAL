#pragma once
// src/dev/github_integration.hpp
//
// Meridian Developer Experience — GitHub Smart Integration:
//  - 1-Click SSH key management and Port 443 fallback (without overwriting existing keys)
//  - Non-blocking, configurable Git lifecycle hints for 'git init' and 'git clone'
//  - GitHub account authorization status and repository linkage

#include <string>
#include <vector>

namespace meridian::dev {

class GitHubIntegration {
public:
    static int handle_gh_command(const std::vector<std::string>& argv);
    static void on_post_command_hint(const std::string& raw_cmd, int exit_code);
    static bool is_hints_enabled();
    static void set_hints_enabled(bool enabled);
};

} // namespace meridian::dev

