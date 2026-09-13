#pragma once
// meridian-terminal / plugins / plugin_manager.hpp
//
// Extensible Plugin Engine for Meridian Shell.
// Provides lifecycle management, command execution hooks, terminal UI overlays,
// and third-party developer extensibility.

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace meridian::plugins {

class MeridianPlugin {
public:
    virtual ~MeridianPlugin() = default;

    virtual std::string name() const = 0;
    virtual std::string version() const = 0;
    virtual std::string description() const = 0;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // Execution lifecycle hooks
    virtual void on_command_pre_exec(const std::string& /*cmd*/) {}
    virtual void on_command_post_exec(const std::string& /*cmd*/, int /*exit_code*/) {}
    virtual void on_directory_changed(const std::string& /*new_dir*/) {}
};

class PluginManager {
public:
    static PluginManager& instance();

    void register_plugin(std::shared_ptr<MeridianPlugin> plugin);
    bool unregister_plugin(const std::string& name);

    const std::vector<std::shared_ptr<MeridianPlugin>>& plugins() const { return plugins_; }
    std::shared_ptr<MeridianPlugin> find_plugin(const std::string& name) const;

    // Dispatch hooks to all active plugins
    void notify_command_pre_exec(const std::string& cmd);
    void notify_command_post_exec(const std::string& cmd, int exit_code);
    void notify_directory_changed(const std::string& new_dir);

    // Formatted list for CLI and Command Palette
    std::string format_plugin_list() const;

private:
    PluginManager();
    void load_builtin_plugins();

    std::vector<std::shared_ptr<MeridianPlugin>> plugins_;
};

} // namespace meridian::plugins
