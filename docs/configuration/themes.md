---
layout: default
title: "Themes"
category: "CONFIGURATION"
status: "implemented"
---

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