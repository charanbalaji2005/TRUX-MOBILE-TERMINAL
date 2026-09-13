---
layout: default
title: "PTY Multiplexer"
category: "Terminal"
---

# PTY layer

`src/core/pty/pty_manager.{hpp,cpp}` wraps a real Linux pseudo-terminal
and the process attached to it.

## What it does

`PtyManager::spawn(SpawnOptions)`:

1. `openpty()` — allocates the master/slave PTY pair with the requested
   initial `winsize` already set.
2. `fork()`.
3. In the child: `close(master)`, `setsid()` (new session, detach from
   any controlling terminal), `ioctl(slave, TIOCSCTTY, 0)` to make the
   PTY slave the controlling terminal, `dup2` the slave onto
   stdin/stdout/stderr, `chdir()` if a working directory was requested,
   then `execve()`/`execv()` the target program directly.
4. In the parent: close the slave fd, keep the master fd for
   `read()`/`write()`, remember the child's pid.

`resize(rows, cols)` calls `ioctl(master_fd, TIOCSWINSZ, ...)` — this is
what makes `SIGWINCH` fire correctly in the child, so `vim`/`htop`/etc.
redraw at the new size.

`wait_for_exit()` blocks on `waitpid()` and normalizes the result to
either an exit code or a "killed by signal N" indication.

## Execution safety

The child execs the target program directly — never `system()`, never
`popen()`, never `/bin/sh -c <user string>`. This matters for the same
reason it matters in Meridian Shell (`docs/shell.md`): a PTY manager
that shells out to interpret what it's spawning is a PTY manager with
an injection surface. `execve`/`execv` take an already-split argv, so
there's no string-reinterpretation step for anything to inject into.

## What's tested

`tests/test_pty_manager.cpp` — all against real spawned processes, not
mocks:

- Spawning `/bin/echo` and reading its actual output back through the
  master fd.
- A `bash -c "exit 3"` child and confirming `wait_for_exit()` reports
  exit code 3.
- `resize()` actually changing the kernel-reported window size
  (verified via a direct `TIOCGWINSZ` ioctl on the master fd, not just
  "the call didn't crash").
- Writing to the master fd and confirming the child's `read` builtin
  actually receives it (real bidirectional I/O through the PTY, not
  just output capture).

`src/app/demo_main.cpp` goes further: it spawns real `/bin/bash`,
has it `printf` real ANSI escape sequences, and feeds the raw bytes
into the ANSI parser + screen buffer, then prints both the raw
byte stream and the decoded result side by side. Run `make demo`.

## What's not here

Nothing GUI-specific — no signal-driven read loop wired to a Qt event
loop, no `SIGWINCH` handler on the terminal's own side (that's the
GUI's job: catch its own window resize, call `PtyManager::resize()`).
This class's job stops at "own the PTY and the child process," which
is exactly the boundary the architecture doc describes.
