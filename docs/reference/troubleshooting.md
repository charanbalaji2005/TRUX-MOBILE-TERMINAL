---
layout: default
title: "Troubleshooting"
category: "REFERENCE"
status: "implemented"
---

<h2 id="troubleshooting-guide">Troubleshooting Common Issues</h2>
<ul>
  <li><strong>Missing Powerline glyphs:</strong> Install a Nerd Font (e.g. <code>FantasqueSansM Nerd Font</code> or <code>JetBrainsMono Nerd Font</code>).</li>
  <li><strong>Image display issues:</strong> Verify that GPU drivers (Mesa / NVIDIA) are active and supporting OpenGL 3.3 Core.</li>
  <li><strong>PTY Permission Denied:</strong> Ensure your user account is in the <code>tty</code> group or that <code>/dev/pts</code> is mounted with standard permissions.</li>
  <li><strong>Keybinding conflicts:</strong> Inspect <code>~/.config/meridian/keybindings.json</code> for duplicate shortcut mappings.</li>
</ul>