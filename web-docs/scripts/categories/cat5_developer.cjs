module.exports = function(addArt) {
  addArt('dev-architecture', 'Architecture', 'DEVELOPER', 'implemented',
    'Architecture breakdown of PTY, VT parser, ScreenBuffer, and renderer layers.',
    [
      { id: 'layers-breakdown', text: 'Architectural Layer Breakdown', level: 2 },
      { id: 'data-flow', text: 'End-to-End Data Flow', level: 2 },
      { id: 'threading-model', text: 'Threading & Concurrency Model', level: 2 }
    ],
    `
<h2 id="layers-breakdown">Architectural Layer Breakdown</h2>
<p>Meridian is engineered as modular C++20 subsystems designed to isolate the core terminal state machine from the windowing environment and background telemetry profilers.</p>

<h2 id="data-flow">End-to-End Data Flow</h2>
<div class="arch-diagram-block"><pre><code class="language-text">User Keystroke ──► Window Event Loop ──► PTY Master FD ──► Kernel PTY ──► Shell Process (bash/zsh)
                                                                                  │ (stdout/stderr)
Display Frame  ◄── GPU Render Pipeline ◄── DamageTracker ◄── ScreenBuffer ◄── ANSI State Machine</code></pre></div>

<h2 id="threading-model">Threading & Concurrency Model</h2>
<p>The I/O read thread continuously polls master PTY file descriptors using non-blocking epoll, buffering raw byte chunks into lock-free ring buffers before handing them off to the ANSI parser thread.</p>
`
  );

  addArt('dev-pty-arch', 'PTY Architecture', 'DEVELOPER', 'implemented',
    'POSIX openpty master/slave descriptor management and asynchronous I/O loops.',
    [
      { id: 'pty-lifecycle', text: 'PTY Descriptor Lifecycle', level: 2 },
      { id: 'termios-spec', text: 'POSIX Termios Configuration', level: 2 },
      { id: 'process-spawning', text: 'Process Spawning & Environment Inheritance', level: 2 }
    ],
    `
<h2 id="pty-lifecycle">PTY Descriptor Lifecycle</h2>
<p><code>pty::PTYSession</code> handles fork/execve, establishes slave descriptor as controlling tty (<code>setsid()</code>), and redirects stdin/stdout/stderr.</p>

<h2 id="termios-spec">POSIX Termios Configuration</h2>
<p>Configures raw mode flags (<code>ICANON</code>, <code>ECHO</code>, <code>ISIG</code>) to allow interactive curses and shell line editors to manage raw keypresses.</p>

<h2 id="process-spawning">Process Spawning & Environment Inheritance</h2>
<p>When launching a new tab, Meridian passes sanitized environment variables while injecting standard terminal indicators (<code>TERM=xterm-256color</code>, <code>COLORTERM=truecolor</code>).</p>
`
  );

  addArt('dev-screen-buffer', 'Screen Buffer', 'DEVELOPER', 'implemented',
    '2D cell matrix, damage tracking, and cursor management.',
    [
      { id: 'cell-struct', text: 'Cell & Attributes Data Structure', level: 2 },
      { id: 'grid-manipulation', text: 'Grid Scrolling & Line Wrapping', level: 2 },
      { id: 'hyperlink-table', text: 'Hyperlink Attribute Registry', level: 2 }
    ],
    `
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
`
  );

  addArt('dev-renderer', 'Renderer', 'DEVELOPER', 'implemented',
    'Glyph caching, texture atlas, and GPU batching.',
    [
      { id: 'render-pipeline-dev', text: 'Render Pipeline Design', level: 2 },
      { id: 'instancing', text: 'Hardware Instanced Quad Rendering', level: 2 },
      { id: 'atlas-management', text: 'Dynamic Atlas Eviction & Bin Packing', level: 2 }
    ],
    `
<h2 id="render-pipeline-dev">Render Pipeline Design</h2>
<p><code>renderer::RenderPipeline</code> batches background color quads, glyph foreground quads, and raster image textures into unified draw calls.</p>

<h2 id="instancing">Hardware Instanced Quad Rendering</h2>
<p>Draws thousands of terminal glyphs in a single GPU draw call using instanced vertex buffers (position, UV texture coordinates, foreground color, background color).</p>

<h2 id="atlas-management">Dynamic Atlas Eviction & Bin Packing</h2>
<p><code>renderer::GlyphAtlas</code> employs a guillotine 2D bin-packing algorithm to pack variable-width Unicode characters, emoji, and Powerline symbols into GPU texture atlases.</p>
`
  );

  addArt('dev-graphics-engine', 'Graphics Engine', 'DEVELOPER', 'implemented',
    'stb_image decoding, aspect fitting, and image caching.',
    [
      { id: 'decoder-internals', text: 'ImageDecoder Internals', level: 2 },
      { id: 'graphic-manager-dev', text: 'GraphicManager Texture Cache', level: 2 },
      { id: 'aspect-math', text: 'Pixel-to-Grid Coordinate Math', level: 2 }
    ],
    `
<h2 id="decoder-internals">ImageDecoder Internals</h2>
<p><code>graphics::ImageDecoder</code> uses <code>stb_image</code> to decode PNG, JPEG, WebP, GIF, and BMP into <code>std::vector&lt;uint8_t&gt;</code> 32-bit RGBA buffers.</p>

<h2 id="graphic-manager-dev">GraphicManager Texture Cache</h2>
<p>Maintains texture handles and ensures images scale smoothly across high-DPI displays without CPU re-decoding.</p>

<h2 id="aspect-math">Pixel-to-Grid Coordinate Math</h2>
<p>Converts terminal cell row/column coordinates into normalized OpenGL viewport coordinates, accounting for cell padding and fractional font metrics.</p>
`
  );

  addArt('dev-ai-engine', 'AI Engine', 'DEVELOPER', 'implemented',
    'Offline natural language intent translation, interactive typo correction popup, compiler diagnostics, and autonomous coding agent.',
    [
      { id: 'typo-popup-algorithm', text: 'Interactive AI Typo Correction Popup & Algorithm', level: 2 },
      { id: 'intent-engine-dev', text: 'Intent Engine (meridian ask)', level: 2 },
      { id: 'diagnostics-dev', text: 'Compiler & Runtime Diagnostics (meridian diag)', level: 2 },
      { id: 'agent-dev', text: 'Autonomous Coding Agent (meridian agent)', level: 2 }
    ],
    `
<h2 id="typo-popup-algorithm">Interactive AI Typo Correction Popup & Algorithm</h2>
<p>When a misspelled or unknown command is entered in interactive mode (e.g. <code>gti status</code>, <code>sl</code>, <code>mkdri test</code>, <code>pyhton script.py</code>), Meridian AI automatically analyzes the command and renders an interactive confirmation popup.</p>

<div class="code-block-wrapper">
  <div class="code-header"><span>Interactive AI Typo Popup Dialog</span></div>
  <pre><code class="language-text">┌── 💡 Meridian AI Typo Correction ──────────────────────────────────────────┐
│ Command 'gti' not found in system PATH or builtins.                        │
│ Did you mean: git status ?                                                 │
│                                                                            │
│ Press [Y/Enter] Run correction   [N/Esc] Keep original                     │
└────────────────────────────────────────────────────────────────────────────┘</code></pre>
</div>

<p><strong>Correction Algorithm:</strong></p>
<ol>
  <li><strong>Optimal String Alignment (OSA):</strong> Calculates edit distance across insertions, deletions, substitutions, and adjacent transpositions (e.g. <code>gerp</code> &rarr; <code>grep</code>).</li>
  <li><strong>Dynamic Proportional Threshold:</strong> Rejects false positives by requiring <code>distance &le; max(1, len / 3)</code> against known builtins and cached <code>$PATH</code> binaries.</li>
  <li><strong>Single-Key Interactive Intercept:</strong> Puts terminal in non-canonical raw mode (<code>ICANON | ECHO</code> cleared) to read response immediately without requiring extra prompts.</li>
</ol>

<h2 id="intent-engine-dev">Intent Engine (meridian ask)</h2>
<pre><code class="language-bash">meridian ask "find all log files modified in the last 24 hours"
# -> find . -name "*.log" -mtime -1</code></pre>

<h2 id="diagnostics-dev">Compiler & Runtime Diagnostics (meridian diag)</h2>
<pre><code class="language-bash">meridian diag "g++: error: cannot find -lutil"
# -> Diagnosis: Missing POSIX utility library. Fix: install glibc-devel / libutil.</code></pre>

<h2 id="agent-dev">Autonomous Coding Agent (meridian agent)</h2>
<pre><code class="language-bash">meridian agent "add unit tests for OSC 8 hyperlinks"</code></pre>
`
  );

  addArt('dev-security', 'Security', 'DEVELOPER', 'implemented',
    'Destructive command risk classification and credential leakage redactor.',
    [
      { id: 'risk-tiers', text: 'Risk Classification Tiers (Low to Critical)', level: 2 },
      { id: 'redaction-engine', text: 'Secret & API Key Redactor', level: 2 },
      { id: 'audit-logging', text: 'Security Audit Log & Whitelisting', level: 2 }
    ],
    `
<h2 id="risk-tiers">Risk Classification Tiers (Low to Critical)</h2>
<p><code>ai::RiskClassifier</code> intercepts commands before execution, flagging destructive patterns (e.g. <code>rm -rf /</code>, <code>mkfs</code>, <code>dd if=/dev/zero</code>, fork bombs).</p>

<h2 id="redaction-engine">Secret & API Key Redactor</h2>
<p><code>ai::SecretRedactor</code> automatically strips Bearer tokens, AWS keys (<code>AKIA...</code>), and RSA/SSH private keys from logs and terminal scrollbacks.</p>

<h2 id="audit-logging">Security Audit Log & Whitelisting</h2>
<p>Flagged commands are recorded in <code>~/.local/share/meridian/security_audit.log</code> with user confirmation timestamps.</p>
`
  );
};
