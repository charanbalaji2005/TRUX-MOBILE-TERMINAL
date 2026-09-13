---
layout: default
title: "Testing & Verification Guide"
category: "DEVELOPMENT"
status: "implemented"
---

<p>This step-by-step guide provides copy-pasteable commands and verification procedures to test every core subsystem of Meridian Terminal.</p>

<h2 id="automated-test-suite">1. Automated Test Suite (133 Tests)</h2>
<p>Run the automated test runner to verify core engine correctness, PTY multiplexing, VT parsing, AST execution, and security:</p>
<pre><code class="language-bash"># Run all 133 automated unit and integration tests
make test

# Or run test binary directly
./build/meridian_tests</code></pre>

<h2 id="interactive-terminal">2. Interactive Terminal & Shell Launch</h2>
<p>Launch the interactive terminal session or standalone shell engine:</p>
<pre><code class="language-bash"># Launch interactive terminal emulator
meridian

# Launch standalone shell engine directly
meridian-shell</code></pre>

<h2 id="shell-pipelines">3. Shell Pipelines & Command Execution</h2>
<p>Verify command pipelines, environment variable expansion, and command substitution engines:</p>
<pre><code class="language-bash"># Test pipelines & coreutils
ps aux | grep -i meridian | head -n 5

# Test variable export and expansion
export PROJECT_NAME="Meridian" && echo "Running $PROJECT_NAME on $SHELL"

# Test command substitution engine
echo "Kernel: $(uname -r) | Current Time: $(date +%T)"</code></pre>

<h2 id="signals-job-control">4. Signals & Job Control (Ctrl+C / Ctrl+Z)</h2>
<p>Verify that POSIX signals are routed strictly to the foreground process group without killing Meridian:</p>
<pre><code class="language-bash"># 1. Start a long running command and cancel it with Ctrl+C:
sleep 10
# Press: Ctrl+C  -> Cancels sleep immediately and returns to prompt

# 2. Test job suspension and resumption:
sleep 50
# Press: Ctrl+Z  -> Suspends job
jobs            # Lists running and stopped background jobs
fg              # Brings suspended job back to foreground</code></pre>

<h2 id="splits-navigation-zoom">5. Window Splits, Directional Navigation & Zoom</h2>
<p>Test the persistent binary space partitioning (BSP) pane tree and keyboard navigation:</p>
<table class="doc-table">
  <thead><tr><th>Keyboard Shortcut</th><th>Action</th><th>Shell Built-in Equivalent</th></tr></thead>
  <tbody>
    <tr><td><code>Ctrl+Shift+D</code></td><td>Split active pane vertically</td><td><code>split v</code></td></tr>
    <tr><td><code>Ctrl+Shift+E</code></td><td>Split active pane horizontally</td><td><code>split h</code></td></tr>
    <tr><td><code>Ctrl+Shift+Z</code></td><td>Toggle zoom on active pane</td><td><code>zoom</code></td></tr>
    <tr><td><code>Alt+Up</code></td><td>Navigate focus to pane above</td><td><code>pane up</code></td></tr>
    <tr><td><code>Alt+Down</code></td><td>Navigate focus to pane below</td><td><code>pane down</code></td></tr>
    <tr><td><code>Alt+Left</code></td><td>Navigate focus to pane on left</td><td><code>pane left</code></td></tr>
    <tr><td><code>Alt+Right</code></td><td>Navigate focus to pane on right</td><td><code>pane right</code></td></tr>
    <tr><td>—</td><td>Inspect active pane matrix</td><td><code>pane list</code></td></tr>
  </tbody>
</table>

<h2 id="raster-image-testing">6. Direct Raster Image Protocol (pic)</h2>
<p>Test inline 32-bit RGBA hardware-blitted raster images:</p>
<pre><code class="language-bash"># Render direct inline image:
pic ~/.config/meridian/gallery/sharingan_eye.png

# Set startup anime artwork theme:
pic set 1

# Enable random artwork on every startup:
pic set random</code></pre>

<h2 id="dev-intelligence-tools">7. Developer Intelligence Tools (monitor, git, files, ssh, perf)</h2>
<p>Test Meridian's integrated developer productivity suite:</p>
<pre><code class="language-bash"># Real-time system monitor (CPU, RAM, Disk, Process metrics):
meridian monitor

# Visual Git branch divergence and staged/unstaged inspector:
meridian git

# Interactive tree file explorer with git badges:
meridian files

# SSH connection manager (~/.ssh/config):
meridian ssh

# Live GPU framerate & PTY latency profiler:
meridian perf</code></pre>

<h2 id="palette-and-search">8. Command Palette & Universal Search</h2>
<p>Launch quick actions and search across history:</p>
<pre><code class="language-bash"># Open fuzzy Command Palette:
palette
# (Or press Ctrl+Shift+P / Ctrl+P anytime)

# Universal search across screen buffers and rich history:
search "git"
# (Or press Ctrl+Shift+F anytime)

# View rich SQLite history database with execution durations:
meridian history</code></pre>

<h2 id="ai-engine-testing">9. Local AI Engine & Error Diagnostics</h2>
<p>Test offline privacy-preserving developer AI:</p>
<pre><code class="language-bash"># Natural language to safe shell command translation:
meridian ask "find all files modified in the last 24 hours"

# Error diagnostics engine:
meridian diag "Segmentation fault (core dumped)"

# Inspect AI privacy mode and risk classifier:
meridian ai status</code></pre>