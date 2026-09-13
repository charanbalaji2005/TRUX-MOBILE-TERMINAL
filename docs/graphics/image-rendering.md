---
layout: default
title: "Image Rendering"
category: "GRAPHICS"
status: "implemented"
---

<h2 id="zero-ascii-spec">Zero-ASCII Engineering Guarantee</h2>
<p>Meridian Shell guarantees that images displayed via <code>pic</code> are rendered as genuine 32-bit hardware textures on the GPU canvas. Meridian contains zero ASCII, Unicode block, half-block, or Braille converters.</p>

<h2 id="comparison-table">Raster vs. Text Cell Approximation Comparison</h2>
<table class="doc-table">
  <thead><tr><th>Feature</th><th>Meridian Direct Raster</th><th>Legacy ASCII / Half-Blocks</th></tr></thead>
  <tbody>
    <tr><td>Pixel Resolution</td><td>Native Image Resolution (e.g. 1920x1080)</td><td>Limited to Cell Grid (e.g. 80x24)</td></tr>
    <tr><td>Color Accuracy</td><td>32-bit Full RGBA (16.7M colors + Alpha)</td><td>256 colors or ANSI approximations</td></tr>
    <tr><td>Distortion</td><td>Zero aspect distortion</td><td>Extreme pixelation and character gaps</td></tr>
    <tr><td>GPU Acceleration</td><td>Hardware texture blitting</td><td>CPU text-character generation</td></tr>
  </tbody>
</table>

<h2 id="memory-footprint">VRAM Footprint & GPU Allocation</h2>
<p>Images are automatically uploaded to dedicated GPU textures with Mipmapping enabled for crisp downscaling and minimal VRAM consumption.</p>