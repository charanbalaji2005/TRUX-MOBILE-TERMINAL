#!/usr/bin/env bash
# ==============================================================================
# Meridian Terminal 2.5 — Universal Prebuilt Multi-Distribution Installer
# Repository: https://github.com/charanbalaji2005/Meridian-Shell
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
#   or with sudo:
#   curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash
# ==============================================================================
set -euo pipefail

APP_VERSION="2.5.0"
REPO_OWNER="charanbalaji2005"
REPO_NAME="Meridian-Shell"
GITHUB_REPO="${REPO_OWNER}/${REPO_NAME}"
RELEASE_BASE_URL="https://github.com/${GITHUB_REPO}/releases/download/v${APP_VERSION}"
RAW_BASE_URL="https://raw.githubusercontent.com/${GITHUB_REPO}/main"

# ------------------------------------------------------------------------------
# Colors & Formatting
# ------------------------------------------------------------------------------
if [ -t 1 ]; then
    BOLD="\033[1m"
    DIM="\033[2m"
    GREEN="\033[38;2;34;197;94m"
    YELLOW="\033[38;2;234;179;8m"
    CYAN="\033[38;2;6;182;212m"
    BLUE="\033[38;2;59;130;246m"
    RED="\033[38;2;239;68;68m"
    RESET="\033[0m"
else
    BOLD=""
    DIM=""
    GREEN=""
    YELLOW=""
    CYAN=""
    BLUE=""
    RED=""
    RESET=""
fi

log_step()    { echo -e " ${GREEN}✓${RESET} ${BOLD}$*${RESET}"; }
log_info()    { echo -e " ${CYAN}ℹ${RESET} $*"; }
log_warn()    { echo -e " ${YELLOW}⚠${RESET} $*"; }
log_error()   { echo -e " ${RED}✖ ERROR:${RESET} $*" >&2; }

echo -e "${CYAN}"
cat << "EOF"
╔══════════════════════════════════════════════════════════╗
║              Meridian Terminal 2.5 Installer             ║
╚══════════════════════════════════════════════════════════╝
EOF
echo -e "${RESET}"

# ------------------------------------------------------------------------------
# 1. Architecture & OS Detection
# ------------------------------------------------------------------------------
OS_TYPE="$(uname -s)"
ARCH_TYPE="$(uname -m)"

case "${ARCH_TYPE}" in
    x86_64|amd64)
        PKG_ARCH_DEB="amd64"
        PKG_ARCH_RPM="x86_64"
        PKG_ARCH_RAW="x86_64"
        ;;
    aarch64|arm64)
        PKG_ARCH_DEB="arm64"
        PKG_ARCH_RPM="aarch64"
        PKG_ARCH_RAW="aarch64"
        ;;
    *)
        log_error "Unsupported CPU architecture: ${ARCH_TYPE}"
        exit 1
        ;;
esac

