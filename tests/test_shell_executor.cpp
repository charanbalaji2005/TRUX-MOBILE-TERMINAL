// tests / test_shell_executor.cpp
//
// These are true integration tests: they run real external programs
// (coreutils) through Executor's fork/exec/pipe/dup2 path, and check
// results by reading back files the child processes actually wrote —
// not mocked output.

#include "mini_test.hpp"
#include "../src/shell/executor.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace meridian::shell;

namespace {

std::string read_file(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string tmp_path(const std::string& name) {
    return "/tmp/meridian_test_" + name + "_" + std::to_string(getpid());
}

} // namespace

MTEST(executor_runs_external_program_and_reports_exit_status) {
    Executor ex;
    int status = ex.run_line("true");
    ASSERT_EQ(status, 0);
    status = ex.run_line("false");
    ASSERT_NE(status, 0);
}

MTEST(executor_redirects_stdout_to_file) {
    Executor ex;
    std::string f = tmp_path("redir");
    int status = ex.run_line("echo hello > " + f);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(read_file(f), std::string("hello\n"));
    std::remove(f.c_str());
}

MTEST(executor_runs_real_pipeline_through_coreutils) {
    Executor ex;
    std::string f = tmp_path("pipeline");
    int status = ex.run_line("printf 'b\\nc\\na\\n' | sort > " + f);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(read_file(f), std::string("a\nb\nc\n"));
    std::remove(f.c_str());
}

MTEST(executor_and_short_circuits_on_failure) {
    Executor ex;
    std::string f = tmp_path("and");
    std::remove(f.c_str());
    ex.run_line("false && echo nope > " + f);
    std::ifstream check(f);
    ASSERT_FALSE(check.good() && check.peek() != std::ifstream::traits_type::eof());
}

MTEST(executor_or_runs_fallback_on_failure) {
    Executor ex;
    std::string f = tmp_path("or");
    std::remove(f.c_str());
    ex.run_line("false || echo yep > " + f);
    ASSERT_EQ(read_file(f), std::string("yep\n"));
    std::remove(f.c_str());
}

MTEST(executor_cd_builtin_changes_real_process_directory) {
    Executor ex;
    char before[4096];
    if (!getcwd(before, sizeof(before))) before[0] = '\0';
    ex.run_line("cd /tmp");
    char after[4096];
    if (!getcwd(after, sizeof(after))) after[0] = '\0';
    ASSERT_EQ(std::string(after), std::string("/tmp"));
    if (chdir(before) != 0) { /* best-effort restore */ }
}

MTEST(executor_export_and_variable_expansion) {
    Executor ex;
    ex.run_line("export MERIDIAN_TEST_VAR=hello123");
    std::string f = tmp_path("var");
    ex.run_line("echo $MERIDIAN_TEST_VAR > " + f);
    ASSERT_EQ(read_file(f), std::string("hello123\n"));
    std::remove(f.c_str());
    unsetenv("MERIDIAN_TEST_VAR");
}

MTEST(executor_command_substitution_uses_own_engine_not_bash) {
    Executor ex;
    std::string f = tmp_path("cmdsub");
    ex.run_line("echo [$(echo inner)] > " + f);
    ASSERT_EQ(read_file(f), std::string("[inner]\n"));
    std::remove(f.c_str());
}

MTEST(executor_exit_builtin_sets_exit_state) {
    Executor ex;
    ASSERT_FALSE(ex.exit_requested());
    ex.run_line("exit 7");
    ASSERT_TRUE(ex.exit_requested());
    ASSERT_EQ(ex.exit_code(), 7);
}

MTEST(executor_dollar_question_reports_last_status) {
    Executor ex;
    ex.run_line("false");
    std::string f = tmp_path("status");
    ex.run_line("echo $? > " + f);
    ASSERT_EQ(read_file(f), std::string("1\n"));
    std::remove(f.c_str());
}

MTEST(executor_single_quotes_prevent_expansion) {
    Executor ex;
    setenv("MERIDIAN_TEST_VAR2", "should_not_appear", 1);
    std::string f = tmp_path("quote");
    ex.run_line("echo '$MERIDIAN_TEST_VAR2' > " + f);
    ASSERT_EQ(read_file(f), std::string("$MERIDIAN_TEST_VAR2\n"));
    std::remove(f.c_str());
    unsetenv("MERIDIAN_TEST_VAR2");
}
