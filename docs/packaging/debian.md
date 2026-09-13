---
layout: default
title: "Ubuntu / Debian / DEB"
category: "PACKAGING"
status: "implemented"
---

<h2 id="debian-deb">Ubuntu & Debian DEB Package</h2>
<pre><code class="language-bash">sudo apt update
sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>

<h2 id="deb-packaging">Building .deb Packages</h2>
<pre><code class="language-bash">dpkg-deb --build packaging/debian meridian-terminal_2.0.0_amd64.deb</code></pre>