# The constraint that decides this project

Read this before writing the package manager. It determines what Meridian Shell
can be, and where you are allowed to ship it.

## W^X on Android 10+

Android 10 (API 29) added an SELinux rule: **an app whose `targetSdkVersion` is
29 or higher may not `execve()` a file inside its own writable data directory.**

A Termux-style terminal needs exactly that. `pkg install python` downloads a
binary, writes it to `$PREFIX/bin/python3`, and execs it. Under `targetSdk >= 29`
that exec is denied — the download succeeds and the program refuses to run.

This is not a bug you can code around. It is the reason Termux pins
`targetSdkVersion 28`, and the reason Termux is not on Google Play: Play has
required a recent target API for years. Termux ships through F-Droid and
GitHub instead.

## Your three options

### A. `targetSdk = 28` — full Termux behaviour, no Play Store

`$PREFIX/bin` is writable *and* executable. `pkg install` works. Arbitrary
binaries run. proot and a Debian rootfs work.

Cost: Google Play will reject the APK. You distribute via GitHub Releases,
F-Droid, or direct download. Users hit "install from unknown sources" once.

**This is what `android/app/build.gradle.kts` currently sets**, because it is
the only option that delivers the product you described.

### B. `targetSdk = 35` — Play-compatible, bundled binaries only

Executables must live in `nativeLibraryDir`, which the system extracts from the
APK and marks read-only + executable. To ship `bash`, you build it for each ABI
and package it as `lib/arm64-v8a/libbash.so`. At runtime it appears at
`/data/app/.../lib/arm64/libbash.so` and execs fine.

What you get: a fixed, curated toolset. `pkg` becomes an enable/disable
front-end over what you pre-bundled, not a real installer.

What you lose: user-installable packages, proot, Debian/Ubuntu userspaces.

Requires `android:extractNativeLibs="true"` and
`packaging { jniLibs { useLegacyPackaging = true } }` — both already set.

### C. Ship both

Same codebase, two product flavours. `Bootstrap.canExecFromDataDir` already
reports which mode is live at runtime, so the rest of the app does not branch.

```kotlin
if (!bootstrap.canExecFromDataDir) {
    // Play build: pkg can only enable bundled binaries
}
```

## Practical consequence for the roadmap

Do not build the package manager until you have picked. Under option A it is a
downloader plus an extractor. Under option B it is a registry over bundled
`.so` files. They share almost no code.

## What *always* works, regardless

- `forkpty()` and a real PTY
- `/system/bin/sh` — Android's own shell, always present and always executable
- Anything shipped inside the APK as `lib*.so`
- Reading and writing files in app-private storage

So a working Meridian terminal running a genuine shell is reachable on any
target API. Only *user-installed packages* depend on this choice.

## Bundling a shell as a native library

For option B (or as a fallback in option A), the pattern is:

```
app/src/main/jniLibs/
├── arm64-v8a/
│   ├── libbash.so          ← the bash executable, renamed
│   └── libncurses.so
└── armeabi-v7a/
    └── ...
```

Constraints the build system imposes:

- The filename **must** match `lib*.so` or the APK packager drops it.
- Binaries must be built against the same NDK API level as the app.
- `LD_LIBRARY_PATH` must include `nativeLibraryDir` — `Bootstrap` already does
  this.

`Bootstrap.resolveShell()` probes, in order: `$PREFIX/bin/bash`,
`$PREFIX/bin/sh`, `nativeLibraryDir/libbash.so`, then `/system/bin/sh`. The
last one always exists, so the app cannot fail to open a terminal.
