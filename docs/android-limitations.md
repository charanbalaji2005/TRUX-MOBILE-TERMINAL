# TRUX Android Platform Constraints & Limitations

This document details the security constraints, target SDK considerations, and execution mechanics on modern Android versions.

---

## 1. Supported Android Versions & Target SDK

| Property | Value | Rationale |
|---|---|---|
| `minSdkVersion` | `26` (Android 8.0 Oreo) | Supports modern NDK C++20, JNI `forkpty`, and notification channels |
| `targetSdkVersion` | `28` (Android 9 Pie) | Required for direct execution of writable ELF binaries in app-private storage |
| `compileSdkVersion` | `35` (Android 15) | Access to modern Android 15 APIs, Material 3 Compose, and Jetpack components |

---

## 2. W^X (Write XOR Execute) Constraint

### The Android 10+ (API 29+) Restriction
Starting in Android 10 (API level 29), the Android platform blocks `execve()` on files residing inside writable application data directories (e.g. `/data/data/<package>/files/` or `/data/user/0/<package>/files/`).
Calling `execve()` on a writable file on API 29+ with `targetSdk >= 29` causes the kernel SELinux / Bionic loader to abort with:
`avc: denied { execute } for path="/data/data/.../bin/bash" dev="dm-X" ino=... scontext=... tcontext=... tclass=file permissive=0`

### TRUX Dual Packaging Strategy

To remain fully functional while targeting multiple distribution channels:

1. **Direct Distribution Strategy (GitHub / F-Droid / Sideload)**:
   - Sets `targetSdkVersion = 28`.
   - Android enforces W^X based on the application's `targetSdkVersion`. Because `targetSdk = 28`, the Linux kernel permits `execve()` on writable app-data binaries across Android 9, 10, 11, 12, 13, 14, and 15.
   - This allows `pkg install` to download, unpack, and execute arbitrary user binaries directly into `$PREFIX/bin`.
   - This is the exact strategy used by Termux on GitHub and F-Droid.

2. **Google Play Store Distribution Strategy**:
   - Google Play policy mandates `targetSdkVersion >= 34` for newly submitted apps.
   - On `targetSdk = 34`, apps cannot execute writable binaries in `filesDir`.
   - To deploy TRUX to the Play Store:
     - Core binaries (e.g. `bash`, `toybox`) are packaged inside the APK as `lib<name>.so` under `lib/<abi>/`.
     - The Android Package Manager installs them as read-only executable files in `applicationInfo.nativeLibraryDir`.
     - For dynamically installed packages (`pkg install`), TRUX can leverage `proot` (ptrace-based virtualization) to emulate execution without requiring root or writable `execve()`.

---

## 3. Storage Restrictions (Scoped Storage)

- Core TRUX data remains strictly within app-private internal storage (`/data/data/com.meridian.shell/files/`).
- External shared storage (`/storage/emulated/0`) requires the `MANAGE_EXTERNAL_STORAGE` permission ("All Files Access") on Android 11+ (API 30+).
- Running `setup-storage` prompts the user with the system settings intent to grant access, then creates symlinks under `~/storage/` to Downloads, DCIM, Documents, and shared phone memory.

---

## 4. Root (`su`) Limitations

- TRUX is designed to run seamlessly on non-rooted Android devices.
- Commands requiring Linux root (such as arbitrary raw socket bindings on port < 1024 or `chown` across system UIDs) are restricted to the app's UID sandbox.
- Standard user-space development (Python, Git, compilation, shell scripting, ADB wireless debugging) does not require root.
