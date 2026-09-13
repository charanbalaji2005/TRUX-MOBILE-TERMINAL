#!/usr/bin/env bash
# ==============================================================================
# Meridian Terminal 2.5 — Universal Installer
# Repository: https://github.com/charanbalaji2005/Meridian-Shell
#
# Usage (Linux / macOS / WSL):
#   curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
#   or:
#   curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash
#
# What this script does:
#   1. Detect OS / Architecture / WSL
#   2. Try to download a prebuilt binary from GitHub Releases
#   3. If no prebuilt exists: install build tools and compile from source
#   4. Install to ~/.local/bin (user mode) or /usr/local/bin (root mode)
#   5. Register desktop entry and shell integration
# ==============================================================================
set -euo pipefail

APP_VERSION="2.5.1"
REPO="charanbalaji2005/Meridian-Shell"
RELEASE_URL="https://github.com/${REPO}/releases/download/v${APP_VERSION}"
SOURCE_URL="https://github.com/${REPO}/archive/refs/heads/main.tar.gz"

# ── Colors ────────────────────────────────────────────────────────────────────
if [ -t 1 ]; then
    BOLD="\033[1m"; DIM="\033[2m"; RESET="\033[0m"
    GREEN="\033[38;2;34;197;94m"; YELLOW="\033[38;2;234;179;8m"
    CYAN="\033[38;2;6;182;212m"; RED="\033[38;2;239;68;68m"
    BLUE="\033[38;2;59;130;246m"
else
    BOLD=""; DIM=""; RESET=""; GREEN=""; YELLOW=""; CYAN=""; RED=""; BLUE=""
fi

log_step()  { echo -e " ${GREEN}✓${RESET} ${BOLD}$*${RESET}"; }
log_info()  { echo -e " ${CYAN}ℹ${RESET} $*"; }
log_warn()  { echo -e " ${YELLOW}⚠${RESET} $*"; }
log_error() { echo -e " ${RED}✖ ERROR:${RESET} $*" >&2; }
log_die()   { log_error "$*"; exit 1; }

echo -e "${CYAN}${BOLD}"
cat << 'EOF'
╔══════════════════════════════════════════════════════════╗
║              Meridian Terminal 2.5 Installer             ║
║        https://github.com/charanbalaji2005/Meridian-Shell║
╚══════════════════════════════════════════════════════════╝
EOF
echo -e "${RESET}"

# ── 1. OS, Architecture & Environment Detection ───────────────────────────────
OS="$(uname -s)"         # Linux | Darwin | MINGW* | MSYS* | CYGWIN*
ARCH="$(uname -m)"       # x86_64 | aarch64 | arm64

# Normalise ARCH
case "${ARCH}" in
    x86_64|amd64)   ARCH="x86_64"; DEB_ARCH="amd64"; RPM_ARCH="x86_64" ;;
    aarch64|arm64)  ARCH="aarch64"; DEB_ARCH="arm64"; RPM_ARCH="aarch64" ;;
    *) log_die "Unsupported CPU architecture: ${ARCH}" ;;
esac

# Windows detection (Git Bash / Cygwin / MSYS2)
IS_WINDOWS=false
case "${OS}" in
    MINGW*|MSYS*|CYGWIN*) IS_WINDOWS=true ;;
esac

# WSL detection
IS_WSL=false
WSL_VER=0
if [ -f /proc/version ] && grep -qi microsoft /proc/version 2>/dev/null; then
    IS_WSL=true
    if grep -qi "wsl2\|microsoft-standard" /proc/version 2>/dev/null; then
        WSL_VER=2
    else
        WSL_VER=1
    fi
fi
[ -n "${WSL_DISTRO_NAME:-}" ] && IS_WSL=true

# macOS detection
IS_MACOS=false
[ "${OS}" = "Darwin" ] && IS_MACOS=true

