---
layout: default
title: "Component Status Matrix"
category: "Status"
---

# Status — what's real here, and what isn't yet

This document exists because the spec this repo was built from
explicitly asked for no faked-out code, an honest split between
"working and verified" / "not implemented" / "implemented but not
testable here," and always-compiled, always-tested progress. This is
the ledger for that, kept current across three build sessions so far.

## Environment this repo was built in

No network access, no `cmake`, no `Qt6`, no `googletest`, no display
server. Only `g++ 13.3` and `make`. That shaped every decision below.

## What's implemented, compiled, and tested

| Component | Files | Status |
|---|---|---|
| VT cell/color/attribute model | `src/core/vt/types.hpp` | done |
| Screen buffer | `src/core/vt/screen_buffer.{hpp,cpp}` | done, unit-tested |
| ANSI/VT parser | `src/core/vt/ansi_parser.{hpp,cpp}` | done, unit-tested incl. adversarial input |
| PTY manager | `src/core/pty/pty_manager.{hpp,cpp}` | done, tested against real spawned processes |
| Shared config store | `src/core/config.{hpp,cpp}` | done, tested (extracted from AI-only config this session) |
| Meridian Shell: lexer/parser/AST | `src/shell/{lexer,parser,ast}.{hpp,cpp}` | done, unit-tested |
| Meridian Shell: executor | `src/shell/executor.{hpp,cpp}` | done, integration-tested |
| Meridian Shell: real job control | `src/shell/executor.{hpp,cpp}` | done, tested via real Ctrl+Z through a real PTY |
| Meridian Shell: builtins | `src/shell/builtins.{hpp,cpp}` | done, tested |
| Meridian Shell: `Shell` class (REPL extracted from main) | `src/shell/shell.{hpp,cpp}` | done, unit-tested (new this session) |
| Meridian AI: secret redactor | `src/ai/secret_redactor.{hpp,cpp}` | done, tested |
| Meridian AI: risk classifier | `src/ai/risk_classifier.{hpp,cpp}` | done, tested |
| Meridian AI: local command analyzer | `src/ai/command_analyzer.{hpp,cpp}` | done, tested against real PATH |
| Meridian AI: `CommandContext` (cwd/git/OS info) | `src/ai/command_context.{hpp,cpp}` | done, tested (new this session) |
| Meridian AI: controller + CLI | `src/ai/ai_controller.{hpp,cpp}`, `src/app/meridian_main.cpp` | done, tested |
| Terminal config (scrollback/shell/font/theme/...) | `src/config/terminal_config.{hpp,cpp}` | done, tested (new this session) |
| Keybindings (spec-matching defaults) | `src/config/keybindings.{hpp,cpp}` | done, tested (new this session) |
| Credential store (file fallback, real Secret Service not available) | `src/security/credential_store.{hpp,cpp}` | done, tested incl. real on-disk permission checks (new this session) |
| Headless PTY+ANSI integration demo | `src/app/demo_main.cpp` | done |
| Standalone `meridian-shell` binary | `src/app/shell_main.cpp` | done |
| `meridian` CLI (`meridian ai ...`) | `src/app/meridian_main.cpp` | done |
| `tests/manual_core_test.sh` (spec §78's exact command list) | `tests/manual_core_test.sh` | done, 11/11 checks pass (new this session) |

**109 tests, 251 assertions, all passing.** Run `make test`. Also
`./tests/manual_core_test.sh` (11/11 checks) — the exact manual test
sequence spec §78 names, run for real against the real binary and
actually checked against expected output. The full suite has been run
clean under `-fsanitize=address,undefined` three times now (after the
core+shell milestone, after job control + the AI layer, and after this
session's config/security/CommandContext/Shell-class additions),
including tests that fork real processes, open real PTYs, deliver real
signals, and pipe real data.

A throughput stress test (2.3 MB / 300k lines of real `seq` output
through the PTY+parser pipeline) was run manually; no crash, no memory
growth, correct end-state.

## File-tree completeness vs. the spec

`docs/architecture.md` has a full table mapping every spec-named file
that doesn't exist under its spec-given name to where its logic
actually lives (mostly deliberate consolidation — e.g.
`terminal/{Screen,Cell,Cursor,Scrollback}.*` -> `core/vt/screen_buffer.*`
— not omission). Placeholder directories (`cmake/`, `resources/{themes,
icons,fonts}/`, `packaging/`) now exist with README notes explaining
why they're empty, rather than being silently absent from the tree.

## Known, documented simplifications

Carried over from earlier sessions — see the comments at each site:
- Combining Unicode marks recognized but not merged onto the base glyph.
- East Asian width uses a pragmatic range table, not full Unicode UAX #11.
- SGR colon-separated subparameters not parsed (semicolon form only).
- Bare `FOO=bar` variable assignment not recognized (only `export` works).
- No tab completion / readline-style line editing.
- Job control: no `%N`-style specs beyond a plain number; a killed
  background job reports "Done" rather than "Killed."
- Risk classifier / secret redactor: pattern-based, not semantic.
- Command analyzer: command-name typos only, not subcommand-level.
- `Config`/`TerminalConfig`/`Keybindings`: flat key=value, not real TOML.

New this session:
- **`CredentialStore`**: the file-based fallback is real and tested
  (including actual on-disk permission verification), but it is
  explicitly NOT the Secret-Service-backed storage the spec prefers —
  `libsecret-1-dev` isn't installed and there's no network to get it.
  `backend_name()` says so at runtime rather than pretending otherwise.
- **`JobControl` stays inside `Executor`** rather than becoming its own
  class matching the spec's `shell/JobControl.{cpp,hpp}` naming — the
  OS-level process-group/tcsetpgrp sequence that creates a job is
  interleaved with pipeline-stage forking in a way that made extraction
  a real refactor of already-tested, sanitizer-clean code for mostly
  cosmetic benefit. Documented as a deliberate call in
  `docs/architecture.md`, not a miss.
- **`Shell::run_interactive`'s `out`/`err` parameters only carry the
  prompt and Shell's own error messages, not builtin/program output** —
  this was actually a bug in the *first version* of the new tests (which
  assumed otherwise), caught immediately by running them and getting
  real failures, not by inspection. See "Bugs this session actually
  caught" below.

## Not done — and why

**Qt6 GUI** and **Groq/Gemini/Ollama Cloud providers**: unchanged — no
Qt6, no network access in this environment. See `docs/gui.md`,
`docs/renderer.md` (new this session, scoped narrowly to spec's exact
`docs/renderer.md` filename), and `docs/ai.md`.

**`cmake` build path**: still unverified by actual execution.

## Bugs actually caught by testing, across all sessions

Kept because "we tested it and it worked first try" is a less credible
claim than showing what testing found.

**Sessions 1-2** (unchanged, see prior write-ups): `_exit()` skipping
iostream flush and dropping redirected builtin output; ANSI OSC-overflow
recovery leaking garbage onto the screen; shell lexer splitting `$(...)`
on internal spaces; `extern char** environ` declared inside a namespace;
a test harness assuming non-blocking `read()` on a blocking PTY fd
(hung the whole suite); plain Levenshtein under-scoring the most common
typo shape (transposition); a redaction regex eating a trailing quote.

**This session:**

1. **`Shell::run_interactive`'s `out` ostream parameter didn't capture
   what the first version of its tests assumed it would.** The initial
   `tests/test_shell_class.cpp` checked `out.str()` for `echo`'s output
   and failed — three tests, four assertions. The actual cause: builtins
   write to the real process `std::cout` directly (which IS fd 1,
   possibly `dup2`'d to a redirect target), not to whatever `ostream`
   `Shell::run_interactive` happens to receive — and that's structurally
   necessary, not a bug in `Shell` itself: a forked child's `std::cout`
   can't be redirected into a parent-process `std::ostringstream` across
   a `fork()` boundary, and even the in-process-builtin fast path would
   need invasive global `std::cout.rdbuf()` swapping to behave
   consistently with the forked path. Fixed by rewriting the doc comment
   on `run_interactive` to state plainly what `out`/`err` actually carry
   (prompt + Shell's own error messages only), and rewriting the tests
   to check real observable behavior instead (history contents, exit
   codes, error-message routing) — output content is already correctly
   verified elsewhere, via real file redirection
   (`tests/test_shell_executor.cpp`) and a real PTY
   (`tests/test_job_control.cpp`, `tests/manual_core_test.sh`).

2. **`Shell(interactive=true)` originally called `enable_job_control()`
   directly from its constructor.** That mutates real process-global
   state — signal dispositions (`SIG_IGN` for `SIGINT`/`SIGTSTP`/etc.)
   and the process's actual group/terminal ownership via `setpgid`/
   `tcsetpgrp` on the real `STDIN_FILENO`. Since all tests in this
   suite run inside one shared process, constructing `Shell(true)` in a
   test — which the very next test (`shell_interactive_prints_a_prompt_
   per_line`) needed to do to test prompt-printing behavior — would have
   silently changed signal handling for every test that ran afterward in
   the same process, including ones (like `test_pty_manager.cpp`'s
   spawns) that assume default disposition in a forked child. Caught by
   reasoning through *why* the first fix above mattered, not by an
   actual observed failure — but real enough that it was fixed before
   any such failure could occur, by decoupling "print a prompt" (the
   constructor's `interactive` flag) from "claim real job control" (now
   an explicit, separately-called `enable_job_control()`, invoked
   exactly once, only by `src/app/shell_main.cpp`, never by a test).

## If you're picking this up next

1. `make test` and `./tests/manual_core_test.sh` — confirm the same
   baseline (109 tests / 251 assertions / 11 manual checks).
2. Try the real `cmake` path on a machine that has it.
3. Pick ONE AI provider (Groq) and implement `AIProvider` against the
   real API, storing its key via `CredentialStore` (swap in a real
   Secret Service backend first if `libsecret-1-dev` is available on
   that machine — the interface is designed for that swap to not touch
   calling code).
4. GUI: start with a plain window rendering the existing
   `ScreenBuffer`/`AnsiParser` as text, per `docs/renderer.md`'s
   concrete-next-step section.
5. If revisiting the `JobControl`-inside-`Executor` consolidation
   decision: extract the `Job` bookkeeping (struct + `jobs_` +
   `find_job`/`reap_job_status_changes`/`do_fg`/`do_bg`/`jobs_report`)
   into its own class first, since that part genuinely doesn't need to
   touch the fork/pipe-wiring loop — leave process-group creation
   (`setpgid`/`tcsetpgrp` inside `run_pipeline`) where it is unless a
   second caller actually needs it independently.
