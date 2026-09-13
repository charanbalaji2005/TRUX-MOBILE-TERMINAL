#pragma once
// meridian-ai / command_analyzer.hpp
//
// Local-only command-name typo detection (spec §38): scans real PATH
// directories for executables, and when a typed command name isn't a
// known builtin or PATH executable, suggests the closest match by edit
// distance. No network call, no LLM — just PATH lookup and a classic
// string-distance algorithm, which is what "local analyzer" means in
// the spec (cloud AI is a separate, explicitly-optional escalation).
//
// Scoped honestly: this catches "gerp" -> "grep" (the command name
// itself is misspelled). It does NOT catch subcommand-level mistakes
// like "npm instal express" -> "npm install express", since recognizing
// that "instal" isn't a valid npm subcommand requires per-program
// knowledge (or an actual LLM) that a generic PATH-based analyzer
// can't have. See docs/ai.md.

#include <optional>
#include <string>
#include <vector>

namespace meridian::ai {

struct Suggestion {
    std::string original;
    std::string suggested;
    int edit_distance = 0;
};

class CommandAnalyzer {
public:
    // `builtins` is the shell's own builtin name list — these count as
    // "known" alongside anything found on PATH.
    explicit CommandAnalyzer(std::vector<std::string> builtins);

    // Scans $PATH (or `path_override` if given, for testability) for
    // executable files. Real filesystem access — no caching across
    // process runs, so this reflects whatever's actually installed.
    void refresh_path_index(const char* path_override = nullptr);

    std::size_t known_executable_count() const { return path_executables_.size(); }

    // Returns nullopt if `command_name` is already known (a builtin, a
    // PATH executable, or an existing absolute/relative executable
    // path), or if no known name is close enough to guess confidently.
    std::optional<Suggestion> analyze(const std::string& command_name) const;

private:
    bool is_known(const std::string& name) const;

    std::vector<std::string> builtins_;
    std::vector<std::string> path_executables_;
};

// Classic Levenshtein edit distance. Exposed for testing and for reuse
// elsewhere (e.g. a future subcommand-level analyzer).
int edit_distance(const std::string& a, const std::string& b);

} // namespace meridian::ai
