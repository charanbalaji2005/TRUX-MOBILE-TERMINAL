---
layout: default
title: "GUI & Canvas Architecture"
category: "Interface"
---

# GUI (Qt6 window, renderer, tabs/panes, AI popup)

**Status: not implemented.** No source files exist under `src/gui/` yet
— `CMakeLists.txt` has a commented-out `find_package(Qt6 ...)` block
rather than a real one, specifically so the build doesn't fail to
configure by referencing a directory that isn't there. See
`docs/status.md` for the full reasoning; short version: Qt6 isn't
installed in the environment this repo was built in, and there's no
network access to install it, so nothing here has ever been compiled,
let alone run against a real display.

## What the design intends (from the original spec, §12-21, §38-46)

- **Window Manager -> Tab Manager -> Pane Manager -> Terminal Session
  Manager**, each session owning one `PtyManager` + `ScreenBuffer` +
  `AnsiParser` from `meridian-core` (see `docs/architecture.md`). The
  GUI layer's job is to read the PTY master fd on Qt's event loop, feed
  bytes to the parser, and paint from the `ScreenBuffer`'s cells — none
  of the actual terminal logic belongs in this layer.
- **GPU-accelerated rendering** with a glyph cache and dirty-cell
  tracking, so heavy output (`yes`, `seq 1000000`) doesn't redraw the
  entire grid every frame. `meridian-core`'s throughput has already
  been stress-tested independent of any renderer (see `docs/status.md`)
  — the renderer's job is to keep up with it, not to also prove the
  parser is fast.
- **Split panes** as a pane tree (vertical/horizontal split, resize,
  focus, close), **tabs** (new/close/next/prev/rename), each holding
  one or more terminal sessions.
- **AI popup** (`Ctrl+Space`): provider/model selector, current-command
  display, Fix/Explain/Generate/Diagnose/Ask actions — all deferred
  until `src/ai/` has at least one real provider implementation to call
  (see `docs/ai.md`). Building the popup UI against an interface with
  no working implementation behind it would just move the "untested"
  problem from the network layer to the UI layer.
- **Mouse reporting** distinguishing terminal selection from
  application mouse reporting (needed by `vim`/`tmux`), **clipboard**
  (copy/paste/primary selection), **scrollback search**
  (Ctrl+Shift+F).
- **Configuration** under `~/.config/meridian/` (`terminal.toml`,
  `keybindings.toml`, `ai.toml`, `themes/`).

## Concrete next step

Start smaller than the full spec: a plain Qt6 window that owns one
`PtyManager`/`ScreenBuffer`/`AnsiParser` triple and renders the grid as
plain text (even a `QPlainTextEdit`-level render, no glyph cache, no
GPU path yet) to prove the Qt6 event-loop integration works end-to-end
against the already-tested core. Only once that's compiling, running,
and actually showing a real `bash` session on screen should GPU
rendering, tabs, panes, and the AI popup layer on top — each verified
before the next is added, same discipline as the rest of this repo.
