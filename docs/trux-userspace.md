# TRUX Userspace & Filesystem Architecture

TRUX sets up a self-contained, POSIX-compliant Linux userspace completely inside Android app-private storage.

---

## 1. Filesystem Layout

All runtime files reside under the app's internal private directory (`context.filesDir`):

```
/data/data/com.meridian.shell/files/
├── home/                         <-- $HOME (User home directory)
│   ├── .bashrc                   <-- Interactive shell config, prompt, aliases
│   ├── .profile                  <-- Login script, sets PATH & env
│   ├── .trux_clipboard           <-- Local clipboard bridge file
│   └── storage/                  <-- Symlinks created by setup-storage
│       ├── shared                -> /storage/emulated/0
│       ├── downloads             -> /storage/emulated/0/Download
│       ├── dcim                  -> /storage/emulated/0/DCIM
│       ├── pictures              -> /storage/emulated/0/Pictures
│       └── documents             -> /storage/emulated/0/Documents
│
├── usr/                          <-- $PREFIX (Linux root prefix)
│   ├── bin/                      <-- Command binaries, scripts, shims
│   │   ├── bash, sh              <-- Shells
│   │   ├── toybox                <-- Multi-call core utilities
│   │   ├── pkg, apt, apt-get     <-- Package management
│   │   ├── dpkg                  <-- Package installer
│   │   ├── curl, wget            <-- HTTP network transfer
│   │   ├── git, python3, pip     <-- Dev toolchains
│   │   ├── nano, edit            <-- Text editors
│   │   ├── adb                   <-- Android Debug Bridge
│   │   └── termux-* / trux-*     <-- Android OS integration bridges
│   │
│   ├── etc/                      <-- Configuration files
│   │   ├── motd                  <-- TRUX startup banner
│   │   └── apt/                  <-- APT sources and lists
│   │
│   ├── lib/                      <-- Shared libraries (.so)
│   ├── libexec/                  <-- Internal helpers
│   ├── share/                    <-- Architecture-independent assets
│   │
│   └── var/                      <-- Variable runtime data
│       ├── cache/apt/archives/   <-- Local package download cache
│       ├── lib/dpkg/status       <-- Installed package database
│       ├── lib/dpkg/info/        <-- Installed package file manifests
│       └── log/                  <-- Local service logs
│
├── tmp/                          <-- $TMPDIR, $TMP, $TEMP (Temporary scratchpad)
├── opt/                          <-- Optional third-party packages
└── environments/                 <-- Root filesystems for distros (Debian, Ubuntu)
```

---

## 2. Dynamic Environment Variables

TRUX resolves paths dynamically using `Bootstrap.kt` and `EnvironmentManager.kt`. No file paths are hardcoded to break across package name changes.

| Variable | Default Value | Description |
|---|---|---|
| `HOME` | `/data/data/<package>/files/home` | User home directory |
| `PREFIX` | `/data/data/<package>/files/usr` | Root prefix for binaries and libraries |
| `TRUX_PREFIX`| `/data/data/<package>/files/usr` | TRUX specific prefix alias |
| `PATH` | `$PREFIX/bin:$nativeLibDir:/system/bin:/system/xbin` | Binary search path priority |
| `TMPDIR` | `/data/data/<package>/files/tmp` | Standard temporary directory |
| `TMP` / `TEMP` | `/data/data/<package>/files/tmp` | POSIX temporary directory fallbacks |
| `TERM` | `xterm-256color` | Terminal capability identifier |
| `COLORTERM` | `truecolor` | 24-bit direct color support |
| `LANG` / `LC_ALL` | `en_US.UTF-8` | UTF-8 encoding configuration |
| `SHELL` | `$PREFIX/bin/bash` or `/system/bin/sh` | Primary shell binary |
| `TRUX` | `1` | TRUX environment presence flag |
| `TRUX_VERSION` | `2.0.1` | Current TRUX release version |

---

## 3. Shell Startup Flow

When a new session opens:
1. `resolveShell()` looks in order for:
   - `$PREFIX/bin/bash`
   - `$PREFIX/bin/sh`
   - `$nativeLibDir/libbash.so`
   - `/system/bin/sh` (safe final fallback)
2. Interactive shells source `~/.profile` which executes:
   - Sets `$PATH` and terminal variables.
   - Sources `~/.bashrc`.
3. `~/.bashrc` sets:
   - Prompt: `trux:~$ ` with green working directory and white prompt.
   - Aliases: `ll='ls -la'`, `la='ls -A'`, `l='ls -CF'`, `adb='$PREFIX/bin/adb'`, `trux='$PREFIX/bin/trux'`.
   - Displays `/data/data/<package>/files/usr/etc/motd` banner.

---

## 4. Android W^X Execution Architecture

- **targetSdk = 28 mode**:
  In accordance with Termux's battle-tested distribution model, targeting Android 9 (API 28) enables `execve()` execution of writable binaries residing directly in the app's `files/usr/bin/` folder.
- **Native Library Execution**:
  Any binaries packaged into the APK's `jniLibs/<abi>/` as `lib<name>.so` are unpacked by Android into `context.applicationInfo.nativeLibraryDir` and are marked executable by the OS. TRUX adds `nativeLibDir` directly to `$PATH` and `LD_LIBRARY_PATH`.
