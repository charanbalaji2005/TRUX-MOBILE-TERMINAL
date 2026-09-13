module.exports = function(addArt) {
  addArt('ref-cli', 'CLI Reference', 'REFERENCE', 'implemented',
    'Complete command-line interface options and subcommands.',
    [
      { id: 'cli-full-ref', text: 'Complete CLI Command Reference', level: 2 },
      { id: 'exit-codes', text: 'CLI Exit Codes', level: 2 }
    ],
    `
<h2 id="cli-full-ref">Complete CLI Command Reference</h2>
<table class="doc-table">
  <thead><tr><th>Command</th><th>Arguments</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>meridian</code></td><td><code>[none]</code></td><td>Launch interactive terminal emulator GUI.</td></tr>
    <tr><td><code>meridian-shell</code></td><td><code>[-c &quot;cmd&quot;]</code></td><td>Launch standalone AST shell process.</td></tr>
    <tr><td><code>meridian ssh</code></td><td><code>[alias]</code></td><td>Manage and connect to SSH remote workspaces.</td></tr>
    <tr><td><code>meridian plugins</code></td><td><code>[none]</code></td><td>List active extensible plugins and lifecycle hooks.</td></tr>
    <tr><td><code>meridian --performance</code></td><td><code>[none]</code></td><td>Display GPU framerate and telemetry profiler HUD.</td></tr>
    <tr><td><code>meridian monitor</code></td><td><code>[none]</code></td><td>Open live CPU, RAM, Disk, and Network dashboard.</td></tr>
    <tr><td><code>meridian git</code></td><td><code>[none]</code></td><td>Inspect Git branch divergence and staged changes.</td></tr>
    <tr><td><code>meridian pic</code></td><td><code>&lt;file&gt;</code></td><td>Display direct 32-bit RGBA inline image.</td></tr>
    <tr><td><code>meridian ask</code></td><td><code>&quot;&lt;intent&gt;&quot;</code></td><td>Translate natural language to safe shell command.</td></tr>
    <tr><td><code>meridian diag</code></td><td><code>&quot;&lt;error&gt;&quot;</code></td><td>Diagnose compiler / runtime errors with suggested fixes.</td></tr>
  </tbody>
</table>

<h2 id="exit-codes">CLI Exit Codes</h2>
<table class="doc-table">
  <thead><tr><th>Code</th><th>Meaning</th></tr></thead>
  <tbody>
    <tr><td><code>0</code></td><td>Success / Clean exit.</td></tr>
    <tr><td><code>1</code></td><td>General command failure / parse error.</td></tr>
    <tr><td><code>126</code></td><td>Command found but not executable.</td></tr>
    <tr><td><code>127</code></td><td>Command not found.</td></tr>
    <tr><td><code>130</code></td><td>Process terminated by SIGINT (Ctrl+C).</td></tr>
  </tbody>
</table>
`
  );

  addArt('ref-config', 'Configuration Reference', 'REFERENCE', 'implemented',
    'Complete schema documentation for config.json.',
    [
      { id: 'config-keys-ref', text: 'Configuration Keys Reference Table', level: 2 }
    ],
    `
<h2 id="config-keys-ref">Configuration Keys Reference Table</h2>
<table class="doc-table">
  <thead><tr><th>Key</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>theme</code></td><td>string</td><td><code>"sharingan_eye"</code></td><td>Default anime wallpaper theme identifier.</td></tr>
    <tr><td><code>random_theme</code></td><td>boolean</td><td><code>false</code></td><td>Rotate anime wallpaper on startup.</td></tr>
    <tr><td><code>font_family</code></td><td>string</td><td><code>"FantasqueSansMNFM"</code></td><td>Primary font family name.</td></tr>
    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Font size in points.</td></tr>
    <tr><td><code>line_height</code></td><td>number</td><td><code>1.2</code></td><td>Line height multiplier.</td></tr>
    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Maximum scrollback buffer lines.</td></tr>
    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity (0.0 to 1.0).</td></tr>
    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>
    <tr><td><code>cursor_shape</code></td><td>string</td><td><code>"block"</code></td><td>Cursor shape: "block", "beam", or "underline".</td></tr>
    <tr><td><code>cursor_blink</code></td><td>boolean</td><td><code>true</code></td><td>Enable cursor blinking animation.</td></tr>
    <tr><td><code>ai_enabled</code></td><td>boolean</td><td><code>true</code></td><td>Enable local offline AI assistance.</td></tr>
    <tr><td><code>telemetry_hud</code></td><td>boolean</td><td><code>false</code></td><td>Show live GPU telemetry HUD overlay.</td></tr>
  </tbody>
</table>
`
  );

  addArt('ref-shortcuts', 'Keyboard Shortcuts', 'REFERENCE', 'implemented',
    'Complete table of all default keyboard shortcuts.',
    [
      { id: 'all-shortcuts-ref', text: 'All Keyboard Shortcuts Reference', level: 2 }
    ],
    `
<h2 id="all-shortcuts-ref">All Keyboard Shortcuts Reference</h2>
<table class="doc-table">
  <thead><tr><th>Shortcut</th><th>Action</th><th>Category</th></tr></thead>
  <tbody>
    <tr><td><code>Ctrl+P</code></td><td>Anime Theme Gallery</td><td>Themes</td></tr>
    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Navigation</td></tr>
    <tr><td><code>Ctrl+Shift+T</code></td><td>New Tab</td><td>Tabs</td></tr>
    <tr><td><code>Ctrl+Shift+W</code></td><td>Close Tab / Pane</td><td>Windows</td></tr>
    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Panes</td></tr>
    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Panes</td></tr>
    <tr><td><code>Ctrl+Shift+Z</code></td><td>Toggle Pane Zoom</td><td>Panes</td></tr>
    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search</td></tr>
    <tr><td><code>Ctrl+Shift+C</code></td><td>Copy Selection</td><td>Clipboard</td></tr>
    <tr><td><code>Ctrl+Shift+V</code></td><td>Paste Clipboard</td><td>Clipboard</td></tr>
    <tr><td><code>Ctrl+L</code></td><td>Clear Screen</td><td>Terminal</td></tr>
    <tr><td><code>Ctrl+PageUp</code> / <code>Ctrl+PageDown</code></td><td>Switch Tabs</td><td>Tabs</td></tr>
  </tbody>
</table>
`
  );

  addArt('ref-env', 'Environment Variables', 'REFERENCE', 'implemented',
    'Environment variables used and exported by Meridian.',
    [
      { id: 'all-env-ref', text: 'Environment Variables Reference', level: 2 }
    ],
    `
<h2 id="all-env-ref">Environment Variables Reference</h2>
<table class="doc-table">
  <thead><tr><th>Variable</th><th>Default</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>TERM</code></td><td><code>xterm-256color</code></td><td>Terminal emulator type.</td></tr>
    <tr><td><code>COLORTERM</code></td><td><code>truecolor</code></td><td>Indicates 24-bit TrueColor RGB support.</td></tr>
    <tr><td><code>MERIDIAN_CONFIG_HOME</code></td><td><code>~/.config/meridian</code></td><td>Override configuration directory.</td></tr>
    <tr><td><code>MERIDIAN_SESSION_ID</code></td><td><code>UUID</code></td><td>Unique session identifier for active window.</td></tr>
    <tr><td><code>MERIDIAN_THEME</code></td><td><code>string</code></td><td>Active anime wallpaper identifier.</td></tr>
  </tbody>
</table>
`
  );

  addArt('ref-faq', 'FAQ', 'REFERENCE', 'implemented',
    'Frequently asked questions about Meridian Shell.',
    [
      { id: 'faq-list', text: 'Frequently Asked Questions', level: 2 }
    ],
    `
<h2 id="faq-list">Frequently Asked Questions</h2>
<p><strong>Q: Does Meridian render real images or ASCII approximations?</strong><br/>
A: Meridian renders genuine 32-bit RGBA hardware raster graphics on the GPU canvas with zero ASCII or half-block approximations.</p>

<p><strong>Q: Is Meridian a fake shell or real terminal?</strong><br/>
A: Meridian is a true terminal emulator running real Linux PTY sessions connected directly to <code>bash</code>, <code>zsh</code>, or <code>fish</code>.</p>

<p><strong>Q: Can I use Meridian over SSH without remote installations?</strong><br/>
A: Yes! Meridian uses standard ANSI and OSC escape sequences that work seamlessly over any remote SSH connection.</p>

<p><strong>Q: Does Meridian require an internet connection?</strong><br/>
A: No. Meridian and its AI engine operate 100% offline with zero cloud dependencies.</p>
`
  );

  addArt('ref-troubleshooting', 'Troubleshooting', 'REFERENCE', 'implemented',
    'Solutions for font rendering, PTY permissions, and graphical display issues.',
    [
      { id: 'troubleshooting-guide', text: 'Troubleshooting Common Issues', level: 2 }
    ],
    `
<h2 id="troubleshooting-guide">Troubleshooting Common Issues</h2>
<ul>
  <li><strong>Missing Powerline glyphs:</strong> Install a Nerd Font (e.g. <code>FantasqueSansM Nerd Font</code> or <code>JetBrainsMono Nerd Font</code>).</li>
  <li><strong>Image display issues:</strong> Verify that GPU drivers (Mesa / NVIDIA) are active and supporting OpenGL 3.3 Core.</li>
  <li><strong>PTY Permission Denied:</strong> Ensure your user account is in the <code>tty</code> group or that <code>/dev/pts</code> is mounted with standard permissions.</li>
  <li><strong>Keybinding conflicts:</strong> Inspect <code>~/.config/meridian/keybindings.json</code> for duplicate shortcut mappings.</li>
</ul>
`
  );
};
