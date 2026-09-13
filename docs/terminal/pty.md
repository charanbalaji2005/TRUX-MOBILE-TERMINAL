---
layout: default
title: "PTY"
category: "TERMINAL"
status: "implemented"
---

<p>Meridian uses POSIX <code>openpty(3)</code> to establish real Linux pseudoterminal sessions, decoupling GUI rendering from shell process execution.</p>

<h2 id="openpty-arch">Linux openpty Master/Slave Architecture</h2>
<div class="arch-diagram-block"><pre><code class="language-text">Meridian UI / Render Canvas
        │ (read / write)
        ▼
   Master PTY FD
═════════════════════════════ Linux Kernel PTY Driver
   Slave PTY FD (/dev/pts/X)
        │
   Forked Process (bash / zsh / ssh / vim)</code></pre></div>

<h2 id="async-io">Non-Blocking Asynchronous I/O Loop</h2>
<p><code>pty::PTYManager</code> manages master file descriptors using non-blocking I/O (<code>O_NONBLOCK</code>) and <code>epoll</code> polling, ensuring high throughput and zero UI stutter during high-volume output (e.g. <code>find /</code> or large compile logs).</p>

<h2 id="signal-handling">Signal Handling & Resizing (SIGWINCH)</h2>
<p>When the terminal window or split pane is resized, Meridian calculates the new row and column count and invokes <code>ioctl(master_fd, TIOCSWINSZ, &ws)</code>, instantly sending <code>SIGWINCH</code> to the foreground process group.</p>

<h2 id="termios-config">Termios Line Discipline & Raw Mode</h2>
<p>Meridian configures the slave PTY with <code>termios</code> flags supporting both cooked mode for standard readline shells and raw mode for screen-oriented curses programs like <code>vim</code> and <code>micro</code>.</p>