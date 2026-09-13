---
layout: default
title: "Graphics Engine"
category: "DEVELOPER"
status: "implemented"
---

<h2 id="decoder-internals">ImageDecoder Internals</h2>
<p><code>graphics::ImageDecoder</code> uses <code>stb_image</code> to decode PNG, JPEG, WebP, GIF, and BMP into <code>std::vector&lt;uint8_t&gt;</code> 32-bit RGBA buffers.</p>

<h2 id="graphic-manager-dev">GraphicManager Texture Cache</h2>
<p>Maintains texture handles and ensures images scale smoothly across high-DPI displays without CPU re-decoding.</p>

<h2 id="aspect-math">Pixel-to-Grid Coordinate Math</h2>
<p>Converts terminal cell row/column coordinates into normalized OpenGL viewport coordinates, accounting for cell padding and fractional font metrics.</p>