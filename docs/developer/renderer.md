---
layout: default
title: "Renderer"
category: "DEVELOPER"
status: "implemented"
---

<h2 id="render-pipeline-dev">Render Pipeline Design</h2>
<p><code>renderer::RenderPipeline</code> batches background color quads, glyph foreground quads, and raster image textures into unified draw calls.</p>

<h2 id="instancing">Hardware Instanced Quad Rendering</h2>
<p>Draws thousands of terminal glyphs in a single GPU draw call using instanced vertex buffers (position, UV texture coordinates, foreground color, background color).</p>

<h2 id="atlas-management">Dynamic Atlas Eviction & Bin Packing</h2>
<p><code>renderer::GlyphAtlas</code> employs a guillotine 2D bin-packing algorithm to pack variable-width Unicode characters, emoji, and Powerline symbols into GPU texture atlases.</p>