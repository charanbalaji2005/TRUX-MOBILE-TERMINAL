

#include "mini_test.hpp"
#include "../src/shell/shell.hpp"

#include <sstream>

using namespace meridian::shell;

MTEST(shell_noninteractive_prints_no_prompt) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("echo hi\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    ASSERT_TRUE(out.str().find("meridian:") == std::string::npos);
}

MTEST(shell_interactive_prints_a_prompt_per_line) {
    Shell shell(/*interactive=*/true);
    std::istringstream in("echo a\necho b\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    // One prompt printed before each of the two lines read, plus a
    // trailing prompt before hitting EOF on the third read attempt.
    std::size_t count = 0, pos = 0;
    while ((pos = out.str().find("meridian:", pos)) != std::string::npos) { count++; pos += 1; }
    ASSERT_EQ(count, (std::size_t)3);
}

MTEST(shell_exit_builtin_stops_the_loop_with_its_code) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("echo before\nexit 5\necho after\n");
    std::ostringstream out, err;
    int status = shell.run_interactive(in, out, err);
    ASSERT_EQ(status, 5);
    // History is a real, checkable side effect: the third line must
    // never have been reached once `exit` fired on the second.
    const auto& h = shell.executor().history();
    ASSERT_EQ(h.size(), (std::size_t)2);
    ASSERT_EQ(h[0], std::string("echo before"));
    ASSERT_EQ(h[1], std::string("exit 5"));
}

MTEST(shell_history_accumulates_across_lines) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("echo a\necho b\necho c\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    ASSERT_EQ(shell.executor().history().size(), (std::size_t)3);
}

MTEST(shell_run_command_matches_dash_c_semantics) {
    Shell shell(/*interactive=*/false);
    std::ostringstream err;
    int status = shell.run_command("exit 3", err);
    ASSERT_EQ(status, 3);
}

MTEST(shell_syntax_error_reported_to_err_stream) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("| bad\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    // Lex/parse errors ARE routed through the err ostream directly by
    // Shell itself (unlike builtin output) — this is real, checkable
    // behavior.
    ASSERT_FALSE(err.str().empty());
    ASSERT_TRUE(err.str().find("meridian-shell:") != std::string::npos);
}

MTEST(shell_valid_command_after_syntax_error_still_runs) {
    Shell shell(/*interactive=*/false);
    std::istringstream in("| bad\necho recovered\n");
    std::ostringstream out, err;
    shell.run_interactive(in, out, err);
    ASSERT_FALSE(err.str().empty()); // the syntax error was reported
    // A syntax error on one line must not stop later lines from running.
    const auto& h = shell.executor().history();
    ASSERT_TRUE(h.size() >= 1);
    ASSERT_EQ(h.back(), std::string("echo recovered"));
}
