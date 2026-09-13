#pragma once
// meridian-terminal / dev / file_explorer.hpp
//
// File explorer sidecar engine. Fast directory tree traversal with
// Git modification status badges, file size formatting, and tree rendering.

#include <cstdint>
#include <string>
#include <vector>

namespace meridian::dev {

struct FileNode {
    std::string name;
    std::string path;
    bool is_directory = false;
    uint64_t size_bytes = 0;
    char git_status = ' '; // 'M', 'A', '?', ' '
    std::vector<FileNode> children;
};

class FileExplorer {
public:
    FileExplorer() = default;

    static FileNode scan_directory(const std::string& root_path, int max_depth = 2);
    static std::string format_tree(const FileNode& node, int indent = 0);
    static std::string get_file_icon(const std::string& filename, bool is_dir);
};

} // namespace meridian::dev