# Linux distro detection
DISTRO="linux"
DISTRO_NAME="Linux"
PKG_MANAGER=""
if [ -f /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
    DISTRO="${ID:-linux}"
    DISTRO_NAME="${PRETTY_NAME:-${NAME:-Linux}}"
fi

# Package manager detection
if command -v apt-get >/dev/null 2>&1; then   PKG_MANAGER="apt";
elif command -v dnf >/dev/null 2>&1;   then   PKG_MANAGER="dnf";
elif command -v yum >/dev/null 2>&1;   then   PKG_MANAGER="yum";
elif command -v pacman >/dev/null 2>&1; then  PKG_MANAGER="pacman";
elif command -v zypper >/dev/null 2>&1; then  PKG_MANAGER="zypper";
elif command -v brew >/dev/null 2>&1;  then   PKG_MANAGER="brew";
elif command -v apk >/dev/null 2>&1;   then   PKG_MANAGER="apk";
fi

log_step "Detected: ${DISTRO_NAME} | ${OS} ${ARCH}"
[ "${IS_WSL}" = true ]   && log_info "WSL${WSL_VER} environment (Windows Subsystem for Linux)"
[ "${IS_MACOS}" = true ] && log_info "macOS $(sw_vers -productVersion 2>/dev/null || true)"
[ "${PKG_MANAGER}" != "" ] && log_info "Package manager: ${PKG_MANAGER}"

# ── 2. Installation Prefix ────────────────────────────────────────────────────
USER_MODE=false
PREFIX=""
EUID_VAL="${EUID:-$(id -u)}"

if [ "${EUID_VAL}" -ne 0 ]; then
    USER_MODE=true
    PREFIX="${HOME}/.local"
    mkdir -p "${PREFIX}/bin"
    log_info "Non-root: installing to ${PREFIX}/bin"
else
    PREFIX="/usr/local"
    mkdir -p "${PREFIX}/bin"
    log_info "Root: installing system-wide to ${PREFIX}/bin"
fi

# ── 3. Download Helper ────────────────────────────────────────────────────────
WORK_DIR="$(mktemp -d /tmp/meridian-install-XXXXXX)"
trap 'rm -rf "${WORK_DIR}"' EXIT
cd "${WORK_DIR}"

dl() {
    local url="$1" dest="$2"
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL --retry 3 --retry-delay 2 "${url}" -o "${dest}"
    elif command -v wget >/dev/null 2>&1; then
        wget -q --tries=3 --waitretry=2 "${url}" -O "${dest}"
    else
        log_die "Neither curl nor wget found. Install one and retry."
    fi
}

# Silent download — returns 0 on success, 1 on 404/error
dl_try() {
    local url="$1" dest="$2"
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL --retry 2 "${url}" -o "${dest}" 2>/dev/null
    elif command -v wget >/dev/null 2>&1; then
        wget -q --tries=2 "${url}" -O "${dest}" 2>/dev/null
    else
        return 1
    fi
}

# ── 4. Install Build Dependencies (for source build fallback) ─────────────────
install_build_deps() {
    log_info "Installing build dependencies (g++, make)..."
    case "${PKG_MANAGER}" in
        apt)
            local pkgs="g++ make git"
            if [ "${EUID_VAL}" -eq 0 ]; then
                DEBIAN_FRONTEND=noninteractive apt-get update -qq 2>/dev/null || true
                DEBIAN_FRONTEND=noninteractive apt-get install -y -qq ${pkgs} 2>/dev/null || true
            else
                log_warn "Need sudo to install build deps. Attempting..."
                sudo DEBIAN_FRONTEND=noninteractive apt-get update -qq 2>/dev/null || true
                sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq ${pkgs} 2>/dev/null || true
            fi
            ;;
        dnf|yum)
            local pkgs="gcc-c++ make git"
            if [ "${EUID_VAL}" -eq 0 ]; then
                "${PKG_MANAGER}" install -y -q ${pkgs} 2>/dev/null || true
            else
                sudo "${PKG_MANAGER}" install -y -q ${pkgs} 2>/dev/null || true
            fi
            ;;
        pacman)
            if [ "${EUID_VAL}" -eq 0 ]; then
                pacman -Sy --noconfirm base-devel git 2>/dev/null || true
            else
                sudo pacman -Sy --noconfirm base-devel git 2>/dev/null || true
            fi
            ;;
        zypper)
            if [ "${EUID_VAL}" -eq 0 ]; then
                zypper install -y gcc-c++ make git 2>/dev/null || true
            else
                sudo zypper install -y gcc-c++ make git 2>/dev/null || true
            fi
            ;;
        apk)
            if [ "${EUID_VAL}" -eq 0 ]; then
                apk add --no-cache g++ make git 2>/dev/null || true
            else
                sudo apk add --no-cache g++ make git 2>/dev/null || true
            fi
            ;;
        brew)
            brew install gcc make git 2>/dev/null || true
            ;;
        *)
            log_warn "Unknown package manager. Please manually install: g++, make, git"
            ;;
    esac

    # Verify
    if ! command -v make >/dev/null 2>&1 || ! command -v g++ >/dev/null 2>&1; then
        log_warn "Build tools not available. Trying alternatives..."
        # Try clang as fallback on macOS or minimal Linux
        if command -v clang++ >/dev/null 2>&1; then
            log_info "Found clang++ — will use it as CXX"
        fi
    fi
}

