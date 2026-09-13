module.exports = function(addArt) {
  addArt('graphics-gpu', 'GPU Renderer', 'GRAPHICS', 'development',
    'Hardware-accelerated shader pipeline, damage tracking, and glyph texture atlas.',
    [
      { id: 'gpu-pipeline', text: 'GPU Render Pipeline Architecture', level: 2 },
      { id: 'damage-tracking', text: 'Damage Rect Tracking Engine', level: 2 },
      { id: 'glyph-atlas-engine', text: 'Sub-Pixel Glyph Texture Atlas', level: 2 },
      { id: 'instancing-dev', text: 'Instanced Quad Rendering', level: 2 }
    ],
    `
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
`
  );

  addArt('graphics-inline-images', 'Inline Images', 'GRAPHICS', 'implemented',
    "Display real raster images directly inside Meridian Shell without converting them into ASCII characters or Unicode blocks. Meridian Shell's graphics system allows images to be rendered at full color and resolution using GPU acceleration.",
    [
      { id: 'pic-command', text: 'The pic Command', level: 2 },
      { id: 'rendering-pipeline', text: 'Rendering Pipeline', level: 2 },
      { id: 'image-formats', text: 'Image Formats', level: 2 },
      { id: 'configuration', text: 'Configuration', level: 2 },
      { id: 'examples', text: 'Examples', level: 2 },
      { id: 'performance', text: 'Performance', level: 2 },
      { id: 'troubleshooting', text: 'Troubleshooting', level: 2 }
    ],
    `
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
`
  );

  addArt('graphics-formats', 'Image Formats', 'GRAPHICS', 'implemented',
    'Supported image formats: PNG, JPEG, WebP, GIF, and BMP.',
    [
      { id: 'format-matrix', text: 'Supported Image Formats Matrix', level: 2 },
      { id: 'color-channels', text: 'Color Channels & Alpha Blending', level: 2 },
      { id: 'decoding-pipeline', text: 'High-Performance Decoding Pipeline', level: 2 }
    ],
    `
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
`
  );

  addArt('graphics-gif', 'GIF', 'GRAPHICS', 'implemented',
    'Animated GIF playback and frame delay timing.',
    [
      { id: 'gif-engine', text: 'Animated GIF Playback Engine', level: 2 },
      { id: 'frame-timing', text: 'Frame Delay & Loop Controls', level: 2 },
      { id: 'memory-management-gif', text: 'Frame Cache & Texture Cycling', level: 2 }
    ],
    `
<h2 id="gif-engine">Animated GIF Playback Engine</h2>
<p><code>graphics::GraphicManager</code> decodes multi-frame GIF images into individual frame texture arrays, managing playback timers according to embedded frame delays.</p>

<h2 id="frame-timing">Frame Delay & Loop Controls</h2>
<pre><code class="language-bash">pic anime_reaction.gif     # Play animated GIF at native frame rate
pic animation.gif --loop 3  # Play GIF for 3 iterations then pause</code></pre>

<h2 id="memory-management-gif">Frame Cache & Texture Cycling</h2>
<p>Frames are stored in contiguous GPU texture 2D arrays, cycling texture slices on timer expiration with negligible CPU impact.</p>
`
  );

  addArt('graphics-kitty', 'Kitty Graphics', 'GRAPHICS', 'development',
    'Chunked 2048-byte transmission protocol with m=1/m=0 control frames.',
    [
      { id: 'kitty-protocol', text: 'Kitty Graphics Protocol Specification', level: 2 },
      { id: 'chunking-safety', text: '2048-Byte Safe Transmission Chunking', level: 2 },
      { id: 'protocol-parameters', text: 'Supported APC Control Keys', level: 2 }
    ],
    `
<h2 id="kitty-protocol">Kitty Graphics Protocol Specification</h2>
<p>Meridian parses Kitty Graphics escape sequences (<code>\\033_G...\\033\\\\</code>) for seamless compatibility with tools like <code>icat</code>, <code>yazi</code>, and <code>ranger</code>.</p>

<h2 id="chunking-safety">2048-Byte Safe Transmission Chunking</h2>
<p>Payloads exceeding 2048 bytes are transmitted in chunks using <code>m=1</code> (more data follows) and <code>m=0</code> (final payload chunk), preventing buffer overflows in PTY drivers.</p>

<h2 id="protocol-parameters">Supported APC Control Keys</h2>
<table class="doc-table">
  <thead><tr><th>Key</th><th>Name</th><th>Values</th></tr></thead>
  <tbody>
    <tr><td><code>a</code></td><td>Action</td><td><code>t</code> (transmit & display), <code>d</code> (delete), <code>p</code> (place)</td></tr>
    <tr><td><code>f</code></td><td>Format</td><td><code>32</code> (RGBA), <code>24</code> (RGB), <code>100</code> (PNG)</td></tr>
    <tr><td><code>m</code></td><td>More Chunks</td><td><code>1</code> (chunk follows), <code>0</code> (final chunk)</td></tr>
    <tr><td><code>s</code> / <code>v</code></td><td>Size</td><td>Width and Height in pixels</td></tr>
  </tbody>
</table>
`
  );

  addArt('graphics-sixel', 'Sixel', 'GRAPHICS', 'development',
    'DEC Sixel graphics protocol decoding and 256-color palette mapping.',
    [
      { id: 'sixel-spec', text: 'DEC Sixel Graphics Decoding', level: 2 },
      { id: 'gnuplot-compat', text: 'Compatibility with Gnuplot & CLI Plotting', level: 2 },
      { id: 'raster-conversion', text: 'Sixel Strip to Texture Conversion', level: 2 }
    ],
    `
<h2 id="sixel-spec">DEC Sixel Graphics Decoding</h2>
<p>Meridian decodes DCS Sixel streams (<code>\\033Pq...\\033\\\\</code>), converting 6-pixel vertical slivers into hardware bitmap textures.</p>

<h2 id="gnuplot-compat">Compatibility with Gnuplot & CLI Plotting</h2>
<pre><code class="language-bash"># Render inline scientific plots directly in terminal
gnuplot -e "set terminal sixelgd; plot sin(x)"</code></pre>

<h2 id="raster-conversion">Sixel Strip to Texture Conversion</h2>
<p>Each Sixel character represents a 6-bit vertical slice of pixels mapped to a color register in the 256-color palette table.</p>
`
  );

  addArt('graphics-rendering', 'Image Rendering', 'GRAPHICS', 'implemented',
    'Detailed comparison between direct raster rendering and character-cell approximations.',
    [
      { id: 'zero-ascii-spec', text: 'Zero-ASCII Engineering Guarantee', level: 2 },
      { id: 'comparison-table', text: 'Raster vs. Text Cell Approximation Comparison', level: 2 },
      { id: 'memory-footprint', text: 'VRAM Footprint & GPU Allocation', level: 2 }
    ],
    `
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
`
  );
};
