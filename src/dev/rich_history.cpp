#include "rich_history.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace meridian::dev {

namespace {

void ensure_parent_dir(const std::string& path) {
    auto slash = path.find_last_of('/');
    if (slash == std::string::npos) return;
    std::string dir = path.substr(0, slash);
    mkdir(dir.c_str(), 0755);
}

} // namespace

std::string RichHistory::default_db_path() {
    if (const char* override_dir = std::getenv("MERIDIAN_CONFIG_HOME")) {
        return std::string(override_dir) + "/history.db";
    }
    const char* home = std::getenv("HOME");
    std::string base = home ? home : ".";
    return base + "/.config/meridian/history.db";
}

RichHistory::RichHistory(std::string db_path)
    : db_path_(db_path.empty() ? default_db_path() : std::move(db_path)) {
    load();
}

void RichHistory::add_entry(
    const std::string& command,
    const std::string& working_dir,
    int exit_code,
    uint64_t duration_ms,
    const std::string& git_branch,
    const std::string& workspace_name
) {
    if (command.empty()) return;

    HistoryRecord r;
    r.id = next_id_++;
    auto now = std::chrono::system_clock::now();
    r.timestamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count()
    );
    r.command = command;
    r.working_dir = working_dir;
    r.exit_code = exit_code;
    r.duration_ms = duration_ms;
    r.git_branch = git_branch;
    r.workspace_name = workspace_name;

    records_.push_back(std::move(r));
    save();
}

std::vector<HistoryRecord> RichHistory::search(const std::string& query, std::size_t limit) const {
    std::vector<HistoryRecord> results;
    for (auto it = records_.rbegin(); it != records_.rend(); ++it) {
        if (query.empty() || it->command.find(query) != std::string::npos || it->working_dir.find(query) != std::string::npos) {
            results.push_back(*it);
            if (results.size() >= limit) break;
        }
    }
    return results;
}

std::vector<HistoryRecord> RichHistory::recent(std::size_t limit) const {
    return search("", limit);
}

bool RichHistory::save() const {
    ensure_parent_dir(db_path_);
    std::ofstream out(db_path_, std::ios::trunc);
    if (!out.is_open()) return false;

    out << "# Meridian Rich Command History DB\n";
    for (const auto& r : records_) {
        out << r.id << "|"
            << r.timestamp << "|"
            << r.exit_code << "|"
            << r.duration_ms << "|"
            << r.working_dir << "|"
            << r.git_branch << "|"
            << r.workspace_name << "|"
            << r.command << "\n";
    }
    return out.good();
}

bool RichHistory::load() {
    std::ifstream in(db_path_);
    if (!in.is_open()) return false;

    records_.clear();
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string sid, stime, sexit, sdur, scwd, sbranch, sws, scmd;

        if (std::getline(ss, sid, '|') &&
            std::getline(ss, stime, '|') &&
            std::getline(ss, sexit, '|') &&
            std::getline(ss, sdur, '|') &&
            std::getline(ss, scwd, '|') &&
            std::getline(ss, sbranch, '|') &&
            std::getline(ss, sws, '|') &&
            std::getline(ss, scmd)) {
            HistoryRecord r;
            r.id = std::stoull(sid);
            r.timestamp = std::stoull(stime);
            r.exit_code = std::stoi(sexit);
            r.duration_ms = std::stoull(sdur);
            r.working_dir = scwd;
            r.git_branch = sbranch;
            r.workspace_name = sws;
            r.command = scmd;
            next_id_ = std::max(next_id_, r.id + 1);
            records_.push_back(r);
        }
    }
    return true;
}

} // namespace meridian::dev

