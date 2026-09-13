---
layout: default
title: "Screen Buffer"
category: "DEVELOPER"
status: "implemented"
---

<h2 id="cell-struct">Cell & Attributes Data Structure</h2>
<p>Each screen cell in <code>vt::ScreenBuffer</code> contains:</p>
<pre><code class="language-cpp">struct Cell {
    char32_t codepoint = U' ';
    uint8_t width = 1;
    Attributes attrs;
};

struct Attributes {
    Color fg = Color::Default();
    Color bg = Color::Default();
    uint8_t flags = 0; // Bold, Italic, Underline, Inverse
    uint32_t hyperlink_id = 0;
};</code></pre>

<h2 id="grid-manipulation">Grid Scrolling & Line Wrapping</h2>
<p>Supports full scrolling regions (<code>DECSTBM</code>), cursor-relative movement, insert/delete lines, and auto-wrapping.</p>

<h2 id="hyperlink-table">Hyperlink Attribute Registry</h2>
<p>Hyperlink URIs from OSC 8 sequences are interned into a deduplicated memory table, associating a lightweight 32-bit ID with each cell without duplicating URL string allocations across thousands of character cells.</p>