#pragma once
// meridian-terminal / workspace / workspace_manager.hpp
//
// Persistent workspace management engine. Saves and restores complete
// multi-pane development environments (panes, working directories,
// environment variables, Git branches, and running process configurations).

#include "pane_tree.hpp"

#include <map>
#include <string>
#include <vector>

namespace meridian::workspace {

struct WorkspacePaneConfig {
    uint32_t id = 1;
    std::string title = "Terminal";
    std::string cwd;
    std::string command;
    SplitDirection split_direction = SplitDirection::Vertical;
    float split_ratio = 0.5f;
    bool is_pinned = false;
};

struct Workspace {
    std::string name;
    std::string root_dir;
    std::string git_branch;
    std::map<std::string, std::string> environment_vars;
    std::vector<WorkspacePaneConfig> panes;
    uint32_t active_pane_id = 1;
    std::string created_at;
};

class WorkspaceManager {
public:
    explicit WorkspaceManager(std::string storage_dir = "");

    std::string storage_dir() const { return storage_dir_; }

    bool save_workspace(const Workspace& ws);
    std::optional<Workspace> load_workspace(const std::string& name) const;
    std::vector<std::string> list_workspaces() const;
    bool delete_workspace(const std::string& name);

    // Create a workspace snapshot from a live PaneTree
    static Workspace capture_live_workspace(
        const std::string& name,
        const std::string& root_dir,
        const std::string& git_branch,
        const PaneTree& tree,
        const std::map<std::string, std::string>& env = {}
    );

    // Reconstruct a PaneTree from a saved workspace
    static std::unique_ptr<PaneTree> restore_to_pane_tree(const Workspace& ws);

private:
    std::string storage_dir_;
    static std::string default_storage_dir();
    std::string workspace_file_path(const std::string& name) const;
};

} // namespace meridian::workspace

