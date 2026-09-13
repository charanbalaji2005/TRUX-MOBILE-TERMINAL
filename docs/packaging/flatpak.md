---
layout: default
title: "Flatpak"
category: "PACKAGING"
status: "planned"
---

<h2 id="flatpak-roadmap-spec">Flatpak Container Roadmap</h2>
<p>Flatpak packaging with <code>org.freedesktop.Flatpak</code> Host PTY permissions is currently planned.</p>

<h2 id="flatpak-permissions">Host PTY & Windowing Permissions</h2>
<p>Terminal emulators require <code>--talk-name=org.freedesktop.Flatpak</code> to spawn host processes via <code>flatpak-spawn --host</code>.</p>