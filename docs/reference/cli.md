---
layout: default
title: "CLI Reference"
category: "REFERENCE"
status: "implemented"
---

<h2 id="cli-full-ref">Complete CLI Command Reference</h2>
<table class="doc-table">
  <thead><tr><th>Command</th><th>Arguments</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>meridian</code></td><td><code>[none]</code></td><td>Launch interactive terminal emulator GUI.</td></tr>
    <tr><td><code>meridian-shell</code></td><td><code>[-c &quot;cmd&quot;]</code></td><td>Launch standalone AST shell process.</td></tr>
    <tr><td><code>meridian ssh</code></td><td><code>[alias]</code></td><td>Manage and connect to SSH remote workspaces.</td></tr>
    <tr><td><code>meridian plugins</code></td><td><code>[none]</code></td><td>List active extensible plugins and lifecycle hooks.</td></tr>
    <tr><td><code>meridian --performance</code></td><td><code>[none]</code></td><td>Display GPU framerate and telemetry profiler HUD.</td></tr>
    <tr><td><code>meridian monitor</code></td><td><code>[none]</code></td><td>Open live CPU, RAM, Disk, and Network dashboard.</td></tr>
    <tr><td><code>meridian git</code></td><td><code>[none]</code></td><td>Inspect Git branch divergence and staged changes.</td></tr>
    <tr><td><code>meridian pic</code></td><td><code>&lt;file&gt;</code></td><td>Display direct 32-bit RGBA inline image.</td></tr>
    <tr><td><code>meridian ask</code></td><td><code>&quot;&lt;intent&gt;&quot;</code></td><td>Translate natural language to safe shell command.</td></tr>
    <tr><td><code>meridian diag</code></td><td><code>&quot;&lt;error&gt;&quot;</code></td><td>Diagnose compiler / runtime errors with suggested fixes.</td></tr>
  </tbody>
</table>

<h2 id="exit-codes">CLI Exit Codes</h2>
<table class="doc-table">
  <thead><tr><th>Code</th><th>Meaning</th></tr></thead>
  <tbody>
    <tr><td><code>0</code></td><td>Success / Clean exit.</td></tr>
    <tr><td><code>1</code></td><td>General command failure / parse error.</td></tr>
    <tr><td><code>126</code></td><td>Command found but not executable.</td></tr>
    <tr><td><code>127</code></td><td>Command not found.</td></tr>
    <tr><td><code>130</code></td><td>Process terminated by SIGINT (Ctrl+C).</td></tr>
  </tbody>
</table>