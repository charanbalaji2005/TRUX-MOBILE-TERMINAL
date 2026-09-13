---
layout: default
title: "Keybindings"
category: "TERMINAL"
status: "implemented"
---

<h2 id="shortcuts-table">Default Keyboard Shortcuts Table</h2>
<table class="doc-table">
  <thead><tr><th>Keybinding</th><th>Action</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>Ctrl+P</code></td><td>Anime Theme Gallery</td><td>Open theme gallery and wallpaper switcher.</td></tr>
    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Open fuzzy-search command palette.</td></tr>
    <tr><td><code>Ctrl+Shift+T</code></td><td>New Tab</td><td>Create a new terminal tab in current working directory.</td></tr>
    <tr><td><code>Ctrl+Shift+W</code></td><td>Close Tab / Pane</td><td>Close the active tab or split pane.</td></tr>
    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Split active window vertically into two panes.</td></tr>
    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Split active window horizontally into two panes.</td></tr>
    <tr><td><code>Ctrl+Shift+Z</code></td><td>Toggle Zoom</td><td>Maximize active pane to fill entire window.</td></tr>
    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search scrollback buffer and command history.</td></tr>
    <tr><td><code>Ctrl+Shift+C</code></td><td>Copy</td><td>Copy selected text to clipboard.</td></tr>
    <tr><td><code>Ctrl+Shift+V</code></td><td>Paste</td><td>Paste clipboard contents into terminal.</td></tr>
    <tr><td><code>Ctrl+L</code></td><td>Clear Screen</td><td>Clear terminal grid while preserving scrollback.</td></tr>
  </tbody>
</table>

<h2 id="customizing-keys">Custom Keymap Configuration</h2>
<p>Override keybindings in <code>~/.config/meridian/keybindings.json</code>:</p>
<pre><code class="language-json">{
  "split_vertical": "Ctrl+Shift+V",
  "split_horizontal": "Ctrl+Shift+H",
  "command_palette": "Ctrl+Space",
  "new_tab": "Ctrl+T"
}</code></pre>

<h2 id="modifiers-syntax">Modifier Key Syntax</h2>
<p>Use standard modifier names: <code>Ctrl</code>, <code>Shift</code>, <code>Alt</code>, <code>Meta</code> / <code>Super</code> combined with <code>+</code>.</p>