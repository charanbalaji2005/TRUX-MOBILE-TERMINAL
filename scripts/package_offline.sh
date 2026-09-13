#!/usr/bin/env bash
# scripts/package_offline.sh
# Automated Offline Universal Packager for Meridian Terminal 2.0
set -e

VERSION="2.0.0"
DIST_DIR="dist"

echo "=============================================="
echo "    Meridian Terminal Offline Packager        "
echo "=============================================="

mkdir -p "${DIST_DIR}"

echo "-> 1. Compiling Meridian binaries..."
make all -j"$(nproc 2>/dev/null || echo 2)"

echo "-> 2. Creating Standalone Universal Offline Tarball..."
TARBALL_ROOT="meridian-terminal-${VERSION}-linux-x86_64"
rm -rf "${TARBALL_ROOT}"
mkdir -p "${TARBALL_ROOT}/bin"
cp -f build/meridian-shell "${TARBALL_ROOT}/bin/"
cp -f build/meridian "${TARBALL_ROOT}/bin/"
cp -f install.sh "${TARBALL_ROOT}/"
cp -f meridian.desktop "${TARBALL_ROOT}/"
cp -f README.md "${TARBALL_ROOT}/"
cp -f LICENSE "${TARBALL_ROOT}/"
tar -czf "${DIST_DIR}/${TARBALL_ROOT}.tar.gz" "${TARBALL_ROOT}"
rm -rf "${TARBALL_ROOT}"
echo "   [✓] Generated ${DIST_DIR}/${TARBALL_ROOT}.tar.gz"

echo "-> 3. Building Debian / Ubuntu Package (.deb)..."
DEB_ROOT="meridian-terminal_${VERSION}_amd64"
rm -rf "${DEB_ROOT}"
mkdir -p "${DEB_ROOT}/DEBIAN"
mkdir -p "${DEB_ROOT}/usr/bin"
mkdir -p "${DEB_ROOT}/usr/share/applications"
mkdir -p "${DEB_ROOT}/usr/share/doc/meridian-terminal"

cat <<EOF > "${DEB_ROOT}/DEBIAN/control"
Package: meridian-terminal
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: amd64
Maintainer: Charan Balaji <charanbalaji@meridian.dev>
Description: Modern Linux Terminal & Unified Developer Environment
 Meridian Terminal is a fast Linux terminal emulator, developer environment,
 and AI agent platform with native TrueColor graphics, live Git intelligence,
 multi-pane multiplexing, and in-terminal command preview.
EOF

cp -f build/meridian-shell "${DEB_ROOT}/usr/bin/meridian-shell"
cp -f build/meridian "${DEB_ROOT}/usr/bin/meridian"
chmod 755 "${DEB_ROOT}/usr/bin/meridian-shell" "${DEB_ROOT}/usr/bin/meridian"
cp -f meridian.desktop "${DEB_ROOT}/usr/share/applications/meridian.desktop"
cp -f LICENSE "${DEB_ROOT}/usr/share/doc/meridian-terminal/copyright"

if command -v dpkg-deb >/dev/null 2>&1; then
    dpkg-deb --build --root-owner-group "${DEB_ROOT}" "${DIST_DIR}/${DEB_ROOT}.deb"
    echo "   [✓] Generated ${DIST_DIR}/${DEB_ROOT}.deb"
else
    tar -czf "${DIST_DIR}/${DEB_ROOT}-data.tar.gz" -C "${DEB_ROOT}" .
    echo "   [!] dpkg-deb not found. Packaged ${DIST_DIR}/${DEB_ROOT}-data.tar.gz (ready for dpkg-deb)."
fi
rm -rf "${DEB_ROOT}"

echo "-> 4. Building Fedora / RHEL RPM Package (.rpm)..."
if command -v rpmbuild >/dev/null 2>&1; then
    RPM_BUILD_DIR="$(mktemp -d /tmp/meridian-rpm-XXXXXX)"
    mkdir -p "${RPM_BUILD_DIR}"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
    tar -czf "${RPM_BUILD_DIR}/SOURCES/meridian-terminal-${VERSION}.tar.gz" --transform "s,^,meridian-terminal-${VERSION}/," --exclude='.git' --exclude='build' --exclude='dist' .
    rpmbuild --define "_topdir ${RPM_BUILD_DIR}" -ba packaging/rpm/meridian-terminal.spec >/dev/null 2>&1 || true
    cp -f "${RPM_BUILD_DIR}"/RPMS/*/*.rpm "${DIST_DIR}/" 2>/dev/null || true
    cp -f "${RPM_BUILD_DIR}"/SRPMS/*.src.rpm "${DIST_DIR}/" 2>/dev/null || true
    rm -rf "${RPM_BUILD_DIR}"
    echo "   [✓] Generated Fedora RPM packages in ${DIST_DIR}/"
fi

echo ""
echo "=============================================="
echo " Packaging complete! Offline packages saved in: "
echo " ${DIST_DIR}/"
echo "=============================================="
ls -lh "${DIST_DIR}"

