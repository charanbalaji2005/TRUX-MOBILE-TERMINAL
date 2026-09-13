// src/shell/autosuggest_engine.cpp
#include "autosuggest_engine.hpp"
#include "../dev/icon_provider.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace meridian::shell {

// ─── Helpers ─────────────────────────────────────────────────────────────────

static bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

std::string AutosuggestEngine::expand_tilde(const std::string& path) {
    if (path.empty() || path[0] != '~') return path;
    const char* home = std::getenv("HOME");
    if (!home) return path;
    return std::string(home) + path.substr(1);
}

std::string AutosuggestEngine::last_word(const std::string& line) {
    auto pos = line.find_last_of(" \t");
    if (pos == std::string::npos) return line;
    return line.substr(pos + 1);
}

std::string AutosuggestEngine::get_suggestion_suffix(
    const std::string& prefix,
    const std::string& full_suggestion
) {
    if (!starts_with(full_suggestion, prefix)) return "";
    return full_suggestion.substr(prefix.size());
}

std::string AutosuggestEngine::accept_next_word(
    const std::string& current,
    const std::string& full_suggestion
) {
    if (!starts_with(full_suggestion, current)) return current;
    std::string suffix = full_suggestion.substr(current.size());
    if (suffix.empty()) return current;

    // Skip leading spaces
    size_t start = 0;
    while (start < suffix.size() && suffix[start] == ' ') start++;
    // Find end of next word
    size_t end = start;
    while (end < suffix.size() && suffix[end] != ' ') end++;
    // Include trailing space if present
    while (end < suffix.size() && suffix[end] == ' ') end++;

    return current + suffix.substr(0, end);
}

// ─── History Ranking ─────────────────────────────────────────────────────────

int AutosuggestEngine::score_history_entry(
    const std::string& prefix,
    const std::string& entry,
    int recency_rank  // 0 = most recent
) {
    if (!starts_with(entry, prefix)) return 0;
    if (entry == prefix) return 0; // Exact match - nothing more to suggest

    int score = 100;

    // Recency bonus: most recent commands score highest
    score -= recency_rank * 2;

    // Prefer shorter completions (less noise)
    int suffix_len = static_cast<int>(entry.size()) - static_cast<int>(prefix.size());
    score -= suffix_len / 8;

    // Small bonus for exact-word matches (prefix ends at word boundary in entry)
    if (prefix.size() < entry.size() && entry[prefix.size()] == ' ')
        score += 5;

    return std::max(1, score);
}

// ─── Filesystem Completion ────────────────────────────────────────────────────

static std::vector<std::pair<std::string, bool>> list_matching_entries(
    const std::string& dir_prefix,
    const std::string& name_prefix,
    bool dirs_only = false
) {
    std::vector<std::pair<std::string, bool>> results; // name, is_dir

    std::string search_dir = dir_prefix.empty() ? "." : dir_prefix;
    DIR* d = opendir(search_dir.c_str());
    if (!d) return results;

    struct dirent* ent;
    while ((ent = readdir(d)) != nullptr) {
        std::string fname = ent->d_name;
        if (fname == "." || fname == "..") continue;
        if (name_prefix.empty() || starts_with(fname, name_prefix)) {
            std::string full = search_dir + "/" + fname;
            struct stat st;
            bool is_dir = false;
            if (stat(full.c_str(), &st) == 0) {
                is_dir = S_ISDIR(st.st_mode);
            }
            if (!dirs_only || is_dir) {
                results.push_back({fname, is_dir});
            }
        }
    }
    closedir(d);

    // Sort: directories first, then alphabetical
    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    return results;
}

// Split a path prefix into (directory_part, name_prefix)
// e.g. "src/grap" -> ("src", "grap")
//      "Doc"       -> (".", "Doc")
static std::pair<std::string, std::string> split_path_prefix(const std::string& word) {
    auto slash = word.rfind('/');
    if (slash == std::string::npos) {
        return {".", word};
    }
    std::string dir = word.substr(0, slash);
    if (dir.empty()) dir = "/";
    return {dir, word.substr(slash + 1)};
}

