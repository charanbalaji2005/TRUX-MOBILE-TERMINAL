#pragma once
// meridian-terminal / dev / ssh_manager.hpp
//
// Native SSH Workspace and Connection Manager for Meridian Terminal.
// Parses ~/.ssh/config, manages saved host profiles, handles connection
// strings, and integrates into the Command Palette and interactive CLI.

#include <string>
#include <vector>
#include <optional>

namespace meridian::dev {

struct SSHHost {
    std::string alias;
    std::string hostname;
    std::string user;
    int port = 22;
    std::string identity_file;
    std::string notes;

    std::string command_line() const {
        std::string cmd = "ssh";
        if (port != 22) cmd += " -p " + std::to_string(port);
        if (!identity_file.empty()) cmd += " -i " + identity_file;
        if (!user.empty()) cmd += " " + user + "@" + hostname;
        else cmd += " " + hostname;
        return cmd;
    }
};

class SSHManager {
public:
    explicit SSHManager(std::string config_path = "");

    // Reload hosts from ~/.ssh/config and Meridian SSH database
    bool load();

    // Query available hosts
    const std::vector<SSHHost>& list_hosts() const { return hosts_; }
    std::optional<SSHHost> find_host(const std::string& query) const;

    // Add or update host configuration
    bool add_host(const SSHHost& host);
    bool remove_host(const std::string& alias);

    // Formatted workspace overview
    std::string format_overview() const;

private:
    std::string config_path_;
    std::vector<SSHHost> hosts_;

    void parse_ssh_config(const std::string& content);
};

} // namespace meridian::dev
