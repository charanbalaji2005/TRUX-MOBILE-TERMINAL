#pragma once
// meridian-terminal / ai / error_diagnostics.hpp
//
// Context-aware terminal error diagnostics. Inspects terminal outputs from
// compilers, runtimes, package managers, and databases to formulate structured
// diagnostic explanations and actionable remediation suggestions.

#include <string>
#include <vector>

namespace meridian::ai {

enum class DiagnosticCategory {
    NodeJs,
    Python,
    CppCompiler,
    Docker,
    Git,
    PortConflict,
    Unknown
};

struct DiagnosticFix {
    std::string label;
    std::string command;
    bool is_safe = true;
};

struct DiagnosticCard {
    DiagnosticCategory category = DiagnosticCategory::Unknown;
    std::string title;
    std::string what_happened;
    std::string evidence;
    std::vector<DiagnosticFix> suggested_fixes;

    bool empty() const { return title.empty(); }
    std::string format() const;
};

class ErrorDiagnostics {
public:
    ErrorDiagnostics() = default;

    DiagnosticCard analyze(const std::string& output) const;
};

} // namespace meridian::ai