std::string AutosuggestEngine::get_filesystem_completion(
    const std::string& word_prefix,
    const std::string& /*cwd*/
) {
    if (word_prefix.empty()) return "";
    std::string expanded = expand_tilde(word_prefix);
    auto [dir_part, name_part] = split_path_prefix(expanded);
    auto entries = list_matching_entries(dir_part, name_part, false);
    if (entries.empty()) return "";

    const auto& [name, is_dir] = entries[0];
    // Reconstruct completion
    std::string prefix_dir;
    auto slash = word_prefix.rfind('/');
    if (slash != std::string::npos) {
        prefix_dir = word_prefix.substr(0, slash + 1);
    }
    return prefix_dir + name + (is_dir ? "/" : "");
}

std::vector<std::string> AutosuggestEngine::get_tab_completions(
    const std::string& word_prefix,
    const std::string& /*cwd*/,
    int max_results
) {
    std::vector<std::string> results;
    if (word_prefix.empty()) return results;

    std::string expanded = expand_tilde(word_prefix);
    auto [dir_part, name_part] = split_path_prefix(expanded);
    auto entries = list_matching_entries(dir_part, name_part, false);

    std::string prefix_dir;
    auto slash = word_prefix.rfind('/');
    if (slash != std::string::npos) {
        prefix_dir = word_prefix.substr(0, slash + 1);
    }

    int n = 0;
    for (const auto& [fname, is_dir] : entries) {
        // Check if executable
        bool is_exec = false;
        std::string fullpath = (dir_part == "." ? "" : dir_part + "/") + fname;
        if (!is_dir && access(fullpath.c_str(), X_OK) == 0) is_exec = true;

        std::string icon = dev::IconProvider::icon_for_file(fname, is_dir, is_exec, ' ');
        std::string color;
        if (is_dir)
            color = "\033[38;2;97;175;239m";       // blue for directories
        else if (is_exec)
            color = "\033[38;2;76;175;80m";        // green for executables
        else
            color = "\033[38;2;200;210;230m";

        std::string entry = color + icon + prefix_dir + fname + (is_dir ? "/" : "") + "\033[0m";
        results.push_back(entry);
        if (++n >= max_results) break;
    }
    return results;
}

std::vector<std::string> AutosuggestEngine::get_tab_completions_for_line(
    const std::string& current_line,
    const std::string& cwd,
    int max_results
) {
    // Extract the word being completed
    std::string word = last_word(current_line);
    if (word.empty()) return get_tab_completions("", cwd, max_results);

    // Determine whether it's a command or file completion
    // If word is the first token, try commands AND files
    auto first_space = current_line.find_first_of(" \t");
    bool is_first_token = (first_space == std::string::npos || first_space >= current_line.size() - 1);

    if (is_first_token && word.find('/') == std::string::npos) {
        // Try builtin/PATH command completions too
        auto cmds = find_commands_with_prefix(word);
        auto files = get_tab_completions(word, cwd, max_results);
        // Merge, commands first
        for (auto& c : cmds) {
            if (static_cast<int>(files.size()) < max_results)
                files.insert(files.begin(), "\033[38;2;76;175;80m  " + c + "\033[0m");
        }
        return files;
    }

    // Check context for dirs-only
    std::string first_token;
    if (first_space != std::string::npos) {
        first_token = current_line.substr(0, first_space);
    }
    // cd should only complete directories
    if (first_token == "cd" || first_token == "pushd" || first_token == "rmdir") {
        std::string expanded = expand_tilde(word);
        auto [dir_part, name_part] = split_path_prefix(expanded);
        auto entries = list_matching_entries(dir_part, name_part, true); // dirs only
        std::string prefix_dir;
        auto slash = word.rfind('/');
        if (slash != std::string::npos) prefix_dir = word.substr(0, slash + 1);
        std::vector<std::string> results;
        for (const auto& [fname, is_dir] : entries) {
            std::string icon = dev::IconProvider::icon_for_file(fname, is_dir, false, ' ');
            results.push_back("\033[38;2;97;175;239m" + icon + prefix_dir + fname + "/\033[0m");
            if (static_cast<int>(results.size()) >= max_results) break;
        }
        return results;
    }

    return get_tab_completions(word, cwd, max_results);
}

