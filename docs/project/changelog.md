---
layout: default
title: "Changelog"
category: "PROJECT"
status: "implemented"
---

<h2 id="changelog-2-0">Version 2.0.0 Release Notes (August 2026)</h2>
<ul>
  <li><strong>Direct Raster Image Rendering:</strong> Added <code>pic</code> command supporting PNG, JPEG, WebP, GIF, BMP with 32-bit RGBA hardware decoding (zero ASCII).</li>
  <li><strong>Advanced Protocol Suite:</strong> Implemented OSC 8 Hyperlinks, OSC 52 Remote Clipboard sync, OSC 7 Working Directory tracking, and OSC 133 semantic prompt markers.</li>
  <li><strong>Native SSH Workspace Manager:</strong> Added <code>meridian ssh</code> for direct host launching from <code>~/.ssh/config</code>.</li>
  <li><strong>Extensible Plugin Engine:</strong> Integrated lifecycle hooks (pre/post execution, background watchers) under <code>~/.config/meridian/plugins/</code>.</li>
  <li><strong>GPU Telemetry Profiler:</strong> Integrated live framerate, frame time, glyph cache hits, and VRAM monitoring (<code>meridian --performance</code>).</li>
  <li><strong>Automated Test Suite:</strong> Reached 133 comprehensive unit and integration tests passing with 435 assertions.</li>
</ul>

<h2 id="changelog-1-5">Version 1.5.0 Release Notes (May 2026)</h2>
<ul>
  <li>Initial implementation of POSIX openpty multiplexer.</li>
  <li>Basic ANSI/VT escape sequence parsing and 24-bit TrueColor RGB.</li>
  <li>Split panes and multi-tab interface.</li>
</ul>