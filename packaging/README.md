# Meridian Terminal Packaging

This directory contains the packaging specifications, manifests, desktop entry files, and installer scripts for distributing Meridian Terminal across all major Linux operating systems and package formats.

## Contents

- **`desktop/`**: Freedesktop XDG `.desktop` file and AppStream `metainfo.xml`
- **`rpm/`**: RPM spec file (`meridian-terminal.spec`) for Fedora, RHEL, openSUSE, and AetherOS
- **`deb/`**: Debian packaging directory (`debian/control`, `debian/rules`, `debian/changelog`, `debian/copyright`, `debian/meridian-terminal.install`) for Ubuntu, Debian, Linux Mint, and Pop!_OS
- **`aur/`**: Arch Linux package recipe (`PKGBUILD`, `.SRCINFO`) for the Arch User Repository (AUR)
- **`flatpak/`**: Flatpak application manifest (`org.meridian_terminal.MeridianTerminal.yaml`) for Flathub
- **`snap/`**: Snapcraft manifest (`snapcraft.yaml`) for the Canonical Snap Store
- **`install.sh`**: Universal auto-detecting one-line Linux installer script

See [`docs/packaging.md`](../docs/packaging.md) for detailed build and release instructions.
