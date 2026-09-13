---
layout: default
title: "Shell & AST Engine"
category: "Terminal"
---

# Meridian Shell

An independent POSIX-ish shell: `src/shell/{ast,lexer,parser,executor,
builtins}.{hpp,cpp}`, built into a standalone `meridian-shell` binary
by `src/app/shell_main.cpp`.

## Grammar

```
sequence  := seq_item ((';' | '&&' | '||' | '&') seq_item)*
seq_item  := pipeline
pipeline  := command ('|' command)*
command   := (WORD | redir WORD)+
redir     := '>' | '>>' | '<' | '2>' | '2>>' | '&>' | '&>>'
```

Words are lexed as segments (`WordPart{text, literal}`), not
pre-resolved strings — a single-quoted segment is marked `literal` and
never scanned for `$...`; everything else is. This matters for
correctness with `&&`/`||`: an unevaluated branch's command
substitutions must never actually run, and lazy per-command expansion
(at execution time, not lex time) is what guarantees that.

## Expansion

Supported inside unquoted or double-quoted text: `$VAR`, `${VAR}`,
`$(command)` (real recursive command substitution — see below), `$?`
(last exit status), `$$` (this process's pid). Single-quoted text is
never expanded. `$(...)` correctly spans internal whitespace and nested
parens even though it's otherwise unquoted (`echo $(echo a b)` lexes as
two words, not four — this needed a real fix; see `docs/status.md`).

## Command substitution doesn't shell out

`$(cmd)` is evaluated by forking, redirecting the child's stdout to a
pipe, and recursively running `cmd` through this *same* lexer/parser/
executor — never through `/bin/sh -c`. This keeps Meridian Shell
consistent with its own execution-safety rule (see below) even for
substitution content, and it's also just correct POSIX subshell
semantics for free: whatever the substituted command does to its
working directory or environment only affects that forked child, never
the parent shell.

## Execution: real fork/exec, not `system()`

Every command — pipeline stage or single command — runs via
`fork()` + `execvp()` (external programs) or, for a lone builtin with no
redirection and no backgrounding, directly in-process (so `cd` actually
changes the real shell's directory, `export` actually sets real
environment for future children, `exit` actually ends the REPL).

A builtin that's part of a multi-stage pipeline, or that has its own
redirection, still gets forked — this is deliberate and matches real
shell behavior: `cd /tmp | ls` should NOT change the parent shell's
directory (POSIX defines each pipeline stage as running in its own
subshell), and Meridian Shell gets that right by construction rather
than as a special case.

Multi-stage pipelines wire `N-1` real `pipe()`s between `N` forked
stages with `dup2`, closing every pipe fd in every child after wiring
its own stdin/stdout, then `waitpid` on all stages (the pipeline's exit
status is the last stage's, per shell convention).

Redirections (`>`, `>>`, `<`, `2>`, `2>>`, `&>`, `&>>`) are applied via
`open()`+`dup2()` in the forked child before exec/builtin execution, so
an explicit redirect on a pipeline stage correctly overrides the pipe
wiring for that fd.

**Never**: `system()`, `popen()`, or building a string and handing it
to `/bin/sh -c`. Every external program is `execvp`'d with an already-
tokenized argv array — there's no reinterpretation step where shell
metacharacters from user input could be re-parsed by another shell.

## Builtins

`cd`, `pwd`, `echo`, `export`, `unset`, `env`, `history`, `jobs`,
`type`/`which`, `help`, `clear`, `exit`. `alias` is accepted as a
no-op. See `docs/status.md` for what's explicitly NOT implemented yet
(`fg`/`bg`, bare `FOO=bar` assignment, tab completion, line editing).

## A subtlety that cost a real debugging session

Builtins that run inside a forked child write to `std::cout`, which is
buffered. `_exit()` — unlike returning normally from `main` — does
**not** flush C++ iostreams. A forked child that wrote to `std::cout`
and then called `_exit(status)` directly would silently lose that
output. Every `_exit()` call site in `executor.cpp` now goes through a
`flush_and_exit()` helper, and `std::cout`/`std::cerr` are flushed
before every `fork()` too (otherwise a parent's unflushed buffer
content gets duplicated into the child's copy on fork, which can print
things twice). Full writeup, plus two other real bugs this caught, is
in `docs/status.md`.

## Job control

Real POSIX job control (spec §31), enabled automatically only for
interactive sessions (`isatty(STDIN_FILENO)` — matches real shells,
which don't do job control in `-c` mode or when piped):

- Each pipeline gets its own process group (`setpgid`), set redundantly
  on both the parent and child side of every `fork()` — the standard
  race-safe pattern, since either side's call succeeding is sufficient
  and calling it twice is harmless.
- Foreground pipelines get the controlling terminal via `tcsetpgrp`
  before the shell waits on them, and the shell reclaims it via
  `tcsetpgrp` afterward — this is what makes Ctrl+C/Ctrl+Z reach the
  *job*, not the shell, while it's running.
- The shell ignores `SIGINT`/`SIGQUIT`/`SIGTSTP`/`SIGTTIN`/`SIGTTOU` for
  itself once job control is enabled; every forked child restores
  default disposition for those signals before exec, so real programs
  still respond to Ctrl+C/Ctrl+Z normally.
- `waitpid(..., WUNTRACED)` on the foreground job detects a stop
  (Ctrl+Z) without the shell needing a `SIGCHLD` handler; on a stop, the
  job is recorded (Running/Stopped) and control returns to the shell
  prompt.
- `fg [%N]` / `bg [%N]` resume a job (`SIGCONT` to `-pgid`); `fg`
  additionally reclaims the terminal and waits again, matching real
  shell semantics. With no `%N`, both act on the most recently created
  job.
- `jobs` reports each job's id, pgid, state (Running/Stopped), and a
  best-effort display of its command line.

**Tested how it actually matters**: `tests/test_job_control.cpp` spawns
the real `meridian-shell` *binary* (not just the `Executor` class) on a
real PTY and writes a genuine `0x1A` (Ctrl+Z) byte to the master fd. The
byte is never delivered to the shell as ordinary input — the kernel's
tty line discipline intercepts it and sends a real `SIGTSTP` to
whichever process group actually owns the terminal at that moment. If
process-group assignment or the `tcsetpgrp` handoff were wrong, this
test would either hang (the OS wouldn't route the signal where the test
expects) or the `jobs`/`bg` output text wouldn't match — it isn't
possible to fake this test passing with code that only looks plausible.

Known simplification: no `%N`-style job specs beyond a plain job
number, and a killed (not cleanly exited) background job is currently
reported as "Done" rather than distinguishing "killed by signal" — see
`docs/status.md`.



`tests/test_shell_lexer.cpp`, `test_shell_parser.cpp` — unit tests
against the AST directly. `tests/test_shell_executor.cpp` — real
integration tests: actual `fork`/`exec` against real coreutils
(`sort`, `printf`, `true`, `false`), verified by reading back files
those real child processes actually wrote, not mocked output.
