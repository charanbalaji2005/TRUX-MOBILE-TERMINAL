---
layout: default
title: "System Architecture"
category: "Developer"
---

# Architecture

Meridian is three independent systems, matching the spec: the terminal
core, Meridian Shell, and Meridian AI. "Independent" is enforced at the
build-graph level, not just conceptually — `meridian-core` has zero
dependency on `meridian-shell-lib` and neither depends on Qt6 or any AI
provider code. A GUI or AI outage can never take down the terminal
core or the shell.

```
                         MERIDIAN
                            |
             +--------------+--------------+
             |              |              |
             v              v              v
       Terminal Core    Meridian Shell   Meridian AI
      (src/core/*)       (src/shell/*)    (src/ai/*)
             |              |
             +------+-------+
                    |
                   PTY
                    |
             Linux Processes
```

## Terminal core (`src/core/`)

- `vt/types.hpp` — `Color`, `Attributes`, `Cell`, and a pragmatic
  East-Asian-width function. No dependencies beyond `<cstdint>`.
- `vt/screen_buffer.{hpp,cpp}` — owns the actual grid: cursor, primary +
  alternate screen, scrollback (`std::deque`), scroll regions. The ANSI
  parser drives this through a small deliberate API (`put_codepoint`,
  `newline`, `erase_in_line`, `enter_alt_screen`, ...) — it never
  reaches into the grid directly. This separation is what makes the
  screen buffer unit-testable without a parser or a PTY at all (see
  `tests/test_screen_buffer.cpp`).
- `vt/ansi_parser.{hpp,cpp}` — a real state machine (`Ground`,
  `Escape`, `CsiParam`, `OscString`, `OscDiscard`, `Utf8Continuation`),
  not a big regex or a giant if/else chain over the whole buffer. Feeds
  decoded codepoints + parsed SGR/cursor/erase operations into a
  `ScreenBuffer` reference. See `docs/terminal-emulation.md` for what
  it does and doesn't support.
- `pty/pty_manager.{hpp,cpp}` — `openpty()` + `fork()` + `execve()`
  directly. See `docs/pty.md`.

A GUI frontend (once built) would own a `PtyManager` + `ScreenBuffer` +
`AnsiParser` per terminal session/pane, read the master fd on its event
loop, feed bytes to the parser, and paint from the `ScreenBuffer`'s
cells. None of that requires changes to this layer.

## Meridian Shell (`src/shell/`)

`ast.hpp` -> `lexer.{hpp,cpp}` -> `parser.{hpp,cpp}` -> `executor.{hpp,cpp}`
+ `builtins.{hpp,cpp}`. See `docs/shell.md` for the grammar and
execution-safety details (fork/exec directly, never `system()` /
`/bin/sh -c` for user commands).

Meridian Shell is a normal program: `src/app/shell_main.cpp` builds it
into a standalone `meridian-shell` binary that reads stdin and writes
stdout like any other shell. `PtyManager` can spawn it exactly the way
it would spawn `/bin/bash` — nothing shell-specific leaks into the PTY
layer.

## Meridian AI (`src/ai/`)

Real local layer: `secret_redactor`, `risk_classifier`,
`command_analyzer`, `command_context`, `ai_controller` — see
`docs/ai.md`. `ai_provider.hpp` remains interface-only (no cloud
provider implemented) — see `docs/ai.md` and `docs/status.md`.

## File-naming mapping against the original spec

The spec's §5 file tree names files this repo deliberately doesn't
have under those names — either consolidated into fewer files, or not
yet extracted into their own module. Stated explicitly here so nothing
reads as silently missing:

