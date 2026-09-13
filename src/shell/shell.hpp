#pragma once
// meridian-shell / shell.hpp
//
// Spec's top-level "Shell" component: wraps Executor with the REPL loop
// and prompt-string logic that used to live directly in
// src/app/shell_main.cpp. Extracting it here means the REPL behavior —
// prompt formatting, line-by-line execution, history, exit handling —
// is unit-testable against plain std::istream/std::ostream, without
// needing a real PTY for every case (job control specifically still
// needs a real PTY to test meaningfully — see tests/test_job_control.cpp
// — but everything else here doesn't).

#include "executor.hpp"
#include <iostream>
#include <string>

namespace meridian::shell {

class Shell {
public:
    // `interactive` controls only prompt printing here. Real job
    // control (process-group/terminal ownership, signal disposition
    // changes) is NOT enabled implicitly by this constructor — it has
    // real, process-global side effects (mutates this process's actual
    // signal handlers and process group), so it's something the caller
    // opts into explicitly via enable_job_control() below, once, in a
    // real process. Bundling it into the constructor would make `Shell`
    // unsafe to construct more than once per process (as tests do) —
    // see tests/test_shell_class.cpp for exactly that scenario.
    explicit Shell(bool interactive);

    // Enables real job control (see docs/shell.md): process groups,
    // tcsetpgrp terminal handoff, ignoring job-control signals in this
    // process. Only call this once, in a real interactive process with
    // an actual controlling terminal (see src/app/shell_main.cpp) —
    // never from a test that shares a process with other tests.
    void enable_job_control() { executor_.enable_job_control(); }

    // Reads lines from `in` until EOF or an `exit` command. Returns the
    // process exit code.
    //
    // IMPORTANT about `out`/`err`: they receive the interactive prompt
    // and this Shell's own error messages (lex/parse failures) — NOT
    // builtin or program output. Builtins write to the real process
    // std::cout/std::cerr directly (which IS fd 1/2, possibly dup2'd to
    // a redirect target), and external programs get their own stdio via
    // exec — neither can be redirected into an arbitrary ostream
    // without OS-level fd redirection, which is what the file- and
    // PTY-based tests exercise instead (see tests/test_shell_executor.cpp,
    // tests/test_job_control.cpp). Passing a stringstream here lets you
    // test prompt behavior, error routing, and control flow without a
    // real terminal — it is deliberately not a general output-capture
    // mechanism, because one can't exist uniformly across both the
    // in-process-builtin and forked-child code paths.
    int run_interactive(std::istream& in, std::ostream& out, std::ostream& err);

    // Runs a single command line (the `-c` mode). Returns its exit code.
    int run_command(const std::string& command, std::ostream& err);

    Executor& executor() { return executor_; }
    bool interactive() const { return interactive_; }

private:
    std::string prompt() const;

    Executor executor_;
    bool interactive_;
};

} // namespace meridian::shell
