export interface NavItem {
  id: string;
  title: string;
  status?: 'implemented' | 'development' | 'experimental' | 'planned';
}

export interface NavCategory {
  title: string;
  icon: string;
  items: NavItem[];
}

export const NAV_STRUCTURE: NavCategory[] = [
  {
    title: 'MERIDIAN SHELL',
    icon: 'Compass',
    items: [
      { id: 'intro', title: 'Introduction', status: 'implemented' },
      { id: 'installation', title: 'Installation', status: 'implemented' },
      { id: 'quickstart', title: 'Quick Start', status: 'implemented' },
      { id: 'first-run', title: 'First Run', status: 'implemented' },
      { id: 'getting-started-config', title: 'Configuration', status: 'implemented' },
    ],
  },
  {
    title: 'TERMINAL',
    icon: 'Terminal',
    items: [
      { id: 'terminal-emulation', title: 'Terminal Emulation', status: 'implemented' },
      { id: 'terminal-shell', title: 'Shell', status: 'implemented' },
      { id: 'terminal-pty', title: 'PTY', status: 'implemented' },
      { id: 'terminal-commands', title: 'Commands', status: 'implemented' },
      { id: 'terminal-ssh', title: 'SSH', status: 'implemented' },
      { id: 'terminal-keybindings', title: 'Keybindings', status: 'implemented' },
      { id: 'terminal-clipboard', title: 'Clipboard', status: 'implemented' },
      { id: 'terminal-scrollback', title: 'Scrollback', status: 'implemented' },
    ],
  },
  {
    title: 'GRAPHICS',
    icon: 'Image',
    items: [
      { id: 'graphics-gpu', title: 'GPU Renderer', status: 'development' },
      { id: 'graphics-inline-images', title: 'Inline Images', status: 'implemented' },
      { id: 'graphics-formats', title: 'Image Formats', status: 'implemented' },
      { id: 'graphics-gif', title: 'GIF', status: 'implemented' },
      { id: 'graphics-kitty', title: 'Kitty Graphics', status: 'implemented' },
      { id: 'graphics-sixel', title: 'Sixel', status: 'implemented' },
      { id: 'graphics-rendering', title: 'Image Rendering', status: 'implemented' },
    ],
  },
  {
    title: 'INTERFACE',
    icon: 'Layout',
    items: [
      { id: 'interface-gui', title: 'GUI', status: 'development' },
      { id: 'interface-windows', title: 'Windows', status: 'implemented' },
      { id: 'interface-tabs', title: 'Tabs', status: 'implemented' },
      { id: 'interface-panes', title: 'Panes', status: 'implemented' },
      { id: 'interface-themes', title: 'Themes', status: 'implemented' },
      { id: 'interface-powerline', title: 'Powerline', status: 'implemented' },
    ],
  },
  {
    title: 'DEVELOPER',
    icon: 'Layers',
    items: [
      { id: 'dev-architecture', title: 'Architecture', status: 'implemented' },
      { id: 'dev-pty-arch', title: 'PTY Architecture', status: 'implemented' },
      { id: 'dev-screen-buffer', title: 'Screen Buffer', status: 'implemented' },
      { id: 'dev-renderer', title: 'Renderer', status: 'implemented' },
      { id: 'dev-graphics-engine', title: 'Graphics Engine', status: 'implemented' },
      { id: 'dev-ai-engine', title: 'AI Engine', status: 'experimental' },
      { id: 'dev-security', title: 'Security', status: 'implemented' },
    ],
  },
  {
    title: 'CONFIGURATION',
    icon: 'Settings',
    items: [
      { id: 'config-file', title: 'Configuration File', status: 'implemented' },
      { id: 'config-themes', title: 'Themes', status: 'implemented' },
      { id: 'config-keybindings', title: 'Keybindings', status: 'implemented' },
      { id: 'config-profiles', title: 'Profiles', status: 'implemented' },
      { id: 'config-env', title: 'Environment Variables', status: 'implemented' },
    ],
  },
  {
    title: 'PACKAGING',
    icon: 'Package',
    items: [
      { id: 'pkg-fedora', title: 'Fedora / RPM', status: 'implemented' },
      { id: 'pkg-debian', title: 'Ubuntu / Debian / DEB', status: 'implemented' },
      { id: 'pkg-arch', title: 'Arch / AUR', status: 'implemented' },
      { id: 'pkg-opensuse', title: 'openSUSE', status: 'implemented' },
      { id: 'pkg-appimage', title: 'AppImage', status: 'implemented' },
      { id: 'pkg-flatpak', title: 'Flatpak', status: 'planned' },
      { id: 'pkg-snap', title: 'Snap', status: 'planned' },
      { id: 'pkg-macos', title: 'macOS', status: 'development' },
      { id: 'pkg-windows', title: 'Windows', status: 'development' },
    ],
  },
  {
    title: 'DEVELOPMENT',
    icon: 'Code',
    items: [
      { id: 'development-building', title: 'Building', status: 'implemented' },
      { id: 'development-testing', title: 'Testing', status: 'implemented' },
      { id: 'development-debugging', title: 'Debugging', status: 'implemented' },
      { id: 'development-contributing', title: 'Contributing', status: 'implemented' },
      { id: 'development-release', title: 'Release Process', status: 'implemented' },
    ],
  },
  {
    title: 'REFERENCE',
    icon: 'BookOpen',
    items: [
      { id: 'ref-cli', title: 'CLI Reference', status: 'implemented' },
      { id: 'ref-config', title: 'Configuration Reference', status: 'implemented' },
      { id: 'ref-shortcuts', title: 'Keyboard Shortcuts', status: 'implemented' },
      { id: 'ref-env', title: 'Environment Variables', status: 'implemented' },
      { id: 'ref-faq', title: 'FAQ', status: 'implemented' },
      { id: 'ref-troubleshooting', title: 'Troubleshooting', status: 'implemented' },
    ],
  },
  {
    title: 'PROJECT',
    icon: 'Info',
    items: [
      { id: 'proj-github', title: 'GitHub', status: 'implemented' },
      { id: 'proj-changelog', title: 'Changelog', status: 'implemented' },
      { id: 'proj-license', title: 'License', status: 'implemented' },
      { id: 'proj-contributing', title: 'Contributing', status: 'implemented' },
    ],
  },
];
