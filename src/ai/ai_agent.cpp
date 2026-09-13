#include "ai_agent.hpp"

#include <array>
#include <cstdio>
#include <memory>
#include <sstream>

namespace meridian::ai {

AiAgent::AiAgent() = default;

void AiAgent::set_goal(const AgentGoal& goal) {
    goal_ = goal;
    steps_.clear();
    current_step_idx_ = 0;
    status_ = AgentStatus::Planning;
    plan_steps_for_goal();
}

std::string AiAgent::execute_shell_command(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    std::string full_cmd = "cd \"" + goal_.working_dir + "\" && " + cmd + " 2>&1";
    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) return "Failed to execute command.";
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

void AiAgent::plan_steps_for_goal() {
    steps_.clear();
    int num = 1;

    // Step 1: Inspect repository state
    {
        AgentStep s;
        s.step_number = num++;
        s.action_type = AgentActionType::InspectEnvironment;
        s.description = "Inspect repository and working tree status";
        s.command = "git status -s && git branch --show-current";
        s.requires_approval = false;
        steps_.push_back(s);
    }

    // Step 2: Build & Test
    {
        AgentStep s;
        s.step_number = num++;
        s.action_type = AgentActionType::RunBuild;
        s.description = "Run build and test suite to discover failures";
        s.command = "make test";
        s.requires_approval = false;
        steps_.push_back(s);
    }

    // Step 3: Diagnose failure (if any)
    {
        AgentStep s;
        s.step_number = num++;
        s.action_type = AgentActionType::DiagnoseFailure;
        s.description = "Analyze error output and pinpoint root cause";
        s.requires_approval = false;
        steps_.push_back(s);
    }

    // Step 4: Propose patch / remediation
    {
        AgentStep s;
        s.step_number = num++;
        s.action_type = AgentActionType::ProposePatch;
        s.description = "Formulate code fix / patch";
        s.requires_approval = true; // User approval gate
        steps_.push_back(s);
    }

    // Step 5: Verification
    {
        AgentStep s;
        s.step_number = num++;
        s.action_type = AgentActionType::VerifyResolution;
        s.description = "Verify build and run manual test suite";
        s.command = "./tests/manual_core_test.sh";
        s.requires_approval = false;
        steps_.push_back(s);
    }

    status_ = AgentStatus::Executing;
}

bool AiAgent::run_next_step() {
    if (current_step_idx_ >= static_cast<int>(steps_.size())) {
        status_ = AgentStatus::Completed;
        return false;
    }

    auto& step = steps_[current_step_idx_];

    if (step.requires_approval && !step.approved) {
        status_ = AgentStatus::WaitingForApproval;
        return false;
    }

    if (!step.command.empty()) {
        step.output = execute_shell_command(step.command);
    } else if (step.action_type == AgentActionType::DiagnoseFailure) {
        std::string prev_out = (current_step_idx_ > 0) ? steps_[current_step_idx_ - 1].output : "";
        auto diag = diagnostics_.analyze(prev_out);
        step.output = diag.empty() ? "No active build failures found." : diag.format();
    } else if (step.action_type == AgentActionType::ProposePatch) {
        step.output = "Patch formulated and ready for review.";
    }

    step.completed = true;
    current_step_idx_++;

    if (current_step_idx_ >= static_cast<int>(steps_.size())) {
        status_ = AgentStatus::Completed;
    }

    return true;
}

void AiAgent::approve_current_step(bool approve) {
    if (current_step_idx_ < static_cast<int>(steps_.size())) {
        auto& step = steps_[current_step_idx_];
        step.approved = approve;
        if (approve) {
            status_ = AgentStatus::Executing;
            run_next_step();
        } else {
            status_ = AgentStatus::Failed;
        }
    }
}

std::string AiAgent::format_progress() const {
    std::ostringstream ss;
    ss << "┌─── Meridian AI Agent ───────────────────────────────────\n";
    ss << "│ Goal: " << goal_.description << "\n";
    ss << "│ Status: ";
    switch (status_) {
        case AgentStatus::Idle: ss << "Idle"; break;
        case AgentStatus::Planning: ss << "Planning..."; break;
        case AgentStatus::Executing: ss << "Executing..."; break;
        case AgentStatus::WaitingForApproval: ss << "Waiting for user approval [Approve/Reject]"; break;
        case AgentStatus::Completed: ss << "Completed Successfully ✓"; break;
        case AgentStatus::Failed: ss << "Failed ✗"; break;
    }
    ss << "\n│ ────────────────────────────────────────────────────────\n";

    for (const auto& s : steps_) {
        ss << "│ [" << s.step_number << "] "
           << (s.completed ? "✓ " : (s.step_number == current_step_idx_ + 1 ? "▶ " : "  "))
           << s.description << "\n";
        if (!s.command.empty()) {
            ss << "│     $ " << s.command << "\n";
        }
    }
    ss << "└─────────────────────────────────────────────────────────\n";
    return ss.str();
}

} // namespace meridian::ai

