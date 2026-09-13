---
layout: default
title: "Architecture"
category: "DEVELOPER"
status: "implemented"
---

<h2 id="layers-breakdown">Architectural Layer Breakdown</h2>
<p>Meridian is engineered as modular C++20 subsystems designed to isolate the core terminal state machine from the windowing environment and background telemetry profilers.</p>

<h2 id="data-flow">End-to-End Data Flow</h2>
<div class="arch-diagram-block"><pre><code class="language-text">User Keystroke ──► Window Event Loop ──► PTY Master FD ──► Kernel PTY ──► Shell Process (bash/zsh)
                                                                                  │ (stdout/stderr)
Display Frame  ◄── GPU Render Pipeline ◄── DamageTracker ◄── ScreenBuffer ◄── ANSI State Machine</code></pre></div>

<h2 id="threading-model">Threading & Concurrency Model</h2>
<p>The I/O read thread continuously polls master PTY file descriptors using non-blocking epoll, buffering raw byte chunks into lock-free ring buffers before handing them off to the ANSI parser thread.</p>