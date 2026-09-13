// tests / test_command_context.cpp
#include "mini_test.hpp"
#include "../src/ai/command_context.hpp"

#include <cstdio>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace meridian::ai;

namespace {
std::string sandbox_dir() {
    return "/tmp/meridian_test_gitctx_" + std::to_string(getpid());
}
} // namespace

MTEST(context_builder_fills_cwd_shell_and_os_info) {
    auto ctx = CommandContextBuilder::build("ls -la");
    ASSERT_EQ(ctx.current_command, std::string("ls -la"));
    ASSERT_FALSE(ctx.working_directory.empty());
    ASSERT_FALSE(ctx.shell_name.empty());
    ASSERT_FALSE(ctx.os_info.empty());
}

MTEST(context_builder_carries_last_command_state) {
    auto ctx = CommandContextBuilder::build("npm run dev", std::string("npm install"), 1, std::string("EADDRINUSE"));
    ASSERT_TRUE(ctx.last_command.has_value());
    if (ctx.last_command) ASSERT_EQ(*ctx.last_command, std::string("npm install"));
    ASSERT_TRUE(ctx.last_exit_code.has_value());
    if (ctx.last_exit_code) ASSERT_EQ(*ctx.last_exit_code, 1);
    ASSERT_TRUE(ctx.last_stderr.has_value());
}

MTEST(context_builder_reports_no_git_repo_outside_one) {
    // /tmp itself is very unlikely to be inside a git repo.
    auto ctx = CommandContextBuilder::build("ls", std::nullopt, std::nullopt, std::nullopt, "/tmp");
    ASSERT_FALSE(ctx.in_git_repo);
    ASSERT_FALSE(ctx.git_branch.has_value());
}

MTEST(git_branch_detected_from_real_dot_git_head_file) {
    // Build a minimal real .git/HEAD (just the one file this code
    // reads) rather than requiring an actual `git` binary or repo.
    std::string dir = sandbox_dir();
    std::string git_dir = dir + "/.git";
    std::string mkdir_cmd = "mkdir -p " + git_dir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort setup */ }

    {
        std::ofstream head(git_dir + "/HEAD");
        head << "ref: refs/heads/feature/cool-thing\n";
    }

    auto result = CommandContextBuilder::find_git_branch(dir);
    ASSERT_TRUE(result.first);
    ASSERT_TRUE(result.second.has_value());
    if (result.second) ASSERT_EQ(*result.second, std::string("feature/cool-thing"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}

MTEST(git_branch_detected_from_subdirectory_by_walking_up) {
    std::string dir = sandbox_dir() + "_walkup";
    std::string git_dir = dir + "/.git";
    std::string subdir = dir + "/src/deep/nested";
    std::string mkdir_cmd = "mkdir -p " + git_dir + " && mkdir -p " + subdir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort */ }

    {
        std::ofstream head(git_dir + "/HEAD");
        head << "ref: refs/heads/main\n";
    }

    auto result = CommandContextBuilder::find_git_branch(subdir);
    ASSERT_TRUE(result.first);
    if (result.second) ASSERT_EQ(*result.second, std::string("main"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}

MTEST(git_branch_handles_detached_head_as_raw_hash) {
    std::string dir = sandbox_dir() + "_detached";
    std::string git_dir = dir + "/.git";
    std::string mkdir_cmd = "mkdir -p " + git_dir;
    if (system(mkdir_cmd.c_str()) != 0) { /* best-effort */ }

    {
        std::ofstream head(git_dir + "/HEAD");
        head << "abc123def456\n"; // detached HEAD: raw commit hash, no "ref:" prefix
    }

    auto result = CommandContextBuilder::find_git_branch(dir);
    ASSERT_TRUE(result.first);
    if (result.second) ASSERT_EQ(*result.second, std::string("abc123def456"));

    std::string cleanup = "rm -rf " + dir;
    if (system(cleanup.c_str()) != 0) { /* best-effort */ }
}
