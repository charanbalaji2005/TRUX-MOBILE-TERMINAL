#pragma once
// meridian-terminal / ai / ai_agent.hpp
//
// Autonomous terminal & coding agent engine ("meridian agent").
// Orchestrates multi-step repository inspection, build execution,
// error diagnostics, patch formulation, permission gates, and verification.

#include "error_diagnostics.hpp"
#include "intent_engine.hpp"
#include "risk_classifier.hpp"

#include <functional>
#include <string>
#include <vector>

namespace meridian::ai {

enum class AgentStatus {
    Idle,
    Planning,
    Executing,
    WaitingForApproval,
    Completed,
    Failed
};

enum class AgentActionType {
    InspectEnvironment,
    RunBuild,
    DiagnoseFailure,
    ProposePatch,
    ApplyPatch,
    VerifyResolution
};

struct AgentStep {
    int step_number = 1;
    AgentActionType action_type = AgentActionType::InspectEnvironment;
    std::string description;
    std::string command;
    std::string target_file;
    std::string proposed_diff;
    std::string output;
    bool requires_approval = false;
    bool approved = false;
    bool completed = false;
};

struct AgentGoal {
    std::string description;
    std::string working_dir = ".";
    int max_steps = 10;
    bool auto_approve_safe = false;
};

class AiAgent {
public:
    AiAgent();

    void set_goal(const AgentGoal& goal);
    AgentStatus status() const { return status_; }
    const std::vector<AgentStep>& steps() const { return steps_; }
    int current_step_index() const { return current_step_idx_; }

    // Advances one step of the agent execution loop
    bool run_next_step();

    // User approves or rejects a pending step
    void approve_current_step(bool approve);

    // Summary reports
    std::string format_progress() const;

private:
    AgentGoal goal_;
    AgentStatus status_ = AgentStatus::Idle;
    std::vector<AgentStep> steps_;
    int current_step_idx_ = 0;

    IntentEngine intent_engine_;
    ErrorDiagnostics diagnostics_;
    RiskClassifier risk_classifier_;

    void plan_steps_for_goal();
    std::string execute_shell_command(const std::string& cmd);
};

} // namespace meridian::ai