# ── 5. Source Build Function ──────────────────────────────────────────────────
build_from_source() {
    log_info "Building Meridian 2.5 from source..."

    # Install build deps first
    install_build_deps

    # Check for make/compiler
    if ! command -v make >/dev/null 2>&1; then
        log_die "make is not available and could not be installed. Please run: sudo apt install build-essential (Ubuntu/Debian) or sudo dnf install make gcc-c++ (Fedora/RHEL) and re-run this installer."
    fi
    if ! command -v g++ >/dev/null 2>&1 && ! command -v c++ >/dev/null 2>&1; then
        log_die "C++ compiler not found. Please install g++ and retry."
    fi

    # Download source
    log_info "Downloading source code from GitHub..."
    dl "${SOURCE_URL}" "meridian-source.tar.gz"
    tar -xzf "meridian-source.tar.gz"
    SRC_DIR="$(find . -maxdepth 1 -type d -name "*Meridian-Shell*" -o -name "*meridian*" | head -n 1)"
    if [ -z "${SRC_DIR}" ]; then
        log_die "Could not find extracted source directory."
    fi

    log_step "Compiling... (this takes 30–90 seconds)"
    cd "${SRC_DIR}"
    NCPU=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

    # Use clang++ if g++ unavailable (macOS Xcode)
    CXX_CMD="g++"
    command -v g++ >/dev/null 2>&1 || CXX_CMD="c++"

    make all -j"${NCPU}" CXX="${CXX_CMD}" 2>&1 | tail -5 || \
        log_die "Build failed. Check errors above. Ensure g++ >= 10 and make are installed."

    log_step "Build complete!"
    cd "${WORK_DIR}"
    echo "${SRC_DIR}"
}

# ── 6. Try Prebuilt Binary Download ──────────────────────────────────────────
INSTALLED=false

log_step "Checking for prebuilt release v${APP_VERSION}..."

# Try the Linux tarball asset
TARBALL="meridian-terminal-${APP_VERSION}-linux-${ARCH}.tar.gz"
if dl_try "${RELEASE_URL}/${TARBALL}" "${TARBALL}"; then
    log_step "Downloaded prebuilt binary tarball!"
    tar -xzf "${TARBALL}" 2>/dev/null || true
    EXTRACT_DIR="$(find . -maxdepth 1 -type d -name "meridian-terminal*" | head -n 1)"
    if [ -n "${EXTRACT_DIR}" ] && [ -f "${EXTRACT_DIR}/bin/meridian-shell" ]; then
        mkdir -p "${PREFIX}/bin"
        cp -f "${EXTRACT_DIR}/bin/meridian"  "${PREFIX}/bin/meridian"  2>/dev/null || true
        cp -f "${EXTRACT_DIR}/bin/meridian-shell" "${PREFIX}/bin/meridian-shell" 2>/dev/null || true
        chmod 755 "${PREFIX}/bin/meridian" "${PREFIX}/bin/meridian-shell" 2>/dev/null || true
        INSTALLED=true
        log_step "Prebuilt binary installed from release tarball."
    fi
fi

