// meridian-ai / secret_redactor.cpp
#include "secret_redactor.hpp"
#include <regex>

namespace meridian::ai {

namespace {

// Each entry: a pattern plus the regex_replace format string that
// redacts the value while preserving surrounding context (the key name,
// quotes, header text) where that's meaningful to keep.
struct Rule {
    std::regex pattern;
    std::string replacement;
};

const std::vector<Rule>& rules() {
    static const std::vector<Rule> r = {
        // NAME=value / NAME: value where NAME looks like a secret's name
        // (API_KEY, ACCESS_TOKEN, DB_PASSWORD, ...). Keeps the name and
        // any quotes, redacts only the value.
        {std::regex(R"(([A-Za-z0-9_]*(?:key|token|secret|password|passwd|pwd)[A-Za-z0-9_]*\s*[:=]\s*)("?)([^\s"'\n]+)("?))",
                     std::regex::icase),
         "$1$2[REDACTED]$4"},

        // Authorization: Bearer <token>
        {std::regex(R"((Authorization:\s*Bearer\s+)([^\s"']+))", std::regex::icase), "$1[REDACTED]"},

        // Known key-prefix formats.
        {std::regex(R"(\bAKIA[0-9A-Z]{16}\b)"), "[REDACTED]"},                    // AWS access key id
        {std::regex(R"(\bghp_[A-Za-z0-9]{20,}\b)"), "[REDACTED]"},                // GitHub PAT (classic)
        {std::regex(R"(\bgithub_pat_[A-Za-z0-9_]{20,}\b)"), "[REDACTED]"},        // GitHub PAT (fine-grained)
        {std::regex(R"(\bsk-[A-Za-z0-9_-]{16,}\b)"), "[REDACTED]"},               // OpenAI/Anthropic-style secret keys
        {std::regex(R"(\bxox[baprs]-[A-Za-z0-9-]{10,}\b)"), "[REDACTED]"},        // Slack tokens

        // PEM private key blocks, redacted whole (not just the header).
        {std::regex(R"(-----BEGIN [^-]+PRIVATE KEY-----[\s\S]*?-----END [^-]+PRIVATE KEY-----)"),
         "-----BEGIN [REDACTED] PRIVATE KEY-----\n[REDACTED]\n-----END [REDACTED] PRIVATE KEY-----"},
    };
    return r;
}

} // namespace

std::string SecretRedactor::redact(const std::string& text, int* redaction_count) const {
    std::string out = text;
    int total = 0;
    for (const auto& rule : rules()) {
        total += static_cast<int>(
            std::distance(std::sregex_iterator(out.begin(), out.end(), rule.pattern), std::sregex_iterator()));
        out = std::regex_replace(out, rule.pattern, rule.replacement);
    }
    if (redaction_count) *redaction_count = total;
    return out;
}

} // namespace meridian::ai
