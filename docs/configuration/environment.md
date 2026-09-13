---
layout: default
title: "Environment Variables"
category: "CONFIGURATION"
status: "implemented"
---

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