#include "intent_engine.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace meridian::ai {

namespace {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

} // namespace

IntentEngine::IntentEngine() = default;

IntentResult IntentEngine::translate(const std::string& query, const std::string& /*cwd*/) const {
    IntentResult res;
    res.prompt = query;
    std::string q = to_lower(query);

    // 1. Find modified files
    if (q.find("find") != std::string::npos && (q.find("modified") != std::string::npos || q.find("days") != std::string::npos)) {
        std::string ext = "*";
        if (q.find("javascript") != std::string::npos || q.find(".js") != std::string::npos) ext = "*.js";
        else if (q.find("python") != std::string::npos || q.find(".py") != std::string::npos) ext = "*.py";
        else if (q.find("cpp") != std::string::npos || q.find("c++") != std::string::npos) ext = "*.cpp";
        else if (q.find("rust") != std::string::npos || q.find(".rs") != std::string::npos) ext = "*.rs";

        std::string days = "7";
        std::regex days_regex(R"((\d+)\s*days?)");
        std::smatch m;
        if (std::regex_search(q, m, days_regex)) {
            days = m[1].str();
        }

        res.generated_command = "find . -type f -name \"" + ext + "\" -mtime -" + days;
        res.explanation = "Searches current directory recursively for files matching '" + ext + "' modified within the last " + days + " days.";
        res.confidence = 0.95f;
    }
    // 2. Kill process on port
    else if (q.find("port") != std::string::npos && (q.find("kill") != std::string::npos || q.find("stop") != std::string::npos || q.find("free") != std::string::npos)) {
        std::string port = "8080";
        std::regex port_regex(R"((\d{2,5}))");
        std::smatch m;
        if (std::regex_search(q, m, port_regex)) {
            port = m[1].str();
        }
        res.generated_command = "lsof -ti:" + port + " | xargs kill -9";
        res.explanation = "Finds the process ID listening on TCP port " + port + " and terminates it.";
        res.alternatives.push_back("fuser -k " + port + "/tcp");
        res.confidence = 0.92f;
    }
    // 3. Git undo last commit
    else if (q.find("undo") != std::string::npos && q.find("commit") != std::string::npos) {
        if (q.find("keep") != std::string::npos || q.find("without losing") != std::string::npos || q.find("soft") != std::string::npos) {
            res.generated_command = "git reset --soft HEAD~1";
            res.explanation = "Undoes the last commit while leaving modified files staged in your working tree.";
        } else {
            res.generated_command = "git reset HEAD~1";
            res.explanation = "Undoes the last commit and unstages changes, keeping file modifications intact.";
        }
        res.confidence = 0.96f;
    }
    // 4. Git diff / branch stats
    else if (q.find("git") != std::string::npos && (q.find("ahead") != std::string::npos || q.find("unpushed") != std::string::npos)) {
        res.generated_command = "git log @{u}..HEAD --oneline";
        res.explanation = "Shows list of commits present on the local branch that have not yet been pushed to remote.";
        res.confidence = 0.94f;
    }
    // 5. Disk space / usage
    else if (q.find("disk") != std::string::npos || q.find("free space") != std::string::npos) {
        res.generated_command = "df -h";
        res.explanation = "Displays filesystem disk space usage in human-readable units (GB/MB).";
        res.alternatives.push_back("du -sh * | sort -h");
        res.confidence = 0.98f;
    }
    // 6. Memory usage
    else if (q.find("memory") != std::string::npos || q.find("ram") != std::string::npos) {
        res.generated_command = "free -h";
        res.explanation = "Displays total, used, and available physical RAM and swap space.";
        res.confidence = 0.98f;
    }
    // 7. Compress folder to tar.gz
    else if (q.find("tar") != std::string::npos || q.find("compress") != std::string::npos || q.find("zip") != std::string::npos) {
        res.generated_command = "tar -czvf archive.tar.gz .";
        res.explanation = "Gzip-compresses the contents of current directory into 'archive.tar.gz'.";
        res.confidence = 0.90f;
    }
    // 8. Docker cleanup
    else if (q.find("docker") != std::string::npos && (q.find("clean") != std::string::npos || q.find("prune") != std::string::npos)) {
        res.generated_command = "docker system prune -f";
        res.explanation = "Removes all unused Docker containers, networks, and dangling images.";
        res.confidence = 0.95f;
    }
    // 9. Dangerous root removal
    else if ((q.find("remove") != std::string::npos || q.find("delete") != std::string::npos) && (q.find("root") != std::string::npos || q.find("rm -rf") != std::string::npos)) {
        res.generated_command = "rm -rf /";
        res.explanation = "Recursively and forcefully deletes the entire root filesystem.";
        res.confidence = 0.99f;
    }
    // Default fallback: search or inspect
    else {
        res.generated_command = "grep -rnI \"" + query + "\" .";
        res.explanation = "Performs a recursive case-sensitive search for '" + query + "' across non-binary files.";
        res.confidence = 0.70f;
    }

    // Safety classification
    auto classified = risk_classifier_.classify(res.generated_command);
    res.risk = classified.level;
    if (!classified.reasons.empty()) {
        res.risk_reason = classified.reasons.front();
    }

    return res;
}

std::string IntentEngine::format_card(const IntentResult& r) {
    std::ostringstream ss;
    ss << "┌─── Meridian Intent ─────────────────────────────────────\n";
    ss << "│ Prompt:      " << r.prompt << "\n";
    ss << "│ Generated:   " << r.generated_command << "\n";
    ss << "│ Explanation: " << r.explanation << "\n";
    ss << "│ Safety Risk: " << to_string(r.risk);
    if (!r.risk_reason.empty()) {
        ss << " (" << r.risk_reason << ")";
    }
    ss << "\n";
    if (!r.alternatives.empty()) {
        ss << "│ Alternative: " << r.alternatives.front() << "\n";
    }
    ss << "└─────────────────────────────────────────────────────────\n";
    return ss.str();
}

} // namespace meridian::ai

