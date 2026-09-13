# Meridian Shell — Android migration audit

Audit of the Meridian Shell tree (178 C++ files, ~18 MB) against the Android
NDK target. Every claim below was checked against the source, not assumed.

## Headline finding

**Meridian has no GUI toolkit dependency.** There is no Qt, no GTK, no X11, no
Wayland and no OpenGL anywhere in the tree. `src/gui/` is a *TUI* — it emits
ANSI strings into a terminal, it does not open a window. `src/core/renderer/`
is a damage-tracking and glyph-atlas layer, not a GPU backend.

That removes the single biggest obstacle a desktop terminal normally faces when
moving to Android. The port is a **frontend replacement**, not a rewrite.

Second finding: `src/core/vt/` compiles with **zero** platform headers. Verified:

```
$ g++ -std=c++20 -fsyntax-only -Isrc src/core/vt/screen_buffer.cpp   # OK
$ g++ -std=c++20 -fsyntax-only -Isrc src/core/vt/ansi_parser.cpp     # OK
$ g++ -std=c++20 -fsyntax-only -Isrc src/core/vt/graphics.cpp        # OK
```

The terminal emulator — the hardest and most valuable part of Meridian — moves
to Android untouched.

## Component table

| Component | Existing implementation | Portable? | Android changes | Reuse strategy |
|---|---|---|---|---|
| `core/vt/screen_buffer.cpp` | Pure C++20 grid, scrollback, alt screen, scroll regions | **Yes, fully** | None | Compiled verbatim by the Android CMake |
| `core/vt/ansi_parser.cpp` | UTF-8 + CSI/OSC/DCS/APC state machine, SGR, DECSET | **Yes, fully** | None | Compiled verbatim |
| `core/vt/types.hpp` | `Cell`/`Attributes`/`Color`, east-asian width | **Yes** | None | Compiled verbatim |
| `core/vt/graphics.cpp` | Sixel/Kitty image protocol | Yes | None | Compiled verbatim; rendering deferred |
| `platform/LinuxPTY.cpp` | `forkpty` + `poll` + `TIOCSWINSZ` | **Nearly** | See "PTY" below | Ported to `AndroidPTY.cpp` |
| `platform/PlatformPTY.hpp` | Clean virtual interface | Yes | None | Contract mirrored by `AndroidPTY` |
| `core/pty/pty_session.cpp` | `run_interactive()` puts **the host's own stdin** in raw mode | **No** | N/A | Not used on Android — no controlling TTY exists |
| `core/pty/pty_manager.cpp` | Desktop multi-PTY manager | Partly | N/A | Superseded by Kotlin `SessionManager` |
| `core/renderer/*` | Damage tracker, glyph atlas, pipeline | Yes (pure C++) | None | Not wired yet; candidate optimisation once Canvas profiling demands it |
| `shell/*` (lexer, parser, executor, builtins) | Real `fork`/`execvp`/`waitpid`/`pipe` shell, job control | Yes, with caveats | Needs a `$PATH` that exists | Buildable as an Android executable — ship as the default shell |
| `shell/line_editor.cpp` | Raw-mode `termios` on **the process's own tty** | No | N/A | Only valid when meridian-shell runs *inside* the PTY child, which is exactly how it will run |
| `gui/*`, `app/meridian_gui.cpp` | ANSI TUI: tabs, clock, system info | Yes but redundant | N/A | Excluded from the Android build; Compose replaces it |
| `ai/*` | Local-only analysis, secret redaction | Yes (`dirent`, `stat`, `utsname` all exist in Bionic) | None | Deferred to phase 2 |
| `dev/*` | Git intel, file explorer, system monitor | Mostly | `/proc` access is restricted on modern Android | Deferred; expect partial functionality |
| `workspace/*` | Pane tree, workspace persistence | Yes | Path handling | Superseded by Kotlin `SessionManager` + Room |
| `security/credential_store.cpp` | File-backed credential store | Yes | Should move to Android Keystore | Deferred |
| `CMakeLists.txt` | Desktop-only, unconditional `target_link_libraries(... util)` | **No** | Must be conditional | Untouched; Android uses a separate CMakeLists |

## The three real incompatibilities

### 1. `-lutil` does not exist on Android

`CMakeLists.txt` links `util` into `meridian-core`, `meridian-shell-lib` and
every executable. On glibc, `forkpty()` lives in `libutil`. In Bionic it is part
of `libc` itself and **there is no `libutil.so`** — the link fails outright.

Fix, when you fold Android into the root CMakeLists:

```cmake
if(NOT ANDROID)
    target_link_libraries(meridian-core PUBLIC util)
endif()
```

The Android build under `android/app/src/main/cpp/CMakeLists.txt` already
avoids it.

### 2. `PtySession::run_interactive()` has no meaning on Android

`src/core/pty/pty_session.cpp` puts *the calling process's* stdin into raw mode
and proxies bytes between it and the PTY master. That assumes the process was
launched from a terminal. An Android app has no controlling TTY at all — there
is nothing to put in raw mode.

This is why `AndroidPTY` + `TerminalCore` exist. Same `forkpty` call, but the
output is fed into `AnsiParser`/`ScreenBuffer` and drawn on a Canvas instead of
being echoed to an inherited stdout.

### 3. The zygote environment is hostile

A process forked from an Android app inherits `LD_PRELOAD`, `BOOTCLASSPATH`,
`ANDROID_*` and an empty-ish `PATH`. `AndroidPTY` wipes the environment in the
child and installs only the Meridian userspace. Skipping this produces shells
that start but behave erratically.

## What is safe on Bionic

Every POSIX header the tree uses exists in Bionic: `unistd.h`, `termios.h`,
`poll.h`, `dirent.h`, `sys/stat.h`, `sys/wait.h`, `sys/ioctl.h`,
`sys/utsname.h`, `sys/sysinfo.h`, `sys/statvfs.h`, `pty.h` (API 23+).

Two behavioural caveats:

- `pwd.h` works but Android has no `/etc/passwd`. `getpwuid()` returns a
  synthetic entry with a useless home directory. Anywhere the code derives
  `$HOME` from `getpwuid`, prefer `getenv("HOME")` — which `AndroidPTY` always
  sets.
- `/proc` is heavily restricted since Android 10. `dev/system_monitor.cpp` will
  return partial data rather than failing loudly.

## Recommended migration architecture

```
Compose UI  (TerminalScreen, GestureSessionDrawer, ExtraKeysRow)
     │
TerminalView            ← custom Canvas View, one per terminal
     │
SessionManager          ← process-scoped, owns every session, no cap
     │
TerminalSession         ← opaque Long handle, no native types
     │
PtyBridge               ← the ONLY file that touches JNI
     ├──────────────────────────────────────────────
     ▼  JNI boundary
TerminalCore (new)      ← reader thread + snapshot packing
     ├── AndroidPTY     (new)     forkpty, env bootstrap, signals
     └── AnsiParser ──► ScreenBuffer      ← REUSED VERBATIM from src/core/vt
```

Roughly 85% of the terminal-emulation logic is reused unchanged. The new code is
one PTY backend, one glue class, one JNI file and the Kotlin frontend.

## Deliberately deferred

Not built yet, and honestly labelled as such:

- Text selection, copy and paste (`TerminalView` handles touch-to-focus only)
- Scrollback viewport scrolling (the buffer holds it; the view draws only the
  visible grid)
- Room persistence of session metadata
- Settings and storage screens
- Package manager and `meridian distro` userspaces
- Wiring `core/renderer/` in place of naive Canvas drawing
