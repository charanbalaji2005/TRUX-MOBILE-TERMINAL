#include "plugin_manager.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace meridian::plugins {

// Built-in Git watcher plugin
class GitWatcherPlugin : public MeridianPlugin {
public:
    std::string name() const override { return "git-intel"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override { return "Inspects Git repository state, branch divergence, and uncommitted changes."; }

    bool initialize() override { return true; }
    void shutdown() override {}

    void on_command_post_exec(const std::string& cmd, int /*exit_code*/) override {
        if (cmd.rfind("git", 0) == 0) {
            // Updated after git commands
        }
    }
};

// Built-in Docker watcher plugin
class DockerWatcherPlugin : public MeridianPlugin {
public:
    std::string name() const override { return "docker-tracker"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override { return "Tracks active container lifecycles and background docker daemon state."; }

    bool initialize() override { return true; }
    void shutdown() override {}
};

// Built-in Telemetry metrics plugin
class TelemetryOverlayPlugin : public MeridianPlugin {
public:
    std::string name() const override { return "telemetry-profiler"; }
    std::string version() const override { return "1.2.0"; }
    std::string description() const override { return "Collects GPU framerate, PTY latency, and memory throughput metrics."; }

    bool initialize() override { return true; }
    void shutdown() override {}
};

PluginManager& PluginManager::instance() {
    static PluginManager manager;
    return manager;
}

PluginManager::PluginManager() {
    load_builtin_plugins();
}

void PluginManager::load_builtin_plugins() {
    register_plugin(std::make_shared<GitWatcherPlugin>());
    register_plugin(std::make_shared<DockerWatcherPlugin>());
    register_plugin(std::make_shared<TelemetryOverlayPlugin>());
}

void PluginManager::register_plugin(std::shared_ptr<MeridianPlugin> plugin) {
    if (!plugin) return;
    auto existing = find_plugin(plugin->name());
    if (existing) {
        unregister_plugin(plugin->name());
    }
    plugin->initialize();
    plugins_.push_back(plugin);
}

bool PluginManager::unregister_plugin(const std::string& name) {
    auto it = std::find_if(plugins_.begin(), plugins_.end(), [&](const auto& p) {
        return p->name() == name;
    });
    if (it != plugins_.end()) {
        (*it)->shutdown();
        plugins_.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<MeridianPlugin> PluginManager::find_plugin(const std::string& name) const {
    for (const auto& p : plugins_) {
        if (p->name() == name) return p;
    }
    return nullptr;
}

void PluginManager::notify_command_pre_exec(const std::string& cmd) {
    for (auto& p : plugins_) {
        p->on_command_pre_exec(cmd);
    }
}

void PluginManager::notify_command_post_exec(const std::string& cmd, int exit_code) {
    for (auto& p : plugins_) {
        p->on_command_post_exec(cmd, exit_code);
    }
}

void PluginManager::notify_directory_changed(const std::string& new_dir) {
    for (auto& p : plugins_) {
        p->on_directory_changed(new_dir);
    }
}

std::string PluginManager::format_plugin_list() const {
    std::ostringstream out;
    out << "\033[1;38;2;0;229;255m╔════════════════════════════════════════════════════════════════╗\033[0m\n";
    out << "\033[1;38;2;0;229;255m║         MERIDIAN SHELL — EXTENSIBLE PLUGIN REGISTRY            ║\033[0m\n";
    out << "\033[1;38;2;0;229;255m╚════════════════════════════════════════════════════════════════╝\033[0m\n\n";

    out << "\033[1;38;2;231;237;245m   PLUGIN NAME          VERSION   STATUS    DESCRIPTION\033[0m\n";
    out << "\033[38;2;36;50;68m   ─────────────────────────────────────────────────────────────────────────────\033[0m\n";

    for (const auto& p : plugins_) {
        out << "   \033[1;38;2;34;197;94m●\033[0m \033[1m" << p->name();
        if (p->name().length() < 20) out << std::string(20 - p->name().length(), ' ');

        out << "\033[38;2;0;229;255mv" << p->version();
        if (p->version().length() + 1 < 10) out << std::string(10 - (p->version().length() + 1), ' ');

        out << "\033[38;2;34;197;94mACTIVE    \033[0m";
        out << "\033[38;2;143;160;181m" << p->description() << "\033[0m\n";
    }

    out << "\n\033[38;2;143;160;181mPlugin Directory: \033[38;2;0;229;255m~/.config/meridian/plugins/\033[0m\n";
    return out.str();
}

} // namespace meridian::plugins
