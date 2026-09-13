---
layout: default
title: "Meridian Shell"
category: "GETTING STARTED"
status: "implemented"
---

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