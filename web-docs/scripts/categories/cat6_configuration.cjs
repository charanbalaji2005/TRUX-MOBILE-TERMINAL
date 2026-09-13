module.exports = function(addArt) {
  addArt('config-file', 'Configuration File', 'CONFIGURATION', 'implemented',
    'Specification of ~/.config/meridian/config.json options.',
    [
      { id: 'config-schema', text: 'Complete JSON Schema', level: 2 },
      { id: 'options-table', text: 'Configuration Options Reference', level: 2 },
      { id: 'validation-rules', text: 'JSON Schema Validation & Fallbacks', level: 2 }
    ],
    `
<h2 id="config-schema">Complete JSON Schema</h2>
<p>Location: <code>~/.config/meridian/config.json</code></p>
<pre><code class="language-json">{
  "theme": "sharingan_eye",
  "random_theme": false,
  "font_family": "FantasqueSansMNFM",
  "font_size": 14,
  "line_height": 1.2,
  "cursor_shape": "block",
  "cursor_blink": true,
  "scrollback_lines": 10000,
  "opacity": 0.95,
  "blur": true,
  "ai_enabled": true,
  "telemetry_hud": false
}</code></pre>

<h2 id="options-table">Configuration Options Reference</h2>
<table class="doc-table">
  <thead><tr><th>Key</th><th>Type</th><th>Default</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>theme</code></td><td>string</td><td><code>"sharingan_eye"</code></td><td>Default startup anime artwork theme.</td></tr>
    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Terminal font size in points.</td></tr>
    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Maximum scrollback buffer lines per pane.</td></tr>
    <tr><td><code>opacity</code></td><td>number</td><td><code>0.95</code></td><td>Window background opacity (0.0 - 1.0).</td></tr>
    <tr><td><code>blur</code></td><td>boolean</td><td><code>true</code></td><td>Enable compositor background blur.</td></tr>
    <tr><td><code>cursor_shape</code></td><td>string</td><td><code>"block"</code></td><td>Cursor shape: "block", "beam", or "underline".</td></tr>
  </tbody>
</table>

<h2 id="validation-rules">JSON Schema Validation & Fallbacks</h2>
<p>Meridian automatically validates config files against an internal schema on startup. If a syntax error is present, Meridian logs a warning and falls back to safe defaults without crashing.</p>
`
  );

  addArt('config-themes', 'Themes', 'CONFIGURATION', 'implemented',
    'Configuring anime wallpaper themes and custom wallpapers.',
    [
      { id: 'theme-configuration', text: 'Theme Configuration Options', level: 2 },
      { id: 'custom-wallpaper-guide', text: 'Custom Wallpaper Paths', level: 2 },
      { id: 'wallpaper-blending', text: 'Wallpaper Opacity & Shading', level: 2 }
    ],
    `
<h2 id="theme-configuration">Theme Configuration Options</h2>
<p>Set <code>random_theme: true</code> in <code>config.json</code> to rotate themes automatically on startup.</p>

<h2 id="custom-wallpaper-guide">Custom Wallpaper Paths</h2>
<pre><code class="language-bash"># Set custom background wallpaper via CLI
pic set /home/charanbalaji/Pictures/custom_wallpaper.png</code></pre>

<h2 id="wallpaper-blending">Wallpaper Opacity & Shading</h2>
<p>Configure the background artwork brightness and contrast to preserve text legibility over bright image areas:</p>
<pre><code class="language-json">{
  "wallpaper_opacity": 0.85,
  "wallpaper_dimming": 0.30
}</code></pre>
`
  );

  addArt('config-keybindings', 'Keybindings', 'CONFIGURATION', 'implemented',
    'Customizing keyboard shortcuts via keybindings.json.',
    [
      { id: 'keymap-json', text: 'keybindings.json Specification', level: 2 },
      { id: 'available-actions', text: 'Available Action Identifiers', level: 2 }
    ],
    `
<h2 id="keymap-json">keybindings.json Specification</h2>
<p>Location: <code>~/.config/meridian/keybindings.json</code></p>
<pre><code class="language-json">{
  "new_tab": "Ctrl+Shift+T",
  "close_tab": "Ctrl+Shift+W",
  "split_vertical": "Ctrl+Shift+D",
  "split_horizontal": "Ctrl+Shift+E",
  "command_palette": "Ctrl+Shift+P",
  "theme_gallery": "Ctrl+P",
  "search_scrollback": "Ctrl+Shift+F"
}</code></pre>

<h2 id="available-actions">Available Action Identifiers</h2>
<table class="doc-table">
  <thead><tr><th>Action ID</th><th>Default Shortcut</th><th>Effect</th></tr></thead>
  <tbody>
    <tr><td><code>new_tab</code></td><td><code>Ctrl+Shift+T</code></td><td>Creates a new shell tab.</td></tr>
    <tr><td><code>close_tab</code></td><td><code>Ctrl+Shift+W</code></td><td>Closes the current tab/pane.</td></tr>
    <tr><td><code>split_vertical</code></td><td><code>Ctrl+Shift+D</code></td><td>Splits active pane vertically.</td></tr>
    <tr><td><code>split_horizontal</code></td><td><code>Ctrl+Shift+E</code></td><td>Splits active pane horizontally.</td></tr>
    <tr><td><code>toggle_zoom</code></td><td><code>Ctrl+Shift+Z</code></td><td>Toggles fullscreen pane zoom.</td></tr>
  </tbody>
</table>
`
  );

  addArt('config-profiles', 'Profiles', 'CONFIGURATION', 'implemented',
    'Setting up multiple terminal profiles (Shell, SSH, Containers).',
    [
      { id: 'profile-management', text: 'Profile Configurations', level: 2 },
      { id: 'profiles-json-spec', text: 'profiles.json Example', level: 2 }
    ],
    `
<h2 id="profile-management">Profile Configurations</h2>
<p>Define custom shell startup profiles in <code>~/.config/meridian/profiles.json</code> for Bash, Zsh, Fish, or direct SSH sessions.</p>

<h2 id="profiles-json-spec">profiles.json Example</h2>
<pre><code class="language-json">{
  "default_profile": "zsh",
  "profiles": [
    {
      "name": "zsh",
      "command": "/bin/zsh",
      "args": ["-l"],
      "env": { "SHELL": "/bin/zsh" }
    },
    {
      "name": "docker-dev",
      "command": "/usr/bin/docker",
      "args": ["exec", "-it", "dev-container", "/bin/bash"],
      "theme": "cyberpunk_edge"
    }
  ]
}</code></pre>
`
  );

  addArt('config-env', 'Environment Variables', 'CONFIGURATION', 'implemented',
    'Environment variables read and set by Meridian Terminal.',
    [
      { id: 'env-table', text: 'Environment Variables Reference', level: 2 },
      { id: 'overriding-env', text: 'Overriding Default Environment Variables', level: 2 }
    ],
    `
<h2 id="env-table">Environment Variables Reference</h2>
<table class="doc-table">
  <thead><tr><th>Variable</th><th>Default Value</th><th>Purpose</th></tr></thead>
  <tbody>
    <tr><td><code>TERM</code></td><td><code>xterm-256color</code></td><td>Standard terminal capability identification.</td></tr>
    <tr><td><code>COLORTERM</code></td><td><code>truecolor</code></td><td>Enables 24-bit TrueColor in CLI applications.</td></tr>
    <tr><td><code>MERIDIAN_CONFIG_HOME</code></td><td><code>~/.config/meridian</code></td><td>Override configuration directory path.</td></tr>
    <tr><td><code>MERIDIAN_SESSION_ID</code></td><td><code>UUID</code></td><td>Unique identifier for active terminal window.</td></tr>
  </tbody>
</table>

<h2 id="overriding-env">Overriding Default Environment Variables</h2>
<p>Set custom environment variables in <code>~/.config/meridian/config.json</code> under the <code>"env"</code> dictionary.</p>
`
  );
};
