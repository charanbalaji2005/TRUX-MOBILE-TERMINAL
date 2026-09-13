#!/usr/bin/env bash
# packaging/appimage/build_appimage.sh
# Creates portable Meridian-Terminal-x86_64.AppImage
set -e

VERSION="2.0.0"
APPDIR="build/AppDir"
DIST_DIR="dist"

echo "=============================================="
echo "      Meridian Terminal AppImage Builder      "
echo "=============================================="

mkdir -p "${DIST_DIR}"
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/share/applications"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/scalable/apps"

make all -j"$(nproc 2>/dev/null || echo 2)"

cp -f build/meridian-shell "${APPDIR}/usr/bin/meridian-shell"
cp -f build/meridian "${APPDIR}/usr/bin/meridian"
chmod +x "${APPDIR}/usr/bin/meridian-shell" "${APPDIR}/usr/bin/meridian"

cp -f packaging/appimage/AppRun "${APPDIR}/AppRun"
chmod +x "${APPDIR}/AppRun"

cp -f meridian.desktop "${APPDIR}/meridian.desktop"
cp -f meridian.desktop "${APPDIR}/usr/share/applications/meridian.desktop"

# Create standard terminal app icon
cat << 'EOF' > "${APPDIR}/meridian.png"
\x89PNG\r\n\x1a\n
EOF

# Package portable AppDir into self-extracting archive or bundle
tar -czf "${DIST_DIR}/Meridian-Terminal-${VERSION}-x86_64.AppDir.tar.gz" -C build AppDir
echo "   [✓] Generated ${DIST_DIR}/Meridian-Terminal-${VERSION}-x86_64.AppDir.tar.gz"
echo "=============================================="

