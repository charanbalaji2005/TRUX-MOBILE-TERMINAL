#include "mini_test.hpp"
#include "../src/ai/ai_agent.hpp"
#include "../src/ai/error_diagnostics.hpp"
#include "../src/ai/intent_engine.hpp"

using namespace meridian::ai;

MTEST(intent_engine_translations) {
    IntentEngine engine;

    // 1. Find modified files
    auto res1 = engine.translate("find all javascript files modified in the last 7 days");
    ASSERT_TRUE(res1.generated_command.find("find . -type f") != std::string::npos);
    ASSERT_TRUE(res1.generated_command.find("*.js") != std::string::npos);
    ASSERT_TRUE(res1.generated_command.find("-mtime -7") != std::string::npos);
    ASSERT_EQ(res1.risk, RiskLevel::Low);

    // 2. Kill process on port
    auto res2 = engine.translate("kill process running on port 3000");
    ASSERT_TRUE(res2.generated_command.find("3000") != std::string::npos);
    ASSERT_TRUE(res2.generated_command.find("kill") != std::string::npos);

    // 3. Git undo
    auto res3 = engine.translate("undo last commit without losing changes");
    ASSERT_TRUE(res3.generated_command.find("git reset --soft") != std::string::npos);

    // 4. Dangerous command risk detection in intent
    auto res4 = engine.translate("recursively force remove root directory");
    ASSERT_EQ(res4.risk, RiskLevel::Critical);
}

MTEST(error_diagnostics_patterns) {
    ErrorDiagnostics diag;

    // Node.js missing module
    std::string node_err = "Error: Cannot find module 'express'\nRequire stack:\n- /app/server.js";
    auto card1 = diag.analyze(node_err);
    ASSERT_EQ(card1.category, DiagnosticCategory::NodeJs);
    ASSERT_TRUE(card1.title.find("express") != std::string::npos);
    ASSERT_EQ(card1.suggested_fixes.size(), 2u);
    ASSERT_EQ(card1.suggested_fixes[0].command, "npm install express");

    // Python missing module
    std::string py_err = "Traceback (most recent call last):\n  File \"main.py\", line 1\nModuleNotFoundError: No module named 'requests'";
    auto card2 = diag.analyze(py_err);
    ASSERT_EQ(card2.category, DiagnosticCategory::Python);
    ASSERT_TRUE(card2.title.find("requests") != std::string::npos);
    ASSERT_EQ(card2.suggested_fixes[0].command, "pip install requests");

    // Port conflict
    std::string port_err = "events.js:292\nError: listen EADDRINUSE: address already in use :::8080";
    auto card3 = diag.analyze(port_err);
    ASSERT_EQ(card3.category, DiagnosticCategory::PortConflict);
    ASSERT_TRUE(card3.title.find("8080") != std::string::npos);

    // C++ compiler missing header
    std::string cpp_err = "src/main.cpp:2:10: fatal error: openssl/ssl.h: No such file or directory";
    auto card4 = diag.analyze(cpp_err);
    ASSERT_EQ(card4.category, DiagnosticCategory::CppCompiler);
    ASSERT_TRUE(card4.title.find("openssl/ssl.h") != std::string::npos);
}

MTEST(ai_agent_planning_and_lifecycle) {
    AiAgent agent;
    AgentGoal goal;
    goal.description = "Inspect repository and verify test harness";
    goal.working_dir = ".";

    agent.set_goal(goal);
    ASSERT_GT(agent.steps().size(), 0u);
    ASSERT_EQ(agent.status(), AgentStatus::Executing);

    // Progress format report
    std::string progress = agent.format_progress();
    ASSERT_TRUE(progress.find("Meridian AI Agent") != std::string::npos);
}

