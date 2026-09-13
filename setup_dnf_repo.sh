#!/usr/bin/env bash
# Adds the Meridian DNF repository to Fedora / RHEL
set -e

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root or with sudo:"
    echo "  curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/setup_dnf_repo.sh | sudo bash"
    exit 1
fi

echo "-> Adding Meridian Terminal repository to /etc/yum.repos.d/meridian.repo..."
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/packaging/rpm/meridian.repo -o /etc/yum.repos.d/meridian.repo

echo "-> Refreshing DNF metadata..."
dnf check-update >/dev/null 2>&1 || true

echo ""
echo "=============================================="
echo " Meridian Repository successfully enabled!    "
echo "=============================================="
echo "You can now install Meridian Terminal using:"
echo "   sudo dnf install meridian-terminal"
echo "   sudo dnf install meridian-shell"
