---
layout: default
title: "Clipboard"
category: "TERMINAL"
status: "implemented"
---

<h2 id="clipboard-sync">System Clipboard Integration</h2>
<p>Meridian integrates with system clipboards across Wayland (<code>wl-clipboard</code>), X11 (<code>xclip</code>/<code>xsel</code>), macOS (<code>pbcopy</code>), and Windows.</p>

<h2 id="osc52-spec">OSC 52 Base64 Remote Clipboard Protocol</h2>
<p>Meridian natively parses OSC 52 sequences (<code>\033]52;c;&lt;base64&gt;\033\\</code>), allowing CLI tools running on remote SSH servers (e.g. <code>tmux</code>, <code>vim</code>, <code>osc52.sh</code>) to copy text directly into your local machine's clipboard without X11 forwarding.</p>

<pre><code class="language-bash"># Example: Copy string to local clipboard over remote SSH
printf "\033]52;c;%s\033\\" "$(echo -n "Hello from Remote Server" | base64)"</code></pre>

<h2 id="bracketed-paste">Bracketed Paste Mode</h2>
<p>When enabled by interactive programs (<code>\033[?2004h</code>), pasted text is enclosed in <code>\033[200~</code> and <code>\033[201~</code> markers, preventing accidental execution of newline-separated commands.</p>

<h2 id="clipboard-security">Clipboard Security & Secret Redaction</h2>
<p>Meridian includes an optional clipboard guard that warns the user if pasted content contains sensitive tokens such as private SSH keys or AWS secret access keys.</p>