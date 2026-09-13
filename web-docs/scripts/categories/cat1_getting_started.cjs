module.exports = function(addArt) {
  addArt('intro', 'Meridian Shell', 'GETTING STARTED', 'implemented',
    'A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.',
    [
      { id: 'quick-start', text: 'Quick Start', level: 2 },
      { id: 'what-is-meridian', text: 'What is Meridian?', level: 2 },
      { id: 'core-features', text: 'Core Features', level: 2 },
      { id: 'architecture', text: 'System Architecture', level: 2 },
      { id: 'direct-image-rendering', text: 'Direct Raster Image Rendering', level: 2 },
      { id: 'terminal-compatibility', text: 'Terminal Compatibility', level: 2 },
      { id: 'packaging', text: 'Distribution & Packaging', level: 2 },
      { id: 'project-status', text: 'Project Status Matrix', level: 2 }
    ],
    `
<div class="hero-section">
  <p class="hero-lead">
    A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.
  </p>
  <div class="hero-actions">
    <a href="#installation" class="btn-primary">Get Started</a>
    <a href="#development-building" class="btn-secondary">Build from Source</a>
    <a href="https://github.com/charanbalaji2005/Meridian-Shell" target="_blank" rel="noreferrer" class="btn-outline">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 22v-4a4.8 4.8 0 0 0-1-3.5c3 0 6-2 6-5.5.08-1.25-.27-2.48-1-3.5.28-1.15.28-2.35 0-3.5 0 0-1 0-3 1.5-2.64-.5-5.36-.5-8 0C6 2 5 2 5 2c-.3 1.15-.3 2.35 0 3.5A5.403 5.403 0 0 0 4 9c0 3.5 3 5.5 6 5.5-.39.49-.68 1.05-.85 1.65-.17.6-.22 1.23-.15 1.85v4"/><path d="M9 18c-4.51 2-5-2-7-2"/></svg>
      GitHub
    </a>
  </div>
</div>

<div class="terminal-screenshot-card">
  <img src="./assets/meridian-terminal-screenshot-1.png" alt="Meridian Terminal Live Interface with Anime Artwork and System Telemetry" class="terminal-screenshot-img" />
  <div class="terminal-screenshot-caption">
    Meridian Shell running live interactive session with GPU-accelerated raster artwork header, multi-badge Powerline status, and system telemetry.
  </div>
</div>

<div class="terminal-screenshot-card">
  <img src="./assets/meridian-terminal-screenshot-2.png" alt="Meridian Terminal Split Panes and Fastfetch Metrics" class="terminal-screenshot-img" />
  <div class="terminal-screenshot-caption">
    Meridian Terminal displaying custom anime wallpapers, live Git branch divergence, and system metrics on Fedora Linux.
  </div>
</div>

<h2 id="quick-start">Quick Start</h2>
<p>Meridian Shell is available for Fedora, Ubuntu, Debian, Arch Linux, openSUSE, and generic Linux platforms. Choose your preferred installation method below:</p>

<div class="note-box note-info">
  <div class="note-title">📌 Important Notice on Package Repositories</div>
  <p>Commands like <code>sudo dnf install meridian-terminal</code> require the Meridian repository to be enabled first. For immediate installation on a fresh machine without adding a repository, use the local package files or the universal curl installer below.</p>
</div>

<h3>1. Local Package Installation</h3>
<div class="code-block-wrapper"><div class="code-header"><span>Fedora / RHEL (Local RPM)</span></div><pre><code class="language-bash">sudo dnf install ./meridian-terminal.rpm</code></pre></div>
<div class="code-block-wrapper"><div class="code-header"><span>Ubuntu / Debian (Local DEB)</span></div><pre><code class="language-bash">sudo apt install ./meridian-terminal.deb</code></pre></div>
<div class="code-block-wrapper"><div class="code-header"><span>Arch Linux (Local Package)</span></div><pre><code class="language-bash">sudo pacman -U ./meridian-terminal.pkg.tar.zst</code></pre></div>

<h3>2. Universal Turnkey Installer</h3>
<div class="code-block-wrapper"><div class="code-header"><span>Bash 1-Liner (User)</span></div><pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre></div>
<div class="code-block-wrapper"><div class="code-header"><span>Bash 1-Liner (System-Wide)</span></div><pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash</code></pre></div>

<h2 id="what-is-meridian">What is Meridian?</h2>
<p><strong>Meridian Shell</strong> is an open-source terminal emulator and developer platform designed to unify the terminal layer with native graphics, extensible developer intelligence, and real pseudoterminal multiplexing. Rather than being a simulated web terminal or electron wrapper, Meridian interacts directly with the Linux kernel via POSIX <code>openpty</code>, supporting full interactive terminal workflows.</p>

<h2 id="core-features">Core Features</h2>
<div class="feature-cards-grid">
  <div class="feat-card"><div class="feat-icon">⚡</div><div class="feat-title">Real PTY Sessions</div><div class="feat-body">Asynchronous POSIX openpty multiplexer with non-blocking I/O and process group signal forwarding.</div></div>
  <div class="feat-card"><div class="feat-icon">🖥️</div><div class="feat-title">VT / ANSI Emulation</div><div class="feat-body">Full escape sequence parsing, alternate screen buffers, 24-bit TrueColor RGB, and cursor addressing.</div></div>
  <div class="feat-card"><div class="feat-icon">🖼️</div><div class="feat-title">Direct Raster Graphics</div><div class="feat-body">Direct 32-bit RGBA inline image decoding with zero ASCII downsampling or character-block compromises.</div></div>
  <div class="feat-card"><div class="feat-icon">🪟</div><div class="feat-title">Tabs & Panes</div><div class="feat-body">Multi-pane splits (Ctrl+Shift+D/E), pane zooming, and persistent workspace layouts across restarts.</div></div>
  <div class="feat-card"><div class="feat-icon">🌐</div><div class="feat-title">Advanced Protocols</div><div class="feat-body">OSC 8 clickable hyperlinks, OSC 52 remote clipboard sync, OSC 7 CWD tracking, and OSC 133 prompt markers.</div></div>
  <div class="feat-card"><div class="feat-icon">🔌</div><div class="feat-title">Extensible Plugins</div><div class="feat-body">Lifecycle hooks for command execution, background watchers, and custom telemetry overlays.</div></div>
</div>

<h2 id="architecture">System Architecture</h2>
<p>Meridian is engineered as decoupled subsystems to guarantee extreme stability and fault isolation:</p>
<div class="arch-diagram-block"><pre><code class="language-text">Meridian Application
   │
   ├── GUI Frontend (Qt6 / Wayland / X11 Canvas)
   │
   ├── Terminal Session Manager
   │   ├── POSIX openpty Multiplexer (Master/Slave FDs)
   │   ├── Real Foreground Shell Process (bash, zsh, fish)
   │   └── Non-blocking Epoll/Kqueue I/O Loop
   │
   ├── Terminal Core Engine
   │   ├── VT / ANSI State Machine (VT100, VT220, XTerm)
   │   ├── ScreenBuffer Matrix (2D Cell Grid + Attributes)
   │   ├── Hyperlink Table & OSC Protocol Handlers
   │   └── High-Capacity Scrollback Buffer
   │
   ├── Hardware Renderer
   │   ├── GPU Render Pipeline & Damage Rect Tracker
   │   ├── Sub-pixel Glyph Texture Atlas (Freetype/Harfbuzz)
   │   └── Direct 32-bit RGBA Image Texture Compositor
   │
   └── Developer Intelligence & Security
       ├── Native SSH Workspace Manager (~/.ssh/config)
       ├── Extensible Plugin Engine (~/.config/meridian/plugins/)
       ├── Risk Classification Interceptor & Secret Redactor
       └── GPU Telemetry & Performance Profiler</code></pre></div>

<h2 id="direct-image-rendering">Direct Raster Image Rendering</h2>
<p>Executing <code>pic image.png</code> produces a <strong>direct full-color raster image</strong> on the terminal canvas.</p>
<div class="flow-diagram">
  <div class="flow-box">PNG / JPEG / WebP / BMP</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">graphics::ImageDecoder (stb_image)</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">32-bit RGBA Pixels</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">GPU Texture</div>
  <div class="flow-sep">→</div>
  <div class="flow-box highlight-box">ACTUAL IMAGE PIXELS ON SCREEN</div>
</div>

<h2 id="terminal-compatibility">Terminal Compatibility</h2>
<p>Meridian is tested against standard Linux CLI applications and full-screen TUI workflows:</p>
<ul>
  <li><strong>Shells:</strong> <code>bash</code>, <code>zsh</code>, <code>fish</code>, <code>sh</code></li>
  <li><strong>Editors:</strong> <code>vim</code>, <code>neovim</code>, <code>nano</code>, <code>emacs</code>, <code>micro</code></li>
  <li><strong>Multiplexers:</strong> <code>tmux</code>, <code>zellij</code>, <code>screen</code></li>
  <li><strong>System Monitors:</strong> <code>top</code>, <code>htop</code>, <code>btop</code>, <code>glances</code>, <code>fastfetch</code></li>
  <li><strong>Tools & Runtimes:</strong> <code>git</code>, <code>ssh</code>, <code>docker</code>, <code>podman</code>, <code>python</code>, <code>node</code>, <code>cargo</code></li>
</ul>

<h2 id="packaging">Distribution & Packaging</h2>
<p>Available for Fedora (RPM), Debian/Ubuntu (.deb), Arch Linux (PKGBUILD), and Universal Linux tarballs.</p>

<h2 id="project-status">Project Status Matrix</h2>
<table class="doc-table">
  <thead><tr><th>Subsystem</th><th>Status</th><th>Notes</th></tr></thead>
  <tbody>
    <tr><td>PTY Core & Job Control</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>Async openpty, process group signals (SIGINT, SIGTSTP, SIGWINCH).</td></tr>
    <tr><td>VT Engine & ANSI Parser</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>VT100, VT220, TrueColor RGB, Alternate Buffer, OSC 7/8/52/133.</td></tr>
    <tr><td>Direct Raster Images (pic)</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>32-bit RGBA hardware texture rendering (zero ASCII).</td></tr>
    <tr><td>SSH Workspace Manager</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>Direct ~/.ssh/config parser and connection launcher.</td></tr>
    <tr><td>Extensible Plugin Engine</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>Lifecycle hooks for pre/post command execution.</td></tr>
    <tr><td>GPU Telemetry Profiler</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>Real-time FPS, frame time, glyph count, VRAM, and PTY latency.</td></tr>
    <tr><td>GUI Windowing & Canvas</td><td><span class="status-tag status-dev">DEVELOPMENT</span></td><td>Qt6 / Wayland / X11 multi-window canvas integration.</td></tr>
    <tr><td>Kitty Graphics Protocol</td><td><span class="status-tag status-dev">DEVELOPMENT</span></td><td>2048-byte safe chunked APC transmission parser.</td></tr>
    <tr><td>DEC Sixel Graphics</td><td><span class="status-tag status-dev">DEVELOPMENT</span></td><td>DCS bitmap stream decoder with 256-color palette.</td></tr>
  </tbody>
</table>
`
  );

  addArt('installation', 'Installation & Build', 'GETTING STARTED', 'implemented',
    'Comprehensive installation options across Linux distributions, macOS, and Windows.',
    [
      { id: 'dnf-rpm', text: 'Fedora & RHEL (DNF / RPM)', level: 2 },
      { id: 'apt-deb', text: 'Ubuntu & Debian (APT / DEB)', level: 2 },
      { id: 'arch-pkg', text: 'Arch Linux (Pacman / PKGBUILD)', level: 2 },
      { id: 'opensuse-pkg', text: 'openSUSE (Zypper / RPM)', level: 2 },
      { id: 'universal-script', text: 'Universal Turnkey Installer', level: 2 },
      { id: 'building-from-source', text: 'Compiling from Source', level: 2 },
      { id: 'dependencies-table', text: 'Build & Runtime Dependencies', level: 2 },
      { id: 'uninstalling', text: 'Uninstallation & Purge', level: 2 }
    ],
    `
<p>Meridian provides native packages for all major Linux distributions, as well as a standalone universal installer.</p>

<h2 id="dnf-rpm">Fedora & RHEL (DNF / RPM)</h2>
<p>To install a locally downloaded RPM package on Fedora 39, 40, 41, or 44:</p>
<pre><code class="language-bash"># Install local RPM archive
sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm

# Or using rpm directly
sudo rpm -Uvh ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>

<div class="note-box note-warning">
  <div class="note-title">⚠️ Repository Installation Requirement</div>
  <p>To install via <code>sudo dnf install meridian-terminal</code> without specifying a local file path, the official Meridian COPR / RPM repository must be added to <code>/etc/yum.repos.d/</code>.</p>
</div>

<h2 id="apt-deb">Ubuntu & Debian (APT / DEB)</h2>
<p>For Debian 12+, Ubuntu 22.04 LTS, Ubuntu 24.04 LTS, and Linux Mint:</p>
<pre><code class="language-bash">sudo apt update
sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>

<h2 id="arch-pkg">Arch Linux (Pacman / PKGBUILD)</h2>
<p>On Arch Linux, Manjaro, or EndeavourOS:</p>
<pre><code class="language-bash">cd packaging/arch
makepkg -si</code></pre>

<h2 id="opensuse-pkg">openSUSE (Zypper / RPM)</h2>
<p>On openSUSE Tumbleweed or Leap:</p>
<pre><code class="language-bash">sudo zypper install ./meridian-terminal.rpm</code></pre>

<h2 id="universal-script">Universal Turnkey Installer</h2>
<p>The universal installer detects your distribution, installs missing dependencies, compiles the latest release, and places binary launchers into <code>/usr/local/bin</code>:</p>
<pre><code class="language-bash"># Standard user installation (~/.local/bin)
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash

# System-wide installation (/usr/local/bin)
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash</code></pre>

<h2 id="building-from-source">Compiling from Source</h2>
<p>Building Meridian from source requires a modern C++20 compiler (GCC 11+ or Clang 13+):</p>
<pre><code class="language-bash">git clone https://github.com/charanbalaji2005/Meridian-Shell.git
cd Meridian-Shell

# Build all binaries (terminal, shell, tests, demo)
make all -j$(nproc)

# Run test suite (133 tests)
make test

# Install to system
sudo make install</code></pre>

<h2 id="dependencies-table">Build & Runtime Dependencies</h2>
<table class="doc-table">
  <thead><tr><th>Package</th><th>Fedora Package</th><th>Debian/Ubuntu Package</th><th>Purpose</th></tr></thead>
  <tbody>
    <tr><td>C++20 Compiler</td><td><code>gcc-c++</code></td><td><code>g++</code></td><td>Core compilation</td></tr>
    <tr><td>POSIX Libutil</td><td><code>glibc-devel</code></td><td><code>libc6-dev</code></td><td>openpty and termios</td></tr>
    <tr><td>Make</td><td><code>make</code></td><td><code>make</code></td><td>Build automation</td></tr>
    <tr><td>Nerd Font</td><td><code>google-noto-sans-fonts</code></td><td><code>fonts-noto</code></td><td>Powerline & glyph rendering</td></tr>
  </tbody>
</table>

<h2 id="uninstalling">Uninstallation & Purge</h2>
<p>Meridian provides a clean uninstaller script that removes all binaries, desktop entries, and icons:</p>
<pre><code class="language-bash"># Standard uninstallation (preserves ~/.config/meridian configs)
sudo ./uninstall.sh

# Complete purge (removes binaries AND config/history databases)
sudo ./uninstall.sh --purge

# For DNF package installations
sudo dnf remove meridian-terminal</code></pre>
`
  );

  addArt('quickstart', 'Quick Start', 'GETTING STARTED', 'implemented',
    'Get started with Meridian Terminal in under one minute.',
    [
      { id: 'launch', text: 'Launching Meridian', level: 2 },
      { id: 'first-commands', text: 'First Commands', level: 2 },
      { id: 'themes', text: 'Anime Theme Selection (Ctrl+P / pic)', level: 2 },
      { id: 'palette', text: 'Command Palette (Ctrl+Shift+P)', level: 2 },
      { id: 'splits', text: 'Window Splits & Panes', level: 2 },
      { id: 'essential-shortcuts', text: 'Essential Daily Shortcuts', level: 2 }
    ],
    `
<h2 id="launch">Launching Meridian</h2>
<p>Launch Meridian from your application menu or terminal:</p>
<pre><code class="language-bash"># Launch full interactive terminal emulator
meridian

# Launch standalone shell engine directly
meridian-shell</code></pre>

<h2 id="first-commands">First Commands</h2>
<p>Try running these built-in developer commands inside Meridian:</p>
<pre><code class="language-bash">meridian monitor        # Live CPU, RAM, Disk, Network, and Process metrics
meridian git            # Visual Git branch divergence and staged/unstaged changes
meridian files          # Interactive tree file explorer with git badges
meridian ssh            # List and connect to SSH remote workspaces
meridian --performance  # Live GPU framerate, PTY latency & telemetry HUD</code></pre>

<h2 id="themes">Anime Theme Selection (Ctrl+P / pic)</h2>
<p>Press <code>Ctrl+P</code> to browse the theme gallery or configure your startup anime wallpaper via the CLI:</p>
<pre><code class="language-bash">pic set sharingan_eye   # Sasuke/Itachi Mangekyō Sharingan (Theme 0)
pic set sakura_girl     # Sakura Blossom Anime Girl (Theme 1)
pic set ribbon_girl     # Monochrome Anime Ribbon Girl (Theme 2)
pic set gojo_purple     # Gojo Satoru: Hollow Purple (Theme 5)
pic set sukuna_shrine   # Sukuna: Malevolent Shrine (Theme 6)
pic set random          # Automatically rotate theme on each startup</code></pre>

<h2 id="palette">Command Palette (Ctrl+Shift+P)</h2>
<p>Press <code>Ctrl+Shift+P</code> to fuzzy-search across all terminal actions, window splits, theme changes, and diagnostics without leaving the keyboard.</p>

<h2 id="splits">Window Splits & Panes</h2>
<ul>
  <li><code>Ctrl+Shift+D</code>: Split active pane vertically.</li>
  <li><code>Ctrl+Shift+E</code>: Split active pane horizontally.</li>
  <li><code>Ctrl+Shift+Z</code>: Toggle zoom on active pane.</li>
  <li><code>Ctrl+Shift+W</code>: Close current pane.</li>
</ul>

<h2 id="essential-shortcuts">Essential Daily Shortcuts</h2>
<table class="doc-table">
  <thead><tr><th>Shortcut</th><th>Action</th></tr></thead>
  <tbody>
    <tr><td><code>Ctrl+Shift+T</code></td><td>Open new terminal tab</td></tr>
    <tr><td><code>Ctrl+Shift+F</code></td><td>Find in terminal scrollback</td></tr>
    <tr><td><code>Ctrl+Shift+C</code> / <code>Ctrl+Shift+V</code></td><td>Copy / Paste</td></tr>
    <tr><td><code>Ctrl+L</code></td><td>Clear screen preserving scrollback</td></tr>
  </tbody>
</table>
`
  );

  addArt('first-run', 'First Run', 'GETTING STARTED', 'implemented',
    'Initial configuration directories, assets, and PATH setup.',
    [
      { id: 'paths', text: 'Default Directories & Paths', level: 2 },
      { id: 'initial-setup', text: 'Automatic Initialization', level: 2 },
      { id: 'env-setup', text: 'Environment Verification', level: 2 },
      { id: 'first-run-checklist', text: 'First-Run Checklist', level: 2 }
    ],
    `
<p>Upon initial launch, Meridian automatically initializes user configuration files and assets in standard XDG locations.</p>

<h2 id="paths">Default Directories & Paths</h2>
<table class="doc-table">
  <thead><tr><th>Path</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>~/.config/meridian/config.json</code></td><td>Main terminal settings, font preferences, and theme configuration.</td></tr>
    <tr><td><code>~/.config/meridian/keybindings.json</code></td><td>Custom keyboard shortcut overrides.</td></tr>
    <tr><td><code>~/.config/meridian/gallery/</code></td><td>Embedded high-resolution anime artwork wallpaper assets.</td></tr>
    <tr><td><code>~/.config/meridian/plugins/</code></td><td>Directory for user-installed extensible Meridian plugins.</td></tr>
    <tr><td><code>~/.local/share/meridian/history.db</code></td><td>Rich command history SQLite database with durations and exit codes.</td></tr>
  </tbody>
</table>

<h2 id="initial-setup">Automatic Initialization</h2>
<p>If <code>~/.config/meridian/config.json</code> does not exist, Meridian populates it with standard defaults:</p>
<pre><code class="language-json">{
  "theme": "sharingan_eye",
  "random_theme": false,
  "font_family": "FantasqueSansMNFM",
  "font_size": 14,
  "cursor_shape": "block",
  "cursor_blink": true,
  "scrollback_lines": 10000,
  "opacity": 0.95,
  "blur": true
}</code></pre>

<h2 id="env-setup">Environment Verification</h2>
<pre><code class="language-bash"># Verify Meridian version
meridian --version

# Verify truecolor terminal support
echo "TERM=$TERM COLORTERM=$COLORTERM"</code></pre>

<h2 id="first-run-checklist">First-Run Checklist</h2>
<ul>
  <li>Ensure your shell defaults to <code>/bin/bash</code>, <code>/bin/zsh</code>, or <code>/usr/bin/fish</code>.</li>
  <li>Install a Nerd Font (e.g., Fantasque Sans Mono or JetBrains Mono) for full Powerline icon rendering.</li>
  <li>Test direct raster image decoding by running <code>pic ~/.config/meridian/gallery/sharingan_eye.png</code>.</li>
</ul>
`
  );

  addArt('getting-started-config', 'Configuration', 'GETTING STARTED', 'implemented',
    'Overview of configuration options, live reload, and default values.',
    [
      { id: 'overview', text: 'Configuration Architecture', level: 2 },
      { id: 'live-reload', text: 'Live Configuration Reloading', level: 2 },
      { id: 'sample', text: 'Comprehensive config.json Specification', level: 2 },
      { id: 'options-table', text: 'Key Configuration Properties', level: 2 }
    ],
    `
<h2 id="overview">Configuration Architecture</h2>
<p>Meridian uses standard JSON for configuration with schema validation. Settings are divided into core terminal emulation, appearance, keybindings, and developer tools.</p>

<h2 id="live-reload">Live Configuration Reloading</h2>
<p>Changes saved to <code>~/.config/meridian/config.json</code> are watched using inotify and reloaded instantly without restarting your active terminal sessions or dropping PTY connections.</p>

<h2 id="sample">Comprehensive config.json Specification</h2>
<pre><code class="language-json">{
  "theme": "sharingan_eye",
  "random_theme": false,
  "font_family": "FantasqueSansMNFM",
  "font_size": 14,
  "line_height": 1.2,
  "cursor_shape": "block",
  "cursor_blink": true,
  "scrollback_lines": 10000,
  "window_padding": { "top": 8, "bottom": 8, "left": 10, "right": 10 },
  "opacity": 0.95,
  "blur": true,
  "ai_enabled": true,
  "ai_auto_suggest": true,
  "telemetry_hud": false
}</code></pre>

<h2 id="options-table">Key Configuration Properties</h2>
<table class="doc-table">
  <thead><tr><th>Property</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>theme</code></td><td>string</td><td><code>"sharingan_eye"</code></td><td>Default anime wallpaper theme.</td></tr>
    <tr><td><code>random_theme</code></td><td>boolean</td><td><code>false</code></td><td>Rotate anime wallpaper on startup.</td></tr>
    <tr><td><code>font_family</code></td><td>string</td><td><code>"FantasqueSansMNFM"</code></td><td>Primary font family name.</td></tr>
    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Font size in points.</td></tr>
    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Lines retained in history deque.</td></tr>
    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity.</td></tr>
    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>
  </tbody>
</table>
`
  );
};
