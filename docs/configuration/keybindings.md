---
layout: default
title: "Keybindings"
category: "CONFIGURATION"
status: "implemented"
---

<h2 id="keymap-json">keybindings.json Specification</h2>
<p>Location: <code>~/.config/meridian/keybindings.json</code></p>
<pre><code class="language-json">{
  "new_tab": "Ctrl+Shift+T",
  "close_tab": "Ctrl+Shift+W",
  "split_vertical": "Ctrl+Shift+D",
  "split_horizontal": "Ctrl+Shift+E",
  "command_palette": "Ctrl+Shift+P",
  "theme_gallery": "Ctrl+P",
  "search_scrollback": "Ctrl+Shift+F"
}</code></pre>

<h2 id="available-actions">Available Action Identifiers</h2>
<table class="doc-table">
  <thead><tr><th>Action ID</th><th>Default Shortcut</th><th>Effect</th></tr></thead>
  <tbody>
    <tr><td><code>new_tab</code></td><td><code>Ctrl+Shift+T</code></td><td>Creates a new shell tab.</td></tr>
    <tr><td><code>close_tab</code></td><td><code>Ctrl+Shift+W</code></td><td>Closes the current tab/pane.</td></tr>
    <tr><td><code>split_vertical</code></td><td><code>Ctrl+Shift+D</code></td><td>Splits active pane vertically.</td></tr>
    <tr><td><code>split_horizontal</code></td><td><code>Ctrl+Shift+E</code></td><td>Splits active pane horizontally.</td></tr>
    <tr><td><code>toggle_zoom</code></td><td><code>Ctrl+Shift+Z</code></td><td>Toggles fullscreen pane zoom.</td></tr>
  </tbody>
</table>