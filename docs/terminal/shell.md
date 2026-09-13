---
layout: default
title: "Shell"
category: "TERMINAL"
status: "implemented"
---

<p>Meridian includes an internal standalone POSIX shell engine (<code>meridian-shell</code>) with recursive-descent parsing, AST execution, and full job control.</p>

<h2 id="ast-engine">POSIX AST Parser & Execution Engine</h2>
<p>Input strings are tokenized by <code>shell::Lexer</code> and structured into an Abstract Syntax Tree by <code>shell::Parser</code>:</p>
<pre><code class="language-text">Input: git status && cargo build --release | tee build.log

AST Structure:
  LogicalAndNode
    ├── CommandNode: "git", ["status"]
    └── PipelineNode
          ├── CommandNode: "cargo", ["build", "--release"]
          └── CommandNode: "tee", ["build.log"]</code></pre>

<h2 id="pipelines-redirections">Pipelines & File Redirections</h2>
<pre><code class="language-bash"># Standard pipeline with stdout/stderr redirection
cat app.log | grep -i "error" | sort | uniq -c > errors.txt 2>&1

# Appending output
echo "build completed at $(date)" >> /tmp/meridian.log</code></pre>

<h2 id="command-substitution">Command Substitution & Expansion</h2>
<p>Supports <code>$(command)</code> substitution, environment variable expansion (<code>$VAR</code>, <code>${VAR}</code>), and exit code inspection (<code>$?</code>).</p>

<h2 id="job-control">Job Control & Background Processes</h2>
<table class="doc-table">
  <thead><tr><th>Command / Key</th><th>Action</th></tr></thead>
  <tbody>
    <tr><td><code>command &</code></td><td>Launch process in background.</td></tr>
    <tr><td><code>Ctrl+Z</code></td><td>Send <code>SIGTSTP</code> to suspend foreground process.</td></tr>
    <tr><td><code>jobs</code></td><td>List active background and suspended jobs.</td></tr>
    <tr><td><code>fg [%id]</code></td><td>Bring background job to foreground.</td></tr>
    <tr><td><code>bg [%id]</code></td><td>Resume suspended job in background.</td></tr>
  </tbody>
</table>

<h2 id="signal-forwarding">Process Group Signals & Terminal Ownership</h2>
<p>When running commands, <code>meridian-shell</code> creates dedicated process groups (<code>setpgid()</code>) and gives controlling terminal ownership via <code>tcsetpgrp()</code> so signals like <code>SIGINT</code> (Ctrl+C) and <code>SIGQUIT</code> (Ctrl+\) cleanly terminate the foreground pipeline without killing the parent shell.</p>