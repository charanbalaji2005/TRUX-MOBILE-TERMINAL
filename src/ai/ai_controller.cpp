// meridian-ai / ai_controller.cpp
#include "ai_controller.hpp"
#include <sstream>

namespace meridian::ai {

namespace {

const std::vector<std::string>& shell_builtin_names() {
    // Mirrors src/shell/builtins.cpp's list. Duplicated rather than
    // linked against meridian-shell-lib on purpose: the AI layer must
    // stay usable even for people running bash/zsh/fish instead of
    // Meridian Shell, so it can't assume the shell library is even
    // linked in. See docs/ai.md.
    static const std::vector<std::string> names = {
        "cd", "pwd", "echo", "exit", "export", "unset", "env",
        "history", "jobs", "fg", "bg", "help", "type", "which", "clear", "alias"
    };
    return names;
}

std::string first_token(const std::string& line) {
    std::size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    std::size_t end = line.find_first_of(" \t", start);
    return line.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

std::string indicator_for_risk(RiskLevel level) {
    switch (level) {
        case RiskLevel::Critical:
        case RiskLevel::High: return "\U0001F512"; // 🔒
        case RiskLevel::Medium: return "\u26A0";    // ⚠
        case RiskLevel::Low: return "";
    }
    return "";
}

} // namespace

AIController::AIController(std::string config_path)
    : config_(std::move(config_path)), analyzer_(shell_builtin_names()) {
    analyzer_.refresh_path_index();
}

void AIController::load() { config_.load(); }
void AIController::save() { config_.save(); }

std::string AIController::status_report() const {
    std::ostringstream out;
    out << "Meridian AI\n"
        << "-------------------------\n"
        << "Status:       " << (enabled() ? "ON" : "OFF") << "\n"
        << "Detection:    " << (detection_enabled() ? "ON" : "OFF") << "\n"
        << "Provider:     " << provider() << "\n"
        << "Model:        " << model() << "\n"
        << "Connection:   N/A (no provider implemented yet — see docs/ai.md)\n"
        << "Privacy:      " << (privacy_mode() ? "ON" : "OFF") << "\n";
    return out.str();
}

std::string AIController::test_providers() const {
    std::ostringstream out;
    out << "Meridian AI Provider Test\n\n";
    for (const char* name : {"Groq", "Gemini", "Ollama Cloud"}) {
        out << name << "\n  Status: NOT IMPLEMENTED (no network access in this build — see docs/ai.md)\n\n";
    }
    return out.str();
}

std::string AIController::analyze_command(const std::string& command_line) {
    if (!enabled() || !detection_enabled()) return "";

    std::string cmd = first_token(command_line);
    if (cmd.empty()) return "";

    std::ostringstream out;
    bool wrote_anything = false;

    auto suggestion = analyzer_.analyze(cmd);
    if (suggestion) {
        std::string corrected = command_line;
        corrected.replace(corrected.find(cmd), cmd.size(), suggestion->suggested);
        out << "\u2726 possible correction: " << corrected
            << "  (\"" << suggestion->original << "\" not found; closest known command is \""
            << suggestion->suggested << "\", edit distance " << suggestion->edit_distance << ")\n";
        wrote_anything = true;
    }

    auto risk = risk_.classify(command_line);
    if (risk.level != RiskLevel::Low) {
        out << indicator_for_risk(risk.level) << " risk: " << to_string(risk.level);
        if (!risk.reasons.empty()) {
            out << " -";
            for (std::size_t i = 0; i < risk.reasons.size(); ++i) {
                out << (i == 0 ? " " : "; ") << risk.reasons[i];
            }
        }
        out << "\n";
        wrote_anything = true;
    }

    return wrote_anything ? out.str() : "";
}

std::string AIController::explain_command(const std::string& command_line) const {
    std::ostringstream out;
    out << "No AI provider is configured (this build has no Groq/Gemini/Ollama Cloud\n"
        << "implementation — see docs/ai.md), so I can't generate a real natural-language\n"
        << "explanation of:\n\n  " << command_line << "\n\n"
        << "What's available without a provider: `meridian ai analyze \"" << command_line
        << "\"` for local typo detection and risk classification.\n";
    return out.str();
}

} // namespace meridian::ai
