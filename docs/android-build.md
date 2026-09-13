# Building TRUX for Android

TRUX is built with Kotlin, Jetpack Compose, C++20, and Android NDK.

---

## 1. Prerequisites

| Tool | Recommended Version |
|---|---|
| JDK | 17 (e.g. `jdk-17.0.12+7`) |
| Android SDK Platform | 35 |
| Android Build Tools | 35.0.0 |
| Android NDK | 27.1.12297006 or higher |
| CMake | 3.22.1 |
| Gradle | 8.9 (via wrapper) |

---

## 2. Build Commands

Set `JAVA_HOME` if not already set:

```powershell
$env:JAVA_HOME = "C:\Users\Charan Balaji\.jdks\jdk-17.0.12+7"
```

### Unit Tests

```bash
cd android
./gradlew testDebugUnitTest
```

### Build Debug APK

```bash
cd android
./gradlew assembleDebug
```

Output APK:
```
android/app/build/outputs/apk/debug/app-debug.apk
```
Copied to workspace root as `TRUX-debug.apk`.

### Build Release APK

```bash
cd android
./gradlew assembleRelease
```

Output APK:
```
android/app/build/outputs/apk/release/app-release-unsigned.apk
```

---

## 3. Installation via ADB

```bash
# Install to connected device or emulator
adb install -r android/app/build/outputs/apk/debug/app-debug.apk

# Launch TRUX MainActivity
adb shell am start -n com.meridian.shell/.MainActivity

# Filter native runtime logs
adb logcat -s MeridianNative:V TRUX:V AndroidRuntime:E
```

---

## 4. Supported ABIs

By default, builds target:
- `arm64-v8a` (Modern physical Android phones & tablets)
- `x86_64` (Standard Android Studio Emulators)
- `armeabi-v7a` (Legacy 32-bit devices)
