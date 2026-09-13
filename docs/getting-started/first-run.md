---
layout: default
title: "First Run"
category: "GETTING STARTED"
status: "implemented"
---

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