// ─── Command Discovery ────────────────────────────────────────────────────────

std::vector<std::string> AutosuggestEngine::find_commands_with_prefix(const std::string& prefix) {
    std::vector<std::string> results;
    if (prefix.empty()) return results;

    // Search PATH
    const char* path_env = std::getenv("PATH");
    if (!path_env) return results;

    std::string path_str = path_env;
    std::istringstream ss(path_str);
    std::string dir;
    while (std::getline(ss, dir, ':')) {
        if (dir.empty()) continue;
        DIR* d = opendir(dir.c_str());
        if (!d) continue;
        struct dirent* ent;
        while ((ent = readdir(d)) != nullptr) {
            std::string fname = ent->d_name;
            if (starts_with(fname, prefix)) {
                std::string full = dir + "/" + fname;
                if (access(full.c_str(), X_OK) == 0) {
                    // Only add if not already in results
                    if (std::find(results.begin(), results.end(), fname) == results.end()) {
                        results.push_back(fname);
                    }
                }
            }
        }
        closedir(d);
        if (static_cast<int>(results.size()) >= 10) break;
    }
    std::sort(results.begin(), results.end());
    results.erase(std::unique(results.begin(), results.end()), results.end());
    if (results.size() > 10) results.resize(10);
    return results;
}

// ─── Main Suggestion Resolver ─────────────────────────────────────────────────

std::string AutosuggestEngine::get_suggestion(
    const std::string& prefix,
    const std::vector<std::string>& history,
    const std::string& cwd
) {
    if (prefix.empty()) return "";

    // 1. Search history for best prefix match
    std::string best;
    int best_score = 0;
    int total = static_cast<int>(history.size());
    for (int i = total - 1; i >= 0; --i) {
        int recency = total - 1 - i;  // 0 for most recent
        int sc = score_history_entry(prefix, history[i], recency);
        if (sc > best_score) {
            best_score = sc;
            best = history[i];
        }
    }

    if (!best.empty() && best_score > 0) return best;

    // 2. Filesystem context completion
    std::string word = last_word(prefix);
    if (!word.empty()) {
        // Context-aware: if last word starts with ./, ~/, ../, or / it's a path
        bool is_path = (word.find('/') != std::string::npos ||
                        word[0] == '~' ||
                        (word.size() >= 2 && word[0] == '.' && word[1] == '/'));

        // Also try for commands that typically take paths as arguments
        if (!is_path) {
            auto first_space = prefix.find_first_of(" \t");
            if (first_space != std::string::npos) {
                std::string first_tok = prefix.substr(0, first_space);
                if (first_tok == "cd" || first_tok == "ls" || first_tok == "cat" ||
                    first_tok == "less" || first_tok == "more" || first_tok == "rm" ||
                    first_tok == "cp" || first_tok == "mv" || first_tok == "vim" ||
                    first_tok == "nano" || first_tok == "touch" || first_tok == "mkdir" ||
                    first_tok == "pic" || first_tok == "open") {
                    is_path = true;
                }
            }
        }

        if (is_path) {
            std::string fs_completion = get_filesystem_completion(word, cwd);
            if (!fs_completion.empty() && fs_completion != word) {
                // Reconstruct full command
                auto last_space = prefix.rfind(' ');
                if (last_space != std::string::npos)
                    return prefix.substr(0, last_space + 1) + fs_completion;
                return fs_completion;
            }
        }
    }

    return "";
}

} // namespace meridian::shell

