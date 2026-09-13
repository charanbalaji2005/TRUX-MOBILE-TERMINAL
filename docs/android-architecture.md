# TRUX Android Architecture

TRUX is an Android-native, Termux-style Linux terminal environment built for high-performance mobile command-line workflows.

---

## 1. System Architecture Overview

The system is partitioned into clear, decoupled layers:

```
┌─────────────────────────────────────────────────────────────┐
│                    Jetpack Compose UI                       │
│  - TerminalView (Canvas-rendered monospace grid)            │
│  - GestureSessionDrawer (Edge swipe opened, ~80% width)     │
│  - CommandCenterTopBar (Clean session pill, no hamburger)   │
│  - ExtraKeysRow (ESC, TAB, CTRL, ALT, arrows, etc.)         │
│  - NewSessionSheet & SessionActionsSheet                    │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                    Kotlin Session Layer                     │
│  - SessionManager (multi-session concurrency, lifecycle)    │
│  - TerminalSession (I/O streaming, resize, state tracking)  │
│  - EnvironmentManager (dynamic path & rootfs resolution)    │
│  - PackageInstaller & PackageRepository (APT/DPKG logic)    │
│  - StorageManager (scoped & shared storage integration)     │
│  - TruxDatabase / MeridianDatabase (Room persistence)       │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                    JNI Bridge (meridian_jni)                 │
│  - Native lifecycle marshaling, buffer copy prevention      │
│  - Handle validation, thread-safe PTY write/read            │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                    C++20 Terminal Core                      │
│  - TerminalCore (state machine, scrollback buffer)          │
│  - AndroidPTY (forkpty(), master/slave pseudoterminal)      │
│  - VT / ANSI Parser (escape sequences, SGR color, cursor)   │
│  - Screen Buffer (cells, colors, styles, generation counter)│
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                    Linux Userspace ($PREFIX)                │
│  - Shell: bash / sh / dash                                  │
│  - Utilities: toybox, coreutils, sed, awk, grep, tar, etc.  │
│  - Tools: curl, wget, git, python3, pip, nano, adb          │
│  - Package manager: pkg, apt, apt-get, dpkg                 │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Real PTY Execution Pipeline

TRUX executes genuine native processes inside Linux PTYs. Output is never simulated or faked.

1. **PTY Creation**:
   `AndroidPTY` calls `openpty()` / `forkpty()` in C++, setting up master and slave file descriptors, establishing process group leader (`setsid`), and configuring termios (`xterm-256color`).
2. **Shell Launch**:
   The child process configures environment variables (`PREFIX`, `HOME`, `PATH`, `TERM`, `SHELL`, `TRUX=1`, `TRUX_VERSION=2.0.1`), executes `chdir()` to working directory, and invokes `execve()` for `bash` or `sh`.
3. **Reader Thread**:
   A dedicated POSIX reader thread waits on `poll()` on the master PTY descriptor. Reads raw bytes without busy-looping and dispatches to the VT/ANSI state machine.
4. **ANSI/VT State Processing**:
   Escape sequences are parsed, cursor positions updated, and text formatted into the screen buffer cells with attributes (256-color foreground/background, bold, underline, inverse).
5. **Canvas Rendering**:
   `TerminalView` renders the grid via optimized hardware-accelerated Canvas operations, using pre-measured character glyphs and batch line drawing.

---

## 3. Gesture-First Navigation

- **No Visible Hamburger Button**: In accordance with the minimal, screen-maximizing terminal philosophy, no hamburger menu button clutters the UI.
- **Left-Edge Gesture**: Sliding in from within `28dp` of the left bezel smoothly slides in the `ModalNavigationDrawer`.
- **Drawer Contents**:
  - TRUX metallic branding & version
  - "New Session" quick action
  - Active sessions list with running/stopped status indicators
  - Long-press to rename, restart, or terminate sessions
  - Swiping left or tapping scrim immediately dismisses the drawer
- **Android Back Handling**:
  - If drawer is open -> closes drawer.
  - If action sheet is open -> closes sheet.
  - While terminal is focused -> does not destroy active session.

---

## 4. Session Persistence Architecture

- **Persistent vs Live State Separation**:
  - Live OS processes and PTY file descriptors cannot survive an Android process kill.
  - Metadata (Session ID, Name, Shell, Environment, Working Directory, Dimensions, Created At, Last Active) is persisted to SQLite via Room in `trux.db`.
- **Restoration Flow**:
  - When the app returns from process death, `SessionManager.restoreFromDatabase()` restores session records and creates fresh live PTY processes in the user's last working directory.
  - Exited sessions are cleanly marked and purged so dead processes never linger.

---

## 5. Security & Isolation

- All default terminal data is stored strictly in app-private storage: `/data/data/com.meridian.shell/files/`.
- No cloud dependencies or telemetry: zero Firebase, Supabase, or remote database tracking.
- Passwords, keys, and clipboard contents are never logged.
- Shared storage access (`/storage/emulated/0`) requires explicit user permission via `setup-storage`.
