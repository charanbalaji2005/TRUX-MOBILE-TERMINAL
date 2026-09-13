#pragma once
// meridian-terminal / dev / rich_history.hpp
//
// Rich command history database. Captures timestamps, working directories,
// exit status codes, execution durations, and Git branch context.

#include <cstdint>
#include <string>
#include <vector>

namespace meridian::dev {

struct HistoryRecord {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    std::string command;
    std::string working_dir;
    int exit_code = 0;
    uint64_t duration_ms = 0;
    std::string git_branch;
    std::string workspace_name;
};

class RichHistory {
public:
    explicit RichHistory(std::string db_path = "");

    void add_entry(
        const std::string& command,
        const std::string& working_dir = ".",
        int exit_code = 0,
        uint64_t duration_ms = 0,
        const std::string& git_branch = "",
        const std::string& workspace_name = ""
    );

    std::vector<HistoryRecord> search(const std::string& query, std::size_t limit = 20) const;
    std::vector<HistoryRecord> recent(std::size_t limit = 20) const;

    std::size_t size() const { return records_.size(); }
    const std::vector<HistoryRecord>& all_records() const { return records_; }

    bool save() const;
    bool load();

private:
    std::string db_path_;
    std::vector<HistoryRecord> records_;
    uint64_t next_id_ = 1;

    static std::string default_db_path();
};

} // namespace meridian::dev

