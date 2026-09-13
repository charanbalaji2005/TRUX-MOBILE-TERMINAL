---
layout: default
title: "Building"
category: "DEVELOPMENT"
status: "implemented"
---

<h2 id="prerequisites">Compiler & Library Prerequisites</h2>
<p>Requires GCC 11+ or Clang 13+ with C++20 support, and standard POSIX <code>libutil</code>.</p>

<h2 id="make-targets">Make Targets & Build Commands</h2>
<pre><code class="language-bash">make all -j$(nproc)  # Compile all targets
make test            # Run automated test suite (133 tests)
make demo            # Run graphical demo
sudo make install    # Install binaries and assets to system
make clean           # Clean build artifacts</code></pre>

<h2 id="custom-cxxflags">Custom Compiler Flags & Optimization</h2>
<pre><code class="language-bash"># Compile with aggressive optimizations and LTO
CXXFLAGS="-std=c++20 -O3 -flto -march=native" make all -j$(nproc)</code></pre>