---
layout: default
title: "Tabs"
category: "INTERFACE"
status: "implemented"
---

<h2 id="tabs-workflow">Tabbed Workflow</h2>
<p>Tabs maintain independent PTY sessions, working directories, and scrollback histories.</p>

<h2 id="tabs-shortcuts-spec">Tab Shortcuts Reference</h2>
<ul>
  <li><code>Ctrl+Shift+T</code>: New Tab in current directory.</li>
  <li><code>Ctrl+Shift+W</code>: Close current Tab.</li>
  <li><code>Ctrl+PageUp</code> / <code>Ctrl+PageDown</code>: Switch between tabs sequentially.</li>
  <li><code>Alt+1..9</code>: Jump directly to Tab N.</li>
</ul>

<h2 id="cwd-tab-titles">Dynamic CWD & Process Titles</h2>
<p>Tab headers update dynamically in response to OSC 7 directory tracking and foreground process execution (e.g., <code>vim: src/main.cpp</code> or <code>cargo: build</code>).</p>