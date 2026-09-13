import React, { useEffect, useState } from 'react';
import { HelpCircle, ThumbsUp, ThumbsDown, CheckCircle2 } from 'lucide-react';
import { TranslationStrings } from '../i18n/translations';

interface TableOfContentsProps {
  articleId: string;
  headings: { id: string; text: string; level: number }[];
  t: TranslationStrings;
}

export const TableOfContents: React.FC<TableOfContentsProps> = ({
  articleId,
  headings,
  t,
}) => {
  const [activeHeadingId, setActiveHeadingId] = useState<string>('');
  const [feedbackGiven, setFeedbackGiven] = useState<boolean>(false);
  const [vote, setVote] = useState<'up' | 'down' | null>(null);

  // Check if feedback was already provided for this specific page
  useEffect(() => {
    const saved = localStorage.getItem(`meridian_feedback_${articleId}`);
    if (saved) {
      setFeedbackGiven(true);
      setVote(saved as 'up' | 'down');
    } else {
      setFeedbackGiven(false);
      setVote(null);
    }
  }, [articleId]);

  useEffect(() => {
    if (headings.length > 0) {
      setActiveHeadingId(headings[0].id);
    }

    const observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            setActiveHeadingId(entry.target.id);
          }
        });
      },
      { rootMargin: '-70px 0% -60% 0%' }
    );

    headings.forEach((h) => {
      const el = document.getElementById(h.id);
      if (el) observer.observe(el);
    });

    return () => observer.disconnect();
  }, [headings]);

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
      setActiveHeadingId(id);
    }
  };

  const handleVote = (choice: 'up' | 'down') => {
    setVote(choice);
    setFeedbackGiven(true);
    localStorage.setItem(`meridian_feedback_${articleId}`, choice);
  };

  return (
    <aside className="claude-toc">
      <div className="toc-sticky-container">
        <h4 className="toc-section-heading">{t.onThisPage}</h4>
        {headings.length === 0 ? (
          <p className="toc-empty">{t.overview}</p>
        ) : (
          <ul className="toc-items-list">
            {headings.map((h) => {
              const isActive = activeHeadingId === h.id;
              return (
                <li
                  key={h.id}
                  className={`toc-item level-${h.level} ${isActive ? 'active' : ''}`}
                >
                  <button
                    onClick={() => scrollToHeading(h.id)}
                    className="toc-link-btn"
                  >
                    {h.text}
                  </button>
                </li>
              );
            })}
          </ul>
        )}

        {/* Was this helpful card - One-time per page */}
        <div className="was-this-helpful-card">
          {feedbackGiven ? (
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
                  onClick={() => handleVote('up')}
                  className="feedback-btn"
                  title="Yes, helpful"
                  aria-label="Yes, this was helpful"
                >
                  <ThumbsUp size={14} />
                </button>
                <button
                  onClick={() => handleVote('down')}
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
    </aside>
  );
};
