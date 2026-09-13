import React, { useState, useRef, useEffect } from 'react';
import { Search, Moon, Sun, Menu, X, Globe, ChevronDown, Check } from 'lucide-react';
import { SupportedLanguage, LANGUAGE_OPTIONS, TranslationStrings } from '../i18n/translations';

interface HeaderProps {
  onOpenSearch: () => void;
  onToggleSidebar: () => void;
  isSidebarOpen: boolean;
  theme: 'dark' | 'light';
  onToggleTheme: () => void;
  onNavigate: (id: string) => void;
  language: SupportedLanguage;
  onSelectLanguage: (lang: SupportedLanguage) => void;
  t: TranslationStrings;
}

export const Header: React.FC<HeaderProps> = ({
  onOpenSearch,
  onToggleSidebar,
  isSidebarOpen,
  theme,
  onToggleTheme,
  onNavigate,
  language,
  onSelectLanguage,
  t,
}) => {
  const [isLangOpen, setIsLangOpen] = useState(false);
  const langMenuRef = useRef<HTMLDivElement>(null);

  // Close language menu on click outside
  useEffect(() => {
    const handleClickOutside = (e: MouseEvent) => {
      if (langMenuRef.current && !langMenuRef.current.contains(e.target as Node)) {
        setIsLangOpen(false);
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const currentLang = LANGUAGE_OPTIONS.find((l) => l.code === language) || LANGUAGE_OPTIONS[0];

  return (
    <header className="meridian-header">
      <div className="header-left">
        <button
          className="mobile-menu-btn"
          onClick={onToggleSidebar}
          aria-label="Toggle navigation"
        >
          {isSidebarOpen ? <X size={18} /> : <Menu size={18} />}
        </button>

        <a href="#intro" onClick={() => onNavigate('intro')} className="brand-logo" title="Meridian Shell Documentation">
          {/* Meridian Shell Logo matching screenshot */}
          <div className="logo-badge">
            <svg viewBox="0 0 100 100" width="28" height="28" className="logo-svg">
              <circle cx="50" cy="50" r="44" fill="none" stroke="#00A8B5" strokeWidth="6" />
              <path
                d="M 28 68 L 28 34 L 50 56 L 72 34 L 72 68"
                fill="none"
                stroke="#00A8B5"
                strokeWidth="7"
                strokeLinecap="round"
                strokeLinejoin="round"
              />
              <path
                d="M 20 54 Q 50 20 80 54"
                fill="none"
                stroke="#00A8B5"
                strokeWidth="3.5"
                strokeDasharray="4 4"
                opacity="0.85"
              />
            </svg>
          </div>
          <div className="brand-text-block">
            <span className="brand-name">MERIDIAN SHELL</span>
            <span className="brand-tag">{t.brandDocs}</span>
          </div>
        </a>
      </div>

      <div className="header-right">
        <nav className="header-nav-links">
          <button onClick={() => onNavigate('ref-cli')} className="header-nav-link">
            {t.apiDocs}
          </button>
          <button onClick={() => onNavigate('proj-changelog')} className="header-nav-link">
            {t.releaseNotes}
          </button>
          <button onClick={() => onNavigate('quickstart')} className="header-nav-link">
            {t.gettingStarted}
          </button>
        </nav>

        {/* International Language Selector with Telugu */}
        <div className="header-lang-wrapper" ref={langMenuRef}>
          <button
            className="header-lang-btn"
            onClick={() => setIsLangOpen(!isLangOpen)}
            aria-label="Select Language"
            aria-expanded={isLangOpen}
          >
            <Globe size={14} />
            <span>{currentLang.nativeName}</span>
            <ChevronDown size={12} className={`lang-chevron ${isLangOpen ? 'open' : ''}`} />
          </button>

          {isLangOpen && (
            <div className="lang-dropdown-menu">
              <div className="lang-menu-header">Select Language</div>
              <ul className="lang-list">
                {LANGUAGE_OPTIONS.map((opt) => (
                  <li key={opt.code}>
                    <button
                      className={`lang-option-btn ${opt.code === language ? 'active' : ''}`}
                      onClick={() => {
                        onSelectLanguage(opt.code);
                        setIsLangOpen(false);
                      }}
                    >
                      <span className="lang-flag">{opt.flag}</span>
                      <span className="lang-label">{opt.nativeName}</span>
                      <span className="lang-sub">{opt.name}</span>
                      {opt.code === language && <Check size={14} className="lang-check" />}
                    </button>
                  </li>
                ))}
              </ul>
            </div>
          )}
        </div>

        <button className="header-search-btn" onClick={onOpenSearch} aria-label={t.search}>
          <Search size={14} className="search-icon" />
          <span className="search-placeholder">{t.search}</span>
          <kbd className="search-kbd">Ctrl K</kbd>
        </button>

        <button
          onClick={onToggleTheme}
          className="header-theme-btn"
          title={`Switch to ${theme === 'dark' ? 'light' : 'dark'} mode`}
          aria-label="Toggle theme"
        >
          {theme === 'dark' ? <Moon size={16} /> : <Sun size={16} />}
        </button>
      </div>
    </header>
  );
};
