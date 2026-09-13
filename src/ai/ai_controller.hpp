#pragma once
// meridian-ai / ai_controller.hpp
//
// Ties together Config + CommandAnalyzer + RiskClassifier +
// SecretRedactor into the state machine spec §36/§37/§59/§61/§64
// describe (on/off, detection on/off, privacy mode, provider/model
// selection) and the local-only analysis spec §38-41 describes.
//
// What this does NOT do: call any AI provider. `explain()` and
// `fix_via_provider()`-shaped operations that need real language
// generation report plainly that no provider is configured/implemented
// rather than fabricating a plausible-looking answer — see docs/ai.md.

#include "command_analyzer.hpp"
#include "../core/config.hpp"
#include "risk_classifier.hpp"
#include "secret_redactor.hpp"
#include <string>
#include <vector>

namespace meridian::ai {

class AIController {
public:
    // `config_path` is injected (not hardcoded to ~/.config/meridian)
    // so tests can point it at a throwaway file.
    explicit AIController(std::string config_path);

    void load();
    void save();

    bool enabled() const { return config_.get_bool("ai.enabled", false); }
    bool detection_enabled() const { return config_.get_bool("ai.detect", true); }
    bool privacy_mode() const { return config_.get_bool("ai.privacy", false); }
    std::string provider() const { return config_.get("ai.provider", "(none configured)"); }
    std::string model() const { return config_.get("ai.model", "(none configured)"); }

    void set_enabled(bool v) { config_.set_bool("ai.enabled", v); }
    void set_detection_enabled(bool v) { config_.set_bool("ai.detect", v); }
    void set_privacy_mode(bool v) { config_.set_bool("ai.privacy", v); }
    void set_provider(const std::string& p) { config_.set("ai.provider", p); }

    std::string status_report() const;

    // Real local analysis: PATH-based typo suggestion + risk
    // classification. Returns a formatted, ready-to-print report. Safe
    // to call regardless of enabled()/privacy_mode() — this never
    // touches the network, so those settings don't gate it; they gate
    // whether a GUI/CLI layer *would* also escalate to a cloud provider
    // (which isn't implemented — see explain_command()).
    std::string analyze_command(const std::string& command_line);

    // Always honest about there being no provider behind this yet.
    std::string explain_command(const std::string& command_line) const;

    // Pure secret redaction, exposed directly for the `meridian ai
    // redact` CLI command and for reuse by a future provider layer.
    std::string redact(const std::string& text) const { return redactor_.redact(text); }

    // Report of provider connectivity — since no provider is
    // implemented, this always reports so, per provider, honestly
    // (spec §62 requires "do not crash the terminal" and a clear
    // "Provider unavailable" message — this is that message).
    std::string test_providers() const;

private:
    Config config_;
    CommandAnalyzer analyzer_;
    RiskClassifier risk_;
    SecretRedactor redactor_;
};

} // namespace meridian::ai