# Try .deb (apt-based, root only)
if [ "${INSTALLED}" = false ] && [ "${USER_MODE}" = false ] && [ "${PKG_MANAGER}" = "apt" ]; then
    DEB_FILE="meridian-terminal_${APP_VERSION}_${DEB_ARCH}.deb"
    if dl_try "${RELEASE_URL}/${DEB_FILE}" "${DEB_FILE}"; then
        if command -v dpkg >/dev/null 2>&1; then
            log_step "Installing via dpkg..."
            dpkg -i "./${DEB_FILE}" 2>/dev/null && apt-get install -f -y -qq 2>/dev/null && INSTALLED=true || true
        fi
    fi
fi

# ── 7. Source Build Fallback ──────────────────────────────────────────────────
if [ "${INSTALLED}" = false ]; then
    log_info "Prebuilt binary not available. Building from source..."
    SRC_BUILD_DIR="$(build_from_source)"
    if [ -n "${SRC_BUILD_DIR}" ] && [ -d "${WORK_DIR}/${SRC_BUILD_DIR}" ]; then
        BUILD_OUT="${WORK_DIR}/${SRC_BUILD_DIR}/build"
    elif [ -n "${SRC_BUILD_DIR}" ] && [ -d "${SRC_BUILD_DIR}" ]; then
        BUILD_OUT="${SRC_BUILD_DIR}/build"
    else
        log_die "Build output directory not found."
    fi

    mkdir -p "${PREFIX}/bin"
    cp -f "${BUILD_OUT}/meridian"       "${PREFIX}/bin/meridian"       2>/dev/null || log_die "meridian binary not built."
    cp -f "${BUILD_OUT}/meridian-shell" "${PREFIX}/bin/meridian-shell" 2>/dev/null || log_die "meridian-shell binary not built."
    chmod 755 "${PREFIX}/bin/meridian" "${PREFIX}/bin/meridian-shell"
    INSTALLED=true
fi

# ── 8. Verify Installation ────────────────────────────────────────────────────
MERIDIAN_BIN="${PREFIX}/bin/meridian-shell"
if [ ! -f "${MERIDIAN_BIN}" ]; then
    MERIDIAN_BIN="${HOME}/.local/bin/meridian-shell"
fi
if [ ! -f "${MERIDIAN_BIN}" ]; then
    log_die "Installation failed — binary not found at ${PREFIX}/bin/meridian-shell"
fi
log_step "Binary verified: $(du -sh "${MERIDIAN_BIN}" | cut -f1) at ${MERIDIAN_BIN}"

