---
layout: default
title: "Installation & Multi-OS Guide"
category: "GETTING STARTED"
status: "production"
---

# Installation & Multi-OS Setup Guide

Meridian Terminal 2.5 provides native prebuilt packages across **Linux**, **macOS**, **WSL**, and **Windows**, along with a 1-line universal installer and 1-click GitHub connection.

---

## ⚡ Universal 1-Line Installer (Recommended)

The universal installer automatically identifies your Linux distribution, macOS version, or WSL environment, selects the optimal prebuilt package, verifies SHA256 checksums, and configures desktop integration and VS Code terminal profiles:

```bash
# Recommended for standard user (installs to ~/.local/bin):
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash

# Or system-wide with sudo (installs to /usr/local/bin):
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash
```

---

## 📦 OS-Specific Installation Commands

### 1. 🐧 Ubuntu / Debian / Pop!_OS / Linux Mint
```bash
# Direct prebuilt DEB package
curl -fsSLO https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/meridian-terminal_2.5.0_amd64.deb
sudo apt install ./meridian-terminal_2.5.0_amd64.deb
```

### 2. 🎩 Fedora / RHEL / CentOS / Rocky Linux / AlmaLinux
```bash
# Direct prebuilt RPM package via DNF
sudo dnf install https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/meridian-terminal-2.5.0-1.fc44.x86_64.rpm
```

### 3. 🏹 Arch Linux / Manjaro
```bash
# Via AUR (yay / paru)
yay -S meridian-terminal
```

### 4. 🦎 openSUSE Tumbleweed & Leap
```bash
sudo zypper install https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/meridian-terminal-2.5.0-1.fc44.x86_64.rpm
```

### 5. 🏔️ Alpine Linux
```bash
apk add libstdc++ libgcc bash
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
```

### 6. 🪟 Windows Subsystem for Linux (WSL & WSL2)
Inside your Ubuntu, Debian, or Fedora WSL terminal:
```bash
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
```
> **WSL Display Modes**:
> - `meridian-shell` runs in any Windows Terminal or VS Code console.
> - `meridian` automatically launches its hardware-accelerated GUI on Windows 11 with WSLg.

### 7. 🍏 macOS (Apple Silicon M1-M5 & Intel x86_64)
```bash
# Via Homebrew Cask
brew tap charanbalaji2005/meridian https://github.com/charanbalaji2005/Meridian-Shell
brew install --cask meridian-terminal
```
Or download direct disk images:
- [Apple Silicon DMG (`arm64`)](https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/Meridian-Terminal-2.5.0-macOS-arm64.dmg)
- [Intel DMG (`x86_64`)](https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/Meridian-Terminal-2.5.0-macOS-x86_64.dmg)

### 8. 🪟 Windows Native (PowerShell)
```powershell
irm https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/dist/install.ps1 | iex
```

---

## 🔑 How to Connect to GitHub (`meridian gh`)

Meridian includes a **1-Click GitHub Integration** designed for developer ease without exposing account passwords or overwriting existing keys:

```text
┌─── GitHub Integration Status ────────────────────────────────────────────────┐
│ Account : @charanbalaji2005
│ SSH     : ✓ Connected (Port 443)
│ GitHub  : ✓ Authenticated
│ Key     : ~/.ssh/id_ed25519
│ Remote  : git@github.com:charanbalaji2005/Meridian-Shell.git
└──────────────────────────────────────────────────────────────────────────────┘
```

### Step 1: Connect Account & Verify SSH
```bash
meridian gh connect
```
- Generates a dedicated ED25519 key if none exists.
- Configures `~/.ssh/config` to connect over **Port 443** (avoiding ISP/network port 22 blocks).
- Tests the connection and verifies write authorization.

### Step 2: Check Connection Status
```bash
meridian gh status
```

### Step 3: View Public Key
```bash
meridian gh key
```

### Step 4: Link a New Repository
```bash
meridian gh repo create my-awesome-app
```

### Step 5: Smart, Non-Blocking Git Hints
When you run `git init` in a directory without an upstream remote, Meridian prints helpful next steps. To customize hints:
```bash
meridian gh hints on     # Turn hints on
meridian gh hints off    # Turn hints off
```

---

## 💻 VS Code Auto-Detection & Profiles

```bash
meridian vscode status   # Check current profile configuration
meridian vscode enable   # Set Meridian as VS Code's default terminal
meridian vscode disable  # Reset back to standard bash/zsh
```

---

## 🔄 Updating Meridian

```bash
meridian update --check  # Check if a new release is available
meridian update          # Download, verify SHA256 checksum, and apply update
```

---

## 📊 Usage Statistics & Telemetry

```bash
meridian stats               # Display global installation overview
meridian stats --year 2026   # Display metrics for 2026
meridian stats --growth      # Display yearly adoption growth rates

meridian telemetry status    # View privacy policy & telemetry state
meridian telemetry enable    # Opt-in to anonymous usage counts
meridian telemetry disable   # Opt-out completely (Zero network metrics)
```