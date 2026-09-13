---
layout: default
title: "Image Formats"
category: "GRAPHICS"
status: "implemented"
---

<h2 id="format-matrix">Supported Image Formats Matrix</h2>
<table class="doc-table">
  <thead><tr><th>Format</th><th>Extension</th><th>Decoding Engine</th><th>Alpha Channel</th></tr></thead>
  <tbody>
    <tr><td>PNG</td><td><code>.png</code></td><td>stb_image / Native</td><td>Yes (RGBA8888)</td></tr>
    <tr><td>JPEG / JPG</td><td><code>.jpg</code>, <code>.jpeg</code></td><td>stb_image / Native</td><td>No (RGB888)</td></tr>
    <tr><td>WebP</td><td><code>.webp</code></td><td>libwebp / stb_image</td><td>Yes (RGBA8888)</td></tr>
    <tr><td>GIF</td><td><code>.gif</code></td><td>stb_image / Animated</td><td>Yes (Indexed / RGBA)</td></tr>
    <tr><td>BMP</td><td><code>.bmp</code></td><td>stb_image / Native</td><td>Optional</td></tr>
  </tbody>
</table>

<h2 id="color-channels">Color Channels & Alpha Blending</h2>
<p>All decoded images are normalized to 32-bit RGBA with full premultiplied alpha compositing over terminal background wallpapers and color themes.</p>

<h2 id="decoding-pipeline">High-Performance Decoding Pipeline</h2>
<p>Image loading and decompression are performed asynchronously in background worker threads, preventing large 4K / 8K image files from causing terminal frame drops or PTY input lag.</p>