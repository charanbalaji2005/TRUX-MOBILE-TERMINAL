---
layout: default
title: "Themes"
category: "INTERFACE"
status: "implemented"
---

<h2 id="anime-gallery">Embedded Anime Theme Gallery</h2>
<table class="doc-table">
  <thead><tr><th>Index</th><th>Identifier</th><th>Artwork Description</th></tr></thead>
  <tbody>
    <tr><td>0</td><td><code>sharingan_eye</code></td><td>Sasuke / Itachi Mangekyō Sharingan Eye</td></tr>
    <tr><td>1</td><td><code>sakura_girl</code></td><td>Sakura Blossom Anime Girl</td></tr>
    <tr><td>2</td><td><code>ribbon_girl</code></td><td>Monochrome Anime Ribbon Girl</td></tr>
    <tr><td>3</td><td><code>cyberpunk_edge</code></td><td>Cyberpunk Edgerunners Neon City</td></tr>
    <tr><td>4</td><td><code>mecha_warrior</code></td><td>Gundam Mecha Orbital Warrior</td></tr>
    <tr><td>5</td><td><code>gojo_purple</code></td><td>Gojo Satoru: Hollow Purple (JJK)</td></tr>
    <tr><td>6</td><td><code>sukuna_shrine</code></td><td>Ryomen Sukuna: Malevolent Shrine (JJK)</td></tr>
    <tr><td>7</td><td><code>naruto_rasengan</code></td><td>Naruto Uzumaki: Nine-Tails Rasengan</td></tr>
    <tr><td>8</td><td><code>demon_slayer_tanjiro</code></td><td>Tanjiro Kamado: Sun Breathing</td></tr>
    <tr><td>9</td><td><code>nezuko_blood</code></td><td>Nezuko Kamado: Blood Demon Art</td></tr>
  </tbody>
</table>

<h2 id="theme-selection-cli">CLI Theme Commands (pic set)</h2>
<pre><code class="language-bash">pic set 2               # Set Theme 2 (Ribbon Girl)
pic set gojo_purple     # Set by theme name
pic set random          # Randomize theme on each startup
pic set /path/to/my.png # Set custom user wallpaper</code></pre>

<h2 id="wallpaper-customization">Custom Wallpaper & Scaling Modes</h2>
<p>Configure wallpaper placement modes in <code>config.json</code>: <code>"fit"</code>, <code>"fill"</code>, <code>"center"</code>, or <code>"stretch"</code>.</p>