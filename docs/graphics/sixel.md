---
layout: default
title: "Sixel"
category: "GRAPHICS"
status: "development"
---

<h2 id="sixel-spec">DEC Sixel Graphics Decoding</h2>
<p>Meridian decodes DCS Sixel streams (<code>\033Pq...\033\\</code>), converting 6-pixel vertical slivers into hardware bitmap textures.</p>

<h2 id="gnuplot-compat">Compatibility with Gnuplot & CLI Plotting</h2>
<pre><code class="language-bash"># Render inline scientific plots directly in terminal
gnuplot -e "set terminal sixelgd; plot sin(x)"</code></pre>

<h2 id="raster-conversion">Sixel Strip to Texture Conversion</h2>
<p>Each Sixel character represents a 6-bit vertical slice of pixels mapped to a color register in the 256-color palette table.</p>