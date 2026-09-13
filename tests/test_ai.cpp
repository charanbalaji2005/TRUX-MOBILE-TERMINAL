// tests / test_ai.cpp
#include "mini_test.hpp"
#include "../src/ai/ai_controller.hpp"
#include "../src/ai/command_analyzer.hpp"
#include "../src/ai/risk_classifier.hpp"
#include "../src/ai/secret_redactor.hpp"
#include "../src/core/config.hpp"

#include <cstdio>
#include <unistd.h>

using namespace meridian::ai;

// ---- SecretRedactor ----

MTEST(redactor_finds_key_value_style_secret) {
    SecretRedactor r;
    int count = 0;
    std::string out = r.redact("API_KEY=sk-abc123def456ghi789", &count);
    ASSERT_TRUE(out.find("sk-abc123def456ghi789") == std::string::npos);
    ASSERT_TRUE(out.find("API_KEY=") != std::string::npos); // key name preserved
    ASSERT_TRUE(out.find("[REDACTED]") != std::string::npos);
    ASSERT_TRUE(count >= 1);
}

MTEST(redactor_finds_bearer_token) {
    SecretRedactor r;
    std::string out = r.redact("curl -H \"Authorization: Bearer abc123xyz789\" https://api.example.com");
    ASSERT_TRUE(out.find("abc123xyz789") == std::string::npos);
    ASSERT_TRUE(out.find("Authorization: Bearer [REDACTED]") != std::string::npos);
}

MTEST(redactor_bearer_token_does_not_eat_trailing_quote) {
    // Regression test: the token pattern used to be \S+, which greedily
    // consumed a trailing closing quote as part of the "token" and
    // silently dropped it from the output.
    SecretRedactor r;
    std::string out = r.redact("curl -H \"Authorization: Bearer xyz789token\"");
    ASSERT_TRUE(out.find("[REDACTED]\"") != std::string::npos);
}

MTEST(redactor_finds_aws_access_key) {
    SecretRedactor r;
    std::string out = r.redact("export AWS_KEY_ID_UNUSED_NAME AKIAIOSFODNN7EXAMPLE end");
    ASSERT_TRUE(out.find("AKIAIOSFODNN7EXAMPLE") == std::string::npos);
}

MTEST(redactor_finds_pem_private_key_block) {
    SecretRedactor r;
    std::string input =
        "before\n-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEA...\n-----END RSA PRIVATE KEY-----\nafter";
    std::string out = r.redact(input);
    ASSERT_TRUE(out.find("MIIEpAIBAAKCAQEA") == std::string::npos);
    ASSERT_TRUE(out.find("before") != std::string::npos);
    ASSERT_TRUE(out.find("after") != std::string::npos);
}

MTEST(redactor_leaves_ordinary_text_alone) {
    SecretRedactor r;
    int count = 0;
    std::string input = "ls -la /home/user/projects";
    std::string out = r.redact(input, &count);
    ASSERT_EQ(out, input);
    ASSERT_EQ(count, 0);
}

// ---- RiskClassifier ----

MTEST(risk_classifies_plain_ls_as_low) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("ls -la").level == RiskLevel::Low);
}

MTEST(risk_classifies_sudo_as_high) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("sudo systemctl restart nginx").level == RiskLevel::High);
}

MTEST(risk_classifies_rm_rf_root_as_critical) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("rm -rf /").level == RiskLevel::Critical);
}

MTEST(risk_classifies_rm_rf_subdir_as_high_not_critical) {
    RiskClassifier c;
    auto result = c.classify("rm -rf /home/user/build");
    ASSERT_TRUE(result.level == RiskLevel::High);
}

MTEST(risk_classifies_plain_rm_as_medium) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("rm old_file.txt").level == RiskLevel::Medium);
}

MTEST(risk_classifies_fork_bomb_as_critical) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify(":(){ :|:& };:").level == RiskLevel::Critical);
}

MTEST(risk_classifies_pipe_to_shell_as_high) {
    RiskClassifier c;
    ASSERT_TRUE(c.classify("curl https://example.com/install.sh | bash").level == RiskLevel::High);
}

// ---- CommandAnalyzer ----

MTEST(analyzer_suggests_correction_for_transposed_typo) {
    CommandAnalyzer a({"echo", "exit"});
    a.refresh_path_index(); // real $PATH scan
    auto s = a.analyze("gerp");
    ASSERT_TRUE(s.has_value());
    if (s) ASSERT_EQ(s->suggested, std::string("grep"));
}

