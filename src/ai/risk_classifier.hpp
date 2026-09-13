#pragma once
// meridian-ai / risk_classifier.hpp
//
// Classifies a command line's risk level (spec §41/§73) using local
// pattern matching only — no network, no AI model, fully testable.
// This is deliberately conservative and pattern-based rather than a
// claim of true semantic understanding of what a command will do.

#include <string>
#include <vector>

namespace meridian::ai {

enum class RiskLevel { Low, Medium, High, Critical };

std::string to_string(RiskLevel level);

struct RiskResult {
    RiskLevel level = RiskLevel::Low;
    std::vector<std::string> reasons; // human-readable, e.g. "matches rm -rf"
};

class RiskClassifier {
public:
    RiskResult classify(const std::string& command_line) const;
};

} // namespace meridian::ai
