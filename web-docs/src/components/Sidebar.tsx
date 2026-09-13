import React, { useState } from 'react';
import { NAV_STRUCTURE } from '../data/docsNav';
import { SupportedLanguage } from '../i18n/translations';
import { getLocalizedCategoryTitle, getLocalizedArticleTitle } from '../i18n/articleTranslations';
import {
  ChevronDown,
  ChevronUp,
  Info,
  Download,
  Zap,
  Sliders,
  Terminal,
  Cpu,
  List,
  Globe,
  Keyboard,
  Clipboard,
  History,
  Monitor,
  CheckSquare,
  FileImage,
  Film,
  Sparkles,
  Grid,
  Maximize2,
  Layout,
  Layers,
  Settings,
  Package,
  Code,
  FileText,
  X
} from 'lucide-react';

interface SidebarProps {
  activeId: string;
  onSelect: (id: string) => void;
  isOpen: boolean;
  onClose?: () => void;
  language: SupportedLanguage;
}

const itemIconMap: Record<string, React.ReactNode> = {
  'intro': <Info size={15} />,
  'installation': <Download size={15} />,
  'quickstart': <Zap size={15} />,
  'first-run': <CheckSquare size={15} />,
  'getting-started-config': <Sliders size={15} />,

  'terminal-emulation': <Terminal size={15} />,
  'terminal-shell': <Terminal size={15} />,
  'terminal-pty': <Cpu size={15} />,
  'terminal-commands': <List size={15} />,
  'terminal-ssh': <Globe size={15} />,
  'terminal-keybindings': <Keyboard size={15} />,
  'terminal-clipboard': <Clipboard size={15} />,
  'terminal-scrollback': <History size={15} />,

  'graphics-gpu': <Monitor size={15} />,
  'graphics-inline-images': <CheckSquare size={15} />,
  'graphics-formats': <FileImage size={15} />,
  'graphics-gif': <Film size={15} />,
  'graphics-kitty': <Sparkles size={15} />,
  'graphics-sixel': <Grid size={15} />,
  'graphics-rendering': <Maximize2 size={15} />,

  'interface-gui': <Layout size={15} />,
  'interface-windows': <Layout size={15} />,
  'interface-tabs': <Layers size={15} />,
  'interface-panes': <Grid size={15} />,
  'interface-themes': <Sliders size={15} />,
  'interface-powerline': <Terminal size={15} />,

  'dev-architecture': <Layers size={15} />,
  'dev-pty-arch': <Cpu size={15} />,
  'dev-screen-buffer': <Grid size={15} />,
  'dev-renderer': <Monitor size={15} />,
  'dev-graphics-engine': <FileImage size={15} />,
  'dev-ai-engine': <Sparkles size={15} />,
  'dev-security': <CheckSquare size={15} />,

  'config-file': <FileText size={15} />,
  'config-themes': <Sliders size={15} />,
  'config-keybindings': <Keyboard size={15} />,
  'config-profiles': <List size={15} />,
  'config-env': <Sliders size={15} />,

  'pkg-fedora': <Package size={15} />,
  'pkg-debian': <Package size={15} />,
  'pkg-arch': <Package size={15} />,
  'pkg-opensuse': <Package size={15} />,
  'pkg-appimage': <Package size={15} />,
  'pkg-flatpak': <Package size={15} />,
  'pkg-snap': <Package size={15} />,
  'pkg-macos': <Package size={15} />,
  'pkg-windows': <Package size={15} />,

  'development-building': <Code size={15} />,
  'development-testing': <CheckSquare size={15} />,
  'development-debugging': <Settings size={15} />,
  'development-contributing': <FileText size={15} />,
  'development-release': <Package size={15} />,

  'ref-cli': <Terminal size={15} />,
  'ref-config': <Settings size={15} />,
  'ref-shortcuts': <Keyboard size={15} />,
  'ref-env': <Sliders size={15} />,
  'ref-faq': <Info size={15} />,
  'ref-troubleshooting': <Settings size={15} />,

  'proj-github': <Globe size={15} />,
  'proj-changelog': <History size={15} />,
  'proj-license': <FileText size={15} />,
  'proj-contributing': <FileText size={15} />,
};

export const Sidebar: React.FC<SidebarProps> = ({
  activeId,
  onSelect,
  isOpen,
  onClose,
  language,
}) => {
  const [openCategories, setOpenCategories] = useState<Record<string, boolean>>({
    'MERIDIAN SHELL': true,
    'TERMINAL': true,
    'GRAPHICS': true,
    'INTERFACE': true,
    'DEVELOPER': true,
    'CONFIGURATION': true,
    'PACKAGING': true,
    'DEVELOPMENT': true,
    'REFERENCE': true,
    'PROJECT': true,
  });

  const toggleCategory = (title: string) => {
    setOpenCategories((prev) => ({
      ...prev,
      [title]: !prev[title],
    }));
  };

  return (
    <aside className={`meridian-sidebar ${isOpen ? 'open' : ''}`}>
      {onClose && (
        <div className="sidebar-mobile-header">
          <span className="sidebar-mobile-title">Documentation</span>
          <button className="sidebar-close-btn" onClick={onClose} aria-label="Close menu">
            <X size={18} />
          </button>
        </div>
      )}

      <nav className="sidebar-scroll-nav">
        {NAV_STRUCTURE.map((category) => {
          const isCategoryOpen = openCategories[category.title] ?? true;
          const localizedCategory = getLocalizedCategoryTitle(category.title, language);

          return (
            <div key={category.title} className="sidebar-group">
              <button
                className="sidebar-group-header"
                onClick={() => toggleCategory(category.title)}
              >
                <span className="group-title">{localizedCategory}</span>
                <span className="group-chevron">
                  {isCategoryOpen ? <ChevronUp size={14} /> : <ChevronDown size={14} />}
                </span>
              </button>

              {isCategoryOpen && (
                <ul className="sidebar-items-list">
                  {category.items.map((item) => {
                    const isActive = item.id === activeId;
                    const icon = itemIconMap[item.id] || <FileText size={15} />;
                    const localizedTitle = getLocalizedArticleTitle(item.id, item.title, language);

                    return (
                      <li key={item.id}>
                        <button
                          className={`sidebar-link ${isActive ? 'active' : ''}`}
                          onClick={() => {
                            onSelect(item.id);
                            if (onClose) onClose();
                          }}
                        >
                          <span className="link-icon">{icon}</span>
                          <span className="link-text">{localizedTitle}</span>
                        </button>
                      </li>
                    );
                  })}
                </ul>
              )}
            </div>
          );
        })}
      </nav>
    </aside>
  );
};