MTEST(analyzer_makes_no_suggestion_for_known_command) {
    CommandAnalyzer a({"echo"});
    a.refresh_path_index();
    auto s = a.analyze("echo");
    ASSERT_FALSE(s.has_value());
}

MTEST(analyzer_makes_no_suggestion_for_wildly_different_bogus_name) {
    CommandAnalyzer a({"echo"});
    a.refresh_path_index();
    auto s = a.analyze("totally_bogus_command_xyz_987654");
    ASSERT_FALSE(s.has_value());
}

MTEST(analyzer_respects_explicit_path_override) {
    // Use an isolated fake PATH so this test doesn't depend on exactly
    // what's installed on whatever machine runs it.
    std::string dir = "/tmp/meridian_ai_test_bin";
    std::string mkdir_cmd = "mkdir -p " + dir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort */ }
    std::string touch_cmd = "touch " + dir + "/mytool && chmod +x " + dir + "/mytool";
    if (system(touch_cmd.c_str()) != 0) { /* best-effort */ }

    CommandAnalyzer a({});
    a.refresh_path_index(dir.c_str());
    auto s = a.analyze("mytoo"); // one char short of "mytool"
    ASSERT_TRUE(s.has_value());
    if (s) ASSERT_EQ(s->suggested, std::string("mytool"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}

MTEST(edit_distance_basic_cases) {
    ASSERT_EQ(edit_distance("", ""), 0);
    ASSERT_EQ(edit_distance("abc", "abc"), 0);
    ASSERT_EQ(edit_distance("abc", "abd"), 1);
    ASSERT_EQ(edit_distance("gerp", "grep"), 1); // adjacent transposition
    ASSERT_EQ(edit_distance("kitten", "sitting"), 3);
}

// ---- Config ----

MTEST(config_round_trips_values) {
    std::string path = "/tmp/meridian_test_config_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    {
        meridian::Config c(path);
        c.set("ai.provider", "groq");
        c.set_bool("ai.enabled", true);
        ASSERT_TRUE(c.save());
    }
    {
        meridian::Config c(path);
        ASSERT_TRUE(c.load());
        ASSERT_EQ(c.get("ai.provider"), std::string("groq"));
        ASSERT_TRUE(c.get_bool("ai.enabled", false));
    }
    std::remove(path.c_str());
}

MTEST(config_missing_file_is_not_an_error) {
    meridian::Config c("/tmp/meridian_definitely_does_not_exist_12345.toml");
    ASSERT_FALSE(c.load()); // false = "didn't exist", not a crash
    ASSERT_EQ(c.get("anything", "default"), std::string("default"));
}

// ---- AIController integration ----

MTEST(controller_analyze_is_silent_when_ai_disabled) {
    std::string path = "/tmp/meridian_test_ctrl_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    AIController ctrl(path);
    ASSERT_FALSE(ctrl.enabled()); // default off
    std::string report = ctrl.analyze_command("gerp foo");
    ASSERT_EQ(report, std::string(""));
    std::remove(path.c_str());
}

MTEST(controller_analyze_detects_typo_and_risk_when_enabled) {
    std::string path = "/tmp/meridian_test_ctrl2_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    AIController ctrl(path);
    ctrl.set_enabled(true);
    ctrl.set_detection_enabled(true);

    std::string report = ctrl.analyze_command("gerp foo");
    ASSERT_TRUE(report.find("grep") != std::string::npos);

    std::string risky = ctrl.analyze_command("sudo rm -rf /");
    ASSERT_TRUE(risky.find("CRITICAL") != std::string::npos);

    std::remove(path.c_str());
}

MTEST(controller_status_report_reflects_state) {
    std::string path = "/tmp/meridian_test_ctrl3_" + std::to_string(getpid()) + ".toml";
    std::remove(path.c_str());
    AIController ctrl(path);
    ctrl.set_enabled(true);
    ctrl.set_provider("groq");
    std::string status = ctrl.status_report();
    ASSERT_TRUE(status.find("ON") != std::string::npos);
    ASSERT_TRUE(status.find("groq") != std::string::npos);
    std::remove(path.c_str());
}

MTEST(controller_explain_is_honest_about_no_provider) {
    AIController ctrl("/tmp/meridian_test_ctrl4.toml");
    std::string explanation = ctrl.explain_command("chmod 755 app.sh");
    ASSERT_TRUE(explanation.find("No AI provider") != std::string::npos);
}
