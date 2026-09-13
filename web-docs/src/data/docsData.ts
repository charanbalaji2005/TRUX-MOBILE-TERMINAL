export interface NavItem {
  id: string;
  title: string;
  status?: 'implemented' | 'development' | 'experimental';
}

export interface NavCategory {
  title: string;
  icon: string;
  items: NavItem[];
}

export const NAV_STRUCTURE: NavCategory[] = [
  {
    title: 'Getting Started',
    icon: 'Compass',
    items: [
      { id: 'intro', title: 'Introduction', status: 'implemented' },
      { id: 'installation', title: 'Installation Guide', status: 'implemented' },
      { id: 'quickstart', title: 'Quick Start & First Run', status: 'implemented' },
      { id: 'lifecycle', title: 'Updates & Uninstallation', status: 'implemented' },
    ],
  },
  {
    title: 'Terminal & Shell',
    icon: 'Terminal',
    items: [
      { id: 'terminal', title: 'Terminal & Shell Engine', status: 'implemented' },
      { id: 'keybindings', title: 'Keybindings & Shortcuts', status: 'implemented' },
    ],
  },
  {
    title: 'Graphics & Artwork',
    icon: 'Image',
    items: [
      { id: 'raster-images', title: 'Direct Raster Image Pipeline', status: 'implemented' },
      { id: 'anime-gallery', title: 'Anime Gallery & Themes', status: 'implemented' },
    ],
  },
  {
    title: 'AI & Intelligence',
    icon: 'Sparkles',
    items: [
      { id: 'ai-assistant', title: 'Intent & Error Diagnostics', status: 'implemented' },
      { id: 'ai-safety', title: 'Safety Defense & Redaction', status: 'implemented' },
    ],
  },
  {
    title: 'Developer & Core',
    icon: 'Layers',
    items: [
      { id: 'architecture', title: 'PTY & Core Architecture', status: 'implemented' },
      { id: 'dev-tools', title: 'Developer Tooling & Workspaces', status: 'implemented' },
    ],
  },
  {
    title: 'Community & Status',
    icon: 'Users',
    items: [
      { id: 'contributing', title: 'Contributing Guidelines', status: 'implemented' },
      { id: 'status-matrix', title: 'Component Status Matrix', status: 'implemented' },
    ],
  },
];
