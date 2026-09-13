import React, { useState, useEffect } from 'react';
import { Header } from './components/Header';
import { Sidebar } from './components/Sidebar';
import { Breadcrumbs } from './components/Breadcrumbs';
import { DocContent } from './components/DocContent';
import { TableOfContents } from './components/TableOfContents';
import { SearchModal } from './components/SearchModal';
import { CookieConsent } from './components/CookieConsent';
import { DOCS_ARTICLES } from './data/docsContent';
import { SupportedLanguage, TRANSLATIONS } from './i18n/translations';

export const App: React.FC = () => {
  const [activeId, setActiveId] = useState<string>('graphics-inline-images');
  const [isSearchOpen, setIsSearchOpen] = useState<boolean>(false);
  const [isSidebarOpen, setIsSidebarOpen] = useState<boolean>(false);
  const [theme, setTheme] = useState<'dark' | 'light'>('light');
  const [language, setLanguage] = useState<SupportedLanguage>('en');

  // Handle URL hash changes
  useEffect(() => {
    const handleHash = () => {
      const hash = window.location.hash.replace(/^#/, '');
      if (hash && DOCS_ARTICLES[hash]) {
        setActiveId(hash);
      }
    };

    handleHash();
    window.addEventListener('hashchange', handleHash);
    return () => window.removeEventListener('hashchange', handleHash);
  }, []);

  // Prevent body scroll when mobile sidebar is open
  useEffect(() => {
    if (isSidebarOpen && window.innerWidth <= 768) {
      document.body.style.overflow = 'hidden';
    } else {
      document.body.style.overflow = '';
    }
  }, [isSidebarOpen]);

  // Keyboard shortcut listener for search (press '/' or 'Ctrl+K')
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (
        (e.key === '/' && document.activeElement?.tagName !== 'INPUT' && document.activeElement?.tagName !== 'TEXTAREA') ||
        ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === 'k')
      ) {
        e.preventDefault();
        setIsSearchOpen(true);
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);

  // Handle Theme: Default is LIGHT
  useEffect(() => {
    const savedTheme = (localStorage.getItem('meridian_theme') as 'dark' | 'light') || 'light';
    setTheme(savedTheme);
    document.documentElement.setAttribute('data-theme', savedTheme);
  }, []);

  // Handle Language
  useEffect(() => {
    const savedLang = (localStorage.getItem('meridian_lang') as SupportedLanguage) || 'en';
    if (TRANSLATIONS[savedLang]) {
      setLanguage(savedLang);
    }
  }, []);

  const handleSelectLanguage = (lang: SupportedLanguage) => {
    setLanguage(lang);
    localStorage.setItem('meridian_lang', lang);
  };

  const toggleTheme = () => {
    const nextTheme = theme === 'dark' ? 'light' : 'dark';
    setTheme(nextTheme);
    document.documentElement.setAttribute('data-theme', nextTheme);
    localStorage.setItem('meridian_theme', nextTheme);
  };

  const handleSelectArticle = (id: string) => {
    if (DOCS_ARTICLES[id]) {
      setActiveId(id);
      window.location.hash = id;
      setIsSidebarOpen(false);
      window.scrollTo({ top: 0, behavior: 'smooth' });
    }
  };

  const currentArticle = DOCS_ARTICLES[activeId] || DOCS_ARTICLES['graphics-inline-images'] || DOCS_ARTICLES['intro'];
  const t = TRANSLATIONS[language] || TRANSLATIONS.en;

  return (
    <div className="claude-docs-app">
      <Header
        onOpenSearch={() => setIsSearchOpen(true)}
        onToggleSidebar={() => setIsSidebarOpen(!isSidebarOpen)}
        isSidebarOpen={isSidebarOpen}
        theme={theme}
        onToggleTheme={toggleTheme}
        onNavigate={handleSelectArticle}
        language={language}
        onSelectLanguage={handleSelectLanguage}
        t={t}
      />

      <div className="claude-layout-container">
        {/* Mobile Backdrop */}
        <div
          className={`claude-backdrop ${isSidebarOpen ? 'active' : ''}`}
          onClick={() => setIsSidebarOpen(false)}
          aria-hidden="true"
        />

        <Sidebar
          activeId={activeId}
          onSelect={handleSelectArticle}
          isOpen={isSidebarOpen}
          onClose={() => setIsSidebarOpen(false)}
          language={language}
        />

        <main className="claude-main-area">
          <div className="claude-article-wrapper">
            <Breadcrumbs
              articleId={activeId}
              category={currentArticle.category}
              title={currentArticle.title}
              onNavigateHome={() => handleSelectArticle('intro')}
              language={language}
              t={t}
            />

            <DocContent
              article={currentArticle}
              onNavigate={handleSelectArticle}
              language={language}
              t={t}
            />
          </div>

          <TableOfContents
            articleId={activeId}
            headings={currentArticle.headings}
            t={t}
          />
        </main>
      </div>

      <footer className="claude-footer">
        <div className="footer-inner">
          <div className="footer-brand">
            <div className="footer-logo">
              <svg viewBox="0 0 100 100" width="18" height="18">
                <circle cx="50" cy="50" r="44" fill="none" stroke="#00A8B5" strokeWidth="6" />
                <path d="M 28 68 L 28 34 L 50 56 L 72 34 L 72 68" fill="none" stroke="#00A8B5" strokeWidth="7" strokeLinecap="round" strokeLinejoin="round" />
              </svg>
            </div>
            <span className="footer-title">Meridian Shell</span>
          </div>
          <div className="footer-links">
            <button onClick={() => handleSelectArticle('intro')} className="footer-link-btn">{t.docTitle}</button>
            <a href="https://github.com/charanbalaji2005/Meridian-Shell" target="_blank" rel="noreferrer" className="footer-link">GitHub</a>
            <button onClick={() => handleSelectArticle('proj-changelog')} className="footer-link-btn">{t.releaseNotes}</button>
            <button onClick={() => handleSelectArticle('proj-contributing')} className="footer-link-btn">Contributing</button>
            <button onClick={() => handleSelectArticle('proj-license')} className="footer-link-btn">License</button>
            <button onClick={() => handleSelectArticle('dev-security')} className="footer-link-btn">Security</button>
          </div>
          <div className="footer-copy">
            GNU GPL v3 or later
          </div>
        </div>
      </footer>

      <SearchModal
        isOpen={isSearchOpen}
        onClose={() => setIsSearchOpen(false)}
        onSelectArticle={handleSelectArticle}
        language={language}
        t={t}
      />

      <CookieConsent t={t} />
    </div>
  );
};
