---
layout: default
title: "AI Engine"
category: "DEVELOPER"
status: "implemented"
---

<h2 id="typo-popup-algorithm">Interactive AI Typo Correction Popup & Algorithm</h2>
<p>When a misspelled or unknown command is entered in interactive mode (e.g. <code>gti status</code>, <code>sl</code>, <code>mkdri test</code>, <code>pyhton script.py</code>), Meridian AI automatically analyzes the command and renders an interactive confirmation popup.</p>

<div class="code-block-wrapper">
  <div class="code-header"><span>Interactive AI Typo Popup Dialog</span></div>
  <pre><code class="language-text">┌── 💡 Meridian AI Typo Correction ──────────────────────────────────────────┐
│ Command 'gti' not found in system PATH or builtins.                        │
│ Did you mean: git status ?                                                 │
│                                                                            │
│ Press [Y/Enter] Run correction   [N/Esc] Keep original                     │
└────────────────────────────────────────────────────────────────────────────┘</code></pre>
</div>

<p><strong>Correction Algorithm:</strong></p>
<ol>
  <li><strong>Optimal String Alignment (OSA):</strong> Calculates edit distance across insertions, deletions, substitutions, and adjacent transpositions (e.g. <code>gerp</code> &rarr; <code>grep</code>).</li>
  <li><strong>Dynamic Proportional Threshold:</strong> Rejects false positives by requiring <code>distance &le; max(1, len / 3)</code> against known builtins and cached <code>$PATH</code> binaries.</li>
  <li><strong>Single-Key Interactive Intercept:</strong> Puts terminal in non-canonical raw mode (<code>ICANON | ECHO</code> cleared) to read response immediately without requiring extra prompts.</li>
</ol>

<h2 id="intent-engine-dev">Intent Engine (meridian ask)</h2>
<pre><code class="language-bash">meridian ask "find all log files modified in the last 24 hours"
# -> find . -name "*.log" -mtime -1</code></pre>

<h2 id="diagnostics-dev">Compiler & Runtime Diagnostics (meridian diag)</h2>
<pre><code class="language-bash">meridian diag "g++: error: cannot find -lutil"
# -> Diagnosis: Missing POSIX utility library. Fix: install glibc-devel / libutil.</code></pre>

<h2 id="agent-dev">Autonomous Coding Agent (meridian agent)</h2>
<pre><code class="language-bash">meridian agent "add unit tests for OSC 8 hyperlinks"</code></pre>