---
layout: default
title: "Fedora RPM & Linux Packages"
category: "Packaging"
---

# Meridian Terminal — Packaging & Distribution Guide

This document is the complete guide for building, maintaining, and publishing Meridian Terminal packages across all major Linux distribution ecosystems (Fedora/RHEL/AetherOS, Ubuntu/Debian/Mint, Arch/Manjaro, Flatpak, Snap, and the Universal Installer).

---

## 1. Distribution Matrix

| Distribution Target | Package Type | Source Files | Build / Publish Command |
| :--- | :--- | :--- | :--- |
| **Fedora / RHEL / AetherOS / openSUSE** | `.rpm` | `packaging/rpm/meridian-terminal.spec` | `rpmbuild -ba` or Fedora COPR |
| **Ubuntu / Debian / Linux Mint / Pop!_OS** | `.deb` | `packaging/deb/debian/*` | `dpkg-buildpackage` or Launchpad PPA |
| **Arch Linux / Manjaro / EndeavourOS** | AUR | `packaging/aur/PKGBUILD`, `.SRCINFO` | `makepkg` or `aurpublish` |
| **Universal Linux Desktop** | Flatpak | `packaging/flatpak/*.yaml` | `flatpak-builder` / Flathub |
| **Universal Linux (Canonical / Snap)** | Snap | `packaging/snap/snapcraft.yaml` | `snapcraft` / Snap Store |
| **One-Line Universal Installer** | Shell Script | `install.sh` | `curl -fsSL https://.../install.sh \| bash` |

---

## 2. Universal One-Line Installer (`install.sh`)

The universal installer automatically inspects `/etc/os-release`, determines architecture and tool availability, builds or downloads precompiled releases, and sets up system/user paths with desktop menu entries.

### User Commands:
```bash
# Recommended default installation (installs to ~/.local for standard users, /usr/local with sudo)
curl -fsSL https://raw.githubusercontent.com/meridian-terminal/meridian-terminal/main/install.sh | bash

# Local invocation options:
./install.sh --user                  # Install into ~/.local without sudo
./install.sh --system                # Install into /usr/local
./install.sh --prefix /opt/meridian  # Custom target directory
./install.sh --dry-run               # Inspect environment without changes
./install.sh --uninstall             # Cleanly remove installed binaries and assets
```

---

## 3. Fedora, RHEL & AetherOS Packaging (RPM)

### Building Locally:
```bash
# Install toolchain
sudo dnf install -y gcc-c++ make rpm-build rpmdevtools

# Setup rpmbuild tree
rpmdev-setuptree

# Create source tarball
make tarball VERSION=0.1.0
cp build/meridian-terminal-0.1.0.tar.gz ~/rpmbuild/SOURCES/

# Copy spec file and build
cp packaging/rpm/meridian-terminal.spec ~/rpmbuild/SPECS/
rpmbuild -ba ~/rpmbuild/SPECS/meridian-terminal.spec

# Built packages will be located in:
# ~/rpmbuild/RPMS/x86_64/meridian-terminal-0.1.0-1.fc*.x86_64.rpm
```

### Publishing to Fedora COPR:
1. Create a project at [copr.fedorainfracloud.org](https://copr.fedorainfracloud.org/).
2. Add a new package pointing to your GitHub repository and the `packaging/rpm/meridian-terminal.spec` path.
3. Users can then install with:
   ```bash
   sudo dnf copr enable username/meridian-terminal
   sudo dnf install meridian-terminal
   ```

---

## 4. Ubuntu, Debian & Linux Mint Packaging (DEB)

### Building Locally:
```bash
# Install packaging tools
sudo apt update && sudo apt install -y build-essential debhelper devscripts dpkg-dev

# Prepare debian/ directory
cp -r packaging/deb/debian .

# Build the binary package
dpkg-buildpackage -us -uc -b

# Install the generated .deb:
sudo dpkg -i ../meridian-terminal_0.1.0-1_amd64.deb
```

### Publishing to Ubuntu Launchpad PPA:
1. Build source package: `debuild -S -sa`
2. Upload to Launchpad: `dput ppa:username/meridian-terminal ../meridian-terminal_0.1.0-1_source.changes`
3. Users can install with:
   ```bash
   sudo add-apt-repository ppa:username/meridian-terminal
   sudo apt update
   sudo apt install meridian-terminal
   ```

---

## 5. Arch Linux & AUR Packaging

### Testing Locally:
```bash
cd packaging/aur
makepkg -sric
```

### Publishing to the Arch User Repository:
1. Clone your AUR repository:
   ```bash
   git clone ssh://aur@aur.archlinux.org/meridian-terminal.git
   ```
2. Copy `PKGBUILD` and generate `.SRCINFO`:
   ```bash
   cp /path/to/packaging/aur/PKGBUILD meridian-terminal/
   cd meridian-terminal
   makepkg --printsrcinfo > .SRCINFO
   git add PKGBUILD .SRCINFO
   git commit -m "Release v0.1.0"
   git push origin master
   ```
3. Users can install via any AUR helper:
   ```bash
   yay -S meridian-terminal
   # or
   paru -S meridian-terminal
   ```

---

## 6. Flatpak (Flathub)

### Building Locally:
```bash
# Install flatpak and flatpak-builder
flatpak install flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
flatpak-builder --user --install --force-clean build-dir packaging/flatpak/org.meridian_terminal.MeridianTerminal.yaml

# Run Flatpak:
flatpak run org.meridian_terminal.MeridianTerminal
```

### Submitting to Flathub:
1. Fork [flathub/flathub](https://github.com/flathub/flathub).
2. Create a new branch with `org.meridian_terminal.MeridianTerminal.yaml` and desktop/metainfo files.
3. Open a Pull Request on Flathub.
4. Users install with:
   ```bash
   flatpak install flathub org.meridian_terminal.MeridianTerminal
   ```

---

## 7. Snap Store

### Building Locally:
```bash
# Install snapcraft
sudo snap install snapcraft --classic

# Build snap
snapcraft --directory=packaging/snap

# Install locally in dangerous mode for testing:
sudo snap install packaging/snap/meridian-terminal_0.1.0_amd64.snap --dangerous --classic
```

### Publishing:
```bash
snapcraft login
snapcraft register meridian-terminal
snapcraft upload packaging/snap/meridian-terminal_0.1.0_amd64.snap --release=stable
```
Users install with:
```bash
sudo snap install meridian-terminal --classic
```

---

## 8. GitHub Actions Automated Release Pipeline

The repository includes `.github/workflows/release.yml`. When you push a Git version tag (e.g. `git tag v0.1.0 && git push origin v0.1.0`):
1. **Debian runner** builds `meridian-terminal_0.1.0-1_amd64.deb`.
2. **Fedora container** builds `meridian-terminal-0.1.0-1.fc44.x86_64.rpm` and source RPM.
3. **Universal runner** creates `meridian-terminal-0.1.0-linux-x86_64.tar.gz`.
4. Calculates SHA256 checksums (`SHA256SUMS.txt`).
5. Publishes all assets to the official GitHub Releases page automatically.

