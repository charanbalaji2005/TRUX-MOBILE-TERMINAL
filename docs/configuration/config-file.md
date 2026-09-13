---
layout: default
title: "Configuration File"
category: "CONFIGURATION"
status: "implemented"
---

<h2 id="config-schema">Complete JSON Schema</h2>
<p>Location: <code>~/.config/meridian/config.json</code></p>
<pre><code class="language-json">{
  "theme": "sharingan_eye",
  "random_theme": false,
  "font_family": "FantasqueSansMNFM",
  "font_size": 14,
  "line_height": 1.2,
  "cursor_shape": "block",
  "cursor_blink": true,
  "scrollback_lines": 10000,
  "opacity": 0.95,
  "blur": true,
  "ai_enabled": true,
  "telemetry_hud": false
}</code></pre>

<h2 id="options-table">Configuration Options Reference</h2>
<table class="doc-table">
  <thead><tr><th>Key</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>theme</code></td><td>string</td><td><code>"sharingan_eye"</code></td><td>Default startup anime artwork theme.</td></tr>
    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Terminal font size in points.</td></tr>
    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Maximum scrollback buffer lines per pane.</td></tr>
    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity (0.0 - 1.0).</td></tr>
    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>
    <tr><td><code>cursor_shape</code></td><td>string</td><td><code>"block"</code></td><td>Cursor shape: "block", "beam", or "underline".</td></tr>
  </tbody>
</table>

<h2 id="validation-rules">JSON Schema Validation & Fallbacks</h2>
<p>Meridian automatically validates config files against an internal schema on startup. If a syntax error is present, Meridian logs a warning and falls back to safe defaults without crashing.</p>