| Spec names | This repo has | Why |
|---|---|---|
| `terminal/{Terminal,TerminalState,Screen,Cell,Cursor,Scrollback}.{cpp,hpp}` | `core/vt/types.hpp` + `core/vt/screen_buffer.{hpp,cpp}` | One cohesive grid+cursor+scrollback+alt-screen model was more natural to build and test as a unit than six separately-linked classes with a shared cursor/grid to keep in sync across file boundaries. `Cell`/`Attributes`/`Color` do exist, in `types.hpp`, just not as their own `.cpp` (they're plain structs with no logic to compile separately). |
| `parser/{AnsiParser,VTStateMachine}.{cpp,hpp}` | `core/vt/ansi_parser.{hpp,cpp}` | The state machine *is* `AnsiParser`'s private `enum class State` and its dispatch — splitting the state enum into a separate class from the code that transitions it would separate two halves of one thing. |
| `pty/{Pty,PtyProcess}.{cpp,hpp}` | `core/pty/pty_manager.{hpp,cpp}` | `PtyManager` already owns both the PTY fds and the child process lifecycle together (spawn/read/write/resize/wait_for_exit) — that pairing is the whole point of the class, per `docs/pty.md`. |
| `process/{Process,ProcessGroup}.{cpp,hpp}` | Inline in `shell/executor.{hpp,cpp}` | Process/process-group management here is specifically shell pipeline/job-control logic (tied to `Pipeline`/`Job` structs) rather than a general-purpose reusable process wrapper — extracting a generic version with no second caller yet felt like speculative abstraction rather than real reuse. |
| `shell/JobControl.{cpp,hpp}` | Inline in `shell/executor.{hpp,cpp}` (the `Job` struct, `jobs_`, `find_job`/`reap_job_status_changes`/`do_fg`/`do_bg`/`jobs_report`) | Job-list bookkeeping is reasonably separable from pipeline execution, but the fork/`setpgid`/`tcsetpgrp` sequence that *creates* a job is deeply interleaved with the pipe-wiring loop that spawns pipeline stages — see `run_pipeline` in `executor.cpp`. Splitting the bookkeeping half out into its own class while leaving the OS-level half in `Executor` was judged not worth the risk of touching already-tested, sanitizer-clean job-control code for a mostly-cosmetic reorganization. Documented here as a deliberate call, not an oversight. |
| `shell/Shell.{cpp,hpp}` | `shell/shell.{hpp,cpp}` | This one **is** extracted, as of this session — see `docs/shell.md`. |
| `shell/AST.cpp` | `shell/ast.hpp` only | `Word`/`Command`/`Pipeline`/`Sequence` are plain data structs with no member functions — there's no logic to put in a `.cpp`. |
| `ai/{AIResponse,CommandContext}.hpp` | `AIResponse`-shaped struct inline in `ai_provider.hpp`; `CommandContext` **is** its own file (`ai/command_context.{hpp,cpp}`, added this session) | `AIResponse` has no behavior of its own to warrant a separate file while its only consumer (`AIProvider`) doesn't exist yet; `CommandContext` gathers real local state (cwd/git/OS info) and earned its own file and tests. |

Nothing in `src/renderer/`, `src/ui/`, `src/tabs/`, `src/panes/`,
`src/clipboard/`, `src/input/`, `src/ai/AIPopup.*`, or
`src/ai/providers/*Provider.cpp` exists at all — those aren't
naming/consolidation differences, they're genuinely not built, blocked
by the lack of Qt6/network in this environment. See `docs/gui.md` and
`docs/ai.md`.

## Why this split

1. **Testability.** Every piece above the PTY boundary can be unit- or
   integration-tested without a display server, and everything up to
   and including the PTY layer can be tested without Qt6 or a network
   connection. That's the whole reason this repo's test suite exists
   and passes in a sandbox with neither.
2. **Spec compliance.** §3 and §80 both require the terminal to work
   fully without Meridian Shell or Meridian AI. That's not just a
   feature checkbox — it's the reason `meridian-core` doesn't `#include`
   anything from `src/shell/` or `src/ai/`.
3. **Incremental delivery.** The GUI and AI layers can be built later,
   by someone with Qt6 and network access, against a core that's
   already proven correct — rather than everything getting built and
   debugged together as one unverifiable mass.
