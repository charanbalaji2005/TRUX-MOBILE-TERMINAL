---
layout: default
title: "Configuration"
category: "GETTING STARTED"
status: "implemented"
---

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