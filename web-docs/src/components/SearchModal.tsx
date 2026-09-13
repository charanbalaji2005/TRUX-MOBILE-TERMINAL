import React, { useState, useEffect, useRef } from 'react';
import { Search, X, ChevronRight, FileText } from 'lucide-react';
import { DOCS_ARTICLES, DocArticle } from '../data/docsContent';
import { SupportedLanguage, TranslationStrings } from '../i18n/translations';
import { getLocalizedArticleTitle, getLocalizedCategoryTitle } from '../i18n/articleTranslations';

interface SearchModalProps {
  isOpen: boolean;
  onClose: () => void;
  onSelectArticle: (id: string) => void;
  language: SupportedLanguage;
  t: TranslationStrings;
}

export const SearchModal: React.FC<SearchModalProps> = ({
  isOpen,
  onClose,
  onSelectArticle,
  language,
  t,
}) => {
  const [query, setQuery] = useState('');
  const [selectedIndex, setSelectedIndex] = useState(0);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (isOpen) {
      setTimeout(() => inputRef.current?.focus(), 50);
      setQuery('');
      setSelectedIndex(0);
    }
  }, [isOpen]);

  if (!isOpen) return null;

  const results: { article: DocArticle; matchText: string }[] = Object.values(DOCS_ARTICLES)
    .filter((art) => {
      if (!query.trim()) return true;
      const q = query.toLowerCase();
      const localizedTitle = getLocalizedArticleTitle(art.id, art.title, language).toLowerCase();
      return (
        localizedTitle.includes(q) ||
        art.title.toLowerCase().includes(q) ||
        art.category.toLowerCase().includes(q) ||
        art.summary.toLowerCase().includes(q) ||
        art.body.toLowerCase().includes(q)
      );
    })
    .map((art) => {
      let matchText = art.summary;
      if (query.trim()) {
        const q = query.toLowerCase();
        const idx = art.body.toLowerCase().indexOf(q);
        if (idx !== -1) {
          const start = Math.max(0, idx - 40);
          const end = Math.min(art.body.length, idx + query.length + 60);
          matchText = '...' + art.body.substring(start, end).replace(/<[^>]*>?/gm, '') + '...';
        }
      }
      return { article: art, matchText };
    })
    .slice(0, 8);

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Escape') {
      onClose();
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      setSelectedIndex((prev) => (prev + 1) % Math.max(1, results.length));
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      setSelectedIndex((prev) => (prev - 1 + results.length) % Math.max(1, results.length));
    } else if (e.key === 'Enter' && results[selectedIndex]) {
      onSelectArticle(results[selectedIndex].article.id);
      onClose();
    }
  };

  return (
    <div className="claude-search-backdrop" onClick={onClose}>
      <div
        className="claude-search-modal"
        onClick={(e) => e.stopPropagation()}
        onKeyDown={handleKeyDown}
      >
        <div className="search-header-row">
          <Search size={18} className="search-input-icon" />
          <input
            ref={inputRef}
            type="text"
            className="search-text-input"
            placeholder={t.searchPlaceholder}
            value={query}
            onChange={(e) => {
              setQuery(e.target.value);
              setSelectedIndex(0);
            }}
          />
          <button className="search-close-btn" onClick={onClose} aria-label="Close search">
            <X size={18} />
          </button>
        </div>

        <div className="search-results-container">
          {results.length === 0 ? (
            <div className="search-empty-state">
              <p>No documentation found matching &ldquo;{query}&rdquo;</p>
            </div>
          ) : (
            <ul className="search-results-list">
              {results.map(({ article, matchText }, i) => (
                <li
                  key={article.id}
                  className={`search-result-row ${i === selectedIndex ? 'selected' : ''}`}
                  onClick={() => {
                    onSelectArticle(article.id);
                    onClose();
                  }}
                  onMouseEnter={() => setSelectedIndex(i)}
                >
                  <FileText size={16} className="row-icon" />
                  <div className="row-details">
                    <div className="row-title-bar">
                      <span className="row-title">{getLocalizedArticleTitle(article.id, article.title, language)}</span>
                      <span className="row-badge">{getLocalizedCategoryTitle(article.category.toUpperCase(), language)}</span>
                    </div>
                    <p className="row-snippet">{matchText}</p>
                  </div>
                  <ChevronRight size={14} className="row-arrow" />
                </li>
              ))}
            </ul>
          )}
        </div>

        <div className="search-footer-hints">
          <span className="hint-pill"><kbd>↑</kbd> <kbd>↓</kbd> {t.navHintNavigate}</span>
          <span className="hint-pill"><kbd>↵</kbd> {t.navHintSelect}</span>
          <span className="hint-pill"><kbd>ESC</kbd> {t.navHintClose}</span>
        </div>
      </div>
    </div>
  );
};
