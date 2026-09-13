---
layout: default
title: "Windows"
category: "INTERFACE"
status: "implemented"
---

<h2 id="transparency">Window Transparency & Background Blur</h2>
<p>Configure background opacity (<code>0.0</code> to <code>1.0</code>) and enable compositor blur in <code>~/.config/meridian/config.json</code>:</p>
<pre><code class="language-json">{
  "opacity": 0.92,
  "blur": true
}</code></pre>

<h2 id="multi-window">Multi-Window Session Management</h2>
<p>Press <code>Ctrl+Shift+N</code> to open a new independent window sharing the same session daemon.</p>

<h2 id="geometry-persistence">Window Geometry & State Persistence</h2>
<p>Window position, dimensions, maximized state, and split layouts are automatically serialized and restored across application relaunches.</p>