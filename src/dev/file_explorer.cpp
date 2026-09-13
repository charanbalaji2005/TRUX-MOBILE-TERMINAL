#include "file_explorer.hpp"
#include "icon_provider.hpp"

#include <algorithm>
#include <dirent.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace meridian::dev {

std::string FileExplorer::get_file_icon(const std::string& filename, bool is_dir) {
    bool is_exec = false;
    if (!is_dir) {
        is_exec = (access(filename.c_str(), X_OK) == 0);
    }
    return IconProvider::icon_for_file(filename, is_dir, is_exec, ' ');
}

FileNode FileExplorer::scan_directory(const std::string& root_path, int max_depth) {
    FileNode root;
    auto last_slash = root_path.find_last_of('/');
    root.name = (last_slash != std::string::npos) ? root_path.substr(last_slash + 1) : root_path;
    root.path = root_path;
    root.is_directory = true;

    if (max_depth <= 0) return root;

    DIR* d = opendir(root_path.c_str());
    if (!d) return root;

    struct dirent* ent;
    while ((ent = readdir(d)) != nullptr) {
        std::string fname = ent->d_name;
        if (fname == "." || fname == ".." || fname == ".git" || fname == "build") continue;

        std::string full = root_path + "/" + fname;
        struct stat st;
        if (stat(full.c_str(), &st) != 0) continue;

        bool is_d = S_ISDIR(st.st_mode);
        if (is_d) {
            root.children.push_back(scan_directory(full, max_depth - 1));
        } else {
            FileNode leaf;
            leaf.name = fname;
            leaf.path = full;
            leaf.is_directory = false;
            leaf.size_bytes = st.st_size;
            root.children.push_back(leaf);
        }
    }
    closedir(d);

    // Sort: directories first, then alphabetical
    std::sort(root.children.begin(), root.children.end(), [](const FileNode& a, const FileNode& b) {
        if (a.is_directory != b.is_directory) return a.is_directory > b.is_directory;
        return a.name < b.name;
    });

    return root;
}

std::string FileExplorer::format_tree(const FileNode& node, int indent) {
    std::ostringstream ss;
    std::string prefix(indent * 2, ' ');

    ss << prefix << get_file_icon(node.name, node.is_directory) << " " << node.name;
    if (node.git_status != ' ') {
        ss << " [" << node.git_status << "]";
    }
    ss << "\n";

    for (const auto& ch : node.children) {
        ss << format_tree(ch, indent + 1);
    }
    return ss.str();
}

} // namespace meridian::dev

