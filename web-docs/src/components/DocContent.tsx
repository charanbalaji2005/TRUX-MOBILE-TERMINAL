import React, { useEffect, useState } from 'react';
import { DocArticle } from '../data/docsContent';
import { NAV_STRUCTURE } from '../data/docsNav';
import {
  Copy,
  Check,
  ArrowLeft,
  ArrowRight,
  List,
  ChevronDown,
  ChevronUp,
  HelpCircle,
  ThumbsUp,
  ThumbsDown,
  CheckCircle2
} from 'lucide-react';
import { SupportedLanguage, TranslationStrings } from '../i18n/translations';
import { getLocalizedArticleTitle } from '../i18n/articleTranslations';

interface DocContentProps {
  article: DocArticle;
  onNavigate: (id: string) => void;
  language: SupportedLanguage;
  t: TranslationStrings;
}

export const DocContent: React.FC<DocContentProps> = ({
  article,
  onNavigate,
  language,
  t,
}) => {
  const [copiedPage, setCopiedPage] = useState(false);
  const [isMobileTocOpen, setIsMobileTocOpen] = useState(false);
  const [mobileFeedbackGiven, setMobileFeedbackGiven] = useState(false);

  // Check saved feedback for this page
  useEffect(() => {
    const saved = localStorage.getItem(`meridian_feedback_${article.id}`);
    setMobileFeedbackGiven(!!saved);
  }, [article.id]);

  const handleMobileVote = (choice: 'up' | 'down') => {
    setMobileFeedbackGiven(true);
    localStorage.setItem(`meridian_feedback_${article.id}`, choice);
  };

  // Flatten navigation items to calculate previous & next article
  const allNavItems: { id: string; title: string }[] = [];
  NAV_STRUCTURE.forEach((cat) => {
    cat.items.forEach((item) => {
      allNavItems.push(item);
    });
  });

  const currentIndex = allNavItems.findIndex((item) => item.id === article.id);
  const prevItem = currentIndex > 0 ? allNavItems[currentIndex - 1] : null;
  const nextItem = currentIndex < allNavItems.length - 1 ? allNavItems[currentIndex + 1] : null;

  // Copy entire page content to clipboard
  const handleCopyPage = () => {
    const rawContent = `# ${localizedTitle}\nCategory: ${article.category}\nLast Updated: ${article.lastUpdated}\n\n${article.summary}\n\n` +
      article.body.replace(/<[^>]*>?/gm, '').replace(/\n{3,}/g, '\n\n');
    navigator.clipboard.writeText(rawContent.trim());
    setCopiedPage(true);
    setTimeout(() => setCopiedPage(false), 2000);
  };

  const scrollToHeading = (id: string) => {
    const el = document.getElementById(id);
    if (el) {
      const topOffset = 75;
      const elementPosition = el.getBoundingClientRect().top;
      const offsetPosition = elementPosition + window.pageYOffset - topOffset;
      window.scrollTo({
        top: offsetPosition,
        behavior: 'smooth',
      });
      setIsMobileTocOpen(false);
    }
  };

  useEffect(() => {
    // 1. Enhance code-block-wrapper headers with copy button
    const wrappers = document.querySelectorAll('.doc-body-content .code-block-wrapper');
    wrappers.forEach((wrapper) => {
      const header = wrapper.querySelector('.code-header');
      const codeEl = wrapper.querySelector('code');
      const preEl = wrapper.querySelector('pre');
      if (header && !header.querySelector('.code-copy-btn')) {
        const copyBtn = document.createElement('button');
        copyBtn.className = 'code-copy-btn';
        copyBtn.setAttribute('aria-label', 'Copy command');
        copyBtn.innerHTML = `
          <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
          <span>${t.copy}</span>
        `;

        copyBtn.addEventListener('click', () => {
          const text = codeEl?.innerText || preEl?.textContent || '';
          navigator.clipboard.writeText(text.trim());
          copyBtn.innerHTML = `
            <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"/></svg>
            <span style="color: #00A8B5; font-weight: 500;">${t.copied}</span>
          `;
          setTimeout(() => {
            copyBtn.innerHTML = `
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
              <span>${t.copy}</span>
            `;
          }, 2000);
        });

        header.appendChild(copyBtn);
      }
    });

    // 2. Enhance all standalone <pre> blocks with Claude-style header bar and copy button
    const preBlocks = document.querySelectorAll('.doc-body-content pre');
    preBlocks.forEach((pre) => {
      if (pre.closest('.code-block-wrapper')) return;
      if (pre.querySelector('.code-header-bar')) return;

      const codeEl = pre.querySelector('code');
      let lang = 'bash';
      if (codeEl) {
        const className = codeEl.className || '';
        const match = className.match(/language-(\w+)/);
        if (match) lang = match[1];
      }

      const headerBar = document.createElement('div');
      headerBar.className = 'code-header-bar';
      headerBar.innerHTML = `
        <span class="code-lang-label">${lang}</span>
        <button class="code-copy-btn" aria-label="Copy code">
          <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
          <span>${t.copy}</span>
        </button>
      `;

      const copyBtn = headerBar.querySelector('.code-copy-btn');
      if (copyBtn) {
        copyBtn.addEventListener('click', () => {
          const text = codeEl?.innerText || pre.textContent || '';
          navigator.clipboard.writeText(text.trim());
          copyBtn.innerHTML = `
            <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"/></svg>
            <span style="color: #00A8B5; font-weight: 500;">${t.copied}</span>
          `;
          setTimeout(() => {
            copyBtn.innerHTML = `
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
              <span>${t.copy}</span>
            `;
          }, 2000);
        });
      }

      pre.insertBefore(headerBar, pre.firstChild);
    });
  }, [article, t]);

  const localizedTitle = getLocalizedArticleTitle(article.id, article.title, language);

  return (
    <article className="claude-article">
      <header className="article-title-header">
        <div className="title-row">
          <h1 className="article-serif-title">{localizedTitle}</h1>
          <button onClick={handleCopyPage} className="copy-llm-btn" aria-label="Copy page content">
            {copiedPage ? <Check size={14} className="copied-icon" /> : <Copy size={14} />}
            <span>{copiedPage ? t.copiedPage : t.copyPage}</span>
          </button>
        </div>
        <p className="article-publish-date">{article.lastUpdated}</p>
      </header>

      {article.summary && (
        <p className="article-lead-intro">{article.summary}</p>
      )}

      {/* Mobile/Tablet Table of Contents Collapsible Box */}
      {article.headings && article.headings.length > 0 && (
        <div className="mobile-toc-accordion">
          <button
            className="mobile-toc-trigger"
            onClick={() => setIsMobileTocOpen(!isMobileTocOpen)}
            aria-expanded={isMobileTocOpen}
          >
            <div className="mobile-toc-label">
              <List size={14} className="mobile-toc-icon" />
              <span>{t.onThisPage}</span>
            </div>
            {isMobileTocOpen ? <ChevronUp size={14} /> : <ChevronDown size={14} />}
          </button>
          {isMobileTocOpen && (
            <ul className="mobile-toc-list">
              {article.headings.map((h) => (
                <li key={h.id} className={`mobile-toc-item level-${h.level}`}>
                  <button
                    onClick={() => scrollToHeading(h.id)}
                    className="mobile-toc-link"
                  >
                    {h.text}
                  </button>
                </li>
              ))}
            </ul>
          )}
        </div>
      )}

      <div
        className="doc-body-content"
        dangerouslySetInnerHTML={{ __html: article.body }}
      />

      {/* Mobile/Tablet Feedback Card */}
      <div className="mobile-feedback-section">
        <div className="was-this-helpful-card">
          {mobileFeedbackGiven ? (
            <div className="feedback-thankyou-state">
              <CheckCircle2 size={16} className="thankyou-icon" />
              <span className="thankyou-text">{t.feedbackThankYou}</span>
            </div>
          ) : (
            <>
              <div className="helpful-title">
                <HelpCircle size={14} className="helpful-icon" />
                <span>{t.wasThisHelpful}</span>
              </div>
              <div className="helpful-actions">
                <button
                  onClick={() => handleMobileVote('up')}
                  className="feedback-btn"
                  title="Yes, helpful"
                  aria-label="Yes, this was helpful"
                >
                  <ThumbsUp size={14} />
                </button>
                <button
                  onClick={() => handleMobileVote('down')}
                  className="feedback-btn"
                  title="No, not helpful"
                  aria-label="No, this was not helpful"
                >
                  <ThumbsDown size={14} />
                </button>
              </div>
            </>
          )}
        </div>
      </div>

      <footer className="article-bottom-pager">
        <div className="pager-container">
          {prevItem ? (
            <button
              onClick={() => onNavigate(prevItem.id)}
              className="claude-pager-card prev-card"
            >
              <ArrowLeft size={16} className="pager-arrow" />
              <div className="pager-meta">
                <span className="pager-label">{t.previous}</span>
                <span className="pager-title">{getLocalizedArticleTitle(prevItem.id, prevItem.title, language)}</span>
              </div>
            </button>
          ) : <div className="pager-spacer" />}

          {nextItem && (
            <button
              onClick={() => onNavigate(nextItem.id)}
              className="claude-pager-card next-card"
            >
              <div className="pager-meta text-right">
                <span className="pager-label">{t.next}</span>
                <span className="pager-title">{getLocalizedArticleTitle(nextItem.id, nextItem.title, language)}</span>
              </div>
              <ArrowRight size={16} className="pager-arrow" />
            </button>
          )}
        </div>
      </footer>
    </article>
  );
};
