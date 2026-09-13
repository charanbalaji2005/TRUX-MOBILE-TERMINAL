---
layout: default
title: "Inline Images"
category: "GRAPHICS"
status: "implemented"
---

<h2 id="pic-command">The pic Command</h2>
<p>The <code>pic</code> command displays an image file directly inside the terminal.</p>

<pre><code class="language-bash">pic image.png
# Display the original raster image</code></pre>

<div class="claude-callout-box">
  <div class="callout-header">
    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" x2="12" y1="8" y2="12"/><line x1="12" x2="12.01" y1="16" y2="16"/></svg>
    <span class="callout-title">Important</span>
  </div>
  <p>Meridian Shell renders images as true raster graphics. Images are not converted into ASCII art, Unicode blocks, or other character-based representations.</p>
</div>

<h2 id="rendering-pipeline">Rendering Pipeline</h2>
<p>Images follow a high-performance rendering pipeline designed for clarity and speed.</p>

<div class="pipeline-diagram-wrapper">
  <div class="pipeline-step">
    <div class="step-card">
      <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="1.8"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/></svg>
      <div class="step-title">Image File</div>
      <div class="step-desc">PNG / JPG / WebP</div>
    </div>
  </div>
  <div class="pipeline-arrow">→</div>
  <div class="pipeline-step">
    <div class="step-card">
      <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="1.8"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
      <div class="step-title">Image Decoder</div>
      <div class="step-desc">Decode & Validate</div>
    </div>
  </div>
  <div class="pipeline-arrow">→</div>
  <div class="pipeline-step">
    <div class="step-card">
      <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="1.8"><rect width="20" height="14" x="2" y="3" rx="2"/><line x1="8" x2="16" y1="21" y2="21"/><line x1="12" x2="12" y1="17" y2="21"/></svg>
      <div class="step-title">RGBA Pixel Buffer</div>
      <div class="step-desc">Raw Pixel Data</div>
    </div>
  </div>
  <div class="pipeline-arrow">→</div>
  <div class="pipeline-step">
    <div class="step-card">
      <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="1.8"><ellipse cx="12" cy="5" rx="9" ry="3"/><path d="M3 5v14a9 3 0 0 0 18 0V5"/><path d="M3 12a9 3 0 0 0 18 0"/></svg>
      <div class="step-title">GPU Texture</div>
      <div class="step-desc">Upload to GPU</div>
    </div>
  </div>
  <div class="pipeline-arrow">→</div>
  <div class="pipeline-step">
    <div class="step-card">
      <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="1.8"><rect width="18" height="18" x="3" y="3" rx="2"/><path d="M3 9h18"/><path d="M9 21V9"/></svg>
      <div class="step-title">Graphics Layer</div>
      <div class="step-desc">Terminal Canvas</div>
    </div>
  </div>
  <div class="pipeline-arrow">→</div>
  <div class="pipeline-step">
    <div class="step-card">
      <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="1.8"><rect width="18" height="18" x="3" y="3" rx="2"/><circle cx="9" cy="9" r="2"/><path d="m21 15-3.086-3.086a2 2 0 0 0-2.828 0L6 21"/></svg>
      <div class="step-title">Rendered Image</div>
      <div class="step-desc">On Screen</div>
    </div>
  </div>
</div>

<p>This ensures smooth rendering, proper scaling, and efficient memory usage.</p>

<h2 id="image-formats">Image Formats</h2>
<p>Meridian natively parses standard compressed raster files using <code>graphics::ImageDecoder</code>:</p>
<table class="doc-table">
  <thead><tr><th>Format</th><th>Status</th><th>Channels</th><th>Notes</th></tr></thead>
  <tbody>
    <tr><td>PNG</td><td>Supported</td><td>RGB / RGBA</td><td>Full alpha transparency channel preserved.</td></tr>
    <tr><td>JPEG / JPG</td><td>Supported</td><td>RGB</td><td>Hardware accelerated baseline and progressive decoding.</td></tr>
    <tr><td>WebP</td><td>Supported</td><td>RGB / RGBA</td><td>Lossy and lossless WebP containers.</td></tr>
    <tr><td>GIF</td><td>Supported</td><td>RGB / RGBA</td><td>Single frame and animated GIF playback.</td></tr>
    <tr><td>BMP</td><td>Supported</td><td>RGB / RGBA</td><td>Uncompressed direct raster bit arrays.</td></tr>
  </tbody>
</table>

<h2 id="configuration">Configuration</h2>
<p>Adjust max texture sizes and caching behavior in <code>~/.config/meridian/config.json</code>:</p>
<pre><code class="language-json">{
  "graphics": {
    "max_inline_width": 1920,
    "max_inline_height": 1080,
    "cache_limit_mb": 256,
    "gpu_acceleration": true
  }
}</code></pre>

<h2 id="examples">Examples</h2>
<pre><code class="language-bash"># View image at original aspect ratio
pic diagram.png

# Scale to specific width maintaining height ratio
pic screenshot.png --width 450

# Inspect decoded raster metadata & GPU texture specifications
pic --debug preview.webp

# Wipe all active graphics from the terminal canvas
pic --clear</code></pre>

<h2 id="performance">Performance</h2>
<p>Because decoded raster buffers are uploaded directly to GPU textures, scrolling through terminal output with rendered images causes zero CPU downsampling penalties and maintains a steady 144 FPS.</p>

<h2 id="troubleshooting">Troubleshooting</h2>
<ul>
  <li><strong>Image not found:</strong> Verify the absolute or relative file path.</li>
  <li><strong>GPU Texture Allocation Error:</strong> Ensure OpenGL 3.3 Core or Vulkan drivers are active (<code>meridian --performance</code>).</li>
  <li><strong>Unsupported File:</strong> Convert esoteric vector formats (SVG) or RAW camera files to standard PNG/JPEG.</li>
</ul>