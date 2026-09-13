module.exports = function(addArt) {
  addArt('terminal-emulation', 'Terminal Emulation', 'TERMINAL', 'implemented',
    'VT100, XTerm, ANSI escapes, alternate screen buffers, and 24-bit TrueColor.',
    [
      { id: 'ansi-vt', text: 'ANSI & VT Escape Sequences', level: 2 },
      { id: 'alt-screen', text: 'Alternate Screen Buffer (DECSET 1049)', level: 2 },
      { id: 'truecolor', text: '24-Bit TrueColor RGB (SGR 38/48)', level: 2 },
      { id: 'cursor-modes', text: 'Cursor Addressing & Terminal Modes', level: 2 },
      { id: 'osc-protocols', text: 'Operating System Commands (OSC 7/8/52/133)', level: 2 }
    ],
    `
<p>Meridian features a high-performance, standard-compliant VT100, VT220, and XTerm terminal emulation engine written in modern C++20.</p>

<h2 id="ansi-vt">ANSI & VT Escape Sequences</h2>
<p>The parser handles standard CSI (Control Sequence Introducer), OSC (Operating System Command), APC (Application Program Command), and DCS (Device Control String) sequences:</p>
<table class="doc-table">
  <thead><tr><th>Sequence</th><th>Name</th><th>Function</th></tr></thead>
  <tbody>
    <tr><td><code>\\033[H</code> / <code>\\033[{r};{c}H</code></td><td>CUP</td><td>Move cursor to row {r}, column {c} (1-indexed).</td></tr>
    <tr><td><code>\\033[2J</code></td><td>ED</td><td>Erase entire display.</td></tr>
    <tr><td><code>\\033[2K</code></td><td>EL</td><td>Erase entire active line.</td></tr>
    <tr><td><code>\\033[?1049h</code></td><td>DECSET</td><td>Switch to alternate screen buffer.</td></tr>
    <tr><td><code>\\033[?1049l</code></td><td>DECRST</td><td>Restore primary screen buffer.</td></tr>
    <tr><td><code>\\033[?25h</code> / <code>\\033[?25l</code></td><td>DECTCEM</td><td>Show / hide text cursor.</td></tr>
    <tr><td><code>\\033[{top};{bot}r</code></td><td>DECSTBM</td><td>Set scrolling margins (top to bottom).</td></tr>
  </tbody>
</table>

<h2 id="alt-screen">Alternate Screen Buffer (DECSET 1049)</h2>
<p>Full-screen interactive applications like <code>vim</code>, <code>nano</code>, <code>tmux</code>, and <code>htop</code> switch to the alternate buffer on launch and restore the primary buffer on exit, preserving command prompt history.</p>

<h2 id="truecolor">24-Bit TrueColor RGB (SGR 38/48)</h2>
<p>Meridian renders full 24-bit RGB colors with 16.7 million distinct hues via standard SGR sequences:</p>
<pre><code class="language-bash"># Set foreground color to Meridian Cyan (#00E5FF)
printf "\\033[38;2;0;229;255mMeridian TrueColor\\033[0m\\n"

# Set background color to Dark Panel (#0E1622)
printf "\\033[48;2;14;22;34m\\033[38;2;0;229;255m Custom Panel \\033[0m\\n"</code></pre>

<h2 id="cursor-modes">Cursor Addressing & Terminal Modes</h2>
<p>Supports block, beam, and underline cursor shapes, cursor save/restore (<code>ESC 7</code> / <code>ESC 8</code>), and bracketed paste mode (<code>\\033[?2004h</code>).</p>

<h2 id="osc-protocols">Operating System Commands (OSC 7/8/52/133)</h2>
<ul>
  <li><strong>OSC 7:</strong> Real-time current working directory tracking for tab titles.</li>
  <li><strong>OSC 8:</strong> Clickable terminal hyperlinks with embedded URIs.</li>
  <li><strong>OSC 52:</strong> Base64 remote clipboard synchronization.</li>
  <li><strong>OSC 133:</strong> Semantic shell integration prompt markers (FTCS).</li>
</ul>
`
  );

  addArt('terminal-shell', 'Shell', 'TERMINAL', 'implemented',
    'Standalone POSIX-compliant AST execution engine, pipelines, and job control.',
    [
      { id: 'ast-engine', text: 'POSIX AST Parser & Execution Engine', level: 2 },
      { id: 'pipelines-redirections', text: 'Pipelines & File Redirections', level: 2 },
      { id: 'command-substitution', text: 'Command Substitution & Expansion', level: 2 },
      { id: 'job-control', text: 'Job Control & Background Processes', level: 2 },
      { id: 'signal-forwarding', text: 'Process Group Signals & Terminal Ownership', level: 2 }
    ],
    `
<p>Meridian includes an internal standalone POSIX shell engine (<code>meridian-shell</code>) with recursive-descent parsing, AST execution, and full job control.</p>

<h2 id="ast-engine">POSIX AST Parser & Execution Engine</h2>
<p>Input strings are tokenized by <code>shell::Lexer</code> and structured into an Abstract Syntax Tree by <code>shell::Parser</code>:</p>
<pre><code class="language-text">Input: git status && cargo build --release | tee build.log

AST Structure:
  LogicalAndNode
    ├── CommandNode: "git", ["status"]
    └── PipelineNode
          ├── CommandNode: "cargo", ["build", "--release"]
          └── CommandNode: "tee", ["build.log"]</code></pre>

<h2 id="pipelines-redirections">Pipelines & File Redirections</h2>
<pre><code class="language-bash"># Standard pipeline with stdout/stderr redirection
cat app.log | grep -i "error" | sort | uniq -c > errors.txt 2>&1

# Appending output
echo "build completed at $(date)" >> /tmp/meridian.log</code></pre>

<h2 id="command-substitution">Command Substitution & Expansion</h2>
<p>Supports <code>$(command)</code> substitution, environment variable expansion (<code>$VAR</code>, <code>\${VAR}</code>), and exit code inspection (<code>$?</code>).</p>

<h2 id="job-control">Job Control & Background Processes</h2>
<table class="doc-table">
  <thead><tr><th>Command / Key</th><th>Action</th></tr></thead>
  <tbody>
    <tr><td><code>command &</code></td><td>Launch process in background.</td></tr>
    <tr><td><code>Ctrl+Z</code></td><td>Send <code>SIGTSTP</code> to suspend foreground process.</td></tr>
    <tr><td><code>jobs</code></td><td>List active background and suspended jobs.</td></tr>
    <tr><td><code>fg [%id]</code></td><td>Bring background job to foreground.</td></tr>
    <tr><td><code>bg [%id]</code></td><td>Resume suspended job in background.</td></tr>
  </tbody>
</table>

<h2 id="signal-forwarding">Process Group Signals & Terminal Ownership</h2>
<p>When running commands, <code>meridian-shell</code> creates dedicated process groups (<code>setpgid()</code>) and gives controlling terminal ownership via <code>tcsetpgrp()</code> so signals like <code>SIGINT</code> (Ctrl+C) and <code>SIGQUIT</code> (Ctrl+\\) cleanly terminate the foreground pipeline without killing the parent shell.</p>
`
  );

  addArt('terminal-pty', 'PTY', 'TERMINAL', 'implemented',
    'Asynchronous Linux openpty pseudoterminal multiplexer with non-blocking I/O.',
    [
      { id: 'openpty-arch', text: 'Linux openpty Master/Slave Architecture', level: 2 },
      { id: 'async-io', text: 'Non-Blocking Asynchronous I/O Loop', level: 2 },
      { id: 'signal-handling', text: 'Signal Handling & Resizing (SIGWINCH)', level: 2 },
      { id: 'termios-config', text: 'Termios Line Discipline & Raw Mode', level: 2 }
    ],
    `
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
`
  );

  addArt('terminal-commands', 'Commands', 'TERMINAL', 'implemented',
    'Meridian CLI commands, builtins, and subcommands.',
    [
      { id: 'cli-subcommands', text: 'CLI Subcommands Index', level: 2 },
      { id: 'shell-builtins', text: 'Shell Built-in Commands', level: 2 },
      { id: 'developer-commands', text: 'Developer Productivity Subcommands', level: 2 }
    ],
    `
<h2 id="cli-subcommands">CLI Subcommands Index</h2>
<table class="doc-table">
  <thead><tr><th>Command</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>meridian</code></td><td>Launch interactive terminal emulator GUI.</td></tr>
    <tr><td><code>meridian-shell</code></td><td>Launch standalone interactive AST shell.</td></tr>
    <tr><td><code>meridian monitor</code></td><td>Open live CPU, RAM, Disk, Network, and Process metrics dashboard.</td></tr>
    <tr><td><code>meridian git</code></td><td>Inspect Git branch divergence, staged/unstaged changes.</td></tr>
    <tr><td><code>meridian files [dir]</code></td><td>View interactive directory tree explorer with Git badges.</td></tr>
    <tr><td><code>meridian ssh [alias]</code></td><td>Manage and connect to SSH remote workspaces.</td></tr>
    <tr><td><code>meridian plugins</code></td><td>List active extensible plugins and lifecycle hooks.</td></tr>
    <tr><td><code>meridian --performance</code></td><td>Display GPU framerate, PTY latency & telemetry profiler.</td></tr>
    <tr><td><code>meridian pic &lt;file&gt;</code></td><td>Display direct 32-bit RGBA inline image.</td></tr>
  </tbody>
</table>

<h2 id="shell-builtins">Shell Built-in Commands</h2>
<p>Inside <code>meridian-shell</code>, built-in commands run directly inside the process without forking:</p>
<pre><code class="language-bash">cd /var/log         # Change current working directory
export FOO="bar"    # Set environment variable
jobs                # List active background jobs
fg %1               # Foreground job 1
bg %1               # Background job 1
history             # View rich command history
exit 0              # Terminate shell session</code></pre>

<h2 id="developer-commands">Developer Productivity Subcommands</h2>
<p>Developer subcommands can be executed from within any shell or invoked directly from your system PATH:</p>
<pre><code class="language-bash"># Launch live resource monitor
meridian monitor

# Show visual Git status
meridian git

# Open interactive directory browser
meridian files /var/log</code></pre>
`
  );

  addArt('terminal-ssh', 'SSH', 'TERMINAL', 'implemented',
    'SSH workspace manager, connection profiles, and remote session management.',
    [
      { id: 'ssh-mgr', text: 'Native SSH Workspace Manager', level: 2 },
      { id: 'ssh-config-parser', text: '~/.ssh/config Automatic Parsing', level: 2 },
      { id: 'remote-compat', text: 'Remote Terminal Compatibility & Protocols', level: 2 },
      { id: 'ssh-security', text: 'Key Management & Security', level: 2 }
    ],
    `
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
`
  );

  addArt('terminal-keybindings', 'Keybindings', 'TERMINAL', 'implemented',
    'Default key shortcuts for tabs, panes, theme switcher, and command palette.',
    [
      { id: 'shortcuts-table', text: 'Default Keyboard Shortcuts Table', level: 2 },
      { id: 'customizing-keys', text: 'Custom Keymap Configuration', level: 2 },
      { id: 'modifiers-syntax', text: 'Modifier Key Syntax', level: 2 }
    ],
    `
<h2 id="shortcuts-table">Default Keyboard Shortcuts Table</h2>
<table class="doc-table">
  <thead><tr><th>Keybinding</th><th>Action</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>Ctrl+P</code></td><td>Anime Theme Gallery</td><td>Open theme gallery and wallpaper switcher.</td></tr>
    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Open fuzzy-search command palette.</td></tr>
    <tr><td><code>Ctrl+Shift+T</code></td><td>New Tab</td><td>Create a new terminal tab in current working directory.</td></tr>
    <tr><td><code>Ctrl+Shift+W</code></td><td>Close Tab / Pane</td><td>Close the active tab or split pane.</td></tr>
    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Split active window vertically into two panes.</td></tr>
    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Split active window horizontally into two panes.</td></tr>
    <tr><td><code>Ctrl+Shift+Z</code></td><td>Toggle Zoom</td><td>Maximize active pane to fill entire window.</td></tr>
    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search scrollback buffer and command history.</td></tr>
    <tr><td><code>Ctrl+Shift+C</code></td><td>Copy</td><td>Copy selected text to clipboard.</td></tr>
    <tr><td><code>Ctrl+Shift+V</code></td><td>Paste</td><td>Paste clipboard contents into terminal.</td></tr>
    <tr><td><code>Ctrl+L</code></td><td>Clear Screen</td><td>Clear terminal grid while preserving scrollback.</td></tr>
  </tbody>
</table>

<h2 id="customizing-keys">Custom Keymap Configuration</h2>
<p>Override keybindings in <code>~/.config/meridian/keybindings.json</code>:</p>
<pre><code class="language-json">{
  "split_vertical": "Ctrl+Shift+V",
  "split_horizontal": "Ctrl+Shift+H",
  "command_palette": "Ctrl+Space",
  "new_tab": "Ctrl+T"
}</code></pre>

<h2 id="modifiers-syntax">Modifier Key Syntax</h2>
<p>Use standard modifier names: <code>Ctrl</code>, <code>Shift</code>, <code>Alt</code>, <code>Meta</code> / <code>Super</code> combined with <code>+</code>.</p>
`
  );

  addArt('terminal-clipboard', 'Clipboard', 'TERMINAL', 'implemented',
    'OSC 52 remote clipboard synchronization, bracketed paste, and system clipboard.',
    [
      { id: 'clipboard-sync', text: 'System Clipboard Integration', level: 2 },
      { id: 'osc52-spec', text: 'OSC 52 Base64 Remote Clipboard Protocol', level: 2 },
      { id: 'bracketed-paste', text: 'Bracketed Paste Mode', level: 2 },
      { id: 'clipboard-security', text: 'Clipboard Security & Secret Redaction', level: 2 }
    ],
    `
<h2 id="clipboard-sync">System Clipboard Integration</h2>
<p>Meridian integrates with system clipboards across Wayland (<code>wl-clipboard</code>), X11 (<code>xclip</code>/<code>xsel</code>), macOS (<code>pbcopy</code>), and Windows.</p>

<h2 id="osc52-spec">OSC 52 Base64 Remote Clipboard Protocol</h2>
<p>Meridian natively parses OSC 52 sequences (<code>\\033]52;c;&lt;base64&gt;\\033\\\\</code>), allowing CLI tools running on remote SSH servers (e.g. <code>tmux</code>, <code>vim</code>, <code>osc52.sh</code>) to copy text directly into your local machine's clipboard without X11 forwarding.</p>

<pre><code class="language-bash"># Example: Copy string to local clipboard over remote SSH
printf "\\033]52;c;%s\\033\\\\" "$(echo -n "Hello from Remote Server" | base64)"</code></pre>

<h2 id="bracketed-paste">Bracketed Paste Mode</h2>
<p>When enabled by interactive programs (<code>\\033[?2004h</code>), pasted text is enclosed in <code>\\033[200~</code> and <code>\\033[201~</code> markers, preventing accidental execution of newline-separated commands.</p>

<h2 id="clipboard-security">Clipboard Security & Secret Redaction</h2>
<p>Meridian includes an optional clipboard guard that warns the user if pasted content contains sensitive tokens such as private SSH keys or AWS secret access keys.</p>
`
  );

  addArt('terminal-scrollback', 'Scrollback', 'TERMINAL', 'implemented',
    'High-capacity scrollback buffer, search highlighting, and memory limits.',
    [
      { id: 'scrollback-arch', text: 'Scrollback Deque Architecture', level: 2 },
      { id: 'search-engine', text: 'Scrollback Search & Highlighting', level: 2 },
      { id: 'memory-management', text: 'Memory Limits & Performance Tuning', level: 2 }
    ],
    `
<h2 id="scrollback-arch">Scrollback Deque Architecture</h2>
<p><code>vt::ScreenBuffer</code> maintains an efficient circular deque for scrollback history with a configurable limit (default: 10,000 lines). When lines scroll off the top of the grid, they are pushed into the scrollback pool, preserving ANSI colors and hyperlink attributes.</p>

<h2 id="search-engine">Scrollback Search & Highlighting</h2>
<p>Pressing <code>Ctrl+Shift+F</code> activates the incremental search engine with real-time match highlighting, regex support, and case-sensitivity toggles.</p>

<h2 id="memory-management">Memory Limits & Performance Tuning</h2>
<p>Each line in the scrollback buffer is stored with sparse cell arrays, consuming less than 20MB of RAM per 10,000 lines of colored output.</p>
<pre><code class="language-json">{
  "scrollback_lines": 50000
}</code></pre>
`
  );
};
