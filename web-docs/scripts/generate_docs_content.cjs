const fs = require('fs');
const path = require('path');

const articles = {};

// Helper to add article
function addArticle(id, title, category, status, summary, headings, body) {
  articles[id] = {
    id,
    title,
    category,
    status: status || 'implemented',
    lastUpdated: 'August 25, 2026',
    headings: headings || [],
    summary: summary || '',
    body: body || ''
  };
}

// 1. GETTING STARTED
addArticle('intro', 'Meridian Shell', 'GETTING STARTED', 'implemented',
  'A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.',
  [
    { id: 'quick-start', text: 'Quick Start', level: 2 },
    { id: 'what-is-meridian', text: 'What is Meridian?', level: 2 },
    { id: 'core-features', text: 'Core Features', level: 2 },
    { id: 'architecture', text: 'Architecture', level: 2 },
    { id: 'direct-image-rendering', text: 'Direct Image Rendering', level: 2 },
    { id: 'terminal-compatibility', text: 'Terminal Compatibility', level: 2 },
    { id: 'packaging', text: 'Packaging', level: 2 },
    { id: 'project-status', text: 'Project Status', level: 2 }
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

<div class="screenshot-container">
  <img src="./assets/meridian_screenshot.png" alt="Meridian Terminal live session running on Fedora Linux" class="meridian-live-screenshot" />
  <div class="screenshot-label">
    <span>Meridian Shell 2.0 on Fedora Linux (x86_64) running Hyprland with anime artwork header (Theme 2 / Ribbon Girl) and live Fastfetch hardware metrics.</span>
  </div>
</div>

<h2 id="quick-start">Quick Start</h2>
<p>You can run or install Meridian Shell on any Linux system through local package archives or the universal turnkey installer.</p>

<div class="note-box note-info">
  <div class="note-title">📌 Important Notice on Package Repositories</div>
  <p>Commands like <code>sudo dnf install meridian-terminal</code> require the Meridian repository to be enabled first. For immediate installation on a fresh machine without adding a repository, use the local package files or the universal curl installer below.</p>
</div>

<h3>1. Local Package Installation</h3>
<div class="code-block-wrapper"><div class="code-header"><span>Fedora / RHEL (Local RPM)</span></div><pre><code class="language-bash">sudo dnf install ./meridian-terminal.rpm</code></pre></div>
<div class="code-block-wrapper"><div class="code-header"><span>Ubuntu / Debian (Local DEB)</span></div><pre><code class="language-bash">sudo apt install ./meridian-terminal.deb</code></pre></div>
<div class="code-block-wrapper"><div class="code-header"><span>Arch Linux (Local Package)</span></div><pre><code class="language-bash">sudo pacman -U ./meridian-terminal.pkg.tar.zst</code></pre></div>

<h3>2. Universal Turnkey Installer</h3>
<div class="code-block-wrapper"><div class="code-header"><span>Bash 1-Liner</span></div><pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre></div>

<h2 id="what-is-meridian">What is Meridian?</h2>
<p><strong>Meridian Shell</strong> is an open-source terminal emulator and developer platform designed to unify the terminal layer with native graphics, extensible developer intelligence, and real pseudoterminal multiplexing.</p>

<h2 id="core-features">Core Features</h2>
<div class="feature-cards-grid">
  <div class="feat-card"><div class="feat-icon">⚡</div><div class="feat-title">Real PTY Sessions</div><div class="feat-body">Asynchronous POSIX openpty multiplexer with non-blocking I/O.</div></div>
  <div class="feat-card"><div class="feat-icon">🖥️</div><div class="feat-title">VT / ANSI Emulation</div><div class="feat-body">Full escape sequence parsing, alternate screen buffers, 24-bit TrueColor.</div></div>
  <div class="feat-card"><div class="feat-icon">🖼️</div><div class="feat-title">Direct Raster Graphics</div><div class="feat-body">Direct 32-bit RGBA inline image decoding with zero ASCII downsampling.</div></div>
  <div class="feat-card"><div class="feat-icon">🪟</div><div class="feat-title">Tabs & Panes</div><div class="feat-body">Multi-pane splits, pane zooming, and persistent workspace layouts.</div></div>
</div>

<h2 id="architecture">Architecture</h2>
<div class="arch-diagram-block"><pre><code class="language-text">Meridian Application
   ↓
Terminal UI (Qt / Canvas)
   ↓
Terminal Core (PTY, VT Parser, ScreenBuffer, Input, Shell Engine)
   ↓
Graphics / Renderer (Text Renderer, GPU Renderer, Image Renderer)
   ↓
Platform Layer (Linux, macOS, Windows)</code></pre></div>

<h2 id="direct-image-rendering">Direct Image Rendering</h2>
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
<p>Meridian is tested against standard Linux CLI applications: <code>bash</code>, <code>zsh</code>, <code>fish</code>, <code>ssh</code>, <code>sudo</code>, <code>vim</code>, <code>nano</code>, <code>tmux</code>, <code>htop</code>, <code>git</code>, <code>python</code>, <code>node</code>, <code>docker</code>.</p>

<h2 id="packaging">Packaging</h2>
<p>Available for Fedora (RPM), Debian/Ubuntu (.deb), Arch Linux (PKGBUILD), and Universal Linux tarballs.</p>

<h2 id="project-status">Project Status</h2>
<p>PTY Core: <span class="status-tag status-impl">IMPLEMENTED</span> | VT Engine: <span class="status-tag status-impl">IMPLEMENTED</span> | Direct Images: <span class="status-tag status-impl">IMPLEMENTED</span> | GPU Renderer: <span class="status-tag status-dev">DEVELOPMENT</span></p>
`
);

addArticle('installation', 'Installation & Build', 'GETTING STARTED', 'implemented',
  'Comprehensive installation options across Linux distributions, macOS, and Windows.',
  [
    { id: 'dnf-rpm', text: 'Fedora & RHEL (DNF / RPM)', level: 2 },
    { id: 'apt-deb', text: 'Debian & Ubuntu (APT / DEB)', level: 2 },
    { id: 'arch-pkg', text: 'Arch Linux (Pacman / PKGBUILD)', level: 2 },
    { id: 'universal-script', text: 'Universal 1-Line Installer', level: 2 }
  ],
  `
<p>Meridian provides native packages for all major Linux distributions.</p>
<h2 id="dnf-rpm">Fedora & RHEL (DNF / RPM)</h2>
<pre><code class="language-bash">sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>
<h2 id="apt-deb">Debian & Ubuntu (APT / DEB)</h2>
<pre><code class="language-bash">sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>
<h2 id="arch-pkg">Arch Linux (Pacman / PKGBUILD)</h2>
<pre><code class="language-bash">cd packaging/arch && makepkg -si</code></pre>
<h2 id="universal-script">Universal 1-Line Installer</h2>
<pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre>
`
);

addArticle('quickstart', 'Quick Start', 'GETTING STARTED', 'implemented',
  'Get started with Meridian Terminal in under one minute.',
  [
    { id: 'launch', text: 'Launching Meridian', level: 2 },
    { id: 'themes', text: 'Anime Theme Selection (Ctrl+P / pic)', level: 2 },
    { id: 'palette', text: 'Command Palette (Ctrl+Shift+P)', level: 2 }
  ],
  `
<h2 id="launch">Launching Meridian</h2>
<pre><code class="language-bash">meridian          # Start interactive terminal environment
meridian-shell    # Start standalone shell engine</code></pre>
<h2 id="themes">Anime Theme Selection (Ctrl+P / pic)</h2>
<pre><code class="language-bash">pic set sharingan_eye   # Sasuke/Itachi Sharingan (Theme 0)
pic set ribbon_girl     # Ribbon Girl (Theme 2)
pic set random          # Rotate themes on startup</code></pre>
<h2 id="palette">Command Palette (Ctrl+Shift+P)</h2>
<p>Press <code>Ctrl+Shift+P</code> to fuzzy-search across window splits, git inspections, error diagnoses, and themes.</p>
`
);

addArticle('first-run', 'First Run', 'GETTING STARTED', 'implemented',
  'Initial configuration directories, assets, and PATH setup.',
  [
    { id: 'paths', text: 'Default Directories & Paths', level: 2 },
    { id: 'env-setup', text: 'Environment Verification', level: 2 }
  ],
  `
<p>Upon initial launch, Meridian automatically initializes configuration files:</p>
<h2 id="paths">Default Directories & Paths</h2>
<ul>
  <li><code>~/.config/meridian/config.json</code>: Main terminal configuration.</li>
  <li><code>~/.config/meridian/gallery/</code>: Embedded anime wallpaper assets.</li>
  <li><code>~/.config/meridian/keybindings.json</code>: User custom keymap overrides.</li>
  <li><code>~/.local/share/meridian/history.db</code>: Rich history database.</li>
</ul>
<h2 id="env-setup">Environment Verification</h2>
<pre><code class="language-bash">meridian --version</code></pre>
`
);

addArticle('getting-started-config', 'Configuration', 'GETTING STARTED', 'implemented',
  'Overview of configuration options, live reload, and default values.',
  [
    { id: 'overview', text: 'Configuration Overview', level: 2 },
    { id: 'sample', text: 'Sample config.json', level: 2 }
  ],
  `
<p>Meridian uses standard JSON for configuration with automatic live reloading.</p>
<h2 id="sample">Sample config.json</h2>
<pre><code class="language-json">{
  "theme": "sharingan_eye",
  "font_family": "FantasqueSansMNFM",
  "font_size": 14,
  "cursor_shape": "block",
  "scrollback_lines": 10000,
  "ai_enabled": true
}</code></pre>
`
);

// 2. TERMINAL
addArticle('terminal-emulation', 'Terminal Emulation', 'TERMINAL', 'implemented',
  'VT100, XTerm, ANSI escapes, alternate screen buffers, and 24-bit TrueColor.',
  [
    { id: 'ansi-vt', text: 'ANSI & VT Sequences', level: 2 },
    { id: 'alt-screen', text: 'Alternate Screen Buffer', level: 2 },
    { id: 'truecolor', text: '24-Bit TrueColor RGB', level: 2 }
  ],
  `
<p>Standard-compliant VT100/XTerm terminal emulation core.</p>
<h2 id="ansi-vt">ANSI & VT Sequences</h2>
<p>Full support for cursor motion, scrolling regions, erase modes, and window titles.</p>
<h2 id="alt-screen">Alternate Screen Buffer</h2>
<p>Full support for full-screen curses applications (vim, nano, tmux, htop).</p>
<h2 id="truecolor">24-Bit TrueColor RGB</h2>
<p>Direct support for <code>\\033[38;2;R;G;Bm</code> providing 16.7M colors.</p>
`
);

addArticle('terminal-shell', 'Shell', 'TERMINAL', 'implemented',
  'Standalone POSIX-compliant AST execution engine, pipelines, and job control.',
  [
    { id: 'ast-executor', text: 'AST Parser & Executor', level: 2 },
    { id: 'pipelines', text: 'Pipelines & Redirections', level: 2 }
  ],
  `
<p>Meridian includes an internal lexer, recursive-descent parser, and AST executor.</p>
<h2 id="pipelines">Pipelines & Redirections</h2>
<pre><code class="language-bash">cat log.txt | grep -i error | wc -l > errors.txt 2>&1</code></pre>
`
);

addArticle('terminal-pty', 'PTY', 'TERMINAL', 'implemented',
  'Asynchronous Linux openpty pseudoterminal multiplexer with non-blocking I/O.',
  [
    { id: 'pty-overview', text: 'Linux openpty Architecture', level: 2 },
    { id: 'signals', text: 'Signal Handling & Resizing', level: 2 }
  ],
  `
<p>Meridian uses POSIX <code>openpty</code> to fork master/slave session pairs.</p>
<h2 id="signals">Signal Handling & Resizing</h2>
<p>Propagates <code>SIGWINCH</code> window resize notifications and forwards <code>SIGINT</code>, <code>SIGTSTP</code>, and <code>SIGQUIT</code> directly to foreground process groups.</p>
`
);

addArticle('terminal-commands', 'Commands', 'TERMINAL', 'implemented',
  'Meridian CLI commands, builtins, and subcommands.',
  [
    { id: 'cli-cmds', text: 'Core Subcommands', level: 2 }
  ],
  `
<p>Meridian provides built-in commands for diagnostics, monitor, git status, and graphics:</p>
<pre><code class="language-bash">meridian ask "find all pdf files"
meridian diag "cannot find -lutil"
meridian monitor
meridian git
meridian pic image.png</code></pre>
`
);

addArticle('terminal-ssh', 'SSH', 'TERMINAL', 'implemented',
  'SSH workspace manager, connection profiles, and remote session management.',
  [
    { id: 'ssh-overview', text: 'SSH Workspace Manager', level: 2 },
    { id: 'ssh-config', text: '~/.ssh/config Integration', level: 2 }
  ],
  `
<p>Meridian integrates with <code>~/.ssh/config</code> to provide a terminal workspace manager.</p>
<h2 id="ssh-overview">SSH Workspace Manager</h2>
<pre><code class="language-bash">meridian ssh              # List available SSH connection profiles
meridian ssh production   # Connect directly to production alias</code></pre>
`
);

addArticle('terminal-keybindings', 'Keybindings', 'TERMINAL', 'implemented',
  'Default key shortcuts for tabs, panes, theme switcher, and command palette.',
  [
    { id: 'default-keys', text: 'Keybinding Table', level: 2 }
  ],
  `
<table class="doc-table">
  <thead><tr><th>Shortcut</th><th>Action</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>Ctrl+P</code></td><td>Anime Gallery</td><td>Open theme gallery switcher</td></tr>
    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Open fuzzy search command palette</td></tr>
    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Split active terminal window vertically</td></tr>
    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Split active terminal window horizontally</td></tr>
    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search across scrollback buffer & history</td></tr>
  </tbody>
</table>
`
);

addArticle('terminal-clipboard', 'Clipboard', 'TERMINAL', 'implemented',
  'OSC 52 remote clipboard synchronization, bracketed paste, and system clipboard.',
  [
    { id: 'osc52', text: 'OSC 52 Clipboard Protocol', level: 2 }
  ],
  `
<p>Meridian supports OSC 52 (<code>\\033]52;c;&lt;base64&gt;\\033\\\\</code>) for remote clipboard copy/paste over SSH sessions seamlessly.</p>
`
);

addArticle('terminal-scrollback', 'Scrollback', 'TERMINAL', 'implemented',
  'High-capacity scrollback buffer, search highlighting, and memory limits.',
  [
    { id: 'scrollback-overview', text: 'Scrollback Engine', level: 2 }
  ],
  `
<p>Meridian stores up to 100,000 lines of scrollback history in a deque with fast substring and regex search.</p>
`
);

// 3. GRAPHICS
addArticle('graphics-gpu', 'GPU Renderer', 'GRAPHICS', 'development',
  'Hardware-accelerated shader pipeline, damage tracking, and glyph texture atlas.',
  [
    { id: 'damage-tracker', text: 'Damage Tracking Engine', level: 2 },
    { id: 'glyph-atlas', text: 'Glyph Texture Atlas', level: 2 }
  ],
  `
<p>Meridian tracks dirty screen rects to minimize draw calls and maximize battery life and framerate.</p>
`
);

addArticle('graphics-inline-images', 'Inline Images', 'GRAPHICS', 'implemented',
  'Direct 32-bit RGBA inline image decoding and rendering via pic command.',
  [
    { id: 'pic-cmd', text: 'The pic Command', level: 2 }
  ],
  `
<pre><code class="language-bash">pic image.png       # Display raw image at (x=30, y=30)
pic --debug img.png # Inspect RGBA texture format and dimensions
pic --clear         # Remove active graphic</code></pre>
`
);

addArticle('graphics-formats', 'Image Formats', 'GRAPHICS', 'implemented',
  'Supported image formats: PNG, JPEG, WebP, GIF, and BMP.',
  [
    { id: 'formats', text: 'Format Support Matrix', level: 2 }
  ],
  `
<p>Meridian uses <code>stb_image</code> to decode PNG, JPEG, WebP, GIF, and BMP directly into 32-bit RGBA pixel buffers.</p>
`
);

addArticle('graphics-gif', 'GIF', 'GRAPHICS', 'implemented',
  'Animated GIF playback and frame delay timing.',
  [
    { id: 'gif-playback', text: 'Animated GIF Engine', level: 2 }
  ],
  `
<p>Meridian decodes multi-frame GIF images and animates them at native frame delays on the canvas.</p>
`
);

addArticle('graphics-kitty', 'Kitty Graphics', 'GRAPHICS', 'implemented',
  'Chunked 2048-byte transmission protocol with m=1/m=0 control frames.',
  [
    { id: 'kitty-chunking', text: 'Chunking & Safety', level: 2 }
  ],
  `
<p>Automatically splits image payloads into 2048-byte safe chunks respecting the 4KB escape sequence limit.</p>
`
);

addArticle('graphics-sixel', 'Sixel', 'GRAPHICS', 'implemented',
  'DEC Sixel graphics protocol decoding and 256-color palette mapping.',
  [
    { id: 'sixel-overview', text: 'DEC Sixel Engine', level: 2 }
  ],
  `
<p>Decodes standard DEC Sixel bitmap escape sequences (<code>DCS q ... ST</code>) for compatibility with gnuplot and legacy tools.</p>
`
);

addArticle('graphics-rendering', 'Image Rendering', 'GRAPHICS', 'implemented',
  'Detailed comparison between direct raster rendering and character-cell approximations.',
  [
    { id: 'zero-ascii', text: 'Zero ASCII / Zero Half-Block Guarantee', level: 2 }
  ],
  `
<p>Meridian completely bypasses ASCII and half-block character approximations, rendering pure 32-bit RGBA hardware textures.</p>
`
);

// 4. INTERFACE
addArticle('interface-gui', 'GUI', 'INTERFACE', 'development',
  'Qt6, Wayland, and X11 terminal canvas and window chrome.',
  [
    { id: 'canvas', text: 'Hardware Canvas & Windowing', level: 2 }
  ],
  `
<p>The GUI frontend integrates the core PTY and VT engine into modern desktop environments.</p>
`
);

addArticle('interface-windows', 'Windows', 'INTERFACE', 'implemented',
  'Multi-window support, transparency, and blur effects.',
  [
    { id: 'windows-overview', text: 'Window Management', level: 2 }
  ],
  `
<p>Supports background transparency, acrylic/mica blur on supported compositors, and custom titles.</p>
`
);

addArticle('interface-tabs', 'Tabs', 'INTERFACE', 'implemented',
  'Tabbed terminal sessions, reordering, and shortcuts.',
  [
    { id: 'tabs-shortcuts', text: 'Tab Shortcuts', level: 2 }
  ],
  `
<p><code>Ctrl+Shift+T</code> to open new tabs, <code>Ctrl+Shift+W</code> to close tabs.</p>
`
);

addArticle('interface-panes', 'Panes', 'INTERFACE', 'implemented',
  'Multi-pane horizontal and vertical splits with persistent state.',
  [
    { id: 'panes-split', text: 'Splitting Panes', level: 2 }
  ],
  `
<p><code>Ctrl+Shift+D</code> for vertical split, <code>Ctrl+Shift+E</code> for horizontal split, <code>Ctrl+Shift+Z</code> for zoom toggle.</p>
`
);

addArticle('interface-themes', 'Themes', 'INTERFACE', 'implemented',
  '14 built-in anime picture themes and custom wallpaper configuration.',
  [
    { id: 'theme-index', text: 'Complete Theme Index', level: 2 }
  ],
  `
<table class="doc-table">
  <thead><tr><th>Index</th><th>ID</th><th>Theme</th></tr></thead>
  <tbody>
    <tr><td>0</td><td>sharingan_eye</td><td>Sasuke / Itachi Mangekyō Sharingan</td></tr>
    <tr><td>1</td><td>sakura_girl</td><td>Sakura Blossom Anime Girl</td></tr>
    <tr><td>2</td><td>ribbon_girl</td><td>Monochrome Anime Ribbon Girl</td></tr>
    <tr><td>5</td><td>gojo_purple</td><td>Gojo Satoru: Hollow Purple (JJK)</td></tr>
    <tr><td>6</td><td>sukuna_shrine</td><td>Sukuna: Malevolent Shrine (JJK)</td></tr>
    <tr><td>7</td><td>naruto_rasengan</td><td>Naruto Uzumaki: Kurama Rasengan</td></tr>
  </tbody>
</table>
`
);

addArticle('interface-powerline', 'Powerline', 'INTERFACE', 'implemented',
  'High-contrast Powerline status badges with Git branch detection.',
  [
    { id: 'badges', text: 'Powerline Badges & Icons', level: 2 }
  ],
  `
<p>Renders real-time Git branch status, OS badges (󰌽 Linux), and window status with sub-millisecond latency.</p>
`
);

// 5. DEVELOPER
addArticle('dev-architecture', 'Architecture', 'DEVELOPER', 'implemented',
  'Architecture breakdown of PTY, VT parser, ScreenBuffer, and renderer layers.',
  [
    { id: 'layers', text: 'Core Architecture Layers', level: 2 }
  ],
  `
<p>Meridian separates the UI from the underlying terminal emulation and PTY multiplexing engines.</p>
`
);

addArticle('dev-pty-arch', 'PTY Architecture', 'DEVELOPER', 'implemented',
  'POSIX openpty master/slave descriptor management and asynchronous I/O loops.',
  [
    { id: 'pty-internals', text: 'PTY Manager Internals', level: 2 }
  ],
  `
<p>Detailed design of non-blocking I/O polling, master/slave fd lifecycle, and POSIX termios flags.</p>
`
);

addArticle('dev-screen-buffer', 'Screen Buffer', 'DEVELOPER', 'implemented',
  '2D cell matrix, damage tracking, and cursor management.',
  [
    { id: 'screen-grid', text: 'Cell Grid Matrix', level: 2 }
  ],
  `
<p>Stores character codepoints, attributes, 24-bit TrueColor RGB, and hyperlink IDs per cell.</p>
`
);

addArticle('dev-renderer', 'Renderer', 'DEVELOPER', 'implemented',
  'Glyph caching, texture atlas, and GPU batching.',
  [
    { id: 'render-pipeline', text: 'Render Pipeline', level: 2 }
  ],
  `
<p>Batches draw calls and optimizes glyph atlas caching for ultra-low latency rendering.</p>
`
);

addArticle('dev-graphics-engine', 'Graphics Engine', 'DEVELOPER', 'implemented',
  'stb_image decoding, aspect fitting, and image caching.',
  [
    { id: 'graphics-core', text: 'Graphics Subsystem', level: 2 }
  ],
  `
<p>Handles direct RGBA image decoding and manages texture lifecycles.</p>
`
);

addArticle('dev-ai-engine', 'AI Engine', 'DEVELOPER', 'experimental',
  'Offline natural language intent translation, compiler diagnostics, and autonomous coding agent.',
  [
    { id: 'ask-engine', text: 'Intent Engine (ask)', level: 2 },
    { id: 'diag-engine', text: 'Compiler Diagnostics (diag)', level: 2 }
  ],
  `
<pre><code class="language-bash">meridian ask "find files larger than 1GB"
meridian diag "cannot find -lutil"</code></pre>
`
);

addArticle('dev-security', 'Security', 'DEVELOPER', 'implemented',
  'Destructive command risk classification and credential leakage redactor.',
  [
    { id: 'risk-class', text: 'Risk Classification Interceptor', level: 2 }
  ],
  `
<p>Classifies commands into Low, Medium, High, and Critical risk tiers and redacts API tokens from output.</p>
`
);

// 6. CONFIGURATION
addArticle('config-file', 'Configuration File', 'CONFIGURATION', 'implemented',
  'Specification of ~/.config/meridian/config.json options.',
  [
    { id: 'schema', text: 'Schema Specification', level: 2 }
  ],
  `
<pre><code class="language-json">{
  "theme": "sharingan_eye",
  "random_theme": false,
  "font_size": 14,
  "cursor_shape": "block",
  "scrollback_lines": 10000
}</code></pre>
`
);

addArticle('config-themes', 'Themes', 'CONFIGURATION', 'implemented',
  'Configuring anime wallpaper themes and custom wallpapers.',
  [
    { id: 'custom-art', text: 'Custom Wallpaper Paths', level: 2 }
  ],
  `
<pre><code class="language-bash">pic set /home/user/Pictures/wallpaper.png</code></pre>
`
);

addArticle('config-keybindings', 'Keybindings', 'CONFIGURATION', 'implemented',
  'Customizing keyboard shortcuts via keybindings.json.',
  [
    { id: 'custom-keys', text: 'Keybinding JSON Format', level: 2 }
  ],
  `
<p>Configure custom keybindings in <code>~/.config/meridian/keybindings.json</code>.</p>
`
);

addArticle('config-profiles', 'Profiles', 'CONFIGURATION', 'implemented',
  'Setting up multiple terminal profiles (Shell, SSH, Containers).',
  [
    { id: 'profile-setup', text: 'Profile Configurations', level: 2 }
  ],
  `
<p>Define custom shell startup commands, environment variables, and working directories per profile.</p>
`
);

addArticle('config-env', 'Environment Variables', 'CONFIGURATION', 'implemented',
  'Environment variables read and set by Meridian Terminal.',
  [
    { id: 'env-vars', text: 'Environment Variable Reference', level: 2 }
  ],
  `
<table class="doc-table">
  <thead><tr><th>Variable</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>TERM</code></td><td>Set to <code>xterm-256color</code> or <code>meridian</code></td></tr>
    <tr><td><code>COLORTERM</code></td><td>Set to <code>truecolor</code></td></tr>
    <tr><td><code>MERIDIAN_CONFIG_HOME</code></td><td>Override config directory path</td></tr>
  </tbody>
</table>
`
);

// 7. PACKAGING
addArticle('pkg-fedora', 'Fedora / RPM', 'PACKAGING', 'implemented',
  'Building and installing RPM packages with DNF and spec files.',
  [
    { id: 'dnf', text: 'DNF Installation', level: 2 }
  ],
  `
<pre><code class="language-bash">sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>
`
);

addArticle('pkg-debian', 'Ubuntu / Debian / DEB', 'PACKAGING', 'implemented',
  'Installing and building .deb packages for Ubuntu, Debian, and Linux Mint.',
  [
    { id: 'dpkg', text: 'APT / DEB Installation', level: 2 }
  ],
  `
<pre><code class="language-bash">sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>
`
);

addArticle('pkg-arch', 'Arch / AUR', 'PACKAGING', 'implemented',
  'PKGBUILD packaging and AUR installation for Arch Linux and Manjaro.',
  [
    { id: 'makepkg', text: 'Arch PKGBUILD', level: 2 }
  ],
  `
<pre><code class="language-bash">cd packaging/arch && makepkg -si</code></pre>
`
);

addArticle('pkg-opensuse', 'openSUSE', 'PACKAGING', 'implemented',
  'Installing RPM packages with Zypper on openSUSE Tumbleweed and Leap.',
  [
    { id: 'zypper', text: 'Zypper Installation', level: 2 }
  ],
  `
<pre><code class="language-bash">sudo zypper install ./meridian-terminal.rpm</code></pre>
`
);

addArticle('pkg-appimage', 'AppImage', 'PACKAGING', 'implemented',
  'Standalone self-contained AppImage package for any Linux distribution.',
  [
    { id: 'appimage-run', text: 'Running AppImage', level: 2 }
  ],
  `
<pre><code class="language-bash">chmod +x meridian-terminal.AppImage && ./meridian-terminal.AppImage</code></pre>
`
);

addArticle('pkg-flatpak', 'Flatpak', 'PACKAGING', 'planned',
  'Flatpak container package roadmap and sandbox permissions.',
  [
    { id: 'flatpak-roadmap', text: 'Flatpak Roadmap', level: 2 }
  ],
  `
<p>Flatpak manifest with Host PTY and Wayland permissions is on the roadmap.</p>
`
);

addArticle('pkg-snap', 'Snap', 'PACKAGING', 'planned',
  'Snap packaging roadmap with classic confinement.',
  [
    { id: 'snap-roadmap', text: 'Snap Package Roadmap', level: 2 }
  ],
  `
<p>Snap packaging with classic confinement is currently planned.</p>
`
);

addArticle('pkg-macos', 'macOS', 'PACKAGING', 'development',
  'macOS Darwin BSD PTY support, Homebrew formula, and DMG installer.',
  [
    { id: 'macos-status', text: 'macOS Status', level: 2 }
  ],
  `
<p>macOS Darwin BSD PTY support and Homebrew formula are under active development.</p>
`
);

addArticle('pkg-windows', 'Windows', 'PACKAGING', 'development',
  'Windows 10/11 ConPTY pseudoterminal adapter and MSI installer.',
  [
    { id: 'conpty', text: 'Windows ConPTY Core', level: 2 }
  ],
  `
<p>Windows Console PTY (ConPTY) adapter is currently in development.</p>
`
);

// 8. DEVELOPMENT
addArticle('development-building', 'Building', 'DEVELOPMENT', 'implemented',
  'Compiling Meridian Terminal with C++20, Make, and dependencies.',
  [
    { id: 'make', text: 'Make Targets', level: 2 }
  ],
  `
<pre><code class="language-bash">make all -j$(nproc)
make test
sudo ./install.sh</code></pre>
`
);

addArticle('development-testing', 'Testing', 'DEVELOPMENT', 'implemented',
  'Automated test suites covering PTY, VT parser, AST executor, and protocols.',
  [
    { id: 'run-tests', text: 'Running Tests', level: 2 }
  ],
  `
<pre><code class="language-bash">make test</code></pre>
`
);

addArticle('development-debugging', 'Debugging', 'DEVELOPMENT', 'implemented',
  'GDB debugging, ASan, Valgrind, and Telemetry profiler.',
  [
    { id: 'telemetry', text: 'Telemetry Profiler', level: 2 }
  ],
  `
<pre><code class="language-bash">meridian --performance</code></pre>
`
);

addArticle('development-contributing', 'Contributing', 'DEVELOPMENT', 'implemented',
  'How to report bugs, resolve visual glitches, add anime themes, and submit PRs.',
  [
    { id: 'pr-guide', text: 'Pull Request Guide', level: 2 }
  ],
  `
<p>Open source contributions and bug fixes are warmly welcomed! Fork and submit a pull request on GitHub.</p>
`
);

addArticle('development-release', 'Release Process', 'DEVELOPMENT', 'implemented',
  'Version tagging, packaging automation, and release checklist.',
  [
    { id: 'release-flow', text: 'Release Checklist', level: 2 }
  ],
  `
<p>Release workflow generates RPM, DEB, Arch, and offline tarballs automatically.</p>
`
);

// 9. REFERENCE
addArticle('ref-cli', 'CLI Reference', 'REFERENCE', 'implemented',
  'Complete command-line interface options and subcommands.',
  [
    { id: 'subcommands-ref', text: 'Subcommands Index', level: 2 }
  ],
  `
<table class="doc-table">
  <thead><tr><th>Command</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>meridian</code></td><td>Launch interactive terminal</td></tr>
    <tr><td><code>meridian ssh [alias]</code></td><td>Manage & connect to SSH workspace</td></tr>
    <tr><td><code>meridian plugins</code></td><td>List active extensible plugins</td></tr>
    <tr><td><code>meridian --performance</code></td><td>Display GPU & PTY telemetry profiler</td></tr>
    <tr><td><code>meridian pic &lt;file&gt;</code></td><td>Display direct inline raster image</td></tr>
  </tbody>
</table>
`
);

addArticle('ref-config', 'Configuration Reference', 'REFERENCE', 'implemented',
  'Complete schema documentation for config.json.',
  [
    { id: 'keys-ref', text: 'Config Keys Table', level: 2 }
  ],
  `
<table class="doc-table">
  <thead><tr><th>Key</th><th>Type</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>theme</code></td><td>string</td><td>Startup anime artwork theme</td></tr>
    <tr><td><code>font_size</code></td><td>number</td><td>Font size in points</td></tr>
    <tr><td><code>scrollback_lines</code></td><td>number</td><td>Max scrollback history lines</td></tr>
  </tbody>
</table>
`
);

addArticle('ref-shortcuts', 'Keyboard Shortcuts', 'REFERENCE', 'implemented',
  'Complete table of all default keyboard shortcuts.',
  [
    { id: 'shortcuts-ref', text: 'Shortcut Reference', level: 2 }
  ],
  `
<table class="doc-table">
  <thead><tr><th>Shortcut</th><th>Action</th></tr></thead>
  <tbody>
    <tr><td><code>Ctrl+P</code></td><td>Anime Theme Gallery</td></tr>
    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td></tr>
    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td></tr>
    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td></tr>
    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td></tr>
  </tbody>
</table>
`
);

addArticle('ref-env', 'Environment Variables', 'REFERENCE', 'implemented',
  'Environment variables used and exported by Meridian.',
  [
    { id: 'env-ref', text: 'Environment Variables', level: 2 }
  ],
  `
<p><code>TERM=xterm-256color</code>, <code>COLORTERM=truecolor</code>, <code>MERIDIAN_CONFIG_HOME</code>.</p>
`
);

addArticle('ref-faq', 'FAQ', 'REFERENCE', 'implemented',
  'Frequently asked questions about Meridian Shell.',
  [
    { id: 'general-faq', text: 'General Questions', level: 2 }
  ],
  `
<p><strong>Q: Does Meridian render real images or ASCII?</strong><br/>A: Meridian renders direct 32-bit RGBA hardware raster graphics with zero ASCII conversion.</p>
`
);

addArticle('ref-troubleshooting', 'Troubleshooting', 'REFERENCE', 'implemented',
  'Solutions for font rendering, PTY permissions, and graphical display issues.',
  [
    { id: 'common-issues', text: 'Common Solutions', level: 2 }
  ],
  `
<p>Ensure Nerd Fonts are installed for Powerline glyphs, and verify GPU drivers for hardware raster graphics.</p>
`
);

// 10. PROJECT
addArticle('proj-github', 'GitHub', 'PROJECT', 'implemented',
  'Source code repository, issue tracker, and community discussions.',
  [
    { id: 'repo-links', text: 'GitHub Links', level: 2 }
  ],
  `
<p>Repository: <a href="https://github.com/charanbalaji2005/Meridian-Shell" target="_blank" style="color: #00E5FF;">https://github.com/charanbalaji2005/Meridian-Shell</a></p>
`
);

addArticle('proj-changelog', 'Changelog', 'PROJECT', 'implemented',
  'Release history and major milestone changes.',
  [
    { id: 'v2-0', text: 'Meridian 2.0.0 (August 2026)', level: 2 }
  ],
  `
<h2 id="v2-0">Meridian 2.0.0 (August 2026)</h2>
<ul>
  <li>Direct 32-bit RGBA inline image decoding engine (zero ASCII).</li>
  <li>OSC 8 Hyperlinks & OSC 52 Remote Clipboard.</li>
  <li>OSC 7 Working Directory tracking & OSC 133 Semantic Prompts.</li>
  <li>Native SSH Workspace Manager & Extensible Plugin Engine.</li>
  <li>GPU Telemetry Profiler (meridian --performance).</li>
</ul>
`
);

addArticle('proj-license', 'License', 'PROJECT', 'implemented',
  'Free and open-source GNU General Public License v3.0.',
  [
    { id: 'gpl', text: 'GNU GPLv3 License', level: 2 }
  ],
  `
<p>Meridian Terminal is free and open-source software licensed under the <strong>GNU General Public License v3.0 or later</strong>.</p>
`
);

addArticle('proj-contributing', 'Contributing', 'PROJECT', 'implemented',
  'Open source contribution guide, bug fixes, and anime theme submissions.',
  [
    { id: 'welcome', text: 'Welcome Contributors!', level: 2 }
  ],
  `
<div class="note-box note-success">
  <div class="note-title">🤝 Open Source Community Welcome!</div>
  <p>Contributions of all kinds are warmly welcomed! Bug fixes, rendering improvements, new anime themes, and shell features are highly encouraged.</p>
</div>
`
);

// Generate final TypeScript content
const tsContent = `export interface DocArticle {
  id: string;
  title: string;
  category: string;
  status?: 'implemented' | 'development' | 'experimental' | 'planned';
  lastUpdated: string;
  headings: { id: string; text: string; level: number }[];
  summary: string;
  body: string;
}

export const DOCS_ARTICLES: Record<string, DocArticle> = ${JSON.stringify(articles, null, 2)};
`;

fs.writeFileSync(path.join(__dirname, '../src/data/docsContent.ts'), tsContent);
console.log(`Successfully generated ${Object.keys(articles).length} complete articles in docsContent.ts!`);
