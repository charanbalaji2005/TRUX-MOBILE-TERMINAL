---
layout: default
title: "GIF"
category: "GRAPHICS"
status: "implemented"
---

<h2 id="gif-engine">Animated GIF Playback Engine</h2>
<p><code>graphics::GraphicManager</code> decodes multi-frame GIF images into individual frame texture arrays, managing playback timers according to embedded frame delays.</p>

<h2 id="frame-timing">Frame Delay & Loop Controls</h2>
<pre><code class="language-bash">pic anime_reaction.gif     # Play animated GIF at native frame rate
pic animation.gif --loop 3  # Play GIF for 3 iterations then pause</code></pre>

<h2 id="memory-management-gif">Frame Cache & Texture Cycling</h2>
<p>Frames are stored in contiguous GPU texture 2D arrays, cycling texture slices on timer expiration with negligible CPU impact.</p>