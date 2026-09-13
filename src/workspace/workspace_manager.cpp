#include "workspace_manager.hpp"

#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace meridian::workspace {

namespace {

void ensure_dir(const std::string& path) {
    if (path.empty()) return;
    std::string partial;
    std::size_t start = (path[0] == '/') ? 1 : 0;
    if (path[0] == '/') partial = "/";
    std::size_t pos = start;
    while (pos <= path.size()) {
        auto slash = path.find('/', pos);
        std::string comp = path.substr(pos, slash == std::string::npos ? std::string::npos : slash - pos);
        if (!comp.empty()) {
            partial += comp;
            mkdir(partial.c_str(), 0755);
            partial += "/";
        }
        if (slash == std::string::npos) break;
        pos = slash + 1;
    }
}

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

} // namespace

std::string WorkspaceManager::default_storage_dir() {
    if (const char* override_dir = std::getenv("MERIDIAN_CONFIG_HOME")) {
        return std::string(override_dir) + "/workspaces";
    }
    const char* home = std::getenv("HOME");
    std::string base = home ? home : ".";
    return base + "/.config/meridian/workspaces";
}

WorkspaceManager::WorkspaceManager(std::string storage_dir)
    : storage_dir_(storage_dir.empty() ? default_storage_dir() : std::move(storage_dir)) {}

std::string WorkspaceManager::workspace_file_path(const std::string& name) const {
    return storage_dir_ + "/" + name + ".workspace";
}

bool WorkspaceManager::save_workspace(const Workspace& ws) {
    if (ws.name.empty()) return false;
    ensure_dir(storage_dir_);

    std::string path = workspace_file_path(ws.name);
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) return false;

    out << "# Meridian Workspace Definition\n";
    out << "name = \"" << ws.name << "\"\n";
    out << "root_dir = \"" << ws.root_dir << "\"\n";
    out << "git_branch = \"" << ws.git_branch << "\"\n";
    out << "active_pane = " << ws.active_pane_id << "\n";

    out << "\n[environment]\n";
    for (const auto& [k, v] : ws.environment_vars) {
        out << k << " = \"" << v << "\"\n";
    }

    out << "\n[panes]\n";
    for (const auto& p : ws.panes) {
        out << "pane." << p.id << ".title = \"" << p.title << "\"\n";
        out << "pane." << p.id << ".cwd = \"" << p.cwd << "\"\n";
        out << "pane." << p.id << ".command = \"" << p.command << "\"\n";
        out << "pane." << p.id << ".split = " << (p.split_direction == SplitDirection::Vertical ? "vertical" : "horizontal") << "\n";
        out << "pane." << p.id << ".ratio = " << p.split_ratio << "\n";
        out << "pane." << p.id << ".pinned = " << (p.is_pinned ? "true" : "false") << "\n";
    }

    return out.good();
}

std::optional<Workspace> WorkspaceManager::load_workspace(const std::string& name) const {
    std::string path = workspace_file_path(name);
    std::ifstream in(path);
    if (!in.is_open()) return std::nullopt;

    Workspace ws;
    ws.name = name;

    std::string line;
    std::string current_section;
    std::map<uint32_t, WorkspacePaneConfig> pane_map;

    while (std::getline(in, line)) {
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') continue;

        if (t.front() == '[' && t.back() == ']') {
            current_section = t.substr(1, t.size() - 2);
            continue;
        }

        auto eq = t.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(t.substr(0, eq));
        std::string val = trim(t.substr(eq + 1));
        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
            val = val.substr(1, val.size() - 2);
        }

        if (current_section == "environment") {
            ws.environment_vars[key] = val;
        } else if (current_section == "panes" || key.rfind("pane.", 0) == 0) {
            // parse pane.<id>.<prop>
            std::string sub = key.substr(5);
            auto dot = sub.find('.');
            if (dot != std::string::npos) {
                uint32_t pid = std::stoul(sub.substr(0, dot));
                std::string prop = sub.substr(dot + 1);
                auto& p = pane_map[pid];
                p.id = pid;
                if (prop == "title") p.title = val;
                else if (prop == "cwd") p.cwd = val;
                else if (prop == "command") p.command = val;
                else if (prop == "split") p.split_direction = (val == "horizontal") ? SplitDirection::Horizontal : SplitDirection::Vertical;
                else if (prop == "ratio") p.split_ratio = std::stof(val);
                else if (prop == "pinned") p.is_pinned = (val == "true" || val == "1");
            }
        } else {
            if (key == "name") ws.name = val;
            else if (key == "root_dir") ws.root_dir = val;
            else if (key == "git_branch") ws.git_branch = val;
            else if (key == "active_pane") ws.active_pane_id = std::stoul(val);
        }
    }

    for (const auto& [_, p] : pane_map) {
        ws.panes.push_back(p);
    }
    if (ws.panes.empty()) {
        WorkspacePaneConfig p;
        p.id = 1;
        p.cwd = ws.root_dir;
        ws.panes.push_back(p);
    }

    return ws;
}

std::vector<std::string> WorkspaceManager::list_workspaces() const {
    std::vector<std::string> results;
    DIR* d = opendir(storage_dir_.c_str());
    if (!d) return results;

    struct dirent* ent;
    while ((ent = readdir(d)) != nullptr) {
        std::string fname = ent->d_name;
        if (fname.size() > 10 && fname.substr(fname.size() - 10) == ".workspace") {
            results.push_back(fname.substr(0, fname.size() - 10));
        }
    }
    closedir(d);
    std::sort(results.begin(), results.end());
    return results;
}

bool WorkspaceManager::delete_workspace(const std::string& name) {
    std::string path = workspace_file_path(name);
    return unlink(path.c_str()) == 0;
}

Workspace WorkspaceManager::capture_live_workspace(
    const std::string& name,
    const std::string& root_dir,
    const std::string& git_branch,
    const PaneTree& tree,
    const std::map<std::string, std::string>& env
) {
    Workspace ws;
    ws.name = name;
    ws.root_dir = root_dir;
    ws.git_branch = git_branch;
    ws.environment_vars = env;
    ws.active_pane_id = tree.active_pane_id();

    auto layouts = tree.compute_layout(100, 100);
    for (const auto& item : layouts) {
        WorkspacePaneConfig p;
        p.id = item.pane_id;
        p.title = item.title;
        p.cwd = item.cwd;
        p.is_pinned = item.is_pinned;
        ws.panes.push_back(p);
    }

    return ws;
}

std::unique_ptr<PaneTree> WorkspaceManager::restore_to_pane_tree(const Workspace& ws) {
    auto tree = std::make_unique<PaneTree>();
    if (ws.panes.empty()) return tree;

    // First pane is root
    uint32_t first_id = ws.panes[0].id;
    for (std::size_t i = 1; i < ws.panes.size(); ++i) {
        const auto& p = ws.panes[i];
        tree->split_pane(first_id, p.split_direction, p.split_ratio, p.cwd);
    }
    tree->set_active_pane(ws.active_pane_id);
    return tree;
}

} // namespace meridian::workspace

