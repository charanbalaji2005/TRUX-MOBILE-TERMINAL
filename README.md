# TRUX-MOBILE-TERMINAL

# 🚀 TRUX — Next-Generation Android Linux Terminal & Userspace

[![Release](https://img.shields.io/badge/Release-v2.0.1-brightgreen.svg)](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases)
[![Platform](https://img.shields.io/badge/Platform-Android%208.0%2B-blue.svg)](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL)
[![Architecture](https://img.shields.io/badge/Arch-arm64--v8a%20%7C%20armeabi--v7a-orange.svg)](#)
[![License](https://img.shields.io/badge/License-GPL--3.0-lightgrey.svg)](LICENSE)

**TRUX** is a native, high-performance Android Linux terminal emulator, POSIX userspace, and developer environment engineered in Kotlin, Jetpack Compose, C++20, and Android NDK with real pseudoterminal (PTY) capabilities.

---

## 🌟 Key Features

### 1. 🖥️ Professional ANSI Startup Dashboard
- Renders **inside the real PTY terminal** upon login (/usr/bin/trux-welcome).
- Pure black background with sleek silver/white metallic typography.
- Fully responsive layout that automatically adjusts to phone portrait, tablet, and landscape screen widths without wrapping.
- Quick command guides, capabilities matrix, and dynamic user/path shell prompt:
  `	ext
  trux@android:~$ 
  trux@android:~/storage$ 
  root@android:~# 
  `

### 2. 📄 Session PDF & Text Exporter
- Export terminal session commands, history, telemetry, and scrollback directly into formatted PDF or TXT documents.
- Saves to device storage (~/storage/downloads/ and Android Downloads).
- Accessible via terminal command (	rux export pdf or 	rux export txt) or from the session actions menu in the UI.
- Instantly viewable and shareable in TRUX\'s Universal File Viewer.

### 3. 💾 On-Device Session & Chat Persistence
- Terminal session outputs and scrollbacks are continuously logged to on-device storage (ilesDir/sessions/<id>.log).
- Full shell command history ($HOME/.history, $HOME/.bash_history) persists across sessions and app restarts.
- Built-in AI Assistant history stored on device ($HOME/.trux_ai_chat.log).
- CLI management: 	rux session list, 	rux session show <id>, 	rux ai history, 	rux ai clear-history.

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

## 📥 Download & Installation

Download the official release APK directly:
- **[Latest Release APK (v2.0.1)](https://github.com/charanbalaji2005/TRUX-MOBILE-TERMINAL/releases/download/v2.0.1/Trux-v2.0.1.apk)**

Or install via ADB:
`ash
adb install -r Trux-v2.0.1.apk
`

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
