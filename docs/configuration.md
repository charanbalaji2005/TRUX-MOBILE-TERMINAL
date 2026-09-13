---
layout: default
title: "Configuration"
category: "Getting Started"
---

# Configuration

Spec §75/§76 describe `~/.config/meridian/{terminal.toml,keybindings.toml,ai.toml}`.
All three exist and are real, backed by the same underlying store
(`src/core/config.{hpp,cpp}` — extracted from what was originally
AI-only config once it became clear terminal settings and keybindings
needed the identical flat `key = "value"` mechanism, rather than
duplicating it three times).

**Not real TOML.** `Config` is a deliberately minimal flat key=value
parser/writer — no nested tables, no arrays, no typed values beyond
what the calling code converts itself (`std::atoi`, `"true"/"1"/"on"`
for booleans). The generated files use TOML-*looking* `key = "value"`
syntax so they're readable and hand-editable, but a real TOML parser
would accept a broader grammar than this one does. Implementing actual
TOML is real, separate scope — tracked as future work, not silently
assumed.

## `terminal.toml` (`src/config/terminal_config.hpp`)

Split honestly by whether a setting means anything yet:

**Real right now, independent of any GUI:**
- `scrollback_lines` — feeds directly into `ScreenBuffer`'s constructor
- `default_shell` — what `PtyManager` spawns when the user hasn't specified one
- `startup_directory`

**Recorded now; inert until a renderer exists to read them:**
- `font_family`, `font_size`, `theme_name`, `cursor_style`,
  `cursor_blink`, `tab_bar_visible`, `padding`

Storing the second group isn't premature — a future GUI reads the same
persisted settings a user configured before that GUI existed, rather
than starting from scratch. It's flagged here so nobody mistakes "the
config value round-trips correctly" (true, tested) for "changing it
does something visible" (not true yet, no renderer to look at it).

## `keybindings.toml` (`src/config/keybindings.hpp`)

An action-name -> key-combo map with defaults taken directly from the
specific shortcuts named elsewhere in the spec (`ai_popup` ->
`Ctrl+Space` per §42, `search` -> `Ctrl+Shift+F` per §23, plus tab/pane/
clipboard defaults). Real and testable without any input layer to
actually intercept these key combos yet — `Keybindings::get(action)`
always returns *something* sensible (the built-in default, or a saved
override), so a future input layer has real data to read from day one
rather than the config plumbing and the input layer needing to be built
simultaneously.

## `ai.toml` (`src/ai/ai_controller.hpp`)

Covered in `docs/ai.md`: `ai.enabled`, `ai.detect`, `ai.privacy`,
`ai.provider`, `ai.model`.

## Path resolution

All three default to `~/.config/meridian/<name>.toml`. Every config-
backed class takes its path as a constructor argument rather than
hardcoding it, specifically so tests can point at a throwaway file
instead of a real user's `~/.config` — `MERIDIAN_CONFIG_HOME` is the
environment-variable override for the `meridian` CLI specifically (see
`docs/ai.md`); `TerminalConfig`/`Keybindings` take an explicit path with
no CLI wiring yet, since there's no terminal-core binary that would
consume them (that's the GUI, which doesn't exist — `docs/gui.md`).

## Testing

`tests/test_config.cpp` covers both `TerminalConfig` (defaults when no
file exists, round-tripping custom values including the boolean
`cursor_blink` field) and `Keybindings` (spec-matching defaults,
override persistence, listing every default action via `all()`).
`tests/test_ai.cpp` covers the AI-specific config path through
`AIController`.
