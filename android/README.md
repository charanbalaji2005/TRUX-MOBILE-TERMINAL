# Meridian Shell for Android

A real Android terminal built on Meridian's own VT engine. Not a simulator: a
`forkpty()` child, a genuine shell, real ANSI parsing.

```
android/app/src/main/
├── cpp/
│   ├── CMakeLists.txt          compiles src/core/vt/ in place — no duplication
│   ├── pty/AndroidPTY.*        forkpty + environment bootstrap  (new)
│   ├── bridge/TerminalCore.*   reader thread + snapshot packing (new)
│   └── jni/meridian_jni.cpp    opaque-handle JNI surface        (new)
└── java/com/meridian/shell/
    ├── PtyBridge.kt            the only file that touches JNI
    ├── TerminalSession.kt      one live terminal
    ├── SessionManager.kt       process-scoped, no session cap
    ├── Bootstrap.kt            $HOME/$PREFIX/$PATH, exec-mode detection
    ├── TerminalView.kt         Canvas renderer + IME integration
    ├── KeyMapper.kt            KeyEvent -> VT bytes
    └── ui/                     Compose: gesture drawer, extra keys
```

Start here: [`docs/android-execution-model.md`](../docs/android-execution-model.md)
— it determines what the package manager can be and where you can ship.

Then: [`docs/android-migration-audit.md`](../docs/android-migration-audit.md)
and [`docs/android-build.md`](../docs/android-build.md).
