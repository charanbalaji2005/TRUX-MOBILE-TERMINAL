---
layout: default
title: "Kitty Graphics"
category: "GRAPHICS"
status: "development"
---

<h2 id="kitty-protocol">Kitty Graphics Protocol Specification</h2>
<p>Meridian parses Kitty Graphics escape sequences (<code>\033_G...\033\\</code>) for seamless compatibility with tools like <code>icat</code>, <code>yazi</code>, and <code>ranger</code>.</p>

<h2 id="chunking-safety">2048-Byte Safe Transmission Chunking</h2>
<p>Payloads exceeding 2048 bytes are transmitted in chunks using <code>m=1</code> (more data follows) and <code>m=0</code> (final payload chunk), preventing buffer overflows in PTY drivers.</p>

<h2 id="protocol-parameters">Supported APC Control Keys</h2>
<table class="doc-table">
  <thead><tr><th>Key</th><th>Name</th><th>Values</th></tr></thead>
  <tbody>
    <tr><td><code>a</code></td><td>Action</td><td><code>t</code> (transmit & display), <code>d</code> (delete), <code>p</code> (place)</td></tr>
    <tr><td><code>f</code></td><td>Format</td><td><code>32</code> (RGBA), <code>24</code> (RGB), <code>100</code> (PNG)</td></tr>
    <tr><td><code>m</code></td><td>More Chunks</td><td><code>1</code> (chunk follows), <code>0</code> (final chunk)</td></tr>
    <tr><td><code>s</code> / <code>v</code></td><td>Size</td><td>Width and Height in pixels</td></tr>
  </tbody>
</table>