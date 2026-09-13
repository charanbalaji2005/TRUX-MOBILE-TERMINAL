---
layout: default
title: "PTY Architecture"
category: "DEVELOPER"
status: "implemented"
---

<h2 id="pty-lifecycle">PTY Descriptor Lifecycle</h2>
<p><code>pty::PTYSession</code> handles fork/execve, establishes slave descriptor as controlling tty (<code>setsid()</code>), and redirects stdin/stdout/stderr.</p>

<h2 id="termios-spec">POSIX Termios Configuration</h2>
<p>Configures raw mode flags (<code>ICANON</code>, <code>ECHO</code>, <code>ISIG</code>) to allow interactive curses and shell line editors to manage raw keypresses.</p>

<h2 id="process-spawning">Process Spawning & Environment Inheritance</h2>
<p>When launching a new tab, Meridian passes sanitized environment variables while injecting standard terminal indicators (<code>TERM=xterm-256color</code>, <code>COLORTERM=truecolor</code>).</p>