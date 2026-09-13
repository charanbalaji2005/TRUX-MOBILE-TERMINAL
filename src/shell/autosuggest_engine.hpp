// src/shell/autosuggest_engine.hpp
#pragma once
//
// Meridian Autosuggestion Engine
//
// Provides real-time ghost (dim inline) autosuggestions for the line editor.
// Never executes commands, never blocks input, and never contacts the network.
//
// Priority:
//   1. Exact prefix match from recent history (frequency + recency scored)
//   2. Context-aware filesystem completion (cd/cat/ls/./src/ prefixes)
//   3. Command prefix match from PATH builtins

#include <string>
#include <vector>

namespace meridian::shell {

struct SuggestionSource {
    enum class Kind { HISTORY, FILESYSTEM, COMMAND };
    Kind kind = Kind::HISTORY;
    std::string full_command;  // The full suggested command text
    int score = 0;             // Higher is better
};

class AutosuggestEngine {
public:
    // Score a list of history records against the current prefix.
    // Returns the best full-command suggestion, or "" if no match.
    static std::string get_suggestion(
        const std::string& prefix,
        const std::vector<std::string>& history,
        const std::string& cwd = "."
    );

    // Get just the suffix that is not part of the prefix.
    // e.g. prefix="git pu", full="git push origin main" -> " push origin main" (with "sh origin main" as the completion part)
    static std::string get_suggestion_suffix(
        const std::string& prefix,
        const std::string& full_suggestion
    );

    // Accept the next word of the suggestion (Ctrl+Right)
    static std::string accept_next_word(
        const std::string& current,
        const std::string& full_suggestion
    );

    // Get filesystem completion candidates for prefix
    // Returns the first (best) completion or ""
    static std::string get_filesystem_completion(
        const std::string& word_prefix,
        const std::string& cwd = "."
    );

    // Get multiple tab completion candidates with icons
    // Each entry is formatted: "icon filename" (with trailing / for directories)
    static std::vector<std::string> get_tab_completions(
        const std::string& word_prefix,
        const std::string& cwd = ".",
        int max_results = 20
    );

    // Get tab completions for the last word in the current line
    // Handles cd/cat/ls context to restrict to directories or files
    static std::vector<std::string> get_tab_completions_for_line(
        const std::string& current_line,
        const std::string& cwd = ".",
        int max_results = 20
    );

private:
    // Score a history entry against the prefix.
    // Higher is better; 0 = no match.
    static int score_history_entry(
        const std::string& prefix,
        const std::string& entry,
        int recency_rank
    );

    // Expand ~ to home directory in a path
    static std::string expand_tilde(const std::string& path);

    // Return the last "word" in a command line (space-separated)
    static std::string last_word(const std::string& line);

    // Find builtin commands and PATH executables matching prefix
    static std::vector<std::string> find_commands_with_prefix(const std::string& prefix);
};

} // namespace meridian::shell

