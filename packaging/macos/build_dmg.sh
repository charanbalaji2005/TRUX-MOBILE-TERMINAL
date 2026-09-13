#!/usr/bin/env bash
# ==============================================================================
# Meridian Terminal 2.5 — macOS DMG Packaging Script
# Generates native .app bundle and distributable .dmg disk image
# ==============================================================================
set -euo pipefail

APP_VERSION="2.5.0"
ARCH="$(uname -m)"
OUTPUT_DIR="dist"
APP_NAME="Meridian.app"
DMG_NAME="Meridian-Terminal-${APP_VERSION}-macOS-${ARCH}.dmg"

echo "==> Building macOS Application Bundle: ${APP_NAME} (${ARCH})..."

# 1. Compile binaries if not present
if [ ! -f "build/meridian" ] || [ ! -f "build/meridian-shell" ]; then
    make all -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
fi

# 2. Assemble .app bundle structure
BUNDLE_DIR="${OUTPUT_DIR}/${APP_NAME}"
CONTENTS_DIR="${BUNDLE_DIR}/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"

rm -rf "${BUNDLE_DIR}"
mkdir -p "${MACOS_DIR}" "${RESOURCES_DIR}"

cp -f build/meridian "${MACOS_DIR}/meridian"
cp -f build/meridian-shell "${MACOS_DIR}/meridian-shell"
chmod +x "${MACOS_DIR}/meridian"*

cp -f packaging/macos/Info.plist "${CONTENTS_DIR}/Info.plist"

if [ -d "resources/images/gallery" ]; then
    mkdir -p "${RESOURCES_DIR}/gallery"
    cp -rf resources/images/gallery/* "${RESOURCES_DIR}/gallery/" 2>/dev/null || true
fi

if [ -f "resources/icons/meridian-terminal.icns" ]; then
    cp -f resources/icons/meridian-terminal.icns "${RESOURCES_DIR}/meridian-terminal.icns"
elif [ -f "resources/icons/meridian-terminal.svg" ]; then
    cp -f resources/icons/meridian-terminal.svg "${RESOURCES_DIR}/meridian-terminal.svg"
fi

echo "==> Application bundle successfully created at ${BUNDLE_DIR}"

# 3. Build DMG Disk Image (if hdiutil is available on macOS)
if command -v hdiutil >/dev/null 2>&1; then
    echo "==> Creating Apple Disk Image (${DMG_NAME})..."
    DMG_TEMP_DIR="$(mktemp -d /tmp/meridian-dmg-XXXXXX)"
    cp -R "${BUNDLE_DIR}" "${DMG_TEMP_DIR}/"
    ln -s /Applications "${DMG_TEMP_DIR}/Applications"

    rm -f "${OUTPUT_DIR}/${DMG_NAME}"
    hdiutil create -volname "Meridian Terminal ${APP_VERSION}" \
                   -srcfolder "${DMG_TEMP_DIR}" \
                   -ov -format UDZO \
                   "${OUTPUT_DIR}/${DMG_NAME}"

    rm -rf "${DMG_TEMP_DIR}"
    echo "==> Successfully generated: ${OUTPUT_DIR}/${DMG_NAME}"
else
    echo "==> Creating macOS ZIP archive fallback (${OUTPUT_DIR}/Meridian-Terminal-${APP_VERSION}-macOS-${ARCH}.zip)..."
    cd "${OUTPUT_DIR}"
    zip -r -q "Meridian-Terminal-${APP_VERSION}-macOS-${ARCH}.zip" "${APP_NAME}"
    cd ..
    echo "==> Successfully generated ZIP bundle"
fi

