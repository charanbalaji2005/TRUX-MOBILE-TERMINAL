---
layout: default
title: "Debugging"
category: "DEVELOPMENT"
status: "implemented"
---

<h2 id="gdb-asan">GDB & AddressSanitizer</h2>
<pre><code class="language-bash"># Build with AddressSanitizer and debug symbols
CXXFLAGS="-std=c++20 -fsanitize=address -g -O0" make all</code></pre>

<h2 id="telemetry-profiler-dev">Telemetry Profiler (meridian --performance)</h2>
<pre><code class="language-bash">meridian --performance
# Outputs real-time FPS, frame time (ms), glyph cache hits, VRAM, and PTY latency.</code></pre>

<h2 id="valgrind-profiling">Valgrind Memory Profiling</h2>
<pre><code class="language-bash">valgrind --leak-check=full --track-origins=yes ./bin/meridian</code></pre>