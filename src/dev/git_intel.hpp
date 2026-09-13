#pragma once
// meridian-terminal / dev / git_intel.hpp
//
// High-speed Git repository intelligence. Inspects local repository status,
// tracks ahead/behind upstream commit divergence, staged/unstaged changes,
// and formats the terminal Git panel.

#include <string>
#include <vector>

namespace meridian::dev {

struct GitFileChange {
    std::string path;
    char index_status = ' ';  // 'M', 'A', 'D', '?'
    char work_status = ' ';   // 'M', 'D', '?'
};

struct GitRepoStatus {
    bool is_git_repo = false;
    std::string root_dir;
    std::string branch_name;
    std::string head_commit_hash;
    int ahead_count = 0;
    int behind_count = 0;
    int staged_count = 0;
    int unstaged_count = 0;
    int untracked_count = 0;
    bool is_clean = true;
    std::vector<GitFileChange> changed_files;

    std::string format_panel() const;
    std::string format_summary_bar() const;
};

class GitIntel {
public:
    GitIntel() = default;

    static GitRepoStatus inspect_directory(const std::string& start_dir = ".");

private:
    static std::string run_git(const std::string& dir, const std::string& args);
};

} // namespace meridian::dev

