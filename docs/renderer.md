---
layout: default
title: "Renderer & Direct Images"
category: "Graphics"
---

# Renderer

**Status: not implemented.** No source exists under `src/renderer/`.
This document is scoped narrowly to spec §16 (GPU Renderer) and §17
(Font System) specifically; for the broader window/tabs/panes/AI-popup
picture, see `docs/gui.md` — this file exists as its own document only
because the spec names `docs/renderer.md` explicitly, and duplicating
`gui.md`'s content here instead of cross-referencing it would just give
two documents to keep in sync instead of one.

## Why nothing's built here

Same root cause as the rest of the GUI layer: Qt6 isn't installed in
this build environment and there's no network access to install it, so
no GPU/glyph-rendering code has been compiled or run. See
`docs/status.md`.

## What already exists for a renderer to consume

The renderer's actual job — reading cell state and painting it — has a
fully real, tested data source waiting for it:

- `meridian::vt::ScreenBuffer` (`src/core/vt/screen_buffer.hpp`) — the
  cell grid, cursor position, and scrollback a renderer would read
  every frame. `Cell::codepoint`/`width`/`attrs` already carry
  everything §9-13 ask a renderer to draw (SGR colors including 24-bit
  RGB, bold/italic/underline/etc, wide-character width for correct
  column advancement).
- Throughput is already characterized independent of any renderer: the
  PTY+parser pipeline was stress-tested processing 300,000 lines / 2.3
  MB of real output with no crash and correct end-state (see
  `docs/status.md`) — so a future renderer's job is specifically to
  keep up with an ingestion path that's already fast, not to also prove
  the parser itself is fast.

## What spec §16/§17 ask for, not yet started

- Glyph cache, dirty-cell tracking, batched rendering, high-DPI support
- Font family/size/weight/line-height/letter-spacing configuration with
  fallback fonts — `TerminalConfig` (`docs/configuration.md`) already
  persists `font_family`/`font_size`, waiting for something to read them
- Frame-rate/input-latency benchmarking against real GPU rendering

## Concrete next step

Per `docs/gui.md`: start smaller than the full spec. A plain Qt6 window
rendering `ScreenBuffer`'s cells as plain text (no glyph cache, no GPU
path) is enough to prove the integration boundary between this
already-tested core and a real renderer works, before any of the
GPU-specific work above begins.
