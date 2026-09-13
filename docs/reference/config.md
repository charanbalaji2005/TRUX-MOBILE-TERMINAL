---
layout: default
title: "Configuration Reference"
category: "REFERENCE"
status: "implemented"
---

<h2 id="config-keys-ref">Configuration Keys Reference Table</h2>
<table class="doc-table">
  <thead><tr><th>Key</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>theme</code></td><td>string</td><td><code>"sharingan_eye"</code></td><td>Default anime wallpaper theme identifier.</td></tr>
    <tr><td><code>random_theme</code></td><td>boolean</td><td><code>false</code></td><td>Rotate anime wallpaper on startup.</td></tr>
    <tr><td><code>font_family</code></td><td>string</td><td><code>"FantasqueSansMNFM"</code></td><td>Primary font family name.</td></tr>
    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Font size in points.</td></tr>
    <tr><td><code>line_height</code></td><td>number</td><td><code>1.2</code></td><td>Line height multiplier.</td></tr>
    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Maximum scrollback buffer lines.</td></tr>
    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity (0.0 to 1.0).</td></tr>
    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>
    <tr><td><code>cursor_shape</code></td><td>string</td><td><code>"block"</code></td><td>Cursor shape: "block", "beam", or "underline".</td></tr>
    <tr><td><code>cursor_blink</code></td><td>boolean</td><td><code>true</code></td><td>Enable cursor blinking animation.</td></tr>
    <tr><td><code>ai_enabled</code></td><td>boolean</td><td><code>true</code></td><td>Enable local offline AI assistance.</td></tr>
    <tr><td><code>telemetry_hud</code></td><td>boolean</td><td><code>false</code></td><td>Show live GPU telemetry HUD overlay.</td></tr>
  </tbody>
</table>