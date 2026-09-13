---
layout: default
title: "Terminal Emulation"
category: "TERMINAL"
status: "implemented"
---

<p>Meridian features a high-performance, standard-compliant VT100, VT220, and XTerm terminal emulation engine written in modern C++20.</p>

<h2 id="ansi-vt">ANSI & VT Escape Sequences</h2>
<p>The parser handles standard CSI (Control Sequence Introducer), OSC (Operating System Command), APC (Application Program Command), and DCS (Device Control String) sequences:</p>
<table class="doc-table">
  <thead><tr><th>Sequence</th><th>Name</th><th>Function</th></tr></thead>
  <tbody>
    <tr><td><code>\033[H</code> / <code>\033[{r};{c}H</code></td><td>CUP</td><td>Move cursor to row {r}, column {c} (1-indexed).</td></tr>
    <tr><td><code>\033[2J</code></td><td>ED</td><td>Erase entire display.</td></tr>
    <tr><td><code>\033[2K</code></td><td>EL</td><td>Erase entire active line.</td></tr>
    <tr><td><code>\033[?1049h</code></td><td>DECSET</td><td>Switch to alternate screen buffer.</td></tr>
    <tr><td><code>\033[?1049l</code></td><td>DECRST</td><td>Restore primary screen buffer.</td></tr>
    <tr><td><code>\033[?25h</code> / <code>\033[?25l</code></td><td>DECTCEM</td><td>Show / hide text cursor.</td></tr>
    <tr><td><code>\033[{top};{bot}r</code></td><td>DECSTBM</td><td>Set scrolling margins (top to bottom).</td></tr>
  </tbody>
</table>

<h2 id="alt-screen">Alternate Screen Buffer (DECSET 1049)</h2>
<p>Full-screen interactive applications like <code>vim</code>, <code>nano</code>, <code>tmux</code>, and <code>htop</code> switch to the alternate buffer on launch and restore the primary buffer on exit, preserving command prompt history.</p>

<h2 id="truecolor">24-Bit TrueColor RGB (SGR 38/48)</h2>
<p>Meridian renders full 24-bit RGB colors with 16.7 million distinct hues via standard SGR sequences:</p>
<pre><code class="language-bash"># Set foreground color to Meridian Cyan (#00E5FF)
printf "\033[38;2;0;229;255mMeridian TrueColor\033[0m\n"

# Set background color to Dark Panel (#0E1622)
printf "\033[48;2;14;22;34m\033[38;2;0;229;255m Custom Panel \033[0m\n"</code></pre>

<h2 id="cursor-modes">Cursor Addressing & Terminal Modes</h2>
<p>Supports block, beam, and underline cursor shapes, cursor save/restore (<code>ESC 7</code> / <code>ESC 8</code>), and bracketed paste mode (<code>\033[?2004h</code>).</p>

<h2 id="osc-protocols">Operating System Commands (OSC 7/8/52/133)</h2>
<ul>
  <li><strong>OSC 7:</strong> Real-time current working directory tracking for tab titles.</li>
  <li><strong>OSC 8:</strong> Clickable terminal hyperlinks with embedded URIs.</li>
  <li><strong>OSC 52:</strong> Base64 remote clipboard synchronization.</li>
  <li><strong>OSC 133:</strong> Semantic shell integration prompt markers (FTCS).</li>
</ul>