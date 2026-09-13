#!/usr/bin/env bash
# scripts/build_all_releases.sh
# Master release generator for Linux, macOS, and Windows offline deployment
set -e

echo "================================================================="
echo "       Meridian 2.0 Master Multi-Platform Release Builder        "
echo "================================================================="

mkdir -p dist

echo "Step 1: Running full test suite..."
make all -j"$(nproc 2>/dev/null || echo 2)"
./build/meridian_tests

echo "Step 2: Building Linux native packages & standalone tarballs..."
./scripts/package_offline.sh

echo "Step 3: Building portable AppImage bundle..."
./packaging/appimage/build_appimage.sh

echo "Step 4: Copying spec and packaging recipes to dist/..."
cp -f packaging/rpm/meridian-terminal.spec dist/
cp -f packaging/arch/PKGBUILD dist/
cp -f packaging/macos/meridian-terminal.rb dist/
cp -f packaging/windows/install.ps1 dist/

echo ""
echo "================================================================="
echo "                All Master Releases Built!                       "
echo "================================================================="
echo "Artifacts generated in dist/:"
ls -lh dist/
echo "================================================================="

