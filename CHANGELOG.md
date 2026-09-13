# Changelog

## Unreleased — file-tree completeness pass (config, security, CommandContext, Shell class, manual test script)

Third work session. Prompted by an explicit request to check the repo
against the spec's full file tree rather than assume coverage — see
`docs/architecture.md`'s new file-naming mapping table for the complete
diff against spec §5, and `docs/status.md` for the running ledger.

### Added

- `tests/manual_core_test.sh` — the exact command sequence spec §78
  names, run for real against the real `meridian-shell` binary with
  actual PASS/FAIL checks per command (11/11 passing), not just a list
  of commands to eyeball.
- `src/ai/command_context.{hpp,cpp}` — real local context gathering:
  cwd, `$SHELL`, OS info via `uname()`, and git branch detection by
  reading `.git/HEAD` directly (no `git` binary dependency), including
  walking up from a subdirectory and handling detached-HEAD state.
- `src/core/config.{hpp,cpp}` — the flat key=value store, extracted
  from what was originally AI-only config (`src/ai/config.*`, removed)
  now that terminal settings and keybindings need the identical
  mechanism.
- `src/config/terminal_config.{hpp,cpp}` — `scrollback_lines`/
  `default_shell`/`startup_directory` (real now) plus `font_family`/
  `font_size`/`theme_name`/`cursor_style`/`cursor_blink`/
  `tab_bar_visible`/`padding` (persisted now, inert until a renderer
  exists — documented as such).
- `src/config/keybindings.{hpp,cpp}` — action-name -> key-combo map
  with defaults matching the spec's own named shortcuts (`Ctrl+Space`
  for the AI popup, `Ctrl+Shift+F` for search, etc).
