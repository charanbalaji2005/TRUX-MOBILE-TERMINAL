# TRUX-MOBILE-TERMINAL

# 🚀 TRUX — Next-Generation Android Linux Terminal & Userspace

[![Release](https://img.shields.io/badge/Release-v2.0.1-brightgreen.svg)](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases)
[![Download APK](https://img.shields.io/badge/Download-Trux--v2.0.1.apk-blue.svg?logo=android)](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases/download/v2.0.1/Trux-v2.0.1.apk)
[![Platform](https://img.shields.io/badge/Platform-Android%208.0%2B-blue.svg)](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL)
[![Architecture](https://img.shields.io/badge/Arch-arm64--v8a%20%7C%20armeabi--v7a-orange.svg)](#)
[![License](https://img.shields.io/badge/License-GPL--3.0-lightgrey.svg)](LICENSE)

**TRUX** is a native, high-performance Android Linux terminal emulator, POSIX userspace, and developer environment engineered in Kotlin, Jetpack Compose, C++20, and Android NDK with real pseudoterminal (PTY) capabilities.

---

## 📥 Direct APK Download Option

You can download and install the official APK directly onto your Android device:

| File | Architecture | Size | Link |
| :--- | :--- | :--- | :--- |
| **Trux-v2.0.1.apk** | rm64-v8a, rmeabi-v7a | ~24.3 MB | [⬇️ **Download Trux-v2.0.1.apk**](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases/download/v2.0.1/Trux-v2.0.1.apk) |

👉 **Alternative Mirrors / All Releases**:
- [GitHub Releases Page](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases)
- [Release Tag v2.0.1](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases/tag/v2.0.1)

### 📲 How to Install:
1. Tap the download link: **[Trux-v2.0.1.apk](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases/download/v2.0.1/Trux-v2.0.1.apk)**.
2. If prompted by your browser, tap **Download anyway**.
3. Open the downloaded file from notifications or your Downloads manager, then tap **Install**.
4. *(Optional)* Install via ADB from your PC:
   `ash
   adb install -r Trux-v2.0.1.apk
   `

---

## 🌟 Key Features

### 1. 🖥️ Professional ANSI Startup Dashboard
- Real terminal ANSI dashboard with sleek monochrome white & silver typography on pure black background.
- Responsive layout with Unicode box-drawing cards (Linux Environment, Package Ecosystem, AI Assistant, App Lock, Universal Viewer, High Performance).
- Command hints:
  ```text
  Type 'help'       → Show available commands
  Type 'trux'       → Explore TRUX tools
  Type 'pkg'        → Manage packages
  Type 'trux ai'    → Use AI assistant
  ```
- Real PTY shell prompt:
  ```text
  trux@android:~$ 
  ```
- CLI controls: `trux welcome`, `trux welcome preview`, `trux welcome enable`, `trux welcome disable`.
- Toggleable in TRUX Settings: *"Show welcome screen on new session"*.

### 2. 📄 Session PDF & Text Exporter
- Export terminal session commands, history, telemetry, and scrollback directly into formatted PDF or TXT documents.
- Saves to device storage (~/storage/downloads/ and Android Downloads).
- Accessible via terminal command (	rux export pdf or 	rux export txt) or from the session actions menu in the UI.
- Instantly viewable and shareable in TRUX's Universal File Viewer.

### 3. 💾 On-Device Session & Chat Persistence
- Terminal session outputs and scrollbacks are continuously logged to on-device storage (ilesDir/sessions/<id>.log).
- Full shell command history ($HOME/.history, $HOME/.bash_history) persists across sessions and app restarts.
- Built-in AI Assistant history stored on device ($HOME/.trux_ai_chat.log).
- CLI management: 	rux session list, 	rux session show <id>, 	rux ai history, 	rux ai clear-history, 	rux ai export-chat.

### 4. 👁️ Universal File Previewer
Open supported files directly from the command line into native overlays:
`ash
open report.pdf
open photo.jpg
open video.mp4
open script.py
open archive.zip
`
- **Supported Formats**: PDF (with page navigation and pinch-to-zoom), Images (JPEG, PNG, WEBP, GIF, SVG), Audio & Video (ExoPlayer), Text & Source Code (with syntax highlighting and line numbers), Markdown, JSON, CSV tables, and ZIP/TAR archives.

### 5. 🔒 Hardware-Backed App Lock
- Biometric authentication (Fingerprint, Face, Device PIN/Pattern/Password).
- FLAG_SECURE prevents recents thumbnail snooping and unauthorized screen capture.
- Terminal PTY sessions continue running smoothly in the background while locked.

### 6. 🤖 Terminal AI Assistant (	rux ai)
- Integrated multi-provider LLM assistant (OpenAI, Anthropic, Local LLMs).
- Generate commands (	rux ai --command <task>), explain errors (	rux ai explain <error>), or chat interactively (	rux ai).
- Persistent on-device chat history (	rux ai history).

### 7. 📦 Full Linux Package Ecosystem
- Termux-compatible package manager (pkg update, pkg install <pkg>).
- Direct APT, APT-GET, and DPKG support.
- Pre-configured shims for git, python, curl, adb, and system monitoring tools.

---

## 🛠️ Building From Source

### Requirements
- Android Studio Ladybug / Jellyfish or newer
- JDK 17
- Android SDK Platform 35
- Android NDK 26.1.10909125+
- CMake 3.22.1+

### Build Steps
`ash
git clone https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL.git
cd TRUX-MOBILE-TERMINAL/android
./gradlew assembleRelease
`
The resulting APK will be placed in ndroid/app/build/outputs/apk/release/.

---

## 👤 Author
Developed with ❤️ by **Charan Balaji**  
GitHub: [@charanbalaji2005](https://github.com/charanbalaji2005)

## 📄 License
TRUX is released under the [GNU General Public License v3.0](LICENSE).