# ── 8b. Install High-Definition Artwork Gallery ───────────────────────────────
log_step "Installing HD Anime & Theme Art Gallery..."
GALLERY_DIR="${HOME}/.config/meridian/gallery"
mkdir -p "${GALLERY_DIR}" 2>/dev/null || true
if [ -d "resources/images/gallery" ]; then
    cp -rf resources/images/gallery/* "${GALLERY_DIR}/" 2>/dev/null || true
fi
if [ "${USER_MODE}" = false ]; then
    mkdir -p /usr/local/share/meridian/images/gallery 2>/dev/null || true
    cp -rf resources/images/gallery/* /usr/local/share/meridian/images/gallery/ 2>/dev/null || true
fi

# ── 9. Desktop & OS-Specific App Integration ──────────────────────────────────
if [ "${IS_MACOS}" = true ]; then
    log_step "Installing macOS Meridian.app bundle with official icon..."
    APP_BUNDLE="${HOME}/Applications/Meridian.app"
    [ "${USER_MODE}" = false ] && APP_BUNDLE="/Applications/Meridian.app"

    mkdir -p "${APP_BUNDLE}/Contents/MacOS" "${APP_BUNDLE}/Contents/Resources" || true
    
    # Copy official macOS .icns icon
    if [ -f "resources/icons/meridian-terminal.icns" ]; then
        cp -f "resources/icons/meridian-terminal.icns" "${APP_BUNDLE}/Contents/Resources/meridian-terminal.icns" 2>/dev/null || true
    fi

    cat > "${APP_BUNDLE}/Contents/Info.plist" << 'PLIST' || true
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>meridian_launcher</string>
    <key>CFBundleIdentifier</key>
    <string>org.meridian-terminal.MeridianTerminal</string>
    <key>CFBundleName</key>
    <string>Meridian Terminal</string>
    <key>CFBundleDisplayName</key>
    <string>Meridian Terminal</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleIconFile</key>
    <string>meridian-terminal.icns</string>
    <key>CFBundleShortVersionString</key>
    <string>2.5.1</string>
    <key>CFBundleVersion</key>
    <string>2.5.1</string>
</dict>
</plist>
PLIST

    cat > "${APP_BUNDLE}/Contents/MacOS/meridian_launcher" << LAUNCHER || true
#!/usr/bin/env bash
exec /usr/bin/osascript -e 'tell application "Terminal" to do script "${PREFIX}/bin/meridian"'
LAUNCHER
    chmod +x "${APP_BUNDLE}/Contents/MacOS/meridian_launcher"
    log_info "Meridian.app installed at ${APP_BUNDLE}"

elif [ "${IS_WINDOWS}" = false ]; then
    log_step "Installing desktop entry and modern Meridian app icons..."

    DESKTOP_DIR="${HOME}/.local/share/applications"
    ICON_DIR="${HOME}/.local/share/icons/hicolor/scalable/apps"
    if [ "${USER_MODE}" = false ]; then
        DESKTOP_DIR="/usr/share/applications"
        ICON_DIR="/usr/share/icons/hicolor/scalable/apps"
    fi
    mkdir -p "${DESKTOP_DIR}" "${ICON_DIR}" || true

    # Install scalable SVG and multi-resolution PNG icons
    if [ -f "resources/icons/meridian-terminal.svg" ]; then
        cp -f "resources/icons/meridian-terminal.svg" "${ICON_DIR}/meridian-terminal.svg" 2>/dev/null || true
        cp -f "resources/icons/meridian-terminal.svg" "${ICON_DIR}/meridian.svg" 2>/dev/null || true
    fi

    for s in 16 24 32 48 64 128 256 512; do
        target_dir="${HOME}/.local/share/icons/hicolor/${s}x${s}/apps"
        [ "${USER_MODE}" = false ] && target_dir="/usr/share/icons/hicolor/${s}x${s}/apps"
        mkdir -p "${target_dir}" 2>/dev/null || true
        if [ -f "resources/icons/meridian_${s}.png" ]; then
            cp -f "resources/icons/meridian_${s}.png" "${target_dir}/meridian-terminal.png" 2>/dev/null || true
            cp -f "resources/icons/meridian_${s}.png" "${target_dir}/meridian.png" 2>/dev/null || true
        fi
    done

    # Write desktop entry inline with Terminal=true for instant graphical launching
    cat > "${DESKTOP_DIR}/meridian.desktop" 2>/dev/null << DESKTOP || true
[Desktop Entry]
Version=1.5
Type=Application
Name=Meridian Terminal
GenericName=Terminal Emulator & AI Dev Shell
Comment=Modern AI developer shell with autosuggestions and live Git intelligence
Exec=${PREFIX}/bin/meridian
Icon=meridian-terminal
Terminal=true
Categories=System;TerminalEmulator;Development;
Keywords=terminal;shell;console;ai;meridian;
StartupNotify=true
DESKTOP
    chmod 644 "${DESKTOP_DIR}/meridian.desktop" 2>/dev/null || true

    # Register shell paths
    if [ "${USER_MODE}" = false ] && [ -w /etc/shells ]; then
        for bp in "${PREFIX}/bin/meridian" "${PREFIX}/bin/meridian-shell"; do
            grep -qxF "${bp}" /etc/shells 2>/dev/null || echo "${bp}" >> /etc/shells || true
        done
    fi
fi

# ── 9b. Windows & WSL Host Integration ───────────────────────────────────────
if [ "${IS_WSL}" = true ]; then
    log_step "Configuring Windows Host Launchers & Windows Terminal..."
    for win_user in /mnt/c/Users/*; do
        if [ -d "${win_user}" ] && [ "$(basename "${win_user}")" != "Public" ] && [ "$(basename "${win_user}")" != "Default" ] && [ "$(basename "${win_user}")" != "Default User" ]; then
            win_bin_dir="${win_user}/.local/bin"
            mkdir -p "${win_bin_dir}" 2>/dev/null || true
            cat > "${win_bin_dir}/meridian.cmd" 2>/dev/null << 'CMD' || true
@echo off
wsl.exe -e meridian-shell %*
CMD
            cat > "${win_bin_dir}/meridian.ps1" 2>/dev/null << 'PS1' || true
wsl.exe -e meridian-shell $args
PS1
            if [ -f "resources/icons/meridian.ico" ]; then
                cp -f "resources/icons/meridian.ico" "${win_bin_dir}/meridian.ico" 2>/dev/null || true
            fi
        fi
    done
fi

# ── 10. PATH Configuration ────────────────────────────────────────────────────
NEEDS_PATH=false
echo ":${PATH}:" | grep -q ":${PREFIX}/bin:" || NEEDS_PATH=true

# Auto-add to common shell configs if in user mode
if [ "${USER_MODE}" = true ] && [ "${NEEDS_PATH}" = true ]; then
    EXPORT_LINE="export PATH=\"\$HOME/.local/bin:\$PATH\""
    for rcfile in "${HOME}/.bashrc" "${HOME}/.zshrc" "${HOME}/.profile" "${HOME}/.bash_profile"; do
        if [ -f "${rcfile}" ] && ! grep -q '.local/bin' "${rcfile}" 2>/dev/null; then
            echo "" >> "${rcfile}"
            echo "# Meridian Terminal" >> "${rcfile}"
            echo "${EXPORT_LINE}" >> "${rcfile}"
            log_info "Added PATH to ${rcfile}"
            break
        fi
    done
fi

# ── 11. IDE Detection & Registration ─────────────────────────────────────────
echo ""
log_info "Scanning for development environments..."

detect_and_register() {
    local name="$1"; shift
    for loc in "$@"; do
        if command -v "${loc}" >/dev/null 2>&1 || [ -d "${HOME}/.config/${loc}" ] || [ -d "${HOME}/.config/${name}" ]; then
            echo -e "   ${GREEN}✓${RESET} ${name}"
            return 0
        fi
    done
    return 0
}

detect_and_register "Visual Studio Code"  "code"          "Code"
detect_and_register "Cursor"              "cursor"        "Cursor"
detect_and_register "Windsurf"            "windsurf"      "Windsurf"
detect_and_register "Zed"                 "zed"           "zed"
detect_and_register "Neovim"              "nvim"          "nvim"
detect_and_register "JetBrains"           ""              "JetBrains"
[ -d "${HOME}/.gemini/antigravity" ] && echo -e "   ${GREEN}✓${RESET} Google Antigravity"
[ -n "${WSL_DISTRO_NAME:-}" ]        && echo -e "   ${CYAN}ℹ${RESET} Windows Terminal (WSL) — Meridian runs natively inside WSL"

# ── 12. Success Banner ────────────────────────────────────────────────────────
echo ""
echo -e "${GREEN}${BOLD}══════════════════════════════════════════════════════════${RESET}"
echo -e " ${GREEN}${BOLD}✔  Meridian Terminal ${APP_VERSION} installed successfully!${RESET}"
echo -e "${GREEN}${BOLD}══════════════════════════════════════════════════════════${RESET}"
echo ""
echo -e "Run these commands to get started:"
echo ""
echo -e "   ${CYAN}meridian-shell${RESET}         Launch Meridian Shell (interactive)"
echo -e "   ${CYAN}meridian${RESET}               Launch full Meridian Terminal"
echo -e "   ${CYAN}meridian update${RESET}        Check for updates"
echo -e "   ${CYAN}meridian stats${RESET}         View session statistics"
echo ""

if [ "${NEEDS_PATH}" = true ] && [ "${USER_MODE}" = true ]; then
    echo -e "${YELLOW}${BOLD}⚠  Add Meridian to your PATH:${RESET}"
    echo ""
    echo "   export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
    echo -e "   ${DIM}Or restart your terminal — it was auto-added to your shell RC file.${RESET}"
    echo ""
fi

if [ "${IS_WSL}" = true ]; then
    echo -e "${CYAN}ℹ  WSL Note:${RESET} Run ${CYAN}meridian-shell${RESET} directly inside your WSL terminal."
    echo -e "   For Windows Terminal integration, add it as a new profile:"
    echo -e "   ${DIM}Profile → New → Command: wsl.exe -e meridian-shell${RESET}"
    echo ""
fi
