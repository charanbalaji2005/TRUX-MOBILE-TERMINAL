---
layout: default
title: "Contributing"
category: "DEVELOPMENT"
status: "implemented"
---

<h2 id="contributing-guide-spec">Open Source Contribution Workflow</h2>
<p>Fork the repository on GitHub, create a feature branch, run <code>make test</code> to verify zero regressions, and open a Pull Request.</p>

<h2 id="code-style-guide">Code Style Guidelines (clang-format)</h2>
<pre><code class="language-bash"># Format all C++ source files
clang-format -i src/**/*.cpp src/**/*.hpp tests/**/*.cpp</code></pre>