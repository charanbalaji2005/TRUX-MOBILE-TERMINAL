---
layout: default
title: "SSH"
category: "TERMINAL"
status: "implemented"
---

<p>Meridian includes a built-in SSH workspace and connection manager in <code>src/dev/ssh_manager.cpp</code>.</p>

<h2 id="ssh-mgr">Native SSH Workspace Manager</h2>
<pre><code class="language-bash"># List all available SSH hosts from ~/.ssh/config
meridian ssh

# Connect directly to a configured host alias
meridian ssh production

# Custom port or identity file connection
meridian ssh staging -p 2222</code></pre>

<h2 id="ssh-config-parser">~/.ssh/config Automatic Parsing</h2>
<p>Meridian automatically parses <code>Host</code>, <code>HostName</code>, <code>User</code>, <code>Port</code>, and <code>IdentityFile</code> directives from <code>~/.ssh/config</code> and presents them in the Command Palette (<code>Ctrl+Shift+P</code>).</p>

<h2 id="remote-compat">Remote Terminal Compatibility & Protocols</h2>
<p>Over remote SSH sessions, Meridian fully supports:</p>
<ul>
  <li><strong>OSC 52:</strong> Seamless remote clipboard synchronization back to your local clipboard.</li>
  <li><strong>OSC 7:</strong> Remote current working directory reporting for tab titles.</li>
  <li><strong>SIGWINCH:</strong> Window resize propagation across SSH channels.</li>
  <li><strong>24-bit TrueColor:</strong> Uncompromised color output for remote vim and tmux.</li>
</ul>

<h2 id="ssh-security">Key Management & Security</h2>
<p>Meridian integrates with <code>ssh-agent</code> and respects encrypted private keys, passing terminal passphrase prompts seamlessly through the PTY layer without leaking credentials into logs.</p>