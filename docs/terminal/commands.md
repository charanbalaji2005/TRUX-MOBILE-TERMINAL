---
layout: default
title: "Commands"
category: "TERMINAL"
status: "implemented"
---

<h2 id="cli-subcommands">CLI Subcommands Index</h2>
<table class="doc-table">
  <thead><tr><th>Command</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>meridian</code></td><td>Launch interactive terminal emulator GUI.</td></tr>
    <tr><td><code>meridian-shell</code></td><td>Launch standalone interactive AST shell.</td></tr>
    <tr><td><code>meridian monitor</code></td><td>Open live CPU, RAM, Disk, Network, and Process metrics dashboard.</td></tr>
    <tr><td><code>meridian git</code></td><td>Inspect Git branch divergence, staged/unstaged changes.</td></tr>
    <tr><td><code>meridian files [dir]</code></td><td>View interactive directory tree explorer with Git badges.</td></tr>
    <tr><td><code>meridian ssh [alias]</code></td><td>Manage and connect to SSH remote workspaces.</td></tr>
    <tr><td><code>meridian plugins</code></td><td>List active extensible plugins and lifecycle hooks.</td></tr>
    <tr><td><code>meridian --performance</code></td><td>Display GPU framerate, PTY latency & telemetry profiler.</td></tr>
    <tr><td><code>meridian pic &lt;file&gt;</code></td><td>Display direct 32-bit RGBA inline image.</td></tr>
  </tbody>
</table>

<h2 id="shell-builtins">Shell Built-in Commands</h2>
<p>Inside <code>meridian-shell</code>, built-in commands run directly inside the process without forking:</p>
<pre><code class="language-bash">cd /var/log         # Change current working directory
export FOO="bar"    # Set environment variable
jobs                # List active background jobs
fg %1               # Foreground job 1
bg %1               # Background job 1
history             # View rich command history
exit 0              # Terminate shell session</code></pre>

<h2 id="developer-commands">Developer Productivity Subcommands</h2>
<p>Developer subcommands can be executed from within any shell or invoked directly from your system PATH:</p>
<pre><code class="language-bash"># Launch live resource monitor
meridian monitor

# Show visual Git status
meridian git

# Open interactive directory browser
meridian files /var/log</code></pre>