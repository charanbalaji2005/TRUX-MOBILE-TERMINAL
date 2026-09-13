module.exports = function(addArt) {
  addArt('proj-github', 'GitHub', 'PROJECT', 'implemented',
    'Source code repository, issue tracker, and community discussions.',
    [
      { id: 'github-links-spec', text: 'Official GitHub Repository', level: 2 },
      { id: 'issue-tracker', text: 'Issue Tracker & Bug Reports', level: 2 },
      { id: 'discussions', text: 'GitHub Discussions & Community', level: 2 }
    ],
    `
<h2 id="github-links-spec">Official GitHub Repository</h2>
<p>The official source code repository is hosted on GitHub:</p>
<p><a href="https://github.com/charanbalaji2005/Meridian-Shell" target="_blank">https://github.com/charanbalaji2005/Meridian-Shell</a></p>

<h2 id="issue-tracker">Issue Tracker & Bug Reports</h2>
<p>Encountered a bug or rendering issue? Submit an issue on our GitHub issue tracker with your OS version, GPU hardware, and terminal logs.</p>

<h2 id="discussions">GitHub Discussions & Community</h2>
<p>Join the community on GitHub Discussions to share anime themes, custom plugin recipes, and feature suggestions.</p>
`
  );

  addArt('proj-changelog', 'Changelog', 'PROJECT', 'implemented',
    'Release history and major milestone changes.',
    [
      { id: 'changelog-2-0', text: 'Version 2.0.0 Release Notes (August 2026)', level: 2 },
      { id: 'changelog-1-5', text: 'Version 1.5.0 Release Notes (May 2026)', level: 2 }
    ],
    `
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
`
  );

  addArt('proj-license', 'License', 'PROJECT', 'implemented',
    'Free and open-source GNU General Public License v3.0.',
    [
      { id: 'license-spec', text: 'GNU General Public License v3.0', level: 2 },
      { id: 'permissions-conditions', text: 'Summary of Permissions & Obligations', level: 2 }
    ],
    `
<h2 id="license-spec">GNU General Public License v3.0</h2>
<p>Meridian Shell is free and open-source software licensed under the <strong>GNU General Public License v3.0 or later (GPL-3.0-or-later)</strong>.</p>

<h2 id="permissions-conditions">Summary of Permissions & Obligations</h2>
<table class="doc-table">
  <thead><tr><th>Permissions</th><th>Conditions</th></tr></thead>
  <tbody>
    <tr><td>Commercial use</td><td>Disclose source code</td></tr>
    <tr><td>Modification</td><td>License and copyright notice</td></tr>
    <tr><td>Distribution</td><td>Same license (copyleft)</td></tr>
    <tr><td>Private use</td><td>State changes</td></tr>
  </tbody>
</table>
`
  );

  addArt('proj-contributing', 'Contributing', 'PROJECT', 'implemented',
    'Open source contribution guide, bug fixes, and anime theme submissions.',
    [
      { id: 'contributor-welcome', text: 'Welcome Open Source Contributors!', level: 2 },
      { id: 'how-to-contribute', text: 'Ways to Contribute', level: 2 }
    ],
    `
<div class="note-box note-success">
  <div class="note-title">Open Source Community Welcome</div>
  <p>Contributions of all kinds are warmly welcomed! Bug fixes, rendering improvements, new anime themes, and shell features are highly encouraged.</p>
</div>

<h2 id="how-to-contribute">Ways to Contribute</h2>
<ul>
  <li><strong>Code Contributions:</strong> Submit PRs for bug fixes, performance optimizations, or protocol enhancements.</li>
  <li><strong>Artwork & Themes:</strong> Contribute high-resolution anime wallpapers formatted for terminal headers.</li>
  <li><strong>Documentation:</strong> Improve tutorials, API references, or translation guides.</li>
  <li><strong>Packaging:</strong> Help maintain native packages for your favorite Linux distribution.</li>
</ul>
`
  );
};
