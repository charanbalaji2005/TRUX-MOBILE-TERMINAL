---
layout: default
title: "Powerline"
category: "INTERFACE"
status: "implemented"
---

<h2 id="context-aware-prompt">Intelligent Context-Aware Powerline Prompt</h2>
<p>Meridian renders a compact, two-line Powerline prompt with high-contrast color segments and context-aware Nerd Font glyphs:</p>

<div class="code-block-wrapper">
  <div class="code-header"><span>Meridian Context-Aware Powerline Prompt</span></div>
  <pre><code class="language-text">   Tue 25 Aug - 23:43   ~/Downloads/meridian-terminal 󰙲   main ↑2 3✗ 2? 
   @charanbalaji  ❯ </code></pre>
</div>

<h2 id="directory-language-icons">Directory & Project Language Icons</h2>
<ul>
  <li><strong>Home:</strong> <code> ~</code></li>
  <li><strong>Downloads:</strong> <code> ~/Downloads</code></li>
  <li><strong>Documents:</strong> <code>󰈙 ~/Documents</code></li>
  <li><strong>Desktop:</strong> <code> ~/Desktop</code></li>
  <li><strong>Pictures & Photos:</strong> <code> ~/Pictures</code></li>
  <li><strong>Projects / Code:</strong> <code> ~/Projects</code></li>
  <li><strong>C++ Project:</strong> <code>󰙲</code> (CMakeLists.txt, Makefile)</li>
  <li><strong>Python Project:</strong> <code></code> (pyproject.toml, requirements.txt)</li>
  <li><strong>Rust Project:</strong> <code></code> (Cargo.toml)</li>
  <li><strong>Node / TypeScript:</strong> <code></code> (package.json)</li>
  <li><strong>Go Project:</strong> <code></code> (go.mod)</li>
  <li><strong>Docker Project:</strong> <code>󰡨</code> (Dockerfile, compose.yaml)</li>
</ul>

<h2 id="rich-git-segment">Rich Git Status & Divergence Segment</h2>
<p>Displays live repository divergence without running slow subprocesses on every keypress:</p>
<ul>
  <li><code> main ✔</code> &mdash; Clean working tree on <code>main</code></li>
  <li><code> main ↑2</code> &mdash; 2 commits ahead of upstream remote</li>
  <li><code> main ↓1</code> &mdash; 1 commit behind upstream remote</li>
  <li><code> main 3✗ 2?</code> &mdash; 3 modified files, 2 untracked files</li>
  <li><code> main ↑2 ↓1 3✗ 2?</code> &mdash; Full upstream and worktree status</li>
</ul>

<h2 id="ssh-and-root">Remote SSH & Elevated Root State</h2>
<ul>
  <li><strong>Remote SSH:</strong> Displays purple badge <code> user@hostname </code> when connected over SSH.</li>
  <li><strong>Elevated Root / Sudo:</strong> Dynamically detects <code>geteuid() == 0</code> and renders crimson prompt <code>⚡ root  ❯</code>.</li>
</ul>