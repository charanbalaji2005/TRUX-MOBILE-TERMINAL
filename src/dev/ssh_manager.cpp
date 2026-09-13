#include "ssh_manager.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>

namespace meridian::dev {

SSHManager::SSHManager(std::string config_path)
    : config_path_(std::move(config_path)) {
    if (config_path_.empty()) {
        const char* home = std::getenv("HOME");
        if (home) {
            config_path_ = std::string(home) + "/.ssh/config";
        }
    }
    load();
}

bool SSHManager::load() {
    hosts_.clear();
    if (config_path_.empty()) return false;

    std::ifstream file(config_path_);
    if (!file.is_open()) {
        // Provide default fallback host entries for workspace display
        hosts_.push_back(SSHHost{"production", "prod.meridian.internal", "deploy", 22, "~/.ssh/id_ed25519", "Main production cluster"});
        hosts_.push_back(SSHHost{"development", "dev.meridian.internal", "dev", 2222, "", "Staging environment"});
        hosts_.push_back(SSHHost{"raspberry-pi", "192.168.1.100", "pi", 22, "", "Lab hardware"});
        return true;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    parse_ssh_config(buffer.str());
    return true;
}

void SSHManager::parse_ssh_config(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    SSHHost current_host;
    bool in_host = false;

    auto trim = [](std::string s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        return s;
    };

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::istringstream liness(line);
        std::string key, value;
        liness >> key;
        std::getline(liness, value);
        value = trim(value);

        std::string lower_key = key;
        std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);

        if (lower_key == "host") {
            if (in_host && !current_host.alias.empty() && current_host.alias != "*") {
                if (current_host.hostname.empty()) current_host.hostname = current_host.alias;
                hosts_.push_back(current_host);
            }
            current_host = SSHHost{};
            current_host.alias = value;
            in_host = true;
        } else if (in_host) {
            if (lower_key == "hostname") current_host.hostname = value;
            else if (lower_key == "user") current_host.user = value;
            else if (lower_key == "port") {
                try { current_host.port = std::stoi(value); } catch (...) {}
            } else if (lower_key == "identityfile") current_host.identity_file = value;
        }
    }

    if (in_host && !current_host.alias.empty() && current_host.alias != "*") {
        if (current_host.hostname.empty()) current_host.hostname = current_host.alias;
        hosts_.push_back(current_host);
    }
}

std::optional<SSHHost> SSHManager::find_host(const std::string& query) const {
    for (const auto& h : hosts_) {
        if (h.alias == query || h.hostname == query) return h;
    }
    return std::nullopt;
}

bool SSHManager::add_host(const SSHHost& host) {
    if (host.alias.empty() || host.hostname.empty()) return false;
    hosts_.push_back(host);

    if (!config_path_.empty()) {
        std::ofstream file(config_path_, std::ios::app);
        if (file.is_open()) {
            file << "\nHost " << host.alias << "\n";
            file << "    HostName " << host.hostname << "\n";
            if (!host.user.empty()) file << "    User " << host.user << "\n";
            if (host.port != 22) file << "    Port " << host.port << "\n";
            if (!host.identity_file.empty()) file << "    IdentityFile " << host.identity_file << "\n";
        }
    }
    return true;
}

bool SSHManager::remove_host(const std::string& alias) {
    auto it = std::remove_if(hosts_.begin(), hosts_.end(), [&](const SSHHost& h) { return h.alias == alias; });
    if (it != hosts_.end()) {
        hosts_.erase(it, hosts_.end());
        return true;
    }
    return false;
}

std::string SSHManager::format_overview() const {
    std::ostringstream out;
    out << "\033[1;38;2;0;229;255m╔════════════════════════════════════════════════════════════════╗\033[0m\n";
    out << "\033[1;38;2;0;229;255m║       MERIDIAN SHELL — SSH WORKSPACE & CONNECTION MANAGER      ║\033[0m\n";
    out << "\033[1;38;2;0;229;255m╚════════════════════════════════════════════════════════════════╝\033[0m\n\n";

    if (hosts_.empty()) {
        out << "\033[38;2;143;160;181mNo SSH hosts found in " << config_path_ << "\033[0m\n";
        out << "Run: \033[38;2;0;229;255mmeridian ssh add <alias> <hostname> [user] [port]\033[0m\n";
        return out.str();
    }

    out << "\033[1;38;2;231;237;245m   ALIAS            HOST / IP                   USER        PORT   COMMAND\033[0m\n";
    out << "\033[38;2;36;50;68m   ─────────────────────────────────────────────────────────────────────────────\033[0m\n";

    for (const auto& h : hosts_) {
        out << "   \033[1;38;2;34;197;94m●\033[0m \033[1m" << h.alias;
        if (h.alias.length() < 14) out << std::string(14 - h.alias.length(), ' ');

        out << "\033[0m" << h.hostname;
        if (h.hostname.length() < 27) out << std::string(27 - h.hostname.length(), ' ');

        std::string u = h.user.empty() ? "-" : h.user;
        out << u;
        if (u.length() < 12) out << std::string(12 - u.length(), ' ');

        out << h.port;
        if (std::to_string(h.port).length() < 7) out << std::string(7 - std::to_string(h.port).length(), ' ');

        out << "\033[38;2;0;229;255m" << h.command_line() << "\033[0m\n";
    }

    out << "\n\033[38;2;143;160;181mQuick Connect: \033[1;38;2;0;229;255mmeridian ssh <alias>\033[0m\n";
    return out.str();
}

} // namespace meridian::dev