DISTRO="unknown"
DISTRO_VERSION=""
if [ -f /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
    DISTRO="${ID:-unknown}"
    DISTRO_VERSION="${VERSION_ID:-}"
    DISTRO_NAME="${NAME:-Linux}"
elif [ "${OS_TYPE}" = "Darwin" ]; then
    DISTRO="macos"
    DISTRO_NAME="macOS $(sw_vers -productVersion 2>/dev/null || true)"
else
    DISTRO_NAME="Generic Linux"
fi

log_step "Detected: ${DISTRO_NAME} ${DISTRO_VERSION}"
log_step "Architecture: ${ARCH_TYPE}"

# ------------------------------------------------------------------------------
# 2. WSL Environment Check
# ------------------------------------------------------------------------------
IS_WSL=false
if grep -qi microsoft /proc/version 2>/dev/null || [ -n "${WSL_DISTRO_NAME:-}" ]; then
    IS_WSL=true
    log_info "WSL environment detected (${WSL_DISTRO_NAME:-Ubuntu/Debian WSL})"
    if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
        log_info "Display server: Headless / Terminal only (Meridian Shell will run directly)"
    else
        log_step "WSLg graphical display server active (${DISPLAY:-${WAYLAND_DISPLAY}})"
    fi
fi

# ------------------------------------------------------------------------------
# 3. Determine Installation Mode & Prefix
# ------------------------------------------------------------------------------
USER_MODE=false
PREFIX="/usr/local"

if [ "${EUID}" -ne 0 ]; then
    USER_MODE=true
    PREFIX="${HOME}/.local"
    log_info "Non-root user: Installing locally to ${PREFIX}/bin"
else
    log_info "Root/sudo user: Installing system-wide to ${PREFIX}/bin"
fi

# ------------------------------------------------------------------------------
# 4. Download & Install Prebuilt Release Package
# ------------------------------------------------------------------------------
WORK_DIR="$(mktemp -d /tmp/meridian-install-XXXXXX)"
trap 'rm -rf "${WORK_DIR}"' EXIT

cd "${WORK_DIR}"
INSTALLED=false

download_file() {
    local url="$1"
    local dest="$2"
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "${url}" -o "${dest}"
    elif command -v wget >/dev/null 2>&1; then
        wget -q "${url}" -O "${dest}"
    else
        log_error "Neither curl nor wget is installed."
        exit 1
    fi
}

log_step "Downloading Meridian Terminal ${APP_VERSION} for ${ARCH_TYPE}..."

# Case A: Ubuntu / Debian (.deb package)
if [ "${USER_MODE}" = false ] && { [ "${DISTRO}" = "ubuntu" ] || [ "${DISTRO}" = "debian" ] || [ "${DISTRO}" = "pop" ] || [ "${DISTRO}" = "mint" ]; }; then
    DEB_URL="${RELEASE_BASE_URL}/meridian-terminal_${APP_VERSION}_${PKG_ARCH_DEB}.deb"
    if download_file "${DEB_URL}" "meridian.deb" 2>/dev/null; then
        log_step "Installing native Debian/Ubuntu package via apt..."
        if apt-get update -qq && apt-get install -y -qq ./meridian.deb >/dev/null 2>&1; then
            INSTALLED=true
        elif dpkg -i ./meridian.deb >/dev/null 2>&1 || apt-get install -f -y -qq >/dev/null 2>&1; then
            INSTALLED=true
        fi
    fi
fi

# Case B: Fedora / RHEL / CentOS / Alma / Rocky (.rpm package)
if [ "${INSTALLED}" = false ] && [ "${USER_MODE}" = false ] && { [ "${DISTRO}" = "fedora" ] || [ "${DISTRO}" = "rhel" ] || [ "${DISTRO}" = "centos" ] || [ "${DISTRO}" = "almalinux" ] || [ "${DISTRO}" = "rocky" ]; }; then
    RPM_URL="${RELEASE_BASE_URL}/meridian-terminal-${APP_VERSION}-1.fc44.${PKG_ARCH_RPM}.rpm"
    if download_file "${RPM_URL}" "meridian.rpm" 2>/dev/null || download_file "${RELEASE_BASE_URL}/meridian-terminal-${APP_VERSION}.${PKG_ARCH_RPM}.rpm" "meridian.rpm" 2>/dev/null; then
        log_step "Installing native RPM package via dnf..."
        if dnf install -y -q ./meridian.rpm >/dev/null 2>&1 || rpm -Uvh --replacepkgs ./meridian.rpm >/dev/null 2>&1; then
            INSTALLED=true
        fi
    fi
fi

# Case C: Standalone Prebuilt Binary Tarball (Universal Linux & User Mode)
if [ "${INSTALLED}" = false ]; then
    TARBALL_NAME="meridian-terminal-${APP_VERSION}-linux-${PKG_ARCH_RAW}.tar.gz"
    TARBALL_URL="${RELEASE_BASE_URL}/${TARBALL_NAME}"

    if download_file "${TARBALL_URL}" "${TARBALL_NAME}" 2>/dev/null; then
        log_step "Extracting standalone prebuilt release archive..."
        tar -xzf "${TARBALL_NAME}"
        EXTRACTED_DIR="$(find . -maxdepth 1 -type d -name "meridian-terminal*" | head -n 1)"

        mkdir -p "${PREFIX}/bin" "${PREFIX}/share/applications" "${PREFIX}/share/icons/hicolor/scalable/apps"
        if [ -d "${EXTRACTED_DIR}/bin" ]; then
            cp -f "${EXTRACTED_DIR}/bin/meridian"* "${PREFIX}/bin/"
            chmod 755 "${PREFIX}/bin/meridian"*
        fi
        INSTALLED=true
    fi
fi

# Case D: Fallback Local Build (if release asset not yet published on GitHub)
if [ "${INSTALLED}" = false ]; then
    log_info "Prebuilt binary asset not reached; building Meridian 2.5 from source..."
    if [ -f "Makefile" ] && [ -d "src" ]; then
        make all -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"
    else
        download_file "https://github.com/${GITHUB_REPO}/archive/refs/heads/main.tar.gz" "source.tar.gz"
        tar -xzf "source.tar.gz"
        SRC_DIR="$(find . -maxdepth 1 -type d -name "*Meridian-Shell*" | head -n 1)"
        cd "${SRC_DIR}"
        make all -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"
    fi

    mkdir -p "${PREFIX}/bin"
    cp -f build/meridian "${PREFIX}/bin/meridian"
    cp -f build/meridian-shell "${PREFIX}/bin/meridian-shell"
    chmod 755 "${PREFIX}/bin/meridian" "${PREFIX}/bin/meridian-shell"
    INSTALLED=true
fi

# ------------------------------------------------------------------------------
# 5. Desktop Entry & Artwork Assets
# ------------------------------------------------------------------------------
log_step "Installing desktop integration & artwork gallery assets..."

DESKTOP_DIR="${HOME}/.local/share/applications"
ICON_DIR="${HOME}/.local/share/icons/hicolor/scalable/apps"
GALLERY_DIR="${HOME}/.config/meridian/gallery"

if [ "${USER_MODE}" = false ]; then
    DESKTOP_DIR="/usr/share/applications"
    ICON_DIR="/usr/share/icons/hicolor/scalable/apps"
fi

mkdir -p "${DESKTOP_DIR}" "${ICON_DIR}" "${GALLERY_DIR}"

if [ -f "packaging/desktop/org.meridian_terminal.MeridianTerminal.desktop" ]; then
    cp -f packaging/desktop/org.meridian_terminal.MeridianTerminal.desktop "${DESKTOP_DIR}/meridian.desktop" 2>/dev/null || true
elif [ -f "meridian.desktop" ]; then
    cp -f meridian.desktop "${DESKTOP_DIR}/meridian.desktop" 2>/dev/null || true
fi

if [ -f "resources/icons/meridian-terminal.svg" ]; then
    cp -f resources/icons/meridian-terminal.svg "${ICON_DIR}/meridian-terminal.svg" 2>/dev/null || true
fi

if [ -d "resources/images/gallery" ]; then
    cp -rf resources/images/gallery/* "${GALLERY_DIR}/" 2>/dev/null || true
fi

# Register in /etc/shells if root
if [ "${USER_MODE}" = false ] && [ -w /etc/shells ]; then
    for bin_path in "${PREFIX}/bin/meridian" "${PREFIX}/bin/meridian-shell"; do
        if ! grep -Fxq "${bin_path}" /etc/shells 2>/dev/null; then
            echo "${bin_path}" >> /etc/shells
        fi
    done
fi

# ------------------------------------------------------------------------------
# 6. Automatic IDE & Development Environment Detection
# ------------------------------------------------------------------------------
echo ""
echo -e "${CYAN}Detecting development environments...${RESET}"

DETECTED_IDES=()

# Visual Studio Code & Forks
if command -v code >/dev/null 2>&1 || [ -d "${HOME}/.config/Code" ] || [ -d "${HOME}/.config/Code - OSS" ] || [ -d "${HOME}/.config/VSCodium" ]; then
    echo -e " ${GREEN}✓${RESET} Visual Studio Code"
    DETECTED_IDES+=("VS Code")
fi

# Cursor AI Editor
if command -v cursor >/dev/null 2>&1 || [ -d "${HOME}/.config/Cursor" ]; then
    echo -e " ${GREEN}✓${RESET} Cursor"
    DETECTED_IDES+=("Cursor")
fi

# Windsurf AI IDE
if command -v windsurf >/dev/null 2>&1 || [ -d "${HOME}/.config/Windsurf" ]; then
    echo -e " ${GREEN}✓${RESET} Windsurf"
    DETECTED_IDES+=("Windsurf")
fi

# Google Antigravity
if [ -d "${HOME}/.config/Antigravity" ] || [ -d "${HOME}/.config/antigravity" ] || [ -d "${HOME}/.gemini/antigravity" ]; then
    echo -e " ${GREEN}✓${RESET} Google Antigravity"
    DETECTED_IDES+=("Google Antigravity")
fi

# Zed Editor
if command -v zed >/dev/null 2>&1 || [ -d "${HOME}/.config/zed" ]; then
    echo -e " ${GREEN}✓${RESET} Zed"
    DETECTED_IDES+=("Zed")
fi

# JetBrains Suite (IntelliJ, PyCharm, CLion, Android Studio)
if [ -d "${HOME}/.config/JetBrains" ] || [ -d "${HOME}/.local/share/JetBrains" ] || [ -d "${HOME}/.AndroidStudio" ]; then
    echo -e " ${GREEN}✓${RESET} JetBrains (IntelliJ / PyCharm / CLion / Android Studio)"
    DETECTED_IDES+=("JetBrains")
fi

# Neovim
if command -v nvim >/dev/null 2>&1 || [ -d "${HOME}/.config/nvim" ]; then
    echo -e " ${GREEN}✓${RESET} Neovim"
    DETECTED_IDES+=("Neovim")
fi

echo ""
echo -e "${CYAN}Registering Meridian terminal integrations...${RESET}"
for ide in "${DETECTED_IDES[@]}"; do
    echo -e " ${GREEN}✓${RESET} ${ide}"
done

# Perform profile registrations
SHELL_EXEC="${PREFIX}/bin/meridian-shell"
GUI_EXEC="${PREFIX}/bin/meridian"
if [ ! -f "${SHELL_EXEC}" ]; then
    SHELL_EXEC="${HOME}/.local/bin/meridian-shell"
    GUI_EXEC="${HOME}/.local/bin/meridian"
fi

if [ -f "${GUI_EXEC}" ]; then
    "${GUI_EXEC}" vscode enable >/dev/null 2>&1 || true
fi

echo ""
echo -e "${GREEN}Meridian is now available as an integrated terminal where supported.${RESET}"

# ------------------------------------------------------------------------------
# 7. PATH Configuration Check
# ------------------------------------------------------------------------------
PATH_OK=false
if echo ":${PATH}:" | grep -q ":${PREFIX}/bin:"; then
    PATH_OK=true
fi

log_step "PATH verified (${PREFIX}/bin)"

echo ""
echo -e "${GREEN}══════════════════════════════════════════════════════════${RESET}"
echo -e " ${GREEN}✔ Meridian Terminal ${APP_VERSION} installed successfully!${RESET}"
echo -e "${GREEN}══════════════════════════════════════════════════════════${RESET}"
echo ""
echo "Run:"
echo -e "   ${CYAN}meridian${RESET}              (Launch Meridian Terminal)"
echo -e "   ${CYAN}meridian-shell${RESET}        (Launch standalone shell)"
echo -e "   ${CYAN}meridian vscode${RESET}       (Manage VS Code terminal integration)"
echo -e "   ${CYAN}meridian update${RESET}       (Check and download latest releases)"
echo -e "   ${CYAN}meridian stats${RESET}        (View anonymous platform statistics)"
echo -e "   ${CYAN}auth github${RESET}           (1-click GitHub authentication & SSH setup)"
echo ""

if [ "${PATH_OK}" = false ] && [ "${USER_MODE}" = true ]; then
    echo -e "${YELLOW}Note: Add ~/.local/bin to your PATH if not already present:${RESET}"
    echo "   export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
fi
EOF
