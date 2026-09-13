export interface DocArticle {
  id: string;
  title: string;
  category: string;
  status?: 'implemented' | 'development' | 'experimental' | 'planned';
  lastUpdated: string;
  headings: { id: string; text: string; level: number }[];
  summary: string;
  body: string;
}

export const DOCS_ARTICLES: Record<string, DocArticle> = {
  "intro": {
    "id": "intro",
    "title": "Meridian Shell",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "quick-start",
        "text": "Quick Start",
        "level": 2
      },
      {
        "id": "what-is-meridian",
        "text": "What is Meridian?",
        "level": 2
      },
      {
        "id": "core-features",
        "text": "Core Features",
        "level": 2
      },
      {
        "id": "architecture",
        "text": "System Architecture",
        "level": 2
      },
      {
        "id": "direct-image-rendering",
        "text": "Direct Raster Image Rendering",
        "level": 2
      },
      {
        "id": "terminal-compatibility",
        "text": "Terminal Compatibility",
        "level": 2
      },
      {
        "id": "packaging",
        "text": "Distribution & Packaging",
        "level": 2
      },
      {
        "id": "project-status",
        "text": "Project Status Matrix",
        "level": 2
      }
    ],
    "summary": "A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.",
    "body": "<div class=\"hero-section\">\n  <p class=\"hero-lead\">\n    A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.\n  </p>\n  <div class=\"hero-actions\">\n    <a href=\"#installation\" class=\"btn-primary\">Get Started</a>\n    <a href=\"#development-building\" class=\"btn-secondary\">Build from Source</a>\n    <a href=\"https://github.com/charanbalaji2005/Meridian-Shell\" target=\"_blank\" rel=\"noreferrer\" class=\"btn-outline\">\n      <svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M15 22v-4a4.8 4.8 0 0 0-1-3.5c3 0 6-2 6-5.5.08-1.25-.27-2.48-1-3.5.28-1.15.28-2.35 0-3.5 0 0-1 0-3 1.5-2.64-.5-5.36-.5-8 0C6 2 5 2 5 2c-.3 1.15-.3 2.35 0 3.5A5.403 5.403 0 0 0 4 9c0 3.5 3 5.5 6 5.5-.39.49-.68 1.05-.85 1.65-.17.6-.22 1.23-.15 1.85v4\"/><path d=\"M9 18c-4.51 2-5-2-7-2\"/></svg>\n      GitHub\n    </a>\n  </div>\n</div>\n\n<div class=\"terminal-screenshot-card\">\n  <img src=\"./assets/meridian-terminal-screenshot-1.png\" alt=\"Meridian Terminal Live Interface with Anime Artwork and System Telemetry\" class=\"terminal-screenshot-img\" />\n  <div class=\"terminal-screenshot-caption\">\n    Meridian Shell running live interactive session with GPU-accelerated raster artwork header, multi-badge Powerline status, and system telemetry.\n  </div>\n</div>\n\n<div class=\"terminal-screenshot-card\">\n  <img src=\"./assets/meridian-terminal-screenshot-2.png\" alt=\"Meridian Terminal Split Panes and Fastfetch Metrics\" class=\"terminal-screenshot-img\" />\n  <div class=\"terminal-screenshot-caption\">\n    Meridian Terminal displaying custom anime wallpapers, live Git branch divergence, and system metrics on Fedora Linux.\n  </div>\n</div>\n\n<h2 id=\"quick-start\">Quick Start</h2>\n<p>Meridian Shell is available for Fedora, Ubuntu, Debian, Arch Linux, openSUSE, and generic Linux platforms. Choose your preferred installation method below:</p>\n\n<div class=\"note-box note-info\">\n  <div class=\"note-title\">📌 Important Notice on Package Repositories</div>\n  <p>Commands like <code>sudo dnf install meridian-terminal</code> require the Meridian repository to be enabled first. For immediate installation on a fresh machine without adding a repository, use the local package files or the universal curl installer below.</p>\n</div>\n\n<h3>1. Local Package Installation</h3>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Fedora / RHEL (Local RPM)</span></div><pre><code class=\"language-bash\">sudo dnf install ./meridian-terminal.rpm</code></pre></div>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Ubuntu / Debian (Local DEB)</span></div><pre><code class=\"language-bash\">sudo apt install ./meridian-terminal.deb</code></pre></div>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Arch Linux (Local Package)</span></div><pre><code class=\"language-bash\">sudo pacman -U ./meridian-terminal.pkg.tar.zst</code></pre></div>\n\n<h3>2. Universal Turnkey Installer</h3>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Bash 1-Liner (User)</span></div><pre><code class=\"language-bash\">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre></div>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Bash 1-Liner (System-Wide)</span></div><pre><code class=\"language-bash\">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash</code></pre></div>\n\n<h2 id=\"what-is-meridian\">What is Meridian?</h2>\n<p><strong>Meridian Shell</strong> is an open-source terminal emulator and developer platform designed to unify the terminal layer with native graphics, extensible developer intelligence, and real pseudoterminal multiplexing. Rather than being a simulated web terminal or electron wrapper, Meridian interacts directly with the Linux kernel via POSIX <code>openpty</code>, supporting full interactive terminal workflows.</p>\n\n<h2 id=\"core-features\">Core Features</h2>\n<div class=\"feature-cards-grid\">\n  <div class=\"feat-card\"><div class=\"feat-icon\">⚡</div><div class=\"feat-title\">Real PTY Sessions</div><div class=\"feat-body\">Asynchronous POSIX openpty multiplexer with non-blocking I/O and process group signal forwarding.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🖥️</div><div class=\"feat-title\">VT / ANSI Emulation</div><div class=\"feat-body\">Full escape sequence parsing, alternate screen buffers, 24-bit TrueColor RGB, and cursor addressing.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🖼️</div><div class=\"feat-title\">Direct Raster Graphics</div><div class=\"feat-body\">Direct 32-bit RGBA inline image decoding with zero ASCII downsampling or character-block compromises.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🪟</div><div class=\"feat-title\">Tabs & Panes</div><div class=\"feat-body\">Multi-pane splits (Ctrl+Shift+D/E), pane zooming, and persistent workspace layouts across restarts.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🌐</div><div class=\"feat-title\">Advanced Protocols</div><div class=\"feat-body\">OSC 8 clickable hyperlinks, OSC 52 remote clipboard sync, OSC 7 CWD tracking, and OSC 133 prompt markers.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🔌</div><div class=\"feat-title\">Extensible Plugins</div><div class=\"feat-body\">Lifecycle hooks for command execution, background watchers, and custom telemetry overlays.</div></div>\n</div>\n\n<h2 id=\"architecture\">System Architecture</h2>\n<p>Meridian is engineered as decoupled subsystems to guarantee extreme stability and fault isolation:</p>\n<div class=\"arch-diagram-block\"><pre><code class=\"language-text\">Meridian Application\n   │\n   ├── GUI Frontend (Qt6 / Wayland / X11 Canvas)\n   │\n   ├── Terminal Session Manager\n   │   ├── POSIX openpty Multiplexer (Master/Slave FDs)\n   │   ├── Real Foreground Shell Process (bash, zsh, fish)\n   │   └── Non-blocking Epoll/Kqueue I/O Loop\n   │\n   ├── Terminal Core Engine\n   │   ├── VT / ANSI State Machine (VT100, VT220, XTerm)\n   │   ├── ScreenBuffer Matrix (2D Cell Grid + Attributes)\n   │   ├── Hyperlink Table & OSC Protocol Handlers\n   │   └── High-Capacity Scrollback Buffer\n   │\n   ├── Hardware Renderer\n   │   ├── GPU Render Pipeline & Damage Rect Tracker\n   │   ├── Sub-pixel Glyph Texture Atlas (Freetype/Harfbuzz)\n   │   └── Direct 32-bit RGBA Image Texture Compositor\n   │\n   └── Developer Intelligence & Security\n       ├── Native SSH Workspace Manager (~/.ssh/config)\n       ├── Extensible Plugin Engine (~/.config/meridian/plugins/)\n       ├── Risk Classification Interceptor & Secret Redactor\n       └── GPU Telemetry & Performance Profiler</code></pre></div>\n\n<h2 id=\"direct-image-rendering\">Direct Raster Image Rendering</h2>\n<p>Executing <code>pic image.png</code> produces a <strong>direct full-color raster image</strong> on the terminal canvas.</p>\n<div class=\"flow-diagram\">\n  <div class=\"flow-box\">PNG / JPEG / WebP / BMP</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box\">graphics::ImageDecoder (stb_image)</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box\">32-bit RGBA Pixels</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box\">GPU Texture</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box highlight-box\">ACTUAL IMAGE PIXELS ON SCREEN</div>\n</div>\n\n<h2 id=\"terminal-compatibility\">Terminal Compatibility</h2>\n<p>Meridian is tested against standard Linux CLI applications and full-screen TUI workflows:</p>\n<ul>\n  <li><strong>Shells:</strong> <code>bash</code>, <code>zsh</code>, <code>fish</code>, <code>sh</code></li>\n  <li><strong>Editors:</strong> <code>vim</code>, <code>neovim</code>, <code>nano</code>, <code>emacs</code>, <code>micro</code></li>\n  <li><strong>Multiplexers:</strong> <code>tmux</code>, <code>zellij</code>, <code>screen</code></li>\n  <li><strong>System Monitors:</strong> <code>top</code>, <code>htop</code>, <code>btop</code>, <code>glances</code>, <code>fastfetch</code></li>\n  <li><strong>Tools & Runtimes:</strong> <code>git</code>, <code>ssh</code>, <code>docker</code>, <code>podman</code>, <code>python</code>, <code>node</code>, <code>cargo</code></li>\n</ul>\n\n<h2 id=\"packaging\">Distribution & Packaging</h2>\n<p>Available for Fedora (RPM), Debian/Ubuntu (.deb), Arch Linux (PKGBUILD), and Universal Linux tarballs.</p>\n\n<h2 id=\"project-status\">Project Status Matrix</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Subsystem</th><th>Status</th><th>Notes</th></tr></thead>\n  <tbody>\n    <tr><td>PTY Core & Job Control</td><td><span class=\"status-tag status-impl\">IMPLEMENTED</span></td><td>Async openpty, process group signals (SIGINT, SIGTSTP, SIGWINCH).</td></tr>\n    <tr><td>VT Engine & ANSI Parser</td><td><span class=\"status-tag status-impl\">IMPLEMENTED</span></td><td>VT100, VT220, TrueColor RGB, Alternate Buffer, OSC 7/8/52/133.</td></tr>\n    <tr><td>Direct Raster Images (pic)</td><td><span class=\"status-tag status-impl\">IMPLEMENTED</span></td><td>32-bit RGBA hardware texture rendering (zero ASCII).</td></tr>\n    <tr><td>SSH Workspace Manager</td><td><span class=\"status-tag status-impl\">IMPLEMENTED</span></td><td>Direct ~/.ssh/config parser and connection launcher.</td></tr>\n    <tr><td>Extensible Plugin Engine</td><td><span class=\"status-tag status-impl\">IMPLEMENTED</span></td><td>Lifecycle hooks for pre/post command execution.</td></tr>\n    <tr><td>GPU Telemetry Profiler</td><td><span class=\"status-tag status-impl\">IMPLEMENTED</span></td><td>Real-time FPS, frame time, glyph count, VRAM, and PTY latency.</td></tr>\n    <tr><td>GUI Windowing & Canvas</td><td><span class=\"status-tag status-dev\">DEVELOPMENT</span></td><td>Qt6 / Wayland / X11 multi-window canvas integration.</td></tr>\n    <tr><td>Kitty Graphics Protocol</td><td><span class=\"status-tag status-dev\">DEVELOPMENT</span></td><td>2048-byte safe chunked APC transmission parser.</td></tr>\n    <tr><td>DEC Sixel Graphics</td><td><span class=\"status-tag status-dev\">DEVELOPMENT</span></td><td>DCS bitmap stream decoder with 256-color palette.</td></tr>\n  </tbody>\n</table>"
  },
  "installation": {
    "id": "installation",
    "title": "Installation & Build",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "dnf-rpm",
        "text": "Fedora & RHEL (DNF / RPM)",
        "level": 2
      },
      {
        "id": "apt-deb",
        "text": "Ubuntu & Debian (APT / DEB)",
        "level": 2
      },
      {
        "id": "arch-pkg",
        "text": "Arch Linux (Pacman / PKGBUILD)",
        "level": 2
      },
      {
        "id": "opensuse-pkg",
        "text": "openSUSE (Zypper / RPM)",
        "level": 2
      },
      {
        "id": "universal-script",
        "text": "Universal Turnkey Installer",
        "level": 2
      },
      {
        "id": "building-from-source",
        "text": "Compiling from Source",
        "level": 2
      },
      {
        "id": "dependencies-table",
        "text": "Build & Runtime Dependencies",
        "level": 2
      },
      {
        "id": "uninstalling",
        "text": "Uninstallation & Purge",
        "level": 2
      }
    ],
    "summary": "Comprehensive installation options across Linux distributions, macOS, and Windows.",
    "body": "<p>Meridian provides native packages for all major Linux distributions, as well as a standalone universal installer.</p>\n\n<h2 id=\"dnf-rpm\">Fedora & RHEL (DNF / RPM)</h2>\n<p>To install a locally downloaded RPM package on Fedora 39, 40, 41, or 44:</p>\n<pre><code class=\"language-bash\"># Install local RPM archive\nsudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm\n\n# Or using rpm directly\nsudo rpm -Uvh ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>\n\n<div class=\"note-box note-warning\">\n  <div class=\"note-title\">⚠️ Repository Installation Requirement</div>\n  <p>To install via <code>sudo dnf install meridian-terminal</code> without specifying a local file path, the official Meridian COPR / RPM repository must be added to <code>/etc/yum.repos.d/</code>.</p>\n</div>\n\n<h2 id=\"apt-deb\">Ubuntu & Debian (APT / DEB)</h2>\n<p>For Debian 12+, Ubuntu 22.04 LTS, Ubuntu 24.04 LTS, and Linux Mint:</p>\n<pre><code class=\"language-bash\">sudo apt update\nsudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>\n\n<h2 id=\"arch-pkg\">Arch Linux (Pacman / PKGBUILD)</h2>\n<p>On Arch Linux, Manjaro, or EndeavourOS:</p>\n<pre><code class=\"language-bash\">cd packaging/arch\nmakepkg -si</code></pre>\n\n<h2 id=\"opensuse-pkg\">openSUSE (Zypper / RPM)</h2>\n<p>On openSUSE Tumbleweed or Leap:</p>\n<pre><code class=\"language-bash\">sudo zypper install ./meridian-terminal.rpm</code></pre>\n\n<h2 id=\"universal-script\">Universal Turnkey Installer</h2>\n<p>The universal installer detects your distribution, installs missing dependencies, compiles the latest release, and places binary launchers into <code>/usr/local/bin</code>:</p>\n<pre><code class=\"language-bash\"># Standard user installation (~/.local/bin)\ncurl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash\n\n# System-wide installation (/usr/local/bin)\ncurl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash</code></pre>\n\n<h2 id=\"building-from-source\">Compiling from Source</h2>\n<p>Building Meridian from source requires a modern C++20 compiler (GCC 11+ or Clang 13+):</p>\n<pre><code class=\"language-bash\">git clone https://github.com/charanbalaji2005/Meridian-Shell.git\ncd Meridian-Shell\n\n# Build all binaries (terminal, shell, tests, demo)\nmake all -j$(nproc)\n\n# Run test suite (133 tests)\nmake test\n\n# Install to system\nsudo make install</code></pre>\n\n<h2 id=\"dependencies-table\">Build & Runtime Dependencies</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Package</th><th>Fedora Package</th><th>Debian/Ubuntu Package</th><th>Purpose</th></tr></thead>\n  <tbody>\n    <tr><td>C++20 Compiler</td><td><code>gcc-c++</code></td><td><code>g++</code></td><td>Core compilation</td></tr>\n    <tr><td>POSIX Libutil</td><td><code>glibc-devel</code></td><td><code>libc6-dev</code></td><td>openpty and termios</td></tr>\n    <tr><td>Make</td><td><code>make</code></td><td><code>make</code></td><td>Build automation</td></tr>\n    <tr><td>Nerd Font</td><td><code>google-noto-sans-fonts</code></td><td><code>fonts-noto</code></td><td>Powerline & glyph rendering</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"uninstalling\">Uninstallation & Purge</h2>\n<p>Meridian provides a clean uninstaller script that removes all binaries, desktop entries, and icons:</p>\n<pre><code class=\"language-bash\"># Standard uninstallation (preserves ~/.config/meridian configs)\nsudo ./uninstall.sh\n\n# Complete purge (removes binaries AND config/history databases)\nsudo ./uninstall.sh --purge\n\n# For DNF package installations\nsudo dnf remove meridian-terminal</code></pre>"
  },
  "quickstart": {
    "id": "quickstart",
    "title": "Quick Start",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "launch",
        "text": "Launching Meridian",
        "level": 2
      },
      {
        "id": "first-commands",
        "text": "First Commands",
        "level": 2
      },
      {
        "id": "themes",
        "text": "Anime Theme Selection (Ctrl+P / pic)",
        "level": 2
      },
      {
        "id": "palette",
        "text": "Command Palette (Ctrl+Shift+P)",
        "level": 2
      },
      {
        "id": "splits",
        "text": "Window Splits & Panes",
        "level": 2
      },
      {
        "id": "essential-shortcuts",
        "text": "Essential Daily Shortcuts",
        "level": 2
      }
    ],
    "summary": "Get started with Meridian Terminal in under one minute.",
    "body": "<h2 id=\"launch\">Launching Meridian</h2>\n<p>Launch Meridian from your application menu or terminal:</p>\n<pre><code class=\"language-bash\"># Launch full interactive terminal emulator\nmeridian\n\n# Launch standalone shell engine directly\nmeridian-shell</code></pre>\n\n<h2 id=\"first-commands\">First Commands</h2>\n<p>Try running these built-in developer commands inside Meridian:</p>\n<pre><code class=\"language-bash\">meridian monitor        # Live CPU, RAM, Disk, Network, and Process metrics\nmeridian git            # Visual Git branch divergence and staged/unstaged changes\nmeridian files          # Interactive tree file explorer with git badges\nmeridian ssh            # List and connect to SSH remote workspaces\nmeridian --performance  # Live GPU framerate, PTY latency & telemetry HUD</code></pre>\n\n<h2 id=\"themes\">Anime Theme Selection (Ctrl+P / pic)</h2>\n<p>Press <code>Ctrl+P</code> to browse the theme gallery or configure your startup anime wallpaper via the CLI:</p>\n<pre><code class=\"language-bash\">pic set sharingan_eye   # Sasuke/Itachi Mangekyō Sharingan (Theme 0)\npic set sakura_girl     # Sakura Blossom Anime Girl (Theme 1)\npic set ribbon_girl     # Monochrome Anime Ribbon Girl (Theme 2)\npic set gojo_purple     # Gojo Satoru: Hollow Purple (Theme 5)\npic set sukuna_shrine   # Sukuna: Malevolent Shrine (Theme 6)\npic set random          # Automatically rotate theme on each startup</code></pre>\n\n<h2 id=\"palette\">Command Palette (Ctrl+Shift+P)</h2>\n<p>Press <code>Ctrl+Shift+P</code> to fuzzy-search across all terminal actions, window splits, theme changes, and diagnostics without leaving the keyboard.</p>\n\n<h2 id=\"splits\">Window Splits & Panes</h2>\n<ul>\n  <li><code>Ctrl+Shift+D</code>: Split active pane vertically.</li>\n  <li><code>Ctrl+Shift+E</code>: Split active pane horizontally.</li>\n  <li><code>Ctrl+Shift+Z</code>: Toggle zoom on active pane.</li>\n  <li><code>Ctrl+Shift+W</code>: Close current pane.</li>\n</ul>\n\n<h2 id=\"essential-shortcuts\">Essential Daily Shortcuts</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Shortcut</th><th>Action</th></tr></thead>\n  <tbody>\n    <tr><td><code>Ctrl+Shift+T</code></td><td>Open new terminal tab</td></tr>\n    <tr><td><code>Ctrl+Shift+F</code></td><td>Find in terminal scrollback</td></tr>\n    <tr><td><code>Ctrl+Shift+C</code> / <code>Ctrl+Shift+V</code></td><td>Copy / Paste</td></tr>\n    <tr><td><code>Ctrl+L</code></td><td>Clear screen preserving scrollback</td></tr>\n  </tbody>\n</table>"
  },
  "first-run": {
    "id": "first-run",
    "title": "First Run",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "paths",
        "text": "Default Directories & Paths",
        "level": 2
      },
      {
        "id": "initial-setup",
        "text": "Automatic Initialization",
        "level": 2
      },
      {
        "id": "env-setup",
        "text": "Environment Verification",
        "level": 2
      },
      {
        "id": "first-run-checklist",
        "text": "First-Run Checklist",
        "level": 2
      }
    ],
    "summary": "Initial configuration directories, assets, and PATH setup.",
    "body": "<p>Upon initial launch, Meridian automatically initializes user configuration files and assets in standard XDG locations.</p>\n\n<h2 id=\"paths\">Default Directories & Paths</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Path</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>~/.config/meridian/config.json</code></td><td>Main terminal settings, font preferences, and theme configuration.</td></tr>\n    <tr><td><code>~/.config/meridian/keybindings.json</code></td><td>Custom keyboard shortcut overrides.</td></tr>\n    <tr><td><code>~/.config/meridian/gallery/</code></td><td>Embedded high-resolution anime artwork wallpaper assets.</td></tr>\n    <tr><td><code>~/.config/meridian/plugins/</code></td><td>Directory for user-installed extensible Meridian plugins.</td></tr>\n    <tr><td><code>~/.local/share/meridian/history.db</code></td><td>Rich command history SQLite database with durations and exit codes.</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"initial-setup\">Automatic Initialization</h2>\n<p>If <code>~/.config/meridian/config.json</code> does not exist, Meridian populates it with standard defaults:</p>\n<pre><code class=\"language-json\">{\n  \"theme\": \"sharingan_eye\",\n  \"random_theme\": false,\n  \"font_family\": \"FantasqueSansMNFM\",\n  \"font_size\": 14,\n  \"cursor_shape\": \"block\",\n  \"cursor_blink\": true,\n  \"scrollback_lines\": 10000,\n  \"opacity\": 0.95,\n  \"blur\": true\n}</code></pre>\n\n<h2 id=\"env-setup\">Environment Verification</h2>\n<pre><code class=\"language-bash\"># Verify Meridian version\nmeridian --version\n\n# Verify truecolor terminal support\necho \"TERM=$TERM COLORTERM=$COLORTERM\"</code></pre>\n\n<h2 id=\"first-run-checklist\">First-Run Checklist</h2>\n<ul>\n  <li>Ensure your shell defaults to <code>/bin/bash</code>, <code>/bin/zsh</code>, or <code>/usr/bin/fish</code>.</li>\n  <li>Install a Nerd Font (e.g., Fantasque Sans Mono or JetBrains Mono) for full Powerline icon rendering.</li>\n  <li>Test direct raster image decoding by running <code>pic ~/.config/meridian/gallery/sharingan_eye.png</code>.</li>\n</ul>"
  },
  "getting-started-config": {
    "id": "getting-started-config",
    "title": "Configuration",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "overview",
        "text": "Configuration Architecture",
        "level": 2
      },
      {
        "id": "live-reload",
        "text": "Live Configuration Reloading",
        "level": 2
      },
      {
        "id": "sample",
        "text": "Comprehensive config.json Specification",
        "level": 2
      },
      {
        "id": "options-table",
        "text": "Key Configuration Properties",
        "level": 2
      }
    ],
    "summary": "Overview of configuration options, live reload, and default values.",
    "body": "<h2 id=\"overview\">Configuration Architecture</h2>\n<p>Meridian uses standard JSON for configuration with schema validation. Settings are divided into core terminal emulation, appearance, keybindings, and developer tools.</p>\n\n<h2 id=\"live-reload\">Live Configuration Reloading</h2>\n<p>Changes saved to <code>~/.config/meridian/config.json</code> are watched using inotify and reloaded instantly without restarting your active terminal sessions or dropping PTY connections.</p>\n\n<h2 id=\"sample\">Comprehensive config.json Specification</h2>\n<pre><code class=\"language-json\">{\n  \"theme\": \"sharingan_eye\",\n  \"random_theme\": false,\n  \"font_family\": \"FantasqueSansMNFM\",\n  \"font_size\": 14,\n  \"line_height\": 1.2,\n  \"cursor_shape\": \"block\",\n  \"cursor_blink\": true,\n  \"scrollback_lines\": 10000,\n  \"window_padding\": { \"top\": 8, \"bottom\": 8, \"left\": 10, \"right\": 10 },\n  \"opacity\": 0.95,\n  \"blur\": true,\n  \"ai_enabled\": true,\n  \"ai_auto_suggest\": true,\n  \"telemetry_hud\": false\n}</code></pre>\n\n<h2 id=\"options-table\">Key Configuration Properties</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Property</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>theme</code></td><td>string</td><td><code>\"sharingan_eye\"</code></td><td>Default anime wallpaper theme.</td></tr>\n    <tr><td><code>random_theme</code></td><td>boolean</td><td><code>false</code></td><td>Rotate anime wallpaper on startup.</td></tr>\n    <tr><td><code>font_family</code></td><td>string</td><td><code>\"FantasqueSansMNFM\"</code></td><td>Primary font family name.</td></tr>\n    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Font size in points.</td></tr>\n    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Lines retained in history deque.</td></tr>\n    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity.</td></tr>\n    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>\n  </tbody>\n</table>"
  },
  "terminal-emulation": {
    "id": "terminal-emulation",
    "title": "Terminal Emulation",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "ansi-vt",
        "text": "ANSI & VT Escape Sequences",
        "level": 2
      },
      {
        "id": "alt-screen",
        "text": "Alternate Screen Buffer (DECSET 1049)",
        "level": 2
      },
      {
        "id": "truecolor",
        "text": "24-Bit TrueColor RGB (SGR 38/48)",
        "level": 2
      },
      {
        "id": "cursor-modes",
        "text": "Cursor Addressing & Terminal Modes",
        "level": 2
      },
      {
        "id": "osc-protocols",
        "text": "Operating System Commands (OSC 7/8/52/133)",
        "level": 2
      }
    ],
    "summary": "VT100, XTerm, ANSI escapes, alternate screen buffers, and 24-bit TrueColor.",
    "body": "<p>Meridian features a high-performance, standard-compliant VT100, VT220, and XTerm terminal emulation engine written in modern C++20.</p>\n\n<h2 id=\"ansi-vt\">ANSI & VT Escape Sequences</h2>\n<p>The parser handles standard CSI (Control Sequence Introducer), OSC (Operating System Command), APC (Application Program Command), and DCS (Device Control String) sequences:</p>\n<table class=\"doc-table\">\n  <thead><tr><th>Sequence</th><th>Name</th><th>Function</th></tr></thead>\n  <tbody>\n    <tr><td><code>\\033[H</code> / <code>\\033[{r};{c}H</code></td><td>CUP</td><td>Move cursor to row {r}, column {c} (1-indexed).</td></tr>\n    <tr><td><code>\\033[2J</code></td><td>ED</td><td>Erase entire display.</td></tr>\n    <tr><td><code>\\033[2K</code></td><td>EL</td><td>Erase entire active line.</td></tr>\n    <tr><td><code>\\033[?1049h</code></td><td>DECSET</td><td>Switch to alternate screen buffer.</td></tr>\n    <tr><td><code>\\033[?1049l</code></td><td>DECRST</td><td>Restore primary screen buffer.</td></tr>\n    <tr><td><code>\\033[?25h</code> / <code>\\033[?25l</code></td><td>DECTCEM</td><td>Show / hide text cursor.</td></tr>\n    <tr><td><code>\\033[{top};{bot}r</code></td><td>DECSTBM</td><td>Set scrolling margins (top to bottom).</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"alt-screen\">Alternate Screen Buffer (DECSET 1049)</h2>\n<p>Full-screen interactive applications like <code>vim</code>, <code>nano</code>, <code>tmux</code>, and <code>htop</code> switch to the alternate buffer on launch and restore the primary buffer on exit, preserving command prompt history.</p>\n\n<h2 id=\"truecolor\">24-Bit TrueColor RGB (SGR 38/48)</h2>\n<p>Meridian renders full 24-bit RGB colors with 16.7 million distinct hues via standard SGR sequences:</p>\n<pre><code class=\"language-bash\"># Set foreground color to Meridian Cyan (#00E5FF)\nprintf \"\\033[38;2;0;229;255mMeridian TrueColor\\033[0m\\n\"\n\n# Set background color to Dark Panel (#0E1622)\nprintf \"\\033[48;2;14;22;34m\\033[38;2;0;229;255m Custom Panel \\033[0m\\n\"</code></pre>\n\n<h2 id=\"cursor-modes\">Cursor Addressing & Terminal Modes</h2>\n<p>Supports block, beam, and underline cursor shapes, cursor save/restore (<code>ESC 7</code> / <code>ESC 8</code>), and bracketed paste mode (<code>\\033[?2004h</code>).</p>\n\n<h2 id=\"osc-protocols\">Operating System Commands (OSC 7/8/52/133)</h2>\n<ul>\n  <li><strong>OSC 7:</strong> Real-time current working directory tracking for tab titles.</li>\n  <li><strong>OSC 8:</strong> Clickable terminal hyperlinks with embedded URIs.</li>\n  <li><strong>OSC 52:</strong> Base64 remote clipboard synchronization.</li>\n  <li><strong>OSC 133:</strong> Semantic shell integration prompt markers (FTCS).</li>\n</ul>"
  },
  "terminal-shell": {
    "id": "terminal-shell",
    "title": "Shell",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "ast-engine",
        "text": "POSIX AST Parser & Execution Engine",
        "level": 2
      },
      {
        "id": "pipelines-redirections",
        "text": "Pipelines & File Redirections",
        "level": 2
      },
      {
        "id": "command-substitution",
        "text": "Command Substitution & Expansion",
        "level": 2
      },
      {
        "id": "job-control",
        "text": "Job Control & Background Processes",
        "level": 2
      },
      {
        "id": "signal-forwarding",
        "text": "Process Group Signals & Terminal Ownership",
        "level": 2
      }
    ],
    "summary": "Standalone POSIX-compliant AST execution engine, pipelines, and job control.",
    "body": "<p>Meridian includes an internal standalone POSIX shell engine (<code>meridian-shell</code>) with recursive-descent parsing, AST execution, and full job control.</p>\n\n<h2 id=\"ast-engine\">POSIX AST Parser & Execution Engine</h2>\n<p>Input strings are tokenized by <code>shell::Lexer</code> and structured into an Abstract Syntax Tree by <code>shell::Parser</code>:</p>\n<pre><code class=\"language-text\">Input: git status && cargo build --release | tee build.log\n\nAST Structure:\n  LogicalAndNode\n    ├── CommandNode: \"git\", [\"status\"]\n    └── PipelineNode\n          ├── CommandNode: \"cargo\", [\"build\", \"--release\"]\n          └── CommandNode: \"tee\", [\"build.log\"]</code></pre>\n\n<h2 id=\"pipelines-redirections\">Pipelines & File Redirections</h2>\n<pre><code class=\"language-bash\"># Standard pipeline with stdout/stderr redirection\ncat app.log | grep -i \"error\" | sort | uniq -c > errors.txt 2>&1\n\n# Appending output\necho \"build completed at $(date)\" >> /tmp/meridian.log</code></pre>\n\n<h2 id=\"command-substitution\">Command Substitution & Expansion</h2>\n<p>Supports <code>$(command)</code> substitution, environment variable expansion (<code>$VAR</code>, <code>${VAR}</code>), and exit code inspection (<code>$?</code>).</p>\n\n<h2 id=\"job-control\">Job Control & Background Processes</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Command / Key</th><th>Action</th></tr></thead>\n  <tbody>\n    <tr><td><code>command &</code></td><td>Launch process in background.</td></tr>\n    <tr><td><code>Ctrl+Z</code></td><td>Send <code>SIGTSTP</code> to suspend foreground process.</td></tr>\n    <tr><td><code>jobs</code></td><td>List active background and suspended jobs.</td></tr>\n    <tr><td><code>fg [%id]</code></td><td>Bring background job to foreground.</td></tr>\n    <tr><td><code>bg [%id]</code></td><td>Resume suspended job in background.</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"signal-forwarding\">Process Group Signals & Terminal Ownership</h2>\n<p>When running commands, <code>meridian-shell</code> creates dedicated process groups (<code>setpgid()</code>) and gives controlling terminal ownership via <code>tcsetpgrp()</code> so signals like <code>SIGINT</code> (Ctrl+C) and <code>SIGQUIT</code> (Ctrl+\\) cleanly terminate the foreground pipeline without killing the parent shell.</p>"
  },
  "terminal-pty": {
    "id": "terminal-pty",
    "title": "PTY",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "openpty-arch",
        "text": "Linux openpty Master/Slave Architecture",
        "level": 2
      },
      {
        "id": "async-io",
        "text": "Non-Blocking Asynchronous I/O Loop",
        "level": 2
      },
      {
        "id": "signal-handling",
        "text": "Signal Handling & Resizing (SIGWINCH)",
        "level": 2
      },
      {
        "id": "termios-config",
        "text": "Termios Line Discipline & Raw Mode",
        "level": 2
      }
    ],
    "summary": "Asynchronous Linux openpty pseudoterminal multiplexer with non-blocking I/O.",
    "body": "<p>Meridian uses POSIX <code>openpty(3)</code> to establish real Linux pseudoterminal sessions, decoupling GUI rendering from shell process execution.</p>\n\n<h2 id=\"openpty-arch\">Linux openpty Master/Slave Architecture</h2>\n<div class=\"arch-diagram-block\"><pre><code class=\"language-text\">Meridian UI / Render Canvas\n        │ (read / write)\n        ▼\n   Master PTY FD\n═════════════════════════════ Linux Kernel PTY Driver\n   Slave PTY FD (/dev/pts/X)\n        │\n   Forked Process (bash / zsh / ssh / vim)</code></pre></div>\n\n<h2 id=\"async-io\">Non-Blocking Asynchronous I/O Loop</h2>\n<p><code>pty::PTYManager</code> manages master file descriptors using non-blocking I/O (<code>O_NONBLOCK</code>) and <code>epoll</code> polling, ensuring high throughput and zero UI stutter during high-volume output (e.g. <code>find /</code> or large compile logs).</p>\n\n<h2 id=\"signal-handling\">Signal Handling & Resizing (SIGWINCH)</h2>\n<p>When the terminal window or split pane is resized, Meridian calculates the new row and column count and invokes <code>ioctl(master_fd, TIOCSWINSZ, &ws)</code>, instantly sending <code>SIGWINCH</code> to the foreground process group.</p>\n\n<h2 id=\"termios-config\">Termios Line Discipline & Raw Mode</h2>\n<p>Meridian configures the slave PTY with <code>termios</code> flags supporting both cooked mode for standard readline shells and raw mode for screen-oriented curses programs like <code>vim</code> and <code>micro</code>.</p>"
  },
  "terminal-commands": {
    "id": "terminal-commands",
    "title": "Commands",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "cli-subcommands",
        "text": "CLI Subcommands Index",
        "level": 2
      },
      {
        "id": "shell-builtins",
        "text": "Shell Built-in Commands",
        "level": 2
      },
      {
        "id": "developer-commands",
        "text": "Developer Productivity Subcommands",
        "level": 2
      }
    ],
    "summary": "Meridian CLI commands, builtins, and subcommands.",
    "body": "<h2 id=\"cli-subcommands\">CLI Subcommands Index</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Command</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>meridian</code></td><td>Launch interactive terminal emulator GUI.</td></tr>\n    <tr><td><code>meridian-shell</code></td><td>Launch standalone interactive AST shell.</td></tr>\n    <tr><td><code>meridian monitor</code></td><td>Open live CPU, RAM, Disk, Network, and Process metrics dashboard.</td></tr>\n    <tr><td><code>meridian git</code></td><td>Inspect Git branch divergence, staged/unstaged changes.</td></tr>\n    <tr><td><code>meridian files [dir]</code></td><td>View interactive directory tree explorer with Git badges.</td></tr>\n    <tr><td><code>meridian ssh [alias]</code></td><td>Manage and connect to SSH remote workspaces.</td></tr>\n    <tr><td><code>meridian plugins</code></td><td>List active extensible plugins and lifecycle hooks.</td></tr>\n    <tr><td><code>meridian --performance</code></td><td>Display GPU framerate, PTY latency & telemetry profiler.</td></tr>\n    <tr><td><code>meridian pic &lt;file&gt;</code></td><td>Display direct 32-bit RGBA inline image.</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"shell-builtins\">Shell Built-in Commands</h2>\n<p>Inside <code>meridian-shell</code>, built-in commands run directly inside the process without forking:</p>\n<pre><code class=\"language-bash\">cd /var/log         # Change current working directory\nexport FOO=\"bar\"    # Set environment variable\njobs                # List active background jobs\nfg %1               # Foreground job 1\nbg %1               # Background job 1\nhistory             # View rich command history\nexit 0              # Terminate shell session</code></pre>\n\n<h2 id=\"developer-commands\">Developer Productivity Subcommands</h2>\n<p>Developer subcommands can be executed from within any shell or invoked directly from your system PATH:</p>\n<pre><code class=\"language-bash\"># Launch live resource monitor\nmeridian monitor\n\n# Show visual Git status\nmeridian git\n\n# Open interactive directory browser\nmeridian files /var/log</code></pre>"
  },
  "terminal-ssh": {
    "id": "terminal-ssh",
    "title": "SSH",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "ssh-mgr",
        "text": "Native SSH Workspace Manager",
        "level": 2
      },
      {
        "id": "ssh-config-parser",
        "text": "~/.ssh/config Automatic Parsing",
        "level": 2
      },
      {
        "id": "remote-compat",
        "text": "Remote Terminal Compatibility & Protocols",
        "level": 2
      },
      {
        "id": "ssh-security",
        "text": "Key Management & Security",
        "level": 2
      }
    ],
    "summary": "SSH workspace manager, connection profiles, and remote session management.",
    "body": "<p>Meridian includes a built-in SSH workspace and connection manager in <code>src/dev/ssh_manager.cpp</code>.</p>\n\n<h2 id=\"ssh-mgr\">Native SSH Workspace Manager</h2>\n<pre><code class=\"language-bash\"># List all available SSH hosts from ~/.ssh/config\nmeridian ssh\n\n# Connect directly to a configured host alias\nmeridian ssh production\n\n# Custom port or identity file connection\nmeridian ssh staging -p 2222</code></pre>\n\n<h2 id=\"ssh-config-parser\">~/.ssh/config Automatic Parsing</h2>\n<p>Meridian automatically parses <code>Host</code>, <code>HostName</code>, <code>User</code>, <code>Port</code>, and <code>IdentityFile</code> directives from <code>~/.ssh/config</code> and presents them in the Command Palette (<code>Ctrl+Shift+P</code>).</p>\n\n<h2 id=\"remote-compat\">Remote Terminal Compatibility & Protocols</h2>\n<p>Over remote SSH sessions, Meridian fully supports:</p>\n<ul>\n  <li><strong>OSC 52:</strong> Seamless remote clipboard synchronization back to your local clipboard.</li>\n  <li><strong>OSC 7:</strong> Remote current working directory reporting for tab titles.</li>\n  <li><strong>SIGWINCH:</strong> Window resize propagation across SSH channels.</li>\n  <li><strong>24-bit TrueColor:</strong> Uncompromised color output for remote vim and tmux.</li>\n</ul>\n\n<h2 id=\"ssh-security\">Key Management & Security</h2>\n<p>Meridian integrates with <code>ssh-agent</code> and respects encrypted private keys, passing terminal passphrase prompts seamlessly through the PTY layer without leaking credentials into logs.</p>"
  },
  "terminal-keybindings": {
    "id": "terminal-keybindings",
    "title": "Keybindings",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "shortcuts-table",
        "text": "Default Keyboard Shortcuts Table",
        "level": 2
      },
      {
        "id": "customizing-keys",
        "text": "Custom Keymap Configuration",
        "level": 2
      },
      {
        "id": "modifiers-syntax",
        "text": "Modifier Key Syntax",
        "level": 2
      }
    ],
    "summary": "Default key shortcuts for tabs, panes, theme switcher, and command palette.",
    "body": "<h2 id=\"shortcuts-table\">Default Keyboard Shortcuts Table</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Keybinding</th><th>Action</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>Ctrl+P</code></td><td>Anime Theme Gallery</td><td>Open theme gallery and wallpaper switcher.</td></tr>\n    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Open fuzzy-search command palette.</td></tr>\n    <tr><td><code>Ctrl+Shift+T</code></td><td>New Tab</td><td>Create a new terminal tab in current working directory.</td></tr>\n    <tr><td><code>Ctrl+Shift+W</code></td><td>Close Tab / Pane</td><td>Close the active tab or split pane.</td></tr>\n    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Split active window vertically into two panes.</td></tr>\n    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Split active window horizontally into two panes.</td></tr>\n    <tr><td><code>Ctrl+Shift+Z</code></td><td>Toggle Zoom</td><td>Maximize active pane to fill entire window.</td></tr>\n    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search scrollback buffer and command history.</td></tr>\n    <tr><td><code>Ctrl+Shift+C</code></td><td>Copy</td><td>Copy selected text to clipboard.</td></tr>\n    <tr><td><code>Ctrl+Shift+V</code></td><td>Paste</td><td>Paste clipboard contents into terminal.</td></tr>\n    <tr><td><code>Ctrl+L</code></td><td>Clear Screen</td><td>Clear terminal grid while preserving scrollback.</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"customizing-keys\">Custom Keymap Configuration</h2>\n<p>Override keybindings in <code>~/.config/meridian/keybindings.json</code>:</p>\n<pre><code class=\"language-json\">{\n  \"split_vertical\": \"Ctrl+Shift+V\",\n  \"split_horizontal\": \"Ctrl+Shift+H\",\n  \"command_palette\": \"Ctrl+Space\",\n  \"new_tab\": \"Ctrl+T\"\n}</code></pre>\n\n<h2 id=\"modifiers-syntax\">Modifier Key Syntax</h2>\n<p>Use standard modifier names: <code>Ctrl</code>, <code>Shift</code>, <code>Alt</code>, <code>Meta</code> / <code>Super</code> combined with <code>+</code>.</p>"
  },
  "terminal-clipboard": {
    "id": "terminal-clipboard",
    "title": "Clipboard",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "clipboard-sync",
        "text": "System Clipboard Integration",
        "level": 2
      },
      {
        "id": "osc52-spec",
        "text": "OSC 52 Base64 Remote Clipboard Protocol",
        "level": 2
      },
      {
        "id": "bracketed-paste",
        "text": "Bracketed Paste Mode",
        "level": 2
      },
      {
        "id": "clipboard-security",
        "text": "Clipboard Security & Secret Redaction",
        "level": 2
      }
    ],
    "summary": "OSC 52 remote clipboard synchronization, bracketed paste, and system clipboard.",
    "body": "<h2 id=\"clipboard-sync\">System Clipboard Integration</h2>\n<p>Meridian integrates with system clipboards across Wayland (<code>wl-clipboard</code>), X11 (<code>xclip</code>/<code>xsel</code>), macOS (<code>pbcopy</code>), and Windows.</p>\n\n<h2 id=\"osc52-spec\">OSC 52 Base64 Remote Clipboard Protocol</h2>\n<p>Meridian natively parses OSC 52 sequences (<code>\\033]52;c;&lt;base64&gt;\\033\\\\</code>), allowing CLI tools running on remote SSH servers (e.g. <code>tmux</code>, <code>vim</code>, <code>osc52.sh</code>) to copy text directly into your local machine's clipboard without X11 forwarding.</p>\n\n<pre><code class=\"language-bash\"># Example: Copy string to local clipboard over remote SSH\nprintf \"\\033]52;c;%s\\033\\\\\" \"$(echo -n \"Hello from Remote Server\" | base64)\"</code></pre>\n\n<h2 id=\"bracketed-paste\">Bracketed Paste Mode</h2>\n<p>When enabled by interactive programs (<code>\\033[?2004h</code>), pasted text is enclosed in <code>\\033[200~</code> and <code>\\033[201~</code> markers, preventing accidental execution of newline-separated commands.</p>\n\n<h2 id=\"clipboard-security\">Clipboard Security & Secret Redaction</h2>\n<p>Meridian includes an optional clipboard guard that warns the user if pasted content contains sensitive tokens such as private SSH keys or AWS secret access keys.</p>"
  },
  "terminal-scrollback": {
    "id": "terminal-scrollback",
    "title": "Scrollback",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "scrollback-arch",
        "text": "Scrollback Deque Architecture",
        "level": 2
      },
      {
        "id": "search-engine",
        "text": "Scrollback Search & Highlighting",
        "level": 2
      },
      {
        "id": "memory-management",
        "text": "Memory Limits & Performance Tuning",
        "level": 2
      }
    ],
    "summary": "High-capacity scrollback buffer, search highlighting, and memory limits.",
    "body": "<h2 id=\"scrollback-arch\">Scrollback Deque Architecture</h2>\n<p><code>vt::ScreenBuffer</code> maintains an efficient circular deque for scrollback history with a configurable limit (default: 10,000 lines). When lines scroll off the top of the grid, they are pushed into the scrollback pool, preserving ANSI colors and hyperlink attributes.</p>\n\n<h2 id=\"search-engine\">Scrollback Search & Highlighting</h2>\n<p>Pressing <code>Ctrl+Shift+F</code> activates the incremental search engine with real-time match highlighting, regex support, and case-sensitivity toggles.</p>\n\n<h2 id=\"memory-management\">Memory Limits & Performance Tuning</h2>\n<p>Each line in the scrollback buffer is stored with sparse cell arrays, consuming less than 20MB of RAM per 10,000 lines of colored output.</p>\n<pre><code class=\"language-json\">{\n  \"scrollback_lines\": 50000\n}</code></pre>"
  },
  "graphics-gpu": {
    "id": "graphics-gpu",
    "title": "GPU Renderer",
    "category": "GRAPHICS",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "gpu-pipeline",
        "text": "GPU Render Pipeline Architecture",
        "level": 2
      },
      {
        "id": "damage-tracking",
        "text": "Damage Rect Tracking Engine",
        "level": 2
      },
      {
        "id": "glyph-atlas-engine",
        "text": "Sub-Pixel Glyph Texture Atlas",
        "level": 2
      },
      {
        "id": "instancing-dev",
        "text": "Instanced Quad Rendering",
        "level": 2
      }
    ],
    "summary": "Hardware-accelerated shader pipeline, damage tracking, and glyph texture atlas.",
    "body": "<h2 id=\"gpu-pipeline\">GPU Render Pipeline Architecture</h2>\n<p>Meridian's GPU renderer transforms terminal cell matrices into instanced draw calls executed on OpenGL 3.3 Core and Vulkan backends. By offloading text glyph blitting, background colors, and raster images to dedicated shader pipelines, Meridian sustains fluid 144Hz+ rendering without consuming significant CPU resources.</p>\n\n<div class=\"arch-diagram-block\"><pre><code class=\"language-text\">ScreenBuffer Grid Cells\n        │ (dirty lines)\n        ▼\n   DamageTracker (Calculates Minimal Bounding Box)\n        │\n   GlyphAtlas (FreeType / HarfBuzz Sub-Pixel Cache)\n        │\n   Instanced Quad VBO (Positions, UVs, RGB Fg/Bg)\n        │\n   GPU Shaders (OpenGL 3.3 Core / Vulkan) ──► Display VSync (144 FPS)</code></pre></div>\n\n<h2 id=\"damage-tracking\">Damage Rect Tracking Engine</h2>\n<p><code>renderer::DamageTracker</code> records bounding boxes of modified cells per frame. Frames with partial updates only re-render dirty regions, saving GPU cycles and laptop battery power.</p>\n\n<h2 id=\"glyph-atlas-engine\">Sub-Pixel Glyph Texture Atlas</h2>\n<p><code>renderer::GlyphAtlas</code> rasterizes character glyphs with sub-pixel anti-aliasing into a high-density 2048x2048 texture atlas, achieving sub-millisecond glyph retrieval.</p>\n\n<h2 id=\"instancing-dev\">Instanced Quad Rendering</h2>\n<p>Rather than issuing individual draw calls for each character cell, Meridian batches up to 20,000 glyphs into a single instanced quad array buffer, achieving ultra-low render latency under 1.2ms per frame.</p>"
  },
  "graphics-inline-images": {
    "id": "graphics-inline-images",
    "title": "Inline Images",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "pic-command",
        "text": "The pic Command",
        "level": 2
      },
      {
        "id": "rendering-pipeline",
        "text": "Rendering Pipeline",
        "level": 2
      },
      {
        "id": "image-formats",
        "text": "Image Formats",
        "level": 2
      },
      {
        "id": "configuration",
        "text": "Configuration",
        "level": 2
      },
      {
        "id": "examples",
        "text": "Examples",
        "level": 2
      },
      {
        "id": "performance",
        "text": "Performance",
        "level": 2
      },
      {
        "id": "troubleshooting",
        "text": "Troubleshooting",
        "level": 2
      }
    ],
    "summary": "Display real raster images directly inside Meridian Shell without converting them into ASCII characters or Unicode blocks. Meridian Shell's graphics system allows images to be rendered at full color and resolution using GPU acceleration.",
    "body": "<h2 id=\"pic-command\">The pic Command</h2>\n<p>The <code>pic</code> command displays an image file directly inside the terminal.</p>\n\n<pre><code class=\"language-bash\">pic image.png\n# Display the original raster image</code></pre>\n\n<div class=\"claude-callout-box\">\n  <div class=\"callout-header\">\n    <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#00A8B5\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"12\" x2=\"12\" y1=\"8\" y2=\"12\"/><line x1=\"12\" x2=\"12.01\" y1=\"16\" y2=\"16\"/></svg>\n    <span class=\"callout-title\">Important</span>\n  </div>\n  <p>Meridian Shell renders images as true raster graphics. Images are not converted into ASCII art, Unicode blocks, or other character-based representations.</p>\n</div>\n\n<h2 id=\"rendering-pipeline\">Rendering Pipeline</h2>\n<p>Images follow a high-performance rendering pipeline designed for clarity and speed.</p>\n\n<div class=\"pipeline-diagram-wrapper\">\n  <div class=\"pipeline-step\">\n    <div class=\"step-card\">\n      <svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#00A8B5\" stroke-width=\"1.8\"><path d=\"M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z\"/><polyline points=\"14 2 14 8 20 8\"/></svg>\n      <div class=\"step-title\">Image File</div>\n      <div class=\"step-desc\">PNG / JPG / WebP</div>\n    </div>\n  </div>\n  <div class=\"pipeline-arrow\">→</div>\n  <div class=\"pipeline-step\">\n    <div class=\"step-card\">\n      <svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#00A8B5\" stroke-width=\"1.8\"><circle cx=\"12\" cy=\"12\" r=\"3\"/><path d=\"M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z\"/></svg>\n      <div class=\"step-title\">Image Decoder</div>\n      <div class=\"step-desc\">Decode & Validate</div>\n    </div>\n  </div>\n  <div class=\"pipeline-arrow\">→</div>\n  <div class=\"pipeline-step\">\n    <div class=\"step-card\">\n      <svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#00A8B5\" stroke-width=\"1.8\"><rect width=\"20\" height=\"14\" x=\"2\" y=\"3\" rx=\"2\"/><line x1=\"8\" x2=\"16\" y1=\"21\" y2=\"21\"/><line x1=\"12\" x2=\"12\" y1=\"17\" y2=\"21\"/></svg>\n      <div class=\"step-title\">RGBA Pixel Buffer</div>\n      <div class=\"step-desc\">Raw Pixel Data</div>\n    </div>\n  </div>\n  <div class=\"pipeline-arrow\">→</div>\n  <div class=\"pipeline-step\">\n    <div class=\"step-card\">\n      <svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#00A8B5\" stroke-width=\"1.8\"><ellipse cx=\"12\" cy=\"5\" rx=\"9\" ry=\"3\"/><path d=\"M3 5v14a9 3 0 0 0 18 0V5\"/><path d=\"M3 12a9 3 0 0 0 18 0\"/></svg>\n      <div class=\"step-title\">GPU Texture</div>\n      <div class=\"step-desc\">Upload to GPU</div>\n    </div>\n  </div>\n  <div class=\"pipeline-arrow\">→</div>\n  <div class=\"pipeline-step\">\n    <div class=\"step-card\">\n      <svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#00A8B5\" stroke-width=\"1.8\"><rect width=\"18\" height=\"18\" x=\"3\" y=\"3\" rx=\"2\"/><path d=\"M3 9h18\"/><path d=\"M9 21V9\"/></svg>\n      <div class=\"step-title\">Graphics Layer</div>\n      <div class=\"step-desc\">Terminal Canvas</div>\n    </div>\n  </div>\n  <div class=\"pipeline-arrow\">→</div>\n  <div class=\"pipeline-step\">\n    <div class=\"step-card\">\n      <svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#00A8B5\" stroke-width=\"1.8\"><rect width=\"18\" height=\"18\" x=\"3\" y=\"3\" rx=\"2\"/><circle cx=\"9\" cy=\"9\" r=\"2\"/><path d=\"m21 15-3.086-3.086a2 2 0 0 0-2.828 0L6 21\"/></svg>\n      <div class=\"step-title\">Rendered Image</div>\n      <div class=\"step-desc\">On Screen</div>\n    </div>\n  </div>\n</div>\n\n<p>This ensures smooth rendering, proper scaling, and efficient memory usage.</p>\n\n<h2 id=\"image-formats\">Image Formats</h2>\n<p>Meridian natively parses standard compressed raster files using <code>graphics::ImageDecoder</code>:</p>\n<table class=\"doc-table\">\n  <thead><tr><th>Format</th><th>Status</th><th>Channels</th><th>Notes</th></tr></thead>\n  <tbody>\n    <tr><td>PNG</td><td>Supported</td><td>RGB / RGBA</td><td>Full alpha transparency channel preserved.</td></tr>\n    <tr><td>JPEG / JPG</td><td>Supported</td><td>RGB</td><td>Hardware accelerated baseline and progressive decoding.</td></tr>\n    <tr><td>WebP</td><td>Supported</td><td>RGB / RGBA</td><td>Lossy and lossless WebP containers.</td></tr>\n    <tr><td>GIF</td><td>Supported</td><td>RGB / RGBA</td><td>Single frame and animated GIF playback.</td></tr>\n    <tr><td>BMP</td><td>Supported</td><td>RGB / RGBA</td><td>Uncompressed direct raster bit arrays.</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"configuration\">Configuration</h2>\n<p>Adjust max texture sizes and caching behavior in <code>~/.config/meridian/config.json</code>:</p>\n<pre><code class=\"language-json\">{\n  \"graphics\": {\n    \"max_inline_width\": 1920,\n    \"max_inline_height\": 1080,\n    \"cache_limit_mb\": 256,\n    \"gpu_acceleration\": true\n  }\n}</code></pre>\n\n<h2 id=\"examples\">Examples</h2>\n<pre><code class=\"language-bash\"># View image at original aspect ratio\npic diagram.png\n\n# Scale to specific width maintaining height ratio\npic screenshot.png --width 450\n\n# Inspect decoded raster metadata & GPU texture specifications\npic --debug preview.webp\n\n# Wipe all active graphics from the terminal canvas\npic --clear</code></pre>\n\n<h2 id=\"performance\">Performance</h2>\n<p>Because decoded raster buffers are uploaded directly to GPU textures, scrolling through terminal output with rendered images causes zero CPU downsampling penalties and maintains a steady 144 FPS.</p>\n\n<h2 id=\"troubleshooting\">Troubleshooting</h2>\n<ul>\n  <li><strong>Image not found:</strong> Verify the absolute or relative file path.</li>\n  <li><strong>GPU Texture Allocation Error:</strong> Ensure OpenGL 3.3 Core or Vulkan drivers are active (<code>meridian --performance</code>).</li>\n  <li><strong>Unsupported File:</strong> Convert esoteric vector formats (SVG) or RAW camera files to standard PNG/JPEG.</li>\n</ul>"
  },
  "graphics-formats": {
    "id": "graphics-formats",
    "title": "Image Formats",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "format-matrix",
        "text": "Supported Image Formats Matrix",
        "level": 2
      },
      {
        "id": "color-channels",
        "text": "Color Channels & Alpha Blending",
        "level": 2
      },
      {
        "id": "decoding-pipeline",
        "text": "High-Performance Decoding Pipeline",
        "level": 2
      }
    ],
    "summary": "Supported image formats: PNG, JPEG, WebP, GIF, and BMP.",
    "body": "<h2 id=\"format-matrix\">Supported Image Formats Matrix</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Format</th><th>Extension</th><th>Decoding Engine</th><th>Alpha Channel</th></tr></thead>\n  <tbody>\n    <tr><td>PNG</td><td><code>.png</code></td><td>stb_image / Native</td><td>Yes (RGBA8888)</td></tr>\n    <tr><td>JPEG / JPG</td><td><code>.jpg</code>, <code>.jpeg</code></td><td>stb_image / Native</td><td>No (RGB888)</td></tr>\n    <tr><td>WebP</td><td><code>.webp</code></td><td>libwebp / stb_image</td><td>Yes (RGBA8888)</td></tr>\n    <tr><td>GIF</td><td><code>.gif</code></td><td>stb_image / Animated</td><td>Yes (Indexed / RGBA)</td></tr>\n    <tr><td>BMP</td><td><code>.bmp</code></td><td>stb_image / Native</td><td>Optional</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"color-channels\">Color Channels & Alpha Blending</h2>\n<p>All decoded images are normalized to 32-bit RGBA with full premultiplied alpha compositing over terminal background wallpapers and color themes.</p>\n\n<h2 id=\"decoding-pipeline\">High-Performance Decoding Pipeline</h2>\n<p>Image loading and decompression are performed asynchronously in background worker threads, preventing large 4K / 8K image files from causing terminal frame drops or PTY input lag.</p>"
  },
  "graphics-gif": {
    "id": "graphics-gif",
    "title": "GIF",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "gif-engine",
        "text": "Animated GIF Playback Engine",
        "level": 2
      },
      {
        "id": "frame-timing",
        "text": "Frame Delay & Loop Controls",
        "level": 2
      },
      {
        "id": "memory-management-gif",
        "text": "Frame Cache & Texture Cycling",
        "level": 2
      }
    ],
    "summary": "Animated GIF playback and frame delay timing.",
    "body": "<h2 id=\"gif-engine\">Animated GIF Playback Engine</h2>\n<p><code>graphics::GraphicManager</code> decodes multi-frame GIF images into individual frame texture arrays, managing playback timers according to embedded frame delays.</p>\n\n<h2 id=\"frame-timing\">Frame Delay & Loop Controls</h2>\n<pre><code class=\"language-bash\">pic anime_reaction.gif     # Play animated GIF at native frame rate\npic animation.gif --loop 3  # Play GIF for 3 iterations then pause</code></pre>\n\n<h2 id=\"memory-management-gif\">Frame Cache & Texture Cycling</h2>\n<p>Frames are stored in contiguous GPU texture 2D arrays, cycling texture slices on timer expiration with negligible CPU impact.</p>"
  },
  "graphics-kitty": {
    "id": "graphics-kitty",
    "title": "Kitty Graphics",
    "category": "GRAPHICS",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "kitty-protocol",
        "text": "Kitty Graphics Protocol Specification",
        "level": 2
      },
      {
        "id": "chunking-safety",
        "text": "2048-Byte Safe Transmission Chunking",
        "level": 2
      },
      {
        "id": "protocol-parameters",
        "text": "Supported APC Control Keys",
        "level": 2
      }
    ],
    "summary": "Chunked 2048-byte transmission protocol with m=1/m=0 control frames.",
    "body": "<h2 id=\"kitty-protocol\">Kitty Graphics Protocol Specification</h2>\n<p>Meridian parses Kitty Graphics escape sequences (<code>\\033_G...\\033\\\\</code>) for seamless compatibility with tools like <code>icat</code>, <code>yazi</code>, and <code>ranger</code>.</p>\n\n<h2 id=\"chunking-safety\">2048-Byte Safe Transmission Chunking</h2>\n<p>Payloads exceeding 2048 bytes are transmitted in chunks using <code>m=1</code> (more data follows) and <code>m=0</code> (final payload chunk), preventing buffer overflows in PTY drivers.</p>\n\n<h2 id=\"protocol-parameters\">Supported APC Control Keys</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Key</th><th>Name</th><th>Values</th></tr></thead>\n  <tbody>\n    <tr><td><code>a</code></td><td>Action</td><td><code>t</code> (transmit & display), <code>d</code> (delete), <code>p</code> (place)</td></tr>\n    <tr><td><code>f</code></td><td>Format</td><td><code>32</code> (RGBA), <code>24</code> (RGB), <code>100</code> (PNG)</td></tr>\n    <tr><td><code>m</code></td><td>More Chunks</td><td><code>1</code> (chunk follows), <code>0</code> (final chunk)</td></tr>\n    <tr><td><code>s</code> / <code>v</code></td><td>Size</td><td>Width and Height in pixels</td></tr>\n  </tbody>\n</table>"
  },
  "graphics-sixel": {
    "id": "graphics-sixel",
    "title": "Sixel",
    "category": "GRAPHICS",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "sixel-spec",
        "text": "DEC Sixel Graphics Decoding",
        "level": 2
      },
      {
        "id": "gnuplot-compat",
        "text": "Compatibility with Gnuplot & CLI Plotting",
        "level": 2
      },
      {
        "id": "raster-conversion",
        "text": "Sixel Strip to Texture Conversion",
        "level": 2
      }
    ],
    "summary": "DEC Sixel graphics protocol decoding and 256-color palette mapping.",
    "body": "<h2 id=\"sixel-spec\">DEC Sixel Graphics Decoding</h2>\n<p>Meridian decodes DCS Sixel streams (<code>\\033Pq...\\033\\\\</code>), converting 6-pixel vertical slivers into hardware bitmap textures.</p>\n\n<h2 id=\"gnuplot-compat\">Compatibility with Gnuplot & CLI Plotting</h2>\n<pre><code class=\"language-bash\"># Render inline scientific plots directly in terminal\ngnuplot -e \"set terminal sixelgd; plot sin(x)\"</code></pre>\n\n<h2 id=\"raster-conversion\">Sixel Strip to Texture Conversion</h2>\n<p>Each Sixel character represents a 6-bit vertical slice of pixels mapped to a color register in the 256-color palette table.</p>"
  },
  "graphics-rendering": {
    "id": "graphics-rendering",
    "title": "Image Rendering",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "zero-ascii-spec",
        "text": "Zero-ASCII Engineering Guarantee",
        "level": 2
      },
      {
        "id": "comparison-table",
        "text": "Raster vs. Text Cell Approximation Comparison",
        "level": 2
      },
      {
        "id": "memory-footprint",
        "text": "VRAM Footprint & GPU Allocation",
        "level": 2
      }
    ],
    "summary": "Detailed comparison between direct raster rendering and character-cell approximations.",
    "body": "<h2 id=\"zero-ascii-spec\">Zero-ASCII Engineering Guarantee</h2>\n<p>Meridian Shell guarantees that images displayed via <code>pic</code> are rendered as genuine 32-bit hardware textures on the GPU canvas. Meridian contains zero ASCII, Unicode block, half-block, or Braille converters.</p>\n\n<h2 id=\"comparison-table\">Raster vs. Text Cell Approximation Comparison</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Feature</th><th>Meridian Direct Raster</th><th>Legacy ASCII / Half-Blocks</th></tr></thead>\n  <tbody>\n    <tr><td>Pixel Resolution</td><td>Native Image Resolution (e.g. 1920x1080)</td><td>Limited to Cell Grid (e.g. 80x24)</td></tr>\n    <tr><td>Color Accuracy</td><td>32-bit Full RGBA (16.7M colors + Alpha)</td><td>256 colors or ANSI approximations</td></tr>\n    <tr><td>Distortion</td><td>Zero aspect distortion</td><td>Extreme pixelation and character gaps</td></tr>\n    <tr><td>GPU Acceleration</td><td>Hardware texture blitting</td><td>CPU text-character generation</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"memory-footprint\">VRAM Footprint & GPU Allocation</h2>\n<p>Images are automatically uploaded to dedicated GPU textures with Mipmapping enabled for crisp downscaling and minimal VRAM consumption.</p>"
  },
  "interface-gui": {
    "id": "interface-gui",
    "title": "GUI",
    "category": "INTERFACE",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "gui-arch",
        "text": "GUI Windowing Architecture",
        "level": 2
      },
      {
        "id": "wayland-x11",
        "text": "Wayland & X11 Compositor Integration",
        "level": 2
      },
      {
        "id": "scaling-dpi",
        "text": "HiDPI & Fractional Scaling",
        "level": 2
      }
    ],
    "summary": "Qt6, Wayland, and X11 terminal canvas and window chrome.",
    "body": "<h2 id=\"gui-arch\">GUI Windowing Architecture</h2>\n<p>Meridian's GUI layer embeds the high-speed PTY engine and OpenGL/Vulkan canvas into native desktop window chrome, providing responsive rendering and native platform integration.</p>\n\n<h2 id=\"wayland-x11\">Wayland & X11 Compositor Integration</h2>\n<p>Supports fractional scaling, client-side decorations (CSD), server-side decorations (SSD), and sub-pixel glyph rendering across GNOME, KDE Plasma, and Hyprland.</p>\n\n<h2 id=\"scaling-dpi\">HiDPI & Fractional Scaling</h2>\n<p>Under Wayland <code>wp-fractional-scale-v1</code>, Meridian dynamically scales glyph texture atlases to exact fractional physical pixel dimensions (125%, 150%, 175%), completely avoiding blurry bitmap bilinear interpolation.</p>"
  },
  "interface-windows": {
    "id": "interface-windows",
    "title": "Windows",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "transparency",
        "text": "Window Transparency & Background Blur",
        "level": 2
      },
      {
        "id": "multi-window",
        "text": "Multi-Window Session Management",
        "level": 2
      },
      {
        "id": "geometry-persistence",
        "text": "Window Geometry & State Persistence",
        "level": 2
      }
    ],
    "summary": "Multi-window support, transparency, and blur effects.",
    "body": "<h2 id=\"transparency\">Window Transparency & Background Blur</h2>\n<p>Configure background opacity (<code>0.0</code> to <code>1.0</code>) and enable compositor blur in <code>~/.config/meridian/config.json</code>:</p>\n<pre><code class=\"language-json\">{\n  \"opacity\": 0.92,\n  \"blur\": true\n}</code></pre>\n\n<h2 id=\"multi-window\">Multi-Window Session Management</h2>\n<p>Press <code>Ctrl+Shift+N</code> to open a new independent window sharing the same session daemon.</p>\n\n<h2 id=\"geometry-persistence\">Window Geometry & State Persistence</h2>\n<p>Window position, dimensions, maximized state, and split layouts are automatically serialized and restored across application relaunches.</p>"
  },
  "interface-tabs": {
    "id": "interface-tabs",
    "title": "Tabs",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "tabs-workflow",
        "text": "Tabbed Workflow",
        "level": 2
      },
      {
        "id": "tabs-shortcuts-spec",
        "text": "Tab Shortcuts Reference",
        "level": 2
      },
      {
        "id": "cwd-tab-titles",
        "text": "Dynamic CWD & Process Titles",
        "level": 2
      }
    ],
    "summary": "Tabbed terminal sessions, reordering, and shortcuts.",
    "body": "<h2 id=\"tabs-workflow\">Tabbed Workflow</h2>\n<p>Tabs maintain independent PTY sessions, working directories, and scrollback histories.</p>\n\n<h2 id=\"tabs-shortcuts-spec\">Tab Shortcuts Reference</h2>\n<ul>\n  <li><code>Ctrl+Shift+T</code>: New Tab in current directory.</li>\n  <li><code>Ctrl+Shift+W</code>: Close current Tab.</li>\n  <li><code>Ctrl+PageUp</code> / <code>Ctrl+PageDown</code>: Switch between tabs sequentially.</li>\n  <li><code>Alt+1..9</code>: Jump directly to Tab N.</li>\n</ul>\n\n<h2 id=\"cwd-tab-titles\">Dynamic CWD & Process Titles</h2>\n<p>Tab headers update dynamically in response to OSC 7 directory tracking and foreground process execution (e.g., <code>vim: src/main.cpp</code> or <code>cargo: build</code>).</p>"
  },
  "interface-panes": {
    "id": "interface-panes",
    "title": "Panes",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "pane-tree-arch",
        "text": "Binary Pane Tree Architecture",
        "level": 2
      },
      {
        "id": "pane-shortcuts-spec",
        "text": "Pane Splitting & Zoom Shortcuts",
        "level": 2
      },
      {
        "id": "pane-resizing",
        "text": "Pane Resizing & Layout Persistence",
        "level": 2
      }
    ],
    "summary": "Multi-pane horizontal and vertical splits with persistent state.",
    "body": "<h2 id=\"pane-tree-arch\">Binary Pane Tree Architecture</h2>\n<p><code>workspace::PaneTree</code> models terminal splits as an n-ary tree of leaves, recalculating fractional layout rectangles on resize.</p>\n\n<h2 id=\"pane-shortcuts-spec\">Pane Splitting & Zoom Shortcuts</h2>\n<ul>\n  <li><code>Ctrl+Shift+D</code>: Split active pane vertically.</li>\n  <li><code>Ctrl+Shift+E</code>: Split active pane horizontally.</li>\n  <li><code>Ctrl+Shift+Z</code>: Toggle zoom on active pane.</li>\n  <li><code>Alt+Arrows</code>: Navigate focus between adjacent panes.</li>\n</ul>\n\n<h2 id=\"pane-resizing\">Pane Resizing & Layout Persistence</h2>\n<p>Drag pane divider borders with the mouse or use <code>Ctrl+Shift+Alt+Arrows</code> to adjust split ratios in 5% increments.</p>"
  },
  "interface-themes": {
    "id": "interface-themes",
    "title": "Themes",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "anime-gallery",
        "text": "Embedded Anime Theme Gallery",
        "level": 2
      },
      {
        "id": "theme-selection-cli",
        "text": "CLI Theme Commands (pic set)",
        "level": 2
      },
      {
        "id": "wallpaper-customization",
        "text": "Custom Wallpaper & Scaling Modes",
        "level": 2
      }
    ],
    "summary": "14 built-in anime picture themes and custom wallpaper configuration.",
    "body": "<h2 id=\"anime-gallery\">Embedded Anime Theme Gallery</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Index</th><th>Identifier</th><th>Artwork Description</th></tr></thead>\n  <tbody>\n    <tr><td>0</td><td><code>sharingan_eye</code></td><td>Sasuke / Itachi Mangekyō Sharingan Eye</td></tr>\n    <tr><td>1</td><td><code>sakura_girl</code></td><td>Sakura Blossom Anime Girl</td></tr>\n    <tr><td>2</td><td><code>ribbon_girl</code></td><td>Monochrome Anime Ribbon Girl</td></tr>\n    <tr><td>3</td><td><code>cyberpunk_edge</code></td><td>Cyberpunk Edgerunners Neon City</td></tr>\n    <tr><td>4</td><td><code>mecha_warrior</code></td><td>Gundam Mecha Orbital Warrior</td></tr>\n    <tr><td>5</td><td><code>gojo_purple</code></td><td>Gojo Satoru: Hollow Purple (JJK)</td></tr>\n    <tr><td>6</td><td><code>sukuna_shrine</code></td><td>Ryomen Sukuna: Malevolent Shrine (JJK)</td></tr>\n    <tr><td>7</td><td><code>naruto_rasengan</code></td><td>Naruto Uzumaki: Nine-Tails Rasengan</td></tr>\n    <tr><td>8</td><td><code>demon_slayer_tanjiro</code></td><td>Tanjiro Kamado: Sun Breathing</td></tr>\n    <tr><td>9</td><td><code>nezuko_blood</code></td><td>Nezuko Kamado: Blood Demon Art</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"theme-selection-cli\">CLI Theme Commands (pic set)</h2>\n<pre><code class=\"language-bash\">pic set 2               # Set Theme 2 (Ribbon Girl)\npic set gojo_purple     # Set by theme name\npic set random          # Randomize theme on each startup\npic set /path/to/my.png # Set custom user wallpaper</code></pre>\n\n<h2 id=\"wallpaper-customization\">Custom Wallpaper & Scaling Modes</h2>\n<p>Configure wallpaper placement modes in <code>config.json</code>: <code>\"fit\"</code>, <code>\"fill\"</code>, <code>\"center\"</code>, or <code>\"stretch\"</code>.</p>"
  },
  "interface-powerline": {
    "id": "interface-powerline",
    "title": "Powerline",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "context-aware-prompt",
        "text": "Intelligent Context-Aware Powerline Prompt",
        "level": 2
      },
      {
        "id": "directory-language-icons",
        "text": "Directory & Project Language Icons",
        "level": 2
      },
      {
        "id": "rich-git-segment",
        "text": "Rich Git Status & Divergence Segment",
        "level": 2
      },
      {
        "id": "ssh-and-root",
        "text": "Remote SSH & Elevated Root State",
        "level": 2
      }
    ],
    "summary": "Intelligent context-aware Powerline status prompt with directory icons, project language detection, and rich Git intelligence.",
    "body": "<h2 id=\"context-aware-prompt\">Intelligent Context-Aware Powerline Prompt</h2>\n<p>Meridian renders a compact, two-line Powerline prompt with high-contrast color segments and context-aware Nerd Font glyphs:</p>\n\n<div class=\"code-block-wrapper\">\n  <div class=\"code-header\"><span>Meridian Context-Aware Powerline Prompt</span></div>\n  <pre><code class=\"language-text\">   Tue 25 Aug - 23:43   ~/Downloads/meridian-terminal 󰙲   main ↑2 3✗ 2? \n   @charanbalaji  ❯ </code></pre>\n</div>\n\n<h2 id=\"directory-language-icons\">Directory & Project Language Icons</h2>\n<ul>\n  <li><strong>Home:</strong> <code> ~</code></li>\n  <li><strong>Downloads:</strong> <code> ~/Downloads</code></li>\n  <li><strong>Documents:</strong> <code>󰈙 ~/Documents</code></li>\n  <li><strong>Desktop:</strong> <code> ~/Desktop</code></li>\n  <li><strong>Pictures & Photos:</strong> <code> ~/Pictures</code></li>\n  <li><strong>Projects / Code:</strong> <code> ~/Projects</code></li>\n  <li><strong>C++ Project:</strong> <code>󰙲</code> (CMakeLists.txt, Makefile)</li>\n  <li><strong>Python Project:</strong> <code></code> (pyproject.toml, requirements.txt)</li>\n  <li><strong>Rust Project:</strong> <code></code> (Cargo.toml)</li>\n  <li><strong>Node / TypeScript:</strong> <code></code> (package.json)</li>\n  <li><strong>Go Project:</strong> <code></code> (go.mod)</li>\n  <li><strong>Docker Project:</strong> <code>󰡨</code> (Dockerfile, compose.yaml)</li>\n</ul>\n\n<h2 id=\"rich-git-segment\">Rich Git Status & Divergence Segment</h2>\n<p>Displays live repository divergence without running slow subprocesses on every keypress:</p>\n<ul>\n  <li><code> main ✔</code> &mdash; Clean working tree on <code>main</code></li>\n  <li><code> main ↑2</code> &mdash; 2 commits ahead of upstream remote</li>\n  <li><code> main ↓1</code> &mdash; 1 commit behind upstream remote</li>\n  <li><code> main 3✗ 2?</code> &mdash; 3 modified files, 2 untracked files</li>\n  <li><code> main ↑2 ↓1 3✗ 2?</code> &mdash; Full upstream and worktree status</li>\n</ul>\n\n<h2 id=\"ssh-and-root\">Remote SSH & Elevated Root State</h2>\n<ul>\n  <li><strong>Remote SSH:</strong> Displays purple badge <code> user@hostname </code> when connected over SSH.</li>\n  <li><strong>Elevated Root / Sudo:</strong> Dynamically detects <code>geteuid() == 0</code> and renders crimson prompt <code>⚡ root  ❯</code>.</li>\n</ul>"
  },
  "dev-architecture": {
    "id": "dev-architecture",
    "title": "Architecture",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "layers-breakdown",
        "text": "Architectural Layer Breakdown",
        "level": 2
      },
      {
        "id": "data-flow",
        "text": "End-to-End Data Flow",
        "level": 2
      },
      {
        "id": "threading-model",
        "text": "Threading & Concurrency Model",
        "level": 2
      }
    ],
    "summary": "Architecture breakdown of PTY, VT parser, ScreenBuffer, and renderer layers.",
    "body": "<h2 id=\"layers-breakdown\">Architectural Layer Breakdown</h2>\n<p>Meridian is engineered as modular C++20 subsystems designed to isolate the core terminal state machine from the windowing environment and background telemetry profilers.</p>\n\n<h2 id=\"data-flow\">End-to-End Data Flow</h2>\n<div class=\"arch-diagram-block\"><pre><code class=\"language-text\">User Keystroke ──► Window Event Loop ──► PTY Master FD ──► Kernel PTY ──► Shell Process (bash/zsh)\n                                                                                  │ (stdout/stderr)\nDisplay Frame  ◄── GPU Render Pipeline ◄── DamageTracker ◄── ScreenBuffer ◄── ANSI State Machine</code></pre></div>\n\n<h2 id=\"threading-model\">Threading & Concurrency Model</h2>\n<p>The I/O read thread continuously polls master PTY file descriptors using non-blocking epoll, buffering raw byte chunks into lock-free ring buffers before handing them off to the ANSI parser thread.</p>"
  },
  "dev-pty-arch": {
    "id": "dev-pty-arch",
    "title": "PTY Architecture",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "pty-lifecycle",
        "text": "PTY Descriptor Lifecycle",
        "level": 2
      },
      {
        "id": "termios-spec",
        "text": "POSIX Termios Configuration",
        "level": 2
      },
      {
        "id": "process-spawning",
        "text": "Process Spawning & Environment Inheritance",
        "level": 2
      }
    ],
    "summary": "POSIX openpty master/slave descriptor management and asynchronous I/O loops.",
    "body": "<h2 id=\"pty-lifecycle\">PTY Descriptor Lifecycle</h2>\n<p><code>pty::PTYSession</code> handles fork/execve, establishes slave descriptor as controlling tty (<code>setsid()</code>), and redirects stdin/stdout/stderr.</p>\n\n<h2 id=\"termios-spec\">POSIX Termios Configuration</h2>\n<p>Configures raw mode flags (<code>ICANON</code>, <code>ECHO</code>, <code>ISIG</code>) to allow interactive curses and shell line editors to manage raw keypresses.</p>\n\n<h2 id=\"process-spawning\">Process Spawning & Environment Inheritance</h2>\n<p>When launching a new tab, Meridian passes sanitized environment variables while injecting standard terminal indicators (<code>TERM=xterm-256color</code>, <code>COLORTERM=truecolor</code>).</p>"
  },
  "dev-screen-buffer": {
    "id": "dev-screen-buffer",
    "title": "Screen Buffer",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "cell-struct",
        "text": "Cell & Attributes Data Structure",
        "level": 2
      },
      {
        "id": "grid-manipulation",
        "text": "Grid Scrolling & Line Wrapping",
        "level": 2
      },
      {
        "id": "hyperlink-table",
        "text": "Hyperlink Attribute Registry",
        "level": 2
      }
    ],
    "summary": "2D cell matrix, damage tracking, and cursor management.",
    "body": "<h2 id=\"cell-struct\">Cell & Attributes Data Structure</h2>\n<p>Each screen cell in <code>vt::ScreenBuffer</code> contains:</p>\n<pre><code class=\"language-cpp\">struct Cell {\n    char32_t codepoint = U' ';\n    uint8_t width = 1;\n    Attributes attrs;\n};\n\nstruct Attributes {\n    Color fg = Color::Default();\n    Color bg = Color::Default();\n    uint8_t flags = 0; // Bold, Italic, Underline, Inverse\n    uint32_t hyperlink_id = 0;\n};</code></pre>\n\n<h2 id=\"grid-manipulation\">Grid Scrolling & Line Wrapping</h2>\n<p>Supports full scrolling regions (<code>DECSTBM</code>), cursor-relative movement, insert/delete lines, and auto-wrapping.</p>\n\n<h2 id=\"hyperlink-table\">Hyperlink Attribute Registry</h2>\n<p>Hyperlink URIs from OSC 8 sequences are interned into a deduplicated memory table, associating a lightweight 32-bit ID with each cell without duplicating URL string allocations across thousands of character cells.</p>"
  },
  "dev-renderer": {
    "id": "dev-renderer",
    "title": "Renderer",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "render-pipeline-dev",
        "text": "Render Pipeline Design",
        "level": 2
      },
      {
        "id": "instancing",
        "text": "Hardware Instanced Quad Rendering",
        "level": 2
      },
      {
        "id": "atlas-management",
        "text": "Dynamic Atlas Eviction & Bin Packing",
        "level": 2
      }
    ],
    "summary": "Glyph caching, texture atlas, and GPU batching.",
    "body": "<h2 id=\"render-pipeline-dev\">Render Pipeline Design</h2>\n<p><code>renderer::RenderPipeline</code> batches background color quads, glyph foreground quads, and raster image textures into unified draw calls.</p>\n\n<h2 id=\"instancing\">Hardware Instanced Quad Rendering</h2>\n<p>Draws thousands of terminal glyphs in a single GPU draw call using instanced vertex buffers (position, UV texture coordinates, foreground color, background color).</p>\n\n<h2 id=\"atlas-management\">Dynamic Atlas Eviction & Bin Packing</h2>\n<p><code>renderer::GlyphAtlas</code> employs a guillotine 2D bin-packing algorithm to pack variable-width Unicode characters, emoji, and Powerline symbols into GPU texture atlases.</p>"
  },
  "dev-graphics-engine": {
    "id": "dev-graphics-engine",
    "title": "Graphics Engine",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "decoder-internals",
        "text": "ImageDecoder Internals",
        "level": 2
      },
      {
        "id": "graphic-manager-dev",
        "text": "GraphicManager Texture Cache",
        "level": 2
      },
      {
        "id": "aspect-math",
        "text": "Pixel-to-Grid Coordinate Math",
        "level": 2
      }
    ],
    "summary": "stb_image decoding, aspect fitting, and image caching.",
    "body": "<h2 id=\"decoder-internals\">ImageDecoder Internals</h2>\n<p><code>graphics::ImageDecoder</code> uses <code>stb_image</code> to decode PNG, JPEG, WebP, GIF, and BMP into <code>std::vector&lt;uint8_t&gt;</code> 32-bit RGBA buffers.</p>\n\n<h2 id=\"graphic-manager-dev\">GraphicManager Texture Cache</h2>\n<p>Maintains texture handles and ensures images scale smoothly across high-DPI displays without CPU re-decoding.</p>\n\n<h2 id=\"aspect-math\">Pixel-to-Grid Coordinate Math</h2>\n<p>Converts terminal cell row/column coordinates into normalized OpenGL viewport coordinates, accounting for cell padding and fractional font metrics.</p>"
  },
  "dev-ai-engine": {
    "id": "dev-ai-engine",
    "title": "AI Engine",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "typo-popup-algorithm",
        "text": "Interactive AI Typo Correction Popup & Algorithm",
        "level": 2
      },
      {
        "id": "intent-engine-dev",
        "text": "Intent Engine (meridian ask)",
        "level": 2
      },
      {
        "id": "diagnostics-dev",
        "text": "Compiler & Runtime Diagnostics (meridian diag)",
        "level": 2
      },
      {
        "id": "agent-dev",
        "text": "Autonomous Coding Agent (meridian agent)",
        "level": 2
      }
    ],
    "summary": "Offline natural language intent translation, interactive typo correction popup, compiler diagnostics, and autonomous coding agent.",
    "body": "<h2 id=\"typo-popup-algorithm\">Interactive AI Typo Correction Popup & Algorithm</h2>\n<p>When a misspelled or unknown command is entered in interactive mode (e.g. <code>gti status</code>, <code>sl</code>, <code>mkdri test</code>, <code>pyhton script.py</code>), Meridian AI automatically analyzes the command and renders an interactive confirmation popup.</p>\n\n<div class=\"code-block-wrapper\">\n  <div class=\"code-header\"><span>Interactive AI Typo Popup Dialog</span></div>\n  <pre><code class=\"language-text\">┌── 💡 Meridian AI Typo Correction ──────────────────────────────────────────┐\n│ Command 'gti' not found in system PATH or builtins.                        │\n│ Did you mean: git status ?                                                 │\n│                                                                            │\n│ Press [Y/Enter] Run correction   [N/Esc] Keep original                     │\n└────────────────────────────────────────────────────────────────────────────┘</code></pre>\n</div>\n\n<p><strong>Correction Algorithm:</strong></p>\n<ol>\n  <li><strong>Optimal String Alignment (OSA):</strong> Calculates edit distance across insertions, deletions, substitutions, and adjacent transpositions (e.g. <code>gerp</code> &rarr; <code>grep</code>).</li>\n  <li><strong>Dynamic Proportional Threshold:</strong> Rejects false positives by requiring <code>distance &le; max(1, len / 3)</code> against known builtins and cached <code>$PATH</code> binaries.</li>\n  <li><strong>Single-Key Interactive Intercept:</strong> Puts terminal in non-canonical raw mode (<code>ICANON | ECHO</code> cleared) to read response immediately without requiring extra prompts.</li>\n</ol>\n\n<h2 id=\"intent-engine-dev\">Intent Engine (meridian ask)</h2>\n<pre><code class=\"language-bash\">meridian ask \"find all log files modified in the last 24 hours\"\n# -> find . -name \"*.log\" -mtime -1</code></pre>\n\n<h2 id=\"diagnostics-dev\">Compiler & Runtime Diagnostics (meridian diag)</h2>\n<pre><code class=\"language-bash\">meridian diag \"g++: error: cannot find -lutil\"\n# -> Diagnosis: Missing POSIX utility library. Fix: install glibc-devel / libutil.</code></pre>\n\n<h2 id=\"agent-dev\">Autonomous Coding Agent (meridian agent)</h2>\n<pre><code class=\"language-bash\">meridian agent \"add unit tests for OSC 8 hyperlinks\"</code></pre>"
  },
  "dev-security": {
    "id": "dev-security",
    "title": "Security",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "risk-tiers",
        "text": "Risk Classification Tiers (Low to Critical)",
        "level": 2
      },
      {
        "id": "redaction-engine",
        "text": "Secret & API Key Redactor",
        "level": 2
      },
      {
        "id": "audit-logging",
        "text": "Security Audit Log & Whitelisting",
        "level": 2
      }
    ],
    "summary": "Destructive command risk classification and credential leakage redactor.",
    "body": "<h2 id=\"risk-tiers\">Risk Classification Tiers (Low to Critical)</h2>\n<p><code>ai::RiskClassifier</code> intercepts commands before execution, flagging destructive patterns (e.g. <code>rm -rf /</code>, <code>mkfs</code>, <code>dd if=/dev/zero</code>, fork bombs).</p>\n\n<h2 id=\"redaction-engine\">Secret & API Key Redactor</h2>\n<p><code>ai::SecretRedactor</code> automatically strips Bearer tokens, AWS keys (<code>AKIA...</code>), and RSA/SSH private keys from logs and terminal scrollbacks.</p>\n\n<h2 id=\"audit-logging\">Security Audit Log & Whitelisting</h2>\n<p>Flagged commands are recorded in <code>~/.local/share/meridian/security_audit.log</code> with user confirmation timestamps.</p>"
  },
  "config-file": {
    "id": "config-file",
    "title": "Configuration File",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "config-schema",
        "text": "Complete JSON Schema",
        "level": 2
      },
      {
        "id": "options-table",
        "text": "Configuration Options Reference",
        "level": 2
      },
      {
        "id": "validation-rules",
        "text": "JSON Schema Validation & Fallbacks",
        "level": 2
      }
    ],
    "summary": "Specification of ~/.config/meridian/config.json options.",
    "body": "<h2 id=\"config-schema\">Complete JSON Schema</h2>\n<p>Location: <code>~/.config/meridian/config.json</code></p>\n<pre><code class=\"language-json\">{\n  \"theme\": \"sharingan_eye\",\n  \"random_theme\": false,\n  \"font_family\": \"FantasqueSansMNFM\",\n  \"font_size\": 14,\n  \"line_height\": 1.2,\n  \"cursor_shape\": \"block\",\n  \"cursor_blink\": true,\n  \"scrollback_lines\": 10000,\n  \"opacity\": 0.95,\n  \"blur\": true,\n  \"ai_enabled\": true,\n  \"telemetry_hud\": false\n}</code></pre>\n\n<h2 id=\"options-table\">Configuration Options Reference</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Key</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>theme</code></td><td>string</td><td><code>\"sharingan_eye\"</code></td><td>Default startup anime artwork theme.</td></tr>\n    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Terminal font size in points.</td></tr>\n    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Maximum scrollback buffer lines per pane.</td></tr>\n    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity (0.0 - 1.0).</td></tr>\n    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>\n    <tr><td><code>cursor_shape</code></td><td>string</td><td><code>\"block\"</code></td><td>Cursor shape: \"block\", \"beam\", or \"underline\".</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"validation-rules\">JSON Schema Validation & Fallbacks</h2>\n<p>Meridian automatically validates config files against an internal schema on startup. If a syntax error is present, Meridian logs a warning and falls back to safe defaults without crashing.</p>"
  },
  "config-themes": {
    "id": "config-themes",
    "title": "Themes",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "theme-configuration",
        "text": "Theme Configuration Options",
        "level": 2
      },
      {
        "id": "custom-wallpaper-guide",
        "text": "Custom Wallpaper Paths",
        "level": 2
      },
      {
        "id": "wallpaper-blending",
        "text": "Wallpaper Opacity & Shading",
        "level": 2
      }
    ],
    "summary": "Configuring anime wallpaper themes and custom wallpapers.",
    "body": "<h2 id=\"theme-configuration\">Theme Configuration Options</h2>\n<p>Set <code>random_theme: true</code> in <code>config.json</code> to rotate themes automatically on startup.</p>\n\n<h2 id=\"custom-wallpaper-guide\">Custom Wallpaper Paths</h2>\n<pre><code class=\"language-bash\"># Set custom background wallpaper via CLI\npic set /home/charanbalaji/Pictures/custom_wallpaper.png</code></pre>\n\n<h2 id=\"wallpaper-blending\">Wallpaper Opacity & Shading</h2>\n<p>Configure the background artwork brightness and contrast to preserve text legibility over bright image areas:</p>\n<pre><code class=\"language-json\">{\n  \"wallpaper_opacity\": 0.85,\n  \"wallpaper_dimming\": 0.30\n}</code></pre>"
  },
  "config-keybindings": {
    "id": "config-keybindings",
    "title": "Keybindings",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "keymap-json",
        "text": "keybindings.json Specification",
        "level": 2
      },
      {
        "id": "available-actions",
        "text": "Available Action Identifiers",
        "level": 2
      }
    ],
    "summary": "Customizing keyboard shortcuts via keybindings.json.",
    "body": "<h2 id=\"keymap-json\">keybindings.json Specification</h2>\n<p>Location: <code>~/.config/meridian/keybindings.json</code></p>\n<pre><code class=\"language-json\">{\n  \"new_tab\": \"Ctrl+Shift+T\",\n  \"close_tab\": \"Ctrl+Shift+W\",\n  \"split_vertical\": \"Ctrl+Shift+D\",\n  \"split_horizontal\": \"Ctrl+Shift+E\",\n  \"command_palette\": \"Ctrl+Shift+P\",\n  \"theme_gallery\": \"Ctrl+P\",\n  \"search_scrollback\": \"Ctrl+Shift+F\"\n}</code></pre>\n\n<h2 id=\"available-actions\">Available Action Identifiers</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Action ID</th><th>Default Shortcut</th><th>Effect</th></tr></thead>\n  <tbody>\n    <tr><td><code>new_tab</code></td><td><code>Ctrl+Shift+T</code></td><td>Creates a new shell tab.</td></tr>\n    <tr><td><code>close_tab</code></td><td><code>Ctrl+Shift+W</code></td><td>Closes the current tab/pane.</td></tr>\n    <tr><td><code>split_vertical</code></td><td><code>Ctrl+Shift+D</code></td><td>Splits active pane vertically.</td></tr>\n    <tr><td><code>split_horizontal</code></td><td><code>Ctrl+Shift+E</code></td><td>Splits active pane horizontally.</td></tr>\n    <tr><td><code>toggle_zoom</code></td><td><code>Ctrl+Shift+Z</code></td><td>Toggles fullscreen pane zoom.</td></tr>\n  </tbody>\n</table>"
  },
  "config-profiles": {
    "id": "config-profiles",
    "title": "Profiles",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "profile-management",
        "text": "Profile Configurations",
        "level": 2
      },
      {
        "id": "profiles-json-spec",
        "text": "profiles.json Example",
        "level": 2
      }
    ],
    "summary": "Setting up multiple terminal profiles (Shell, SSH, Containers).",
    "body": "<h2 id=\"profile-management\">Profile Configurations</h2>\n<p>Define custom shell startup profiles in <code>~/.config/meridian/profiles.json</code> for Bash, Zsh, Fish, or direct SSH sessions.</p>\n\n<h2 id=\"profiles-json-spec\">profiles.json Example</h2>\n<pre><code class=\"language-json\">{\n  \"default_profile\": \"zsh\",\n  \"profiles\": [\n    {\n      \"name\": \"zsh\",\n      \"command\": \"/bin/zsh\",\n      \"args\": [\"-l\"],\n      \"env\": { \"SHELL\": \"/bin/zsh\" }\n    },\n    {\n      \"name\": \"docker-dev\",\n      \"command\": \"/usr/bin/docker\",\n      \"args\": [\"exec\", \"-it\", \"dev-container\", \"/bin/bash\"],\n      \"theme\": \"cyberpunk_edge\"\n    }\n  ]\n}</code></pre>"
  },
  "config-env": {
    "id": "config-env",
    "title": "Environment Variables",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "env-table",
        "text": "Environment Variables Reference",
        "level": 2
      },
      {
        "id": "overriding-env",
        "text": "Overriding Default Environment Variables",
        "level": 2
      }
    ],
    "summary": "Environment variables read and set by Meridian Terminal.",
    "body": "<h2 id=\"env-table\">Environment Variables Reference</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Variable</th><th>Default Value</th><th>Purpose</th></tr></thead>\n  <tbody>\n    <tr><td><code>TERM</code></td><td><code>xterm-256color</code></td><td>Standard terminal capability identification.</td></tr>\n    <tr><td><code>COLORTERM</code></td><td><code>truecolor</code></td><td>Enables 24-bit TrueColor in CLI applications.</td></tr>\n    <tr><td><code>MERIDIAN_CONFIG_HOME</code></td><td><code>~/.config/meridian</code></td><td>Override configuration directory path.</td></tr>\n    <tr><td><code>MERIDIAN_SESSION_ID</code></td><td><code>UUID</code></td><td>Unique identifier for active terminal window.</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"overriding-env\">Overriding Default Environment Variables</h2>\n<p>Set custom environment variables in <code>~/.config/meridian/config.json</code> under the <code>\"env\"</code> dictionary.</p>"
  },
  "pkg-fedora": {
    "id": "pkg-fedora",
    "title": "Fedora / RPM",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "fedora-rpm",
        "text": "Fedora / RHEL RPM Package",
        "level": 2
      },
      {
        "id": "spec-file",
        "text": "RPM Spec File & Compilation",
        "level": 2
      },
      {
        "id": "copr-repo",
        "text": "Fedora COPR Repository Publishing",
        "level": 2
      }
    ],
    "summary": "Building and installing RPM packages with DNF and spec files.",
    "body": "<h2 id=\"fedora-rpm\">Fedora / RHEL RPM Package</h2>\n<pre><code class=\"language-bash\"># Install locally built RPM\nsudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>\n\n<h2 id=\"spec-file\">RPM Spec File & Compilation</h2>\n<p>The spec file is located in <code>packaging/rpm/meridian-terminal.spec</code>. To build locally:</p>\n<pre><code class=\"language-bash\">rpmbuild -ba packaging/rpm/meridian-terminal.spec</code></pre>\n\n<h2 id=\"copr-repo\">Fedora COPR Repository Publishing</h2>\n<p>Meridian packages are automatically built for Fedora 39, 40, 41, and Rawhide via automated COPR webhooks on release tags.</p>"
  },
  "pkg-debian": {
    "id": "pkg-debian",
    "title": "Ubuntu / Debian / DEB",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "debian-deb",
        "text": "Ubuntu & Debian DEB Package",
        "level": 2
      },
      {
        "id": "deb-packaging",
        "text": "Building .deb Packages",
        "level": 2
      }
    ],
    "summary": "Installing and building .deb packages for Ubuntu, Debian, and Linux Mint.",
    "body": "<h2 id=\"debian-deb\">Ubuntu & Debian DEB Package</h2>\n<pre><code class=\"language-bash\">sudo apt update\nsudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>\n\n<h2 id=\"deb-packaging\">Building .deb Packages</h2>\n<pre><code class=\"language-bash\">dpkg-deb --build packaging/debian meridian-terminal_2.0.0_amd64.deb</code></pre>"
  },
  "pkg-arch": {
    "id": "pkg-arch",
    "title": "Arch / AUR",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "arch-aur",
        "text": "Arch Linux PKGBUILD",
        "level": 2
      },
      {
        "id": "aur-install",
        "text": "AUR Helper Installation",
        "level": 2
      }
    ],
    "summary": "PKGBUILD packaging and AUR installation for Arch Linux and Manjaro.",
    "body": "<h2 id=\"arch-aur\">Arch Linux PKGBUILD</h2>\n<pre><code class=\"language-bash\">cd packaging/arch\nmakepkg -si</code></pre>\n\n<h2 id=\"aur-install\">AUR Helper Installation</h2>\n<pre><code class=\"language-bash\">yay -S meridian-terminal-bin</code></pre>"
  },
  "pkg-opensuse": {
    "id": "pkg-opensuse",
    "title": "openSUSE",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "opensuse-zypper",
        "text": "openSUSE Zypper Installation",
        "level": 2
      },
      {
        "id": "obs-build",
        "text": "Open Build Service (OBS) Integration",
        "level": 2
      }
    ],
    "summary": "Installing RPM packages with Zypper on openSUSE Tumbleweed and Leap.",
    "body": "<h2 id=\"opensuse-zypper\">openSUSE Zypper Installation</h2>\n<pre><code class=\"language-bash\">sudo zypper install ./meridian-terminal.rpm</code></pre>\n\n<h2 id=\"obs-build\">Open Build Service (OBS) Integration</h2>\n<p>Meridian maintains spec configurations compatible with openSUSE Tumbleweed and Leap repositories.</p>"
  },
  "pkg-appimage": {
    "id": "pkg-appimage",
    "title": "AppImage",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "appimage-guide",
        "text": "Standalone AppImage Execution",
        "level": 2
      },
      {
        "id": "building-appimage",
        "text": "Building AppImages with appimagetool",
        "level": 2
      }
    ],
    "summary": "Standalone self-contained AppImage package for any Linux distribution.",
    "body": "<h2 id=\"appimage-guide\">Standalone AppImage Execution</h2>\n<pre><code class=\"language-bash\">chmod +x meridian-terminal.AppImage\n./meridian-terminal.AppImage</code></pre>\n\n<h2 id=\"building-appimage\">Building AppImages with appimagetool</h2>\n<pre><code class=\"language-bash\">./scripts/build_appimage.sh</code></pre>"
  },
  "pkg-flatpak": {
    "id": "pkg-flatpak",
    "title": "Flatpak",
    "category": "PACKAGING",
    "status": "planned",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "flatpak-roadmap-spec",
        "text": "Flatpak Container Roadmap",
        "level": 2
      },
      {
        "id": "flatpak-permissions",
        "text": "Host PTY & Windowing Permissions",
        "level": 2
      }
    ],
    "summary": "Flatpak container package roadmap and sandbox permissions.",
    "body": "<h2 id=\"flatpak-roadmap-spec\">Flatpak Container Roadmap</h2>\n<p>Flatpak packaging with <code>org.freedesktop.Flatpak</code> Host PTY permissions is currently planned.</p>\n\n<h2 id=\"flatpak-permissions\">Host PTY & Windowing Permissions</h2>\n<p>Terminal emulators require <code>--talk-name=org.freedesktop.Flatpak</code> to spawn host processes via <code>flatpak-spawn --host</code>.</p>"
  },
  "pkg-snap": {
    "id": "pkg-snap",
    "title": "Snap",
    "category": "PACKAGING",
    "status": "planned",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "snap-roadmap-spec",
        "text": "Snap Package Roadmap",
        "level": 2
      },
      {
        "id": "classic-confinement",
        "text": "Classic Confinement Approval",
        "level": 2
      }
    ],
    "summary": "Snap packaging roadmap with classic confinement.",
    "body": "<h2 id=\"snap-roadmap-spec\">Snap Package Roadmap</h2>\n<p>Snap packaging with classic confinement is planned for the Canonical Snap Store.</p>\n\n<h2 id=\"classic-confinement\">Classic Confinement Approval</h2>\n<p>Terminal emulators require classic confinement to execute user shells and arbitrary binaries outside the snap sandbox.</p>"
  },
  "pkg-macos": {
    "id": "pkg-macos",
    "title": "macOS",
    "category": "PACKAGING",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "macos-status-spec",
        "text": "macOS Darwin Status",
        "level": 2
      },
      {
        "id": "homebrew-install",
        "text": "Homebrew Tap & Installation",
        "level": 2
      }
    ],
    "summary": "macOS Darwin BSD PTY support, Homebrew formula, and DMG installer.",
    "body": "<h2 id=\"macos-status-spec\">macOS Darwin Status</h2>\n<p>Darwin BSD PTY abstraction and Homebrew formula (<code>brew install meridian-terminal</code>) are in active development for Apple Silicon (M1-M4) and Intel x86_64.</p>\n\n<h2 id=\"homebrew-install\">Homebrew Tap & Installation</h2>\n<pre><code class=\"language-bash\">brew tap charanbalaji2005/meridian\nbrew install --cask meridian-terminal</code></pre>"
  },
  "pkg-windows": {
    "id": "pkg-windows",
    "title": "Windows",
    "category": "PACKAGING",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "windows-conpty-spec",
        "text": "Windows ConPTY Core",
        "level": 2
      },
      {
        "id": "winget-install",
        "text": "Windows Package Manager (winget)",
        "level": 2
      }
    ],
    "summary": "Windows 10/11 ConPTY pseudoterminal adapter and MSI installer.",
    "body": "<h2 id=\"windows-conpty-spec\">Windows ConPTY Core</h2>\n<p>Windows Console PTY (ConPTY) adapter and MSI / winget installer are in development.</p>\n\n<h2 id=\"winget-install\">Windows Package Manager (winget)</h2>\n<pre><code class=\"language-powershell\">winget install Meridian.Terminal</code></pre>"
  },
  "development-building": {
    "id": "development-building",
    "title": "Building",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "prerequisites",
        "text": "Compiler & Library Prerequisites",
        "level": 2
      },
      {
        "id": "make-targets",
        "text": "Make Targets & Build Commands",
        "level": 2
      },
      {
        "id": "custom-cxxflags",
        "text": "Custom Compiler Flags & Optimization",
        "level": 2
      }
    ],
    "summary": "Compiling Meridian Terminal with C++20, Make, and dependencies.",
    "body": "<h2 id=\"prerequisites\">Compiler & Library Prerequisites</h2>\n<p>Requires GCC 11+ or Clang 13+ with C++20 support, and standard POSIX <code>libutil</code>.</p>\n\n<h2 id=\"make-targets\">Make Targets & Build Commands</h2>\n<pre><code class=\"language-bash\">make all -j$(nproc)  # Compile all targets\nmake test            # Run automated test suite (133 tests)\nmake demo            # Run graphical demo\nsudo make install    # Install binaries and assets to system\nmake clean           # Clean build artifacts</code></pre>\n\n<h2 id=\"custom-cxxflags\">Custom Compiler Flags & Optimization</h2>\n<pre><code class=\"language-bash\"># Compile with aggressive optimizations and LTO\nCXXFLAGS=\"-std=c++20 -O3 -flto -march=native\" make all -j$(nproc)</code></pre>"
  },
  "development-testing": {
    "id": "development-testing",
    "title": "Testing & Verification Guide",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "automated-test-suite",
        "text": "1. Automated Test Suite (133 Tests)",
        "level": 2
      },
      {
        "id": "interactive-terminal",
        "text": "2. Interactive Terminal & Shell Launch",
        "level": 2
      },
      {
        "id": "shell-pipelines",
        "text": "3. Shell Pipelines & Command Execution",
        "level": 2
      },
      {
        "id": "signals-job-control",
        "text": "4. Signals & Job Control (Ctrl+C / Ctrl+Z)",
        "level": 2
      },
      {
        "id": "splits-navigation-zoom",
        "text": "5. Window Splits, Directional Navigation & Zoom",
        "level": 2
      },
      {
        "id": "raster-image-testing",
        "text": "6. Direct Raster Image Protocol (pic)",
        "level": 2
      },
      {
        "id": "dev-intelligence-tools",
        "text": "7. Developer Intelligence Tools (monitor, git, files, ssh, perf)",
        "level": 2
      },
      {
        "id": "palette-and-search",
        "text": "8. Command Palette & Universal Search",
        "level": 2
      },
      {
        "id": "ai-engine-testing",
        "text": "9. Local AI Engine & Error Diagnostics",
        "level": 2
      }
    ],
    "summary": "Comprehensive step-by-step guide and commands to verify PTY multiplexing, signals, splits, graphics, developer tools, and AI.",
    "body": "<p>This step-by-step guide provides copy-pasteable commands and verification procedures to test every core subsystem of Meridian Terminal.</p>\n\n<h2 id=\"automated-test-suite\">1. Automated Test Suite (133 Tests)</h2>\n<p>Run the automated test runner to verify core engine correctness, PTY multiplexing, VT parsing, AST execution, and security:</p>\n<pre><code class=\"language-bash\"># Run all 133 automated unit and integration tests\nmake test\n\n# Or run test binary directly\n./build/meridian_tests</code></pre>\n\n<h2 id=\"interactive-terminal\">2. Interactive Terminal & Shell Launch</h2>\n<p>Launch the interactive terminal session or standalone shell engine:</p>\n<pre><code class=\"language-bash\"># Launch interactive terminal emulator\nmeridian\n\n# Launch standalone shell engine directly\nmeridian-shell</code></pre>\n\n<h2 id=\"shell-pipelines\">3. Shell Pipelines & Command Execution</h2>\n<p>Verify command pipelines, environment variable expansion, and command substitution engines:</p>\n<pre><code class=\"language-bash\"># Test pipelines & coreutils\nps aux | grep -i meridian | head -n 5\n\n# Test variable export and expansion\nexport PROJECT_NAME=\"Meridian\" && echo \"Running $PROJECT_NAME on $SHELL\"\n\n# Test command substitution engine\necho \"Kernel: $(uname -r) | Current Time: $(date +%T)\"</code></pre>\n\n<h2 id=\"signals-job-control\">4. Signals & Job Control (Ctrl+C / Ctrl+Z)</h2>\n<p>Verify that POSIX signals are routed strictly to the foreground process group without killing Meridian:</p>\n<pre><code class=\"language-bash\"># 1. Start a long running command and cancel it with Ctrl+C:\nsleep 10\n# Press: Ctrl+C  -> Cancels sleep immediately and returns to prompt\n\n# 2. Test job suspension and resumption:\nsleep 50\n# Press: Ctrl+Z  -> Suspends job\njobs            # Lists running and stopped background jobs\nfg              # Brings suspended job back to foreground</code></pre>\n\n<h2 id=\"splits-navigation-zoom\">5. Window Splits, Directional Navigation & Zoom</h2>\n<p>Test the persistent binary space partitioning (BSP) pane tree and keyboard navigation:</p>\n<table class=\"doc-table\">\n  <thead><tr><th>Keyboard Shortcut</th><th>Action</th><th>Shell Built-in Equivalent</th></tr></thead>\n  <tbody>\n    <tr><td><code>Ctrl+Shift+D</code></td><td>Split active pane vertically</td><td><code>split v</code></td></tr>\n    <tr><td><code>Ctrl+Shift+E</code></td><td>Split active pane horizontally</td><td><code>split h</code></td></tr>\n    <tr><td><code>Ctrl+Shift+Z</code></td><td>Toggle zoom on active pane</td><td><code>zoom</code></td></tr>\n    <tr><td><code>Alt+Up</code></td><td>Navigate focus to pane above</td><td><code>pane up</code></td></tr>\n    <tr><td><code>Alt+Down</code></td><td>Navigate focus to pane below</td><td><code>pane down</code></td></tr>\n    <tr><td><code>Alt+Left</code></td><td>Navigate focus to pane on left</td><td><code>pane left</code></td></tr>\n    <tr><td><code>Alt+Right</code></td><td>Navigate focus to pane on right</td><td><code>pane right</code></td></tr>\n    <tr><td>—</td><td>Inspect active pane matrix</td><td><code>pane list</code></td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"raster-image-testing\">6. Direct Raster Image Protocol (pic)</h2>\n<p>Test inline 32-bit RGBA hardware-blitted raster images:</p>\n<pre><code class=\"language-bash\"># Render direct inline image:\npic ~/.config/meridian/gallery/sharingan_eye.png\n\n# Set startup anime artwork theme:\npic set 1\n\n# Enable random artwork on every startup:\npic set random</code></pre>\n\n<h2 id=\"dev-intelligence-tools\">7. Developer Intelligence Tools (monitor, git, files, ssh, perf)</h2>\n<p>Test Meridian's integrated developer productivity suite:</p>\n<pre><code class=\"language-bash\"># Real-time system monitor (CPU, RAM, Disk, Process metrics):\nmeridian monitor\n\n# Visual Git branch divergence and staged/unstaged inspector:\nmeridian git\n\n# Interactive tree file explorer with git badges:\nmeridian files\n\n# SSH connection manager (~/.ssh/config):\nmeridian ssh\n\n# Live GPU framerate & PTY latency profiler:\nmeridian perf</code></pre>\n\n<h2 id=\"palette-and-search\">8. Command Palette & Universal Search</h2>\n<p>Launch quick actions and search across history:</p>\n<pre><code class=\"language-bash\"># Open fuzzy Command Palette:\npalette\n# (Or press Ctrl+Shift+P / Ctrl+P anytime)\n\n# Universal search across screen buffers and rich history:\nsearch \"git\"\n# (Or press Ctrl+Shift+F anytime)\n\n# View rich SQLite history database with execution durations:\nmeridian history</code></pre>\n\n<h2 id=\"ai-engine-testing\">9. Local AI Engine & Error Diagnostics</h2>\n<p>Test offline privacy-preserving developer AI:</p>\n<pre><code class=\"language-bash\"># Natural language to safe shell command translation:\nmeridian ask \"find all files modified in the last 24 hours\"\n\n# Error diagnostics engine:\nmeridian diag \"Segmentation fault (core dumped)\"\n\n# Inspect AI privacy mode and risk classifier:\nmeridian ai status</code></pre>"
  },
  "development-debugging": {
    "id": "development-debugging",
    "title": "Debugging",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "gdb-asan",
        "text": "GDB & AddressSanitizer",
        "level": 2
      },
      {
        "id": "telemetry-profiler-dev",
        "text": "Telemetry Profiler (meridian --performance)",
        "level": 2
      },
      {
        "id": "valgrind-profiling",
        "text": "Valgrind Memory Profiling",
        "level": 2
      }
    ],
    "summary": "GDB debugging, ASan, Valgrind, and Telemetry profiler.",
    "body": "<h2 id=\"gdb-asan\">GDB & AddressSanitizer</h2>\n<pre><code class=\"language-bash\"># Build with AddressSanitizer and debug symbols\nCXXFLAGS=\"-std=c++20 -fsanitize=address -g -O0\" make all</code></pre>\n\n<h2 id=\"telemetry-profiler-dev\">Telemetry Profiler (meridian --performance)</h2>\n<pre><code class=\"language-bash\">meridian --performance\n# Outputs real-time FPS, frame time (ms), glyph cache hits, VRAM, and PTY latency.</code></pre>\n\n<h2 id=\"valgrind-profiling\">Valgrind Memory Profiling</h2>\n<pre><code class=\"language-bash\">valgrind --leak-check=full --track-origins=yes ./bin/meridian</code></pre>"
  },
  "development-contributing": {
    "id": "development-contributing",
    "title": "Contributing",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "contributing-guide-spec",
        "text": "Open Source Contribution Workflow",
        "level": 2
      },
      {
        "id": "code-style-guide",
        "text": "Code Style Guidelines (clang-format)",
        "level": 2
      }
    ],
    "summary": "How to report bugs, resolve visual glitches, add anime themes, and submit PRs.",
    "body": "<h2 id=\"contributing-guide-spec\">Open Source Contribution Workflow</h2>\n<p>Fork the repository on GitHub, create a feature branch, run <code>make test</code> to verify zero regressions, and open a Pull Request.</p>\n\n<h2 id=\"code-style-guide\">Code Style Guidelines (clang-format)</h2>\n<pre><code class=\"language-bash\"># Format all C++ source files\nclang-format -i src/**/*.cpp src/**/*.hpp tests/**/*.cpp</code></pre>"
  },
  "development-release": {
    "id": "development-release",
    "title": "Release Process",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "release-flow-spec",
        "text": "Release & Packaging Automation",
        "level": 2
      },
      {
        "id": "semantic-versioning",
        "text": "Semantic Versioning & Changelogs",
        "level": 2
      }
    ],
    "summary": "Version tagging, packaging automation, and release checklist.",
    "body": "<h2 id=\"release-flow-spec\">Release & Packaging Automation</h2>\n<p>Automated GitHub Actions workflows package RPM, DEB, Arch PKGBUILD, and universal tarballs on version tag creation.</p>\n\n<h2 id=\"semantic-versioning\">Semantic Versioning & Changelogs</h2>\n<p>Meridian strictly follows Semantic Versioning (MAJOR.MINOR.PATCH). Release notes are maintained in <code>docs/project/changelog.md</code>.</p>"
  },
  "ref-cli": {
    "id": "ref-cli",
    "title": "CLI Reference",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "cli-full-ref",
        "text": "Complete CLI Command Reference",
        "level": 2
      },
      {
        "id": "exit-codes",
        "text": "CLI Exit Codes",
        "level": 2
      }
    ],
    "summary": "Complete command-line interface options and subcommands.",
    "body": "<h2 id=\"cli-full-ref\">Complete CLI Command Reference</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Command</th><th>Arguments</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>meridian</code></td><td><code>[none]</code></td><td>Launch interactive terminal emulator GUI.</td></tr>\n    <tr><td><code>meridian-shell</code></td><td><code>[-c &quot;cmd&quot;]</code></td><td>Launch standalone AST shell process.</td></tr>\n    <tr><td><code>meridian ssh</code></td><td><code>[alias]</code></td><td>Manage and connect to SSH remote workspaces.</td></tr>\n    <tr><td><code>meridian plugins</code></td><td><code>[none]</code></td><td>List active extensible plugins and lifecycle hooks.</td></tr>\n    <tr><td><code>meridian --performance</code></td><td><code>[none]</code></td><td>Display GPU framerate and telemetry profiler HUD.</td></tr>\n    <tr><td><code>meridian monitor</code></td><td><code>[none]</code></td><td>Open live CPU, RAM, Disk, and Network dashboard.</td></tr>\n    <tr><td><code>meridian git</code></td><td><code>[none]</code></td><td>Inspect Git branch divergence and staged changes.</td></tr>\n    <tr><td><code>meridian pic</code></td><td><code>&lt;file&gt;</code></td><td>Display direct 32-bit RGBA inline image.</td></tr>\n    <tr><td><code>meridian ask</code></td><td><code>&quot;&lt;intent&gt;&quot;</code></td><td>Translate natural language to safe shell command.</td></tr>\n    <tr><td><code>meridian diag</code></td><td><code>&quot;&lt;error&gt;&quot;</code></td><td>Diagnose compiler / runtime errors with suggested fixes.</td></tr>\n  </tbody>\n</table>\n\n<h2 id=\"exit-codes\">CLI Exit Codes</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Code</th><th>Meaning</th></tr></thead>\n  <tbody>\n    <tr><td><code>0</code></td><td>Success / Clean exit.</td></tr>\n    <tr><td><code>1</code></td><td>General command failure / parse error.</td></tr>\n    <tr><td><code>126</code></td><td>Command found but not executable.</td></tr>\n    <tr><td><code>127</code></td><td>Command not found.</td></tr>\n    <tr><td><code>130</code></td><td>Process terminated by SIGINT (Ctrl+C).</td></tr>\n  </tbody>\n</table>"
  },
  "ref-config": {
    "id": "ref-config",
    "title": "Configuration Reference",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "config-keys-ref",
        "text": "Configuration Keys Reference Table",
        "level": 2
      }
    ],
    "summary": "Complete schema documentation for config.json.",
    "body": "<h2 id=\"config-keys-ref\">Configuration Keys Reference Table</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Key</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>theme</code></td><td>string</td><td><code>\"sharingan_eye\"</code></td><td>Default anime wallpaper theme identifier.</td></tr>\n    <tr><td><code>random_theme</code></td><td>boolean</td><td><code>false</code></td><td>Rotate anime wallpaper on startup.</td></tr>\n    <tr><td><code>font_family</code></td><td>string</td><td><code>\"FantasqueSansMNFM\"</code></td><td>Primary font family name.</td></tr>\n    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Font size in points.</td></tr>\n    <tr><td><code>line_height</code></td><td>number</td><td><code>1.2</code></td><td>Line height multiplier.</td></tr>\n    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Maximum scrollback buffer lines.</td></tr>\n    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity (0.0 to 1.0).</td></tr>\n    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>\n    <tr><td><code>cursor_shape</code></td><td>string</td><td><code>\"block\"</code></td><td>Cursor shape: \"block\", \"beam\", or \"underline\".</td></tr>\n    <tr><td><code>cursor_blink</code></td><td>boolean</td><td><code>true</code></td><td>Enable cursor blinking animation.</td></tr>\n    <tr><td><code>ai_enabled</code></td><td>boolean</td><td><code>true</code></td><td>Enable local offline AI assistance.</td></tr>\n    <tr><td><code>telemetry_hud</code></td><td>boolean</td><td><code>false</code></td><td>Show live GPU telemetry HUD overlay.</td></tr>\n  </tbody>\n</table>"
  },
  "ref-shortcuts": {
    "id": "ref-shortcuts",
    "title": "Keyboard Shortcuts",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "all-shortcuts-ref",
        "text": "All Keyboard Shortcuts Reference",
        "level": 2
      }
    ],
    "summary": "Complete table of all default keyboard shortcuts.",
    "body": "<h2 id=\"all-shortcuts-ref\">All Keyboard Shortcuts Reference</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Shortcut</th><th>Action</th><th>Category</th></tr></thead>\n  <tbody>\n    <tr><td><code>Ctrl+P</code></td><td>Anime Theme Gallery</td><td>Themes</td></tr>\n    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Navigation</td></tr>\n    <tr><td><code>Ctrl+Shift+T</code></td><td>New Tab</td><td>Tabs</td></tr>\n    <tr><td><code>Ctrl+Shift+W</code></td><td>Close Tab / Pane</td><td>Windows</td></tr>\n    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Panes</td></tr>\n    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Panes</td></tr>\n    <tr><td><code>Ctrl+Shift+Z</code></td><td>Toggle Pane Zoom</td><td>Panes</td></tr>\n    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search</td></tr>\n    <tr><td><code>Ctrl+Shift+C</code></td><td>Copy Selection</td><td>Clipboard</td></tr>\n    <tr><td><code>Ctrl+Shift+V</code></td><td>Paste Clipboard</td><td>Clipboard</td></tr>\n    <tr><td><code>Ctrl+L</code></td><td>Clear Screen</td><td>Terminal</td></tr>\n    <tr><td><code>Ctrl+PageUp</code> / <code>Ctrl+PageDown</code></td><td>Switch Tabs</td><td>Tabs</td></tr>\n  </tbody>\n</table>"
  },
  "ref-env": {
    "id": "ref-env",
    "title": "Environment Variables",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "all-env-ref",
        "text": "Environment Variables Reference",
        "level": 2
      }
    ],
    "summary": "Environment variables used and exported by Meridian.",
    "body": "<h2 id=\"all-env-ref\">Environment Variables Reference</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Variable</th><th>Default</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>TERM</code></td><td><code>xterm-256color</code></td><td>Terminal emulator type.</td></tr>\n    <tr><td><code>COLORTERM</code></td><td><code>truecolor</code></td><td>Indicates 24-bit TrueColor RGB support.</td></tr>\n    <tr><td><code>MERIDIAN_CONFIG_HOME</code></td><td><code>~/.config/meridian</code></td><td>Override configuration directory.</td></tr>\n    <tr><td><code>MERIDIAN_SESSION_ID</code></td><td><code>UUID</code></td><td>Unique session identifier for active window.</td></tr>\n    <tr><td><code>MERIDIAN_THEME</code></td><td><code>string</code></td><td>Active anime wallpaper identifier.</td></tr>\n  </tbody>\n</table>"
  },
  "ref-faq": {
    "id": "ref-faq",
    "title": "FAQ",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "faq-list",
        "text": "Frequently Asked Questions",
        "level": 2
      }
    ],
    "summary": "Frequently asked questions about Meridian Shell.",
    "body": "<h2 id=\"faq-list\">Frequently Asked Questions</h2>\n<p><strong>Q: Does Meridian render real images or ASCII approximations?</strong><br/>\nA: Meridian renders genuine 32-bit RGBA hardware raster graphics on the GPU canvas with zero ASCII or half-block approximations.</p>\n\n<p><strong>Q: Is Meridian a fake shell or real terminal?</strong><br/>\nA: Meridian is a true terminal emulator running real Linux PTY sessions connected directly to <code>bash</code>, <code>zsh</code>, or <code>fish</code>.</p>\n\n<p><strong>Q: Can I use Meridian over SSH without remote installations?</strong><br/>\nA: Yes! Meridian uses standard ANSI and OSC escape sequences that work seamlessly over any remote SSH connection.</p>\n\n<p><strong>Q: Does Meridian require an internet connection?</strong><br/>\nA: No. Meridian and its AI engine operate 100% offline with zero cloud dependencies.</p>"
  },
  "ref-troubleshooting": {
    "id": "ref-troubleshooting",
    "title": "Troubleshooting",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "troubleshooting-guide",
        "text": "Troubleshooting Common Issues",
        "level": 2
      }
    ],
    "summary": "Solutions for font rendering, PTY permissions, and graphical display issues.",
    "body": "<h2 id=\"troubleshooting-guide\">Troubleshooting Common Issues</h2>\n<ul>\n  <li><strong>Missing Powerline glyphs:</strong> Install a Nerd Font (e.g. <code>FantasqueSansM Nerd Font</code> or <code>JetBrainsMono Nerd Font</code>).</li>\n  <li><strong>Image display issues:</strong> Verify that GPU drivers (Mesa / NVIDIA) are active and supporting OpenGL 3.3 Core.</li>\n  <li><strong>PTY Permission Denied:</strong> Ensure your user account is in the <code>tty</code> group or that <code>/dev/pts</code> is mounted with standard permissions.</li>\n  <li><strong>Keybinding conflicts:</strong> Inspect <code>~/.config/meridian/keybindings.json</code> for duplicate shortcut mappings.</li>\n</ul>"
  },
  "proj-github": {
    "id": "proj-github",
    "title": "GitHub",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "github-links-spec",
        "text": "Official GitHub Repository",
        "level": 2
      },
      {
        "id": "issue-tracker",
        "text": "Issue Tracker & Bug Reports",
        "level": 2
      },
      {
        "id": "discussions",
        "text": "GitHub Discussions & Community",
        "level": 2
      }
    ],
    "summary": "Source code repository, issue tracker, and community discussions.",
    "body": "<h2 id=\"github-links-spec\">Official GitHub Repository</h2>\n<p>The official source code repository is hosted on GitHub:</p>\n<p><a href=\"https://github.com/charanbalaji2005/Meridian-Shell\" target=\"_blank\">https://github.com/charanbalaji2005/Meridian-Shell</a></p>\n\n<h2 id=\"issue-tracker\">Issue Tracker & Bug Reports</h2>\n<p>Encountered a bug or rendering issue? Submit an issue on our GitHub issue tracker with your OS version, GPU hardware, and terminal logs.</p>\n\n<h2 id=\"discussions\">GitHub Discussions & Community</h2>\n<p>Join the community on GitHub Discussions to share anime themes, custom plugin recipes, and feature suggestions.</p>"
  },
  "proj-changelog": {
    "id": "proj-changelog",
    "title": "Changelog",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "changelog-2-0",
        "text": "Version 2.0.0 Release Notes (August 2026)",
        "level": 2
      },
      {
        "id": "changelog-1-5",
        "text": "Version 1.5.0 Release Notes (May 2026)",
        "level": 2
      }
    ],
    "summary": "Release history and major milestone changes.",
    "body": "<h2 id=\"changelog-2-0\">Version 2.0.0 Release Notes (August 2026)</h2>\n<ul>\n  <li><strong>Direct Raster Image Rendering:</strong> Added <code>pic</code> command supporting PNG, JPEG, WebP, GIF, BMP with 32-bit RGBA hardware decoding (zero ASCII).</li>\n  <li><strong>Advanced Protocol Suite:</strong> Implemented OSC 8 Hyperlinks, OSC 52 Remote Clipboard sync, OSC 7 Working Directory tracking, and OSC 133 semantic prompt markers.</li>\n  <li><strong>Native SSH Workspace Manager:</strong> Added <code>meridian ssh</code> for direct host launching from <code>~/.ssh/config</code>.</li>\n  <li><strong>Extensible Plugin Engine:</strong> Integrated lifecycle hooks (pre/post execution, background watchers) under <code>~/.config/meridian/plugins/</code>.</li>\n  <li><strong>GPU Telemetry Profiler:</strong> Integrated live framerate, frame time, glyph cache hits, and VRAM monitoring (<code>meridian --performance</code>).</li>\n  <li><strong>Automated Test Suite:</strong> Reached 133 comprehensive unit and integration tests passing with 435 assertions.</li>\n</ul>\n\n<h2 id=\"changelog-1-5\">Version 1.5.0 Release Notes (May 2026)</h2>\n<ul>\n  <li>Initial implementation of POSIX openpty multiplexer.</li>\n  <li>Basic ANSI/VT escape sequence parsing and 24-bit TrueColor RGB.</li>\n  <li>Split panes and multi-tab interface.</li>\n</ul>"
  },
  "proj-license": {
    "id": "proj-license",
    "title": "License",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "license-spec",
        "text": "GNU General Public License v3.0",
        "level": 2
      },
      {
        "id": "permissions-conditions",
        "text": "Summary of Permissions & Obligations",
        "level": 2
      }
    ],
    "summary": "Free and open-source GNU General Public License v3.0.",
    "body": "<h2 id=\"license-spec\">GNU General Public License v3.0</h2>\n<p>Meridian Shell is free and open-source software licensed under the <strong>GNU General Public License v3.0 or later (GPL-3.0-or-later)</strong>.</p>\n\n<h2 id=\"permissions-conditions\">Summary of Permissions & Obligations</h2>\n<table class=\"doc-table\">\n  <thead><tr><th>Permissions</th><th>Conditions</th></tr></thead>\n  <tbody>\n    <tr><td>Commercial use</td><td>Disclose source code</td></tr>\n    <tr><td>Modification</td><td>License and copyright notice</td></tr>\n    <tr><td>Distribution</td><td>Same license (copyleft)</td></tr>\n    <tr><td>Private use</td><td>State changes</td></tr>\n  </tbody>\n</table>"
  },
  "proj-contributing": {
    "id": "proj-contributing",
    "title": "Contributing",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "contributor-welcome",
        "text": "Welcome Open Source Contributors!",
        "level": 2
      },
      {
        "id": "how-to-contribute",
        "text": "Ways to Contribute",
        "level": 2
      }
    ],
    "summary": "Open source contribution guide, bug fixes, and anime theme submissions.",
    "body": "<div class=\"note-box note-success\">\n  <div class=\"note-title\">Open Source Community Welcome</div>\n  <p>Contributions of all kinds are warmly welcomed! Bug fixes, rendering improvements, new anime themes, and shell features are highly encouraged.</p>\n</div>\n\n<h2 id=\"how-to-contribute\">Ways to Contribute</h2>\n<ul>\n  <li><strong>Code Contributions:</strong> Submit PRs for bug fixes, performance optimizations, or protocol enhancements.</li>\n  <li><strong>Artwork & Themes:</strong> Contribute high-resolution anime wallpapers formatted for terminal headers.</li>\n  <li><strong>Documentation:</strong> Improve tutorials, API references, or translation guides.</li>\n  <li><strong>Packaging:</strong> Help maintain native packages for your favorite Linux distribution.</li>\n</ul>"
  }
};
