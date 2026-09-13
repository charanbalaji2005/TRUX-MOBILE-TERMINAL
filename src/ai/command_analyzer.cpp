// meridian-ai / command_analyzer.cpp
#include "command_analyzer.hpp"

#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace meridian::ai {

int edit_distance(const std::string& a, const std::string& b) {
    // Optimal String Alignment distance: classic Levenshtein plus an
    // adjacent-transposition case counted as a single operation. Plain
    // Levenshtein charges 2 for "gerp" -> "grep" (a transposed 'e'/'r'),
    // which is the single most common real-world typo shape and would
    // otherwise sit right at the edge of (or past) the suggestion
    // threshold for short command names.
    const std::size_t n = a.size(), m = b.size();
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1));
    for (std::size_t i = 0; i <= n; ++i) dp[i][0] = static_cast<int>(i);
    for (std::size_t j = 0; j <= m; ++j) dp[0][j] = static_cast<int>(j);
    for (std::size_t i = 1; i <= n; ++i) {
        for (std::size_t j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1,          // deletion
                                  dp[i][j - 1] + 1,          // insertion
                                  dp[i - 1][j - 1] + cost}); // substitution
            if (i > 1 && j > 1 && a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1]) {
                dp[i][j] = std::min(dp[i][j], dp[i - 2][j - 2] + 1); // adjacent transposition
            }
        }
    }
    return dp[n][m];
}

CommandAnalyzer::CommandAnalyzer(std::vector<std::string> builtins) : builtins_(std::move(builtins)) {}

void CommandAnalyzer::refresh_path_index(const char* path_override) {
    path_executables_.clear();
    const char* path = path_override ? path_override : std::getenv("PATH");
    if (!path) return;

    std::string p(path);
    std::size_t start = 0;
    while (start <= p.size()) {
        auto colon = p.find(':', start);
        std::string dir = p.substr(start, colon == std::string::npos ? std::string::npos : colon - start);
        if (!dir.empty()) {
            DIR* d = opendir(dir.c_str());
            if (d) {
                struct dirent* entry;
                while ((entry = readdir(d)) != nullptr) {
                    std::string name = entry->d_name;
                    if (name == "." || name == "..") continue;
                    std::string full = dir + "/" + name;
                    struct stat st{};
                    if (stat(full.c_str(), &st) == 0 && S_ISREG(st.st_mode) && access(full.c_str(), X_OK) == 0) {
                        path_executables_.push_back(name);
                    }
                }
                closedir(d);
            }
        }
        if (colon == std::string::npos) break;
        start = colon + 1;
    }
    std::sort(path_executables_.begin(), path_executables_.end());
    path_executables_.erase(std::unique(path_executables_.begin(), path_executables_.end()), path_executables_.end());
}

bool CommandAnalyzer::is_known(const std::string& name) const {
    if (name.find('/') != std::string::npos) {
        // Looks like a path (relative or absolute) rather than a bare
        // command name — check it directly instead of against PATH.
        return access(name.c_str(), X_OK) == 0;
    }
    if (std::find(builtins_.begin(), builtins_.end(), name) != builtins_.end()) return true;
    if (path_executables_.empty()) {
        const_cast<CommandAnalyzer*>(this)->refresh_path_index();
    }
    return std::binary_search(path_executables_.begin(), path_executables_.end(), name);
}

std::optional<Suggestion> CommandAnalyzer::analyze(const std::string& command_name) const {
    if (command_name.empty() || is_known(command_name)) return std::nullopt;

    if (path_executables_.empty()) {
        const_cast<CommandAnalyzer*>(this)->refresh_path_index();
    }

    std::string best;
    int best_distance = -1;
    for (const auto& candidate : builtins_) {
        int d = edit_distance(command_name, candidate);
        if (best_distance < 0 || d < best_distance) { best_distance = d; best = candidate; }
    }
    for (const auto& candidate : path_executables_) {
        int d = edit_distance(command_name, candidate);
        if (best_distance < 0 || d < best_distance) { best_distance = d; best = candidate; }
    }

    if (best_distance < 0) return std::nullopt;

    // Only suggest when the guess is close relative to the word's
    // length — otherwise a short, simply-nonexistent command would get
    // matched to something unrelated just because it happened to be
    // "closest" among thousands of candidates.
    int threshold = std::max(1, static_cast<int>(command_name.size()) / 3);
    if (best_distance > threshold) return std::nullopt;

    return Suggestion{command_name, best, best_distance};
}

} // namespace meridian::ai
