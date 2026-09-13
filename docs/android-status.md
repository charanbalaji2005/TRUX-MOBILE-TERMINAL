# Meridian Android — honest status

Measured against the 83-section master prompt. Nothing here is marked done
unless the code exists in the tree.

Legend: **[x]** implemented · **[~]** partial · **[ ]** not started

## Verification status

The APK has **not** been built. There is no Android SDK or NDK in the
environment this was written in. What *was* verified:

- `src/core/vt/{screen_buffer,ansi_parser,graphics}.cpp` type-check clean
- `android/.../AndroidPTY.cpp` and `TerminalCore.cpp` type-check clean against
  the real engine, `-Wall -Wextra`, no warnings

Everything Kotlin, every Compose layout, every gesture and the whole JNI
boundary is **unverified**. Assume first-build errors.

## Foundation

- [x] §2 repository audit → `docs/android-migration-audit.md`
- [x] §3 Kotlin / Compose / NDK / C++20 / CMake / JNI, no WebView, no RN
- [x] §4 `com.meridian.shell`, arm64-v8a + armeabi-v7a
- [x] §51 project structure
- [x] §52 CMake reuses `src/core/vt` in place, no duplication
- [~] §53 Gradle — files written, **wrapper not committed**, never run
- [ ] §54 APK built and installed

## Terminal core

- [x] §16 real PTY — `forkpty`, non-blocking master, `TIOCSWINSZ`, signals
- [x] §26 terminal emulator — your parser and buffer, reused unmodified
- [x] §40 JNI with opaque `Long` handles, no pointers in Kotlin
- [x] §41 threading — native reader thread, `poll`, UI thread never blocks
- [x] §42 concurrency — per-session mutex, safe teardown off the map lock
- [~] §17 shell — resolves a shell and execs it, but **no bash is bundled**, so
      in practice every session lands on `/system/bin/sh`
- [~] §27 rendering — Canvas, reused arrays, zero steady-state allocation.
      No selection, no scrollback viewport, `core/renderer/` not wired in
- [~] §73 crash safety — exit is detected and shown in the drawer; no
      `[Process exited]` banner in the terminal, no restart affordance there

## UI

- [x] §5 black, minimal, full-screen, no hamburger, no toolbar
- [x] §6 gesture drawer — edge-swipe open, swipe/scrim close, 80% width
- [x] §29/§30 extra-keys row, Ctrl/Alt latching, VT escape mapping
- [x] §64 active session indicator
- [x] §62 rename, §63 close with confirm-if-alive, restart
- [x] §65 black theme, §66 monospace — **system monospace, none bundled**
- [~] §61 new session — creates immediately; no bottom sheet with shell and
      environment pickers
- [~] §68 back button — closes the action sheet; **does not close the drawer**
- [ ] §31 copy / paste / select all — nothing
- [ ] §32 selection, long-press on terminal text — nothing
- [ ] §33 settings screen — nothing
- [ ] §35 storage screen — nothing
- [ ] §71 accessibility

## Sessions

- [x] §7 no session cap; resource exhaustion surfaces as a real error
- [x] §14 switching never destroys a PTY
- [x] §15 create / destroy / switch / rename / restart / close / getAll / getActive
- [x] §37 process-scoped manager, survives rotation
- [~] §8 data model — in memory only
- [ ] §11/§39 Room + SQLite, `meridian.db`
- [ ] §12 per-session directories
- [ ] §13 persist and restore session metadata
- [ ] §15 `persistSession()` / `restoreSessions()`
- [ ] §28 configurable scrollback (hardcoded 10,000)

## Linux userspace — the largest gap

- [~] §18 `$HOME`, `$PREFIX`, `$PATH`, `$TMPDIR`, `$TERM` set; directories
      created. **No binaries populate them.**
- [ ] §19 Debian / Ubuntu via proot
- [ ] §20 `pkg` package manager
- [ ] §21 local package cache
- [ ] §22 shared-storage integration
- [ ] §24 SSH, §25 Git — both depend on shipping binaries
- [ ] §47 clear data / clear cache
- [ ] §36 export / import

See `docs/android-execution-model.md`. The package manager's shape depends
entirely on the `targetSdk` decision, so it was left unwritten on purpose.

## Security and privacy

- [x] §44 no root, no privilege escalation
- [x] §45 no telemetry, no analytics, no network calls in the app itself
- [x] §75 `INTERNET` only
- [x] §76 works fully offline
- [x] §34 local-only by construction; §57 no cloud database anywhere
- [x] §72 native logging carries no terminal content
- [ ] §44 credential store → Android Keystore

## Testing

- [~] §49 six JVM unit tests for `KeyMapper`. That is all.
- [ ] native PTY tests
- [ ] instrumentation tests
- [ ] §48 stress tests (`yes | head -100000`, ANSI floods, large scrollback)
- [ ] §50 multi-session stress test

## Documentation

- [x] `android-migration-audit.md`, `android-build.md`,
      `android-execution-model.md`, this file
- [ ] `android-architecture.md`, `android-pty.md`, `android-storage.md`,
      `android-linux-userspace.md`, `android-security.md`, `android-testing.md`

## Suggested order from here

1. Build it. Fix the first-build errors. Nothing else is meaningful until the
   APK installs and shows a prompt on a real phone.
2. Decide `targetSdk` (see `android-execution-model.md`).
3. Bundle a shell. Until then every session is `/system/bin/sh` and the
   terminal is a demo.
4. Copy / paste and text selection — the first thing a user will reach for.
5. Room persistence and the settings screen.
6. Package manager, then proot distributions.
