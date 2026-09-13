// meridian-ai / risk_classifier.cpp
#include "risk_classifier.hpp"
#include <regex>

namespace meridian::ai {

std::string to_string(RiskLevel level) {
    switch (level) {
        case RiskLevel::Low: return "LOW";
        case RiskLevel::Medium: return "MEDIUM";
        case RiskLevel::High: return "HIGH";
        case RiskLevel::Critical: return "CRITICAL";
    }
    return "LOW";
}

namespace {

struct Pattern {
    std::regex re;
    std::string reason;
};

const std::vector<Pattern>& critical_patterns() {
    static const std::vector<Pattern> p = {
        {std::regex(R"(\brm\s+-[a-zA-Z]*r[a-zA-Z]*f[a-zA-Z]*\b(\s+\S+)*\s+/\s*(\*|)\s*($|[;&|]))"),
         "recursive force-delete targeting the filesystem root"},
        {std::regex(R"(\brm\s+-[a-zA-Z]*f[a-zA-Z]*r[a-zA-Z]*\b(\s+\S+)*\s+/\s*(\*|)\s*($|[;&|]))"),
         "recursive force-delete targeting the filesystem root"},
        {std::regex(R"(:\s*\(\s*\)\s*\{[^}]*:\s*\|\s*:[^}]*\}\s*;\s*:)"), "classic shell fork bomb pattern"},
        {std::regex(R"(\bdd\b.*\bof=/dev/(sd|nvme|hd|disk|xvd)\w*)"), "writes raw data directly onto a disk device"},
        {std::regex(R"(\bmkfs\.\w+)"), "formats a filesystem, destroying existing data on the target"},
        {std::regex(R"(>\s*/dev/(sd|nvme|hd|disk|xvd)\w*)"), "redirects output directly onto a disk device"},
        {std::regex(R"(\bchmod\s+-R\s+777\s+/\s*($|[;&|]))"), "recursively opens permissions on the filesystem root"},
    };
    return p;
}

const std::vector<Pattern>& high_patterns() {
    static const std::vector<Pattern> p = {
        {std::regex(R"(\bsudo\b)"), "runs with elevated (root) privileges"},
        {std::regex(R"(\brm\s+-[a-zA-Z]*r[a-zA-Z]*f[a-zA-Z]*\b)"), "recursive force-delete"},
        {std::regex(R"(\brm\s+-[a-zA-Z]*f[a-zA-Z]*r[a-zA-Z]*\b)"), "recursive force-delete"},
        {std::regex(R"(\b(shutdown|halt|poweroff)\b)"), "shuts down or halts the machine"},
        {std::regex(R"(\breboot\b)"), "reboots the machine"},
        {std::regex(R"(\bkill\s+-(9|KILL)\b)", std::regex::icase), "sends an unblockable kill signal"},
        {std::regex(R"(\bchown\s+-R\b)"), "recursively changes file ownership"},
        {std::regex(R"(\bsystemctl\s+(stop|restart|disable|mask)\b)"), "changes a running system service's state"},
        {std::regex(R"((curl|wget)\b[^|]*\|\s*(sudo\s+)?(sh|bash|zsh)\b)"), "pipes a downloaded script directly into a shell"},
        {std::regex(R"(\bdd\s+)"), "low-level block-device copy tool — destructive if the target is wrong"},
    };
    return p;
}

const std::vector<Pattern>& medium_patterns() {
    static const std::vector<Pattern> p = {
        {std::regex(R"(\brm\b)"), "deletes files"},
        {std::regex(R"(\bchmod\b)"), "changes file permissions"},
        {std::regex(R"(\bchown\b)"), "changes file ownership"},
        {std::regex(R"(\bnpm\s+install\s+-g\b)"), "installs a package globally"},
        {std::regex(R"(\bpip\d?\s+install\b)"), "installs a package, potentially system-wide"},
        {std::regex(R"(\bgit\s+push\s+[^\n]*(--force|-f)\b)"), "force-pushes, which can overwrite remote history"},
        {std::regex(R"(\bkill\b)"), "sends a signal to terminate a process"},
        {std::regex(R"(\bapt(-get)?\s+(remove|purge)\b)"), "removes an installed package"},
    };
    return p;
}

RiskResult classify_against(const std::string& line, const std::vector<Pattern>& patterns, RiskLevel level) {
    RiskResult r;
    for (const auto& p : patterns) {
        if (std::regex_search(line, p.re)) {
            r.reasons.push_back(p.reason);
        }
    }
    if (!r.reasons.empty()) r.level = level;
    return r;
}

} // namespace

RiskResult RiskClassifier::classify(const std::string& command_line) const {
    RiskResult critical = classify_against(command_line, critical_patterns(), RiskLevel::Critical);
    if (!critical.reasons.empty()) return critical;

    RiskResult high = classify_against(command_line, high_patterns(), RiskLevel::High);
    if (!high.reasons.empty()) return high;

    RiskResult medium = classify_against(command_line, medium_patterns(), RiskLevel::Medium);
    if (!medium.reasons.empty()) return medium;

    return RiskResult{RiskLevel::Low, {}};
}

} // namespace meridian::ai
