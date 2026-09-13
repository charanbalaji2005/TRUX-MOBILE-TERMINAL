---
layout: default
title: "GUI"
category: "INTERFACE"
status: "development"
---

<h2 id="gui-arch">GUI Windowing Architecture</h2>
<p>Meridian's GUI layer embeds the high-speed PTY engine and OpenGL/Vulkan canvas into native desktop window chrome, providing responsive rendering and native platform integration.</p>

<h2 id="wayland-x11">Wayland & X11 Compositor Integration</h2>
<p>Supports fractional scaling, client-side decorations (CSD), server-side decorations (SSD), and sub-pixel glyph rendering across GNOME, KDE Plasma, and Hyprland.</p>

<h2 id="scaling-dpi">HiDPI & Fractional Scaling</h2>
<p>Under Wayland <code>wp-fractional-scale-v1</code>, Meridian dynamically scales glyph texture atlases to exact fractional physical pixel dimensions (125%, 150%, 175%), completely avoiding blurry bitmap bilinear interpolation.</p>