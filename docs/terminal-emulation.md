---
layout: default
title: "Terminal Emulation"
category: "Terminal"
---

# Terminal emulation (VT/ANSI engine)

## What's implemented

**Cursor motion:** CUU/CUD/CUF/CUB (`A B C D`), CUP/HVP (`H f`), CHA
(`G`), VPA (`d`), save/restore cursor both as CSI (`s u`) and as the
classic ESC forms (`ESC 7` / `ESC 8`).

**Erasing:** ED (`J`, modes 0/1/2), EL (`K`, modes 0/1/2).

**Line/character editing:** IL/DL (`L`/`M`), DCH/ICH (`P`/`@`).

**Scrolling:** SU/SD (`S`/`T`), DECSTBM scroll regions (`r`), automatic
scroll-on-newline at the bottom of the (scroll-region-aware) screen,
with scrollback capture only for a full-screen scroll on the primary
buffer (not inside a narrower scroll region, and not on the alternate
screen — matching how full-screen apps like `vim`/`htop` behave).

**SGR (colors and attributes):** bold, dim, italic, underline, blink,
reverse, hidden, strikethrough, and their resets; 16-color (30-37,
90-97 fg / 40-47, 100-107 bg), 256-color (`38;5;N` / `48;5;N`), and
24-bit RGB (`38;2;R;G;B` / `48;2;R;G;B`).

**Alternate screen:** `?1049h/l` (with cursor save/restore), `?47h/l`,
`?1047h/l`.

**Other private modes** (cursor visibility `?25`, mouse reporting
`?1000`/`?1002`/`?1003`/`?1006`, bracketed paste `?2004`): parsed and
accepted so they never desync the parser, but not yet wired to any
behavior — there's no renderer yet to hide a cursor in, and no input
layer yet to report mouse events through.

**OSC:** window title (`OSC 0;` / `OSC 2;`, both BEL- and
ESC-\\-terminated) is captured. Other OSC codes are consumed and
discarded without affecting the screen.

**UTF-8:** decoded byte-by-byte with state that persists across
separate `feed()` calls, so a multi-byte sequence split across two PTY
reads decodes correctly (tested — see
`utf8_split_across_two_feed_calls`).

**Width handling:** a pragmatic East-Asian-width table (CJK, Hangul,
most emoji -> width 2; combining marks/variation selectors/ZWJ ->
width 0) rather than the complete Unicode UAX #11 tables.

## What's deliberately not there yet

- Combining marks are recognized (zero-width) but not actually merged
  onto the base glyph.
- SGR colon-subparameter form (`38:2::R:G:B`) isn't parsed, only the
  semicolon form.
- No true terminal-mode handling beyond the alt-screen/mouse/paste bits
  above (e.g. insert mode, `?1h` application cursor keys) — nothing
  here needed them yet since there's no input layer to differentiate
  key-sending behavior.

## Malformed-input safety

This was tested deliberately, not just hoped for — see the "Malformed /
adversarial input" section of `tests/test_ansi_parser.cpp`:

- A CSI sequence that never reaches a valid final byte is capped at 128
  bytes and then abandoned cleanly.
- A fresh ESC arriving in the middle of an unfinished CSI aborts the
  old sequence and starts fresh, rather than getting stuck.
- Stray UTF-8 continuation bytes with no lead byte, and invalid lead
  bytes, are dropped rather than corrupting decode state.
- An OSC string that blows past its length cap (8192 bytes) doesn't
  grow unboundedly, and — importantly — doesn't leak its tail onto the
  screen as literal text either; it's discarded until a real
  terminator (BEL or ST) arrives (see `docs/status.md`, bug #2, for why
  this needed a fix).
- A 5000-byte pseudo-random byte stream mixing control bytes, high
  bytes, and CSI fragments is fed through and simply must not crash —
  `random_byte_soup_never_crashes`.

The parser was also driven with 300,000 lines of real output from a
forked `seq` process (2.3 MB through the PTY) with no crash and correct
end-state cursor/scroll position — see `docs/status.md`.
