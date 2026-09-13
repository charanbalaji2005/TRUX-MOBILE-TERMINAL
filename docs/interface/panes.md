---
layout: default
title: "Panes"
category: "INTERFACE"
status: "implemented"
---

<h2 id="pane-tree-arch">Binary Pane Tree Architecture</h2>
<p><code>workspace::PaneTree</code> models terminal splits as an n-ary tree of leaves, recalculating fractional layout rectangles on resize.</p>

<h2 id="pane-shortcuts-spec">Pane Splitting & Zoom Shortcuts</h2>
<ul>
  <li><code>Ctrl+Shift+D</code>: Split active pane vertically.</li>
  <li><code>Ctrl+Shift+E</code>: Split active pane horizontally.</li>
  <li><code>Ctrl+Shift+Z</code>: Toggle zoom on active pane.</li>
  <li><code>Alt+Arrows</code>: Navigate focus between adjacent panes.</li>
</ul>

<h2 id="pane-resizing">Pane Resizing & Layout Persistence</h2>
<p>Drag pane divider borders with the mouse or use <code>Ctrl+Shift+Alt+Arrows</code> to adjust split ratios in 5% increments.</p>