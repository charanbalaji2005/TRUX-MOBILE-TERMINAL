---
layout: default
title: "Quick Start"
category: "GETTING STARTED"
status: "implemented"
---

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