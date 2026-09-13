#!/usr/bin/env bash
# ==============================================================================
# Meridian Terminal Universal Uninstaller
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/uninstall.sh | sudo bash
#   or: ./uninstall.sh [--user] [--purge]
# ==============================================================================
set -e

echo "=============================================="
echo "      Meridian Terminal 2.0 Uninstaller       "
echo "=============================================="

PURGE_CONFIG=false
for arg in "$@"; do
    if [ "$arg" = "--purge" ] || [ "$arg" = "-p" ]; then
        PURGE_CONFIG=true
    fi
done

# 1. Remove binary files
echo "-> Removing Meridian binaries..."
rm -f /usr/local/bin/meridian /usr/local/bin/meridian-shell 2>/dev/null || true
rm -f /usr/bin/meridian /usr/bin/meridian-shell 2>/dev/null || true
rm -f "${HOME}/.local/bin/meridian" "${HOME}/.local/bin/meridian-shell" 2>/dev/null || true

# 2. Remove desktop application launcher
echo "-> Removing desktop launchers..."
rm -f /usr/share/applications/meridian.desktop 2>/dev/null || true
rm -f /usr/local/share/applications/meridian.desktop 2>/dev/null || true
rm -f "${HOME}/.local/share/applications/meridian.desktop" 2>/dev/null || true

# 3. Remove shared gallery and data
echo "-> Removing shared application data..."
rm -rf /usr/local/share/meridian /usr/share/meridian 2>/dev/null || true

# 4. Remove from /etc/shells if writable
if [ -w /etc/shells ]; then
    echo "-> Cleaning /etc/shells entries..."
    sed -i '\#/usr/local/bin/meridian#d' /etc/shells 2>/dev/null || true
    sed -i '\#/usr/local/bin/meridian-shell#d' /etc/shells 2>/dev/null || true
    sed -i '\#/usr/bin/meridian#d' /etc/shells 2>/dev/null || true
    sed -i '\#/usr/bin/meridian-shell#d' /etc/shells 2>/dev/null || true
fi

# 5. Handle user configurations
if [ "$PURGE_CONFIG" = true ]; then
    echo "-> Purging configuration directory (~/.config/meridian)..."
    rm -rf "${HOME}/.config/meridian" 2>/dev/null || true
else
    echo "Notice: Configuration preserved at ~/.config/meridian (run with --purge to remove)."
fi

echo ""
echo "=============================================="
echo " Meridian Terminal successfully uninstalled!  "
echo "=============================================="
