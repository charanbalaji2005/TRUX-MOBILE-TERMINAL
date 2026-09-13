---
layout: default
title: "Scrollback"
category: "TERMINAL"
status: "implemented"
---

<h2 id="scrollback-arch">Scrollback Deque Architecture</h2>
<p><code>vt::ScreenBuffer</code> maintains an efficient circular deque for scrollback history with a configurable limit (default: 10,000 lines). When lines scroll off the top of the grid, they are pushed into the scrollback pool, preserving ANSI colors and hyperlink attributes.</p>

<h2 id="search-engine">Scrollback Search & Highlighting</h2>
<p>Pressing <code>Ctrl+Shift+F</code> activates the incremental search engine with real-time match highlighting, regex support, and case-sensitivity toggles.</p>

<h2 id="memory-management">Memory Limits & Performance Tuning</h2>
<p>Each line in the scrollback buffer is stored with sparse cell arrays, consuming less than 20MB of RAM per 10,000 lines of colored output.</p>
<pre><code class="language-json">{
  "scrollback_lines": 50000
}</code></pre>