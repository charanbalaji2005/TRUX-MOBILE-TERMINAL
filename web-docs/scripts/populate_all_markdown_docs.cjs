const fs = require('fs');
const path = require('path');

const docsRoot = path.resolve(__dirname, '../../docs');

// Read articles generated in docsContent.ts
const docsContentTs = fs.readFileSync(path.resolve(__dirname, '../src/data/docsContent.ts'), 'utf8');
const match = docsContentTs.match(/export const DOCS_ARTICLES: Record<string, DocArticle> = ([\s\S]+);/);
if (!match) {
  console.error("Could not parse DOCS_ARTICLES from docsContent.ts");
  process.exit(1);
}
const articles = JSON.parse(match[1]);

// Mapping of article id to folder path
const fileMap = {
  'intro': 'getting-started/introduction.md',
  'installation': 'getting-started/installation.md',
  'quickstart': 'getting-started/quickstart.md',
  'first-run': 'getting-started/first-run.md',
  'getting-started-config': 'getting-started/configuration.md',

  'terminal-emulation': 'terminal/terminal-emulation.md',
  'terminal-shell': 'terminal/shell.md',
  'terminal-pty': 'terminal/pty.md',
  'terminal-commands': 'terminal/commands.md',
  'terminal-ssh': 'terminal/ssh.md',
  'terminal-keybindings': 'terminal/keybindings.md',
  'terminal-clipboard': 'terminal/clipboard.md',
  'terminal-scrollback': 'terminal/scrollback.md',

  'graphics-gpu': 'graphics/gpu-renderer.md',
  'graphics-inline-images': 'graphics/inline-images.md',
  'graphics-formats': 'graphics/image-formats.md',
  'graphics-gif': 'graphics/gif.md',
  'graphics-kitty': 'graphics/kitty-graphics.md',
  'graphics-sixel': 'graphics/sixel.md',
  'graphics-rendering': 'graphics/image-rendering.md',

  'interface-gui': 'interface/gui.md',
  'interface-windows': 'interface/windows.md',
  'interface-tabs': 'interface/tabs.md',
  'interface-panes': 'interface/panes.md',
  'interface-themes': 'interface/themes.md',
  'interface-powerline': 'interface/powerline.md',

  'dev-architecture': 'developer/architecture.md',
  'dev-pty-arch': 'developer/pty-architecture.md',
  'dev-screen-buffer': 'developer/screen-buffer.md',
  'dev-renderer': 'developer/renderer.md',
  'dev-graphics-engine': 'developer/graphics-engine.md',
  'dev-ai-engine': 'developer/ai-engine.md',
  'dev-security': 'developer/security.md',

  'config-file': 'configuration/config-file.md',
  'config-themes': 'configuration/themes.md',
  'config-keybindings': 'configuration/keybindings.md',
  'config-profiles': 'configuration/profiles.md',
  'config-env': 'configuration/environment.md',

  'pkg-fedora': 'packaging/fedora.md',
  'pkg-debian': 'packaging/debian.md',
  'pkg-arch': 'packaging/arch.md',
  'pkg-opensuse': 'packaging/opensuse.md',
  'pkg-appimage': 'packaging/appimage.md',
  'pkg-flatpak': 'packaging/flatpak.md',
  'pkg-snap': 'packaging/snap.md',
  'pkg-macos': 'packaging/macos.md',
  'pkg-windows': 'packaging/windows.md',

  'development-building': 'development/building.md',
  'development-testing': 'development/testing.md',
  'development-debugging': 'development/debugging.md',
  'development-contributing': 'development/contributing.md',
  'development-release': 'development/release-process.md',

  'ref-cli': 'reference/cli.md',
  'ref-config': 'reference/config.md',
  'ref-shortcuts': 'reference/shortcuts.md',
  'ref-env': 'reference/environment.md',
  'ref-faq': 'reference/faq.md',
  'ref-troubleshooting': 'reference/troubleshooting.md',

  'proj-github': 'project/github.md',
  'proj-changelog': 'project/changelog.md',
  'proj-license': 'project/license.md',
  'proj-contributing': 'project/contributing.md'
};

mkdirp(path.join(docsRoot, 'project'));

for (const [id, relPath] of Object.entries(fileMap)) {
  const art = articles[id];
  if (!art) continue;

  const targetPath = path.join(docsRoot, relPath);
  mkdirp(path.dirname(targetPath));

  const frontmatter = `---
layout: default
title: "${art.title}"
category: "${art.category}"
status: "${art.status || 'implemented'}"
---

`;

  fs.writeFileSync(targetPath, frontmatter + art.body);
  console.log("Wrote:", relPath);
}

function mkdirp(dir) {
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }
}

console.log("Successfully generated all category markdown files!");
