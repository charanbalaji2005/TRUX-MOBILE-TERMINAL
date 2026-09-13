import React, { useState, useEffect } from 'react';
import { ShieldCheck, X } from 'lucide-react';
import { TranslationStrings } from '../i18n/translations';

interface CookieConsentProps {
  t: TranslationStrings;
}

export const CookieConsent: React.FC<CookieConsentProps> = ({ t }) => {
  const [isVisible, setIsVisible] = useState(false);

  useEffect(() => {
    const consent = localStorage.getItem('meridian_cookie_consent');
    if (!consent) {
      // Delay display slightly for smooth page load
      const timer = setTimeout(() => setIsVisible(true), 1000);
      return () => clearTimeout(timer);
    }
  }, []);

  const handleAccept = () => {
    localStorage.setItem('meridian_cookie_consent', 'accepted');
    setIsVisible(false);
  };

  const handleDecline = () => {
    localStorage.setItem('meridian_cookie_consent', 'declined');
    setIsVisible(false);
  };

  if (!isVisible) return null;

  return (
    <aside className="cookie-consent-toast" role="dialog" aria-label="Cookie consent">
      <div className="cookie-content">
        <div className="cookie-header">
          <ShieldCheck size={18} className="cookie-icon" />
          <h4 className="cookie-title">{t.cookieTitle}</h4>
          <button
            onClick={handleDecline}
            className="cookie-dismiss-btn"
            aria-label="Dismiss cookie notice"
          >
            <X size={16} />
          </button>
        </div>
        <p className="cookie-text">{t.cookieDesc}</p>
        <div className="cookie-actions">
          <button onClick={handleAccept} className="cookie-btn accept-btn">
            {t.acceptAll}
          </button>
          <button onClick={handleDecline} className="cookie-btn decline-btn">
            {t.decline}
          </button>
        </div>
      </div>
    </aside>
  );
};