- `src/security/credential_store.{hpp,cpp}` — `CredentialStore`
  interface plus `FileCredentialStore`, an honestly-labeled fallback
  (`backend_name()` states plainly it's not OS-keyring-backed). Files
  created via `open()` with mode 0600 passed directly to the syscall
  (no create-then-chmod race window); key names sanitized against path
  traversal. Tested including a real `stat()` check of on-disk
  permissions, not just round-trip content.
- `src/shell/shell.{hpp,cpp}` — the `Shell` class, extracting the
  REPL/prompt loop that used to live directly in `shell_main.cpp`,
  making it unit-testable against plain string streams.
- 23 new tests across `test_command_context.cpp`, `test_config.cpp`,
  `test_security.cpp`, `test_shell_class.cpp`, bringing the suite to
  109 tests / 251 assertions.
- `docs/security.md`, `docs/configuration.md`, `docs/renderer.md` —
  the three docs named in spec §84 that didn't exist yet.
- Placeholder `cmake/`, `resources/{themes,icons,fonts}/`, `packaging/`
  directories with README notes, matching spec §5's file tree instead
  of being silently absent.
- `docs/architecture.md`: a full mapping table for every spec-named
  file this repo consolidates differently (e.g.
  `terminal/{Screen,Cell,Cursor,Scrollback}.*` ->
  `core/vt/screen_buffer.*`), stating explicitly which are deliberate
  consolidations vs. genuinely-not-built GUI/provider code.

### Fixed (caught by this session's own testing)

- The first version of `test_shell_class.cpp` assumed
  `Shell::run_interactive`'s `out` parameter would capture builtin
  output (e.g. `echo`) — it doesn't and structurally can't uniformly
  (builtins write to the real process stdout, which is required for
  redirection to work when they run in a forked child). Three tests /
  four assertions failed immediately on first run. Fixed by correcting
  the doc comment to state plainly what `out`/`err` actually carry
  (prompt + Shell's own error messages only) and rewriting the tests to
  check real observable behavior (history, exit codes, error routing).
- `Shell(interactive=true)` originally called `enable_job_control()`
  directly from its constructor — which mutates real process-global
  signal dispositions and process-group/terminal ownership. Since all
  tests share one process, this would have leaked job-control signal
  handling into unrelated later tests. Fixed by decoupling prompt
  printing (constructor flag) from job control (now an explicit,
  separately-called, once-only `enable_job_control()`).

### Still not included

Qt6 GUI and the Groq/Gemini/Ollama Cloud provider implementations —
unchanged, see `docs/status.md`.

## Previous — M2 completion (job control) + M9/M10/M17-partial (local AI)

Second work session on this repo — see `docs/status.md` for the running
ledger across both sessions and the environment constraints (no
network, no Qt6/cmake, no display server) that shaped scope both times.

### Added

- **Real POSIX job control** in Meridian Shell: process groups
  (`setpgid`), terminal foreground handoff (`tcsetpgrp`), `fg`/`bg`
  builtins, `WUNTRACED`-based stop detection, correct signal
  disposition on both sides of `fork()` (shell ignores job-control
  signals once enabled; every child restores default before exec).
  Enabled automatically for interactive sessions only, matching real
  shells. Tested by writing a genuine Ctrl+Z byte to a real PTY hosting
  the real `meridian-shell` binary and confirming actual kernel
  `SIGTSTP` delivery — not simulated.
- **Local-only Meridian AI layer** (`src/ai/`): `SecretRedactor`
  (regex-based: KEY=VALUE env vars, Bearer tokens, AWS/GitHub/Slack/
  OpenAI-style key formats, PEM private key blocks), `RiskClassifier`
  (LOW/MEDIUM/HIGH/CRITICAL via ordered pattern matching), and
  `CommandAnalyzer` (real `$PATH` scan + Optimal String Alignment edit
  distance for command-name typo suggestions). All local, zero network
  calls.
- `Config` — minimal flat key=value config store (not full TOML, and
  documented as such) backing persistent AI state.
- `AIController` — ties the above into the on/off/detect/privacy state
  machine and `analyze`/`explain`/`redact`/`test` operations described
  in the spec, with `explain`/`test` reporting honestly that no
  provider is implemented rather than fabricating output.
- `meridian` CLI binary (`src/app/meridian_main.cpp`):
  `meridian ai on|off|status|detect on|off|privacy on|off|providers|
  use|test|analyze|explain|redact`.
- 26 new tests (job control + AI layer), bringing the suite to 86 tests
  / 192 assertions.

### Fixed (caught by this session's own testing — see `docs/status.md`)

- A test-harness bug (not a product bug): the first job-control test
  assumed `PtyManager::read()` returns quickly when no data is
  available, but the master fd is blocking — a bare `read()` call with
  nothing available blocks indefinitely. Fixed the test to use `poll()`
  before every read.
- `CommandAnalyzer`'s edit distance was plain Levenshtein, which
  charges 2 for an adjacent transposition ("gerp" -> "grep"), silently
  missing the single most common typo shape at the default suggestion
  threshold. Upgraded to Optimal String Alignment distance. Caught by
  testing against this machine's real 1,310-executable PATH, not just
  synthetic test data.
- `SecretRedactor`'s Bearer-token pattern used `\S+` for the token,
  which greedily consumed a trailing closing quote as part of the
  "secret" and silently dropped it from the output. Fixed by excluding
  quote characters from the token pattern.

### Still not included

Qt6 GUI and the Groq/Gemini/Ollama Cloud provider implementations — see
`docs/status.md`, `docs/gui.md`, `docs/ai.md`.

## Previous — M0-M5 core

Everything in this entry was written, compiled, and tested in the same
build environment — see `docs/status.md` for the environment's
constraints (no network, no Qt6/cmake, no display server) and how they
shaped scope.

### Added

- `meridian-core`: VT cell/attribute/color model, screen buffer
  (cursor, primary+alternate screen, scrollback, scroll regions), and a
  real ANSI/VT state-machine parser (SGR, cursor motion, erase,
  alt-screen, OSC title, UTF-8 decode).
- `meridian-core`: PTY manager (`openpty`/`fork`/`execve`), tested
  against real spawned processes.
- Meridian Shell: lexer, recursive-descent parser, executor (real
  `fork`/`execvp`/`pipe`/`dup2`/`waitpid`), builtins (`cd`, `pwd`,
  `echo`, `export`, `unset`, `env`, `history`, `jobs`, `type`/`which`,
  `help`, `clear`, `exit`). Pipelines, redirection (`>`, `>>`, `<`,
  `2>`, `2>>`, `&>`, `&>>`), `;`/`&&`/`||` sequencing, background jobs,
  recursive command substitution (`$(...)`, via the shell's own engine,
  never `/bin/sh -c`).
- Standalone `meridian-shell` binary and a headless PTY+ANSI
  integration demo (`meridian_demo`) spawning a real `bash` process.
- 60 tests (139 assertions) covering the above, including adversarial/
  malformed-input cases for the ANSI parser and real fork/exec
  integration tests for the PTY layer and shell executor.
- `AIProvider` interface (`src/ai/ai_provider.hpp`) — contract only, no
  implementation; see `docs/ai.md`.
- `CMakeLists.txt` (written for a machine with `cmake`; unverified in
  this build environment — see `docs/build.md`) and `Makefile`
  (the build path actually used and verified here).
- Documentation: `docs/architecture.md`, `docs/status.md`,
  `docs/build.md`, `docs/pty.md`, `docs/terminal-emulation.md`,
  `docs/shell.md`, `docs/ai.md`, `docs/providers.md`, `docs/gui.md`.

### Fixed (during this milestone's own testing — see `docs/status.md`)

- `_exit()` in forked shell children was skipping C++ iostream
  flushing, silently dropping output from in-process builtins (e.g.
  `echo hi > file`, `$(echo inner)`).
- ANSI parser's OSC-overflow recovery could leak the tail of a runaway
  OSC string onto the screen as literal text instead of discarding it.
- Shell lexer split `$(...)` on internal whitespace instead of tracking
  substitution nesting (`echo $(echo a b)` lexed as four words instead
  of two).
- `extern char** environ` declared inside a namespace instead of at
  file scope, causing a link error.

### Not included in this milestone

Qt6 GUI (window, GPU renderer, tabs, panes, AI popup) and the Groq/
Gemini/Ollama Cloud AI provider implementations — see `docs/status.md`,
`docs/gui.md`, and `docs/ai.md` for why and what's next.
