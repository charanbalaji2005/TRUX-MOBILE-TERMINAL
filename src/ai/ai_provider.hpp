#pragma once
// meridian-ai-core / ai_provider.hpp
//
// NOT YET IMPLEMENTED. This header exists to pin down the shape of the
// AIProvider interface described in the spec (§47-48) so the rest of the
// architecture — Meridian AI's controller, the popup UI, command
// correction/explain/generate/diagnose — can be built against a stable
// contract later without redesigning it. There is no .cpp file for this
// header, no HTTP client, and nothing here has been compiled against a
// real network call.
//
// Why it stops here: implementing Groq/Gemini/Ollama Cloud for real means
// writing and testing HTTPS + JSON request/response code, and this
// project was built in a sandbox with no network access — there was no
// way to make a real API call, see a real response, or catch a real bug
// in that code. Writing it anyway would mean shipping networking code
// that looks plausible but has never actually talked to a server, which
// is exactly the kind of unverified code this project has otherwise
// avoided. See docs/ai.md and docs/status.md for the concrete next step.

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace meridian::ai {

enum class RiskLevel { Low, Medium, High, Critical };

struct AIResponse {
    std::string explanation;
    std::optional<std::string> suggested_command;
    double confidence = 0.0;
    std::vector<std::string> warnings;
    RiskLevel risk = RiskLevel::Low;
};

struct RequestContext {
    std::string current_command;
    std::optional<std::string> last_command;
    std::optional<int> last_exit_code;
    std::optional<std::string> last_stderr;
    std::string working_directory;
    // Deliberately NOT included by default: full scrollback, full
    // environment, file contents. See §60-61 (secret redaction / context
    // control) — those policies belong here once this is implemented.
};

enum class AIOperation { Fix, Explain, Generate, Diagnose, Ask };

// The common interface every provider (Groq, Gemini, Ollama Cloud, and
// any future OpenAI-compatible endpoint) implements, per spec §48. The
// AI core is meant to talk to providers ONLY through this interface —
// never with provider-specific branches in command-correction/explain
// logic elsewhere.
class AIProvider {
public:
    virtual ~AIProvider() = default;

    virtual std::string id() const = 0;   // e.g. "groq"
    virtual std::string name() const = 0; // e.g. "Groq Cloud"

    virtual bool authenticate(const std::string& api_key) = 0;
    virtual bool validate_credentials() = 0;
    virtual std::vector<std::string> list_models() = 0;

    virtual AIResponse generate(AIOperation op, const RequestContext& ctx, const std::string& model) = 0;

    // Streaming variant: invokes `on_token` as partial text arrives.
    // Returns the same normalized AIResponse once the stream completes.
    virtual AIResponse stream(AIOperation op, const RequestContext& ctx, const std::string& model,
                               const std::function<void(const std::string& partial_text)>& on_token) = 0;

    virtual bool health_check() = 0;
};

} // namespace meridian::ai
