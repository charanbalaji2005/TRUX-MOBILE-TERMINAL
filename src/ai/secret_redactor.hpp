#pragma once
// meridian-ai / secret_redactor.hpp
//
// Pattern-based redaction of likely secrets (API keys, tokens, passwords,
// private key blocks) before any text would be sent to a cloud AI
// provider — spec §60/§65. This is pure local pattern matching: no
// network, fully real, fully testable.
//
// This is a pragmatic v1: known common formats (KEY=VALUE style env
// vars, Authorization: Bearer headers, AWS/GitHub/Slack/OpenAI-style key
// prefixes, PEM private key blocks). It is NOT a claim of catching every
// possible secret shape — see docs/ai.md for what's explicitly out of
// scope and why redaction should never be the only layer of defense.

#include <string>

namespace meridian::ai {

class SecretRedactor {
public:
    // Returns a copy of `text` with likely secrets replaced by
    // "[REDACTED]". If `redaction_count` is non-null, it's set to how
    // many redactions were made (0 if none) — for logging/audit
    // purposes without the caller ever needing to see the actual value.
    std::string redact(const std::string& text, int* redaction_count = nullptr) const;
};

} // namespace meridian::ai
