import React from 'react';
import { ChevronRight } from 'lucide-react';
import { SupportedLanguage, TranslationStrings } from '../i18n/translations';
import { getLocalizedCategoryTitle, getLocalizedArticleTitle } from '../i18n/articleTranslations';

interface BreadcrumbsProps {
  articleId: string;
  category: string;
  title: string;
  onNavigateHome: () => void;
  language: SupportedLanguage;
  t: TranslationStrings;
}

export const Breadcrumbs: React.FC<BreadcrumbsProps> = ({
  articleId,
  category,
  title,
  onNavigateHome,
  language,
  t,
}) => {
  const localizedCategory = getLocalizedCategoryTitle(category.toUpperCase(), language);
  const localizedTitle = getLocalizedArticleTitle(articleId, title, language);

  return (
    <nav className="claude-breadcrumbs" aria-label="Breadcrumbs">
      <button onClick={onNavigateHome} className="breadcrumb-link">
        {t.allCollections}
      </button>
      <ChevronRight size={13} className="breadcrumb-separator" />
      <button onClick={onNavigateHome} className="breadcrumb-link">
        {t.meridianShell}
      </button>
      <ChevronRight size={13} className="breadcrumb-separator" />
      <span className="breadcrumb-link">{localizedCategory}</span>
      <ChevronRight size={13} className="breadcrumb-separator" />
      <span className="breadcrumb-current">{localizedTitle}</span>
    </nav>
  );
};
