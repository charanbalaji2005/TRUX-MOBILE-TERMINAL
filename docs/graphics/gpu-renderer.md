---
layout: default
title: "GPU Renderer"
category: "GRAPHICS"
status: "development"
---

<h2 id="gpu-pipeline">GPU Render Pipeline Architecture</h2>
<p>Meridian's GPU renderer transforms terminal cell matrices into instanced draw calls executed on OpenGL 3.3 Core and Vulkan backends. By offloading text glyph blitting, background colors, and raster images to dedicated shader pipelines, Meridian sustains fluid 144Hz+ rendering without consuming significant CPU resources.</p>

<div class="arch-diagram-block"><pre><code class="language-text">ScreenBuffer Grid Cells
        │ (dirty lines)
        ▼
   DamageTracker (Calculates Minimal Bounding Box)
        │
   GlyphAtlas (FreeType / HarfBuzz Sub-Pixel Cache)
        │
   Instanced Quad VBO (Positions, UVs, RGB Fg/Bg)
        │
   GPU Shaders (OpenGL 3.3 Core / Vulkan) ──► Display VSync (144 FPS)</code></pre></div>

<h2 id="damage-tracking">Damage Rect Tracking Engine</h2>
<p><code>renderer::DamageTracker</code> records bounding boxes of modified cells per frame. Frames with partial updates only re-render dirty regions, saving GPU cycles and laptop battery power.</p>

<h2 id="glyph-atlas-engine">Sub-Pixel Glyph Texture Atlas</h2>
<p><code>renderer::GlyphAtlas</code> rasterizes character glyphs with sub-pixel anti-aliasing into a high-density 2048x2048 texture atlas, achieving sub-millisecond glyph retrieval.</p>

<h2 id="instancing-dev">Instanced Quad Rendering</h2>
<p>Rather than issuing individual draw calls for each character cell, Meridian batches up to 20,000 glyphs into a single instanced quad array buffer, achieving ultra-low render latency under 1.2ms per frame.</p>