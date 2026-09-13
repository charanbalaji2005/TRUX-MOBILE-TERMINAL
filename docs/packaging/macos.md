---
layout: default
title: "macOS Packaging & Distribution"
category: "PACKAGING"
status: "production"
---

# macOS Native Packaging & Distribution

Meridian Terminal provides a **native macOS `.app` application bundle** and disk image (`.dmg`) tailored for **Apple Silicon (M1/M2/M3/M4/M5)** and **Intel x86_64** architectures.

```text
┌─────────────────────────────────────────────────────────────┐
│                    Meridian.app (macOS)                    │
│                                                             │
│  Contents/                                                  │
│  ├── Info.plist                                             │
│  ├── MacOS/                                                 │
│  │   ├── meridian         (GPU Terminal GUI & Platform)     │
│  │   └── meridian-shell   (Standalone C++20 POSIX Shell)    │
│  └── Resources/                                             │
│      ├── meridian-terminal.icns                             │
│      └── gallery/ (Embedded Anime Artwork)                  │
└─────────────────────────────────────────────────────────────┘
```

---

## 1. Quick Installation via Homebrew Cask

The easiest way to install and keep Meridian updated on macOS is via Homebrew:

```bash
brew tap charanbalaji2005/meridian https://github.com/charanbalaji2005/Meridian-Shell
brew install --cask meridian-terminal
```

This installs `Meridian.app` into `/Applications` and creates command-line symlinks for `meridian` and `meridian-shell` in your `$PATH`.

---

## 2. Direct `.dmg` Disk Image Download

You can download prebuilt disk images directly from [GitHub Releases](https://github.com/charanbalaji2005/Meridian-Shell/releases):

| Architecture | Package | Description |
| :--- | :--- | :--- |
| **Apple Silicon** | `Meridian-Terminal-2.5.0-macOS-arm64.dmg` | Native build for M1, M2, M3, M4, M5 Macs |
| **Intel x86_64** | `Meridian-Terminal-2.5.0-macOS-x86_64.dmg` | Native 64-bit Intel Mac build |

### Installation Steps:
1. Double-click the downloaded `.dmg` file to mount it.
2. Drag **Meridian.app** into your **Applications** folder.
3. Launch Meridian from Spotlight, Launchpad, or the Applications folder.

---

## 3. Native In-Terminal Updates on macOS

Meridian includes an integrated cross-platform update engine. On macOS, running:

```bash
meridian update
```

automatically performs:
1. Architecture check (`arm64` vs `x86_64`).
2. Queries the latest GitHub release.
3. Downloads and verifies the disk image / application bundle.
4. Seamlessly updates `/Applications/Meridian.app`.
5. Prompts you to restart the terminal to enjoy the latest features.

```text
$ meridian update

Meridian Update Engine
─────────────────────────────────────────────
Current version : 2.5.0
Latest release  : 2.5.0
Platform        : macOS arm64

✔ You are already running the latest version of Meridian Terminal.
```

---

## 4. Building `.app` and `.dmg` from Source

To build a native `.app` and `.dmg` on macOS locally:

```bash
# Clone the repository
git clone https://github.com/charanbalaji2005/Meridian-Shell.git
cd Meridian-Shell

# Build binaries and package DMG
./packaging/macos/build_dmg.sh
```

The compiled `Meridian.app` and `Meridian-Terminal-2.5.0-macOS-<arch>.dmg` will be placed in the `dist/` directory.

---

## 5. Apple Code Signing & Notarization Workflow

For production distribution outside the Mac App Store:

```bash
# 1. Code sign the application bundle with Developer ID Application certificate
codesign --deep --force --verify --verbose \
         --sign "Developer ID Application: YOUR_NAME (TEAM_ID)" \
         --options runtime \
         dist/Meridian.app

# 2. Package into signed DMG
./packaging/macos/build_dmg.sh
codesign --sign "Developer ID Application: YOUR_NAME (TEAM_ID)" dist/*.dmg

# 3. Notarize with Apple Notary Service
xcrun notarytool submit dist/*.dmg \
                 --keychain-profile "AC_PASSWORD" \
                 --wait

# 4. Staple notarization ticket
xcrun stapler staple dist/*.dmg
```