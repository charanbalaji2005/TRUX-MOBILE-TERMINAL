// docs/assets/docs.js — Client scripts for Meridian Documentation

document.addEventListener('DOMContentLoaded', () => {
  // 1. Table of Contents Generator & Scroll-Spy
  const contentArea = document.getElementById('docBodyContent');
  const tocList = document.getElementById('tocList');

  if (contentArea && tocList) {
    const headings = contentArea.querySelectorAll('h2, h3');
    if (headings.length > 0) {
      headings.forEach((h, index) => {
        if (!h.id) {
          h.id = h.textContent.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/(^-|-$)/g, '') || `heading-${index}`;
        }
        const li = document.createElement('li');
        const a = document.createElement('a');
        a.href = `#${h.id}`;
        a.textContent = h.textContent.replace(/^[#\s]+/, '');
        a.className = `toc-link ${h.tagName.toLowerCase()}`;
        li.appendChild(a);
        tocList.appendChild(li);
      });

      // Scroll spy
      const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
          if (entry.isIntersecting) {
            const id = entry.target.getAttribute('id');
            document.querySelectorAll('.toc-link').forEach(link => {
              link.classList.toggle('active', link.getAttribute('href') === `#${id}`);
            });
          }
        });
      }, { rootMargin: '-10% 0px -80% 0px' });

      headings.forEach(h => observer.observe(h));
    } else {
      tocList.innerHTML = '<li style="font-size: 12px; color: var(--text-subtle);">Overview</li>';
    }
  }

  // 2. Mobile Drawer Toggle
  const mobileBtn = document.getElementById('mobileMenuBtn');
  const sidebar = document.getElementById('meridianSidebar');
  const backdrop = document.getElementById('mobileBackdrop');
  const closeBtn = document.getElementById('sidebarCloseBtn');

  function openSidebar() {
    if (sidebar) sidebar.classList.add('open');
    if (backdrop) backdrop.classList.add('open');
  }

  function closeSidebar() {
    if (sidebar) sidebar.classList.remove('open');
    if (backdrop) backdrop.classList.remove('open');
  }

  if (mobileBtn) mobileBtn.addEventListener('click', openSidebar);
  if (closeBtn) closeBtn.addEventListener('click', closeSidebar);
  if (backdrop) backdrop.addEventListener('click', closeSidebar);

  // 3. Theme Toggle
  const themeBtn = document.getElementById('themeToggleBtn');
  const savedTheme = localStorage.getItem('meridian-docs-theme') || 'dark';
  document.documentElement.setAttribute('data-theme', savedTheme);

  if (themeBtn) {
    themeBtn.addEventListener('click', () => {
      const cur = document.documentElement.getAttribute('data-theme') || 'dark';
      const next = cur === 'dark' ? 'light' : 'dark';
      document.documentElement.setAttribute('data-theme', next);
      localStorage.setItem('meridian-docs-theme', next);
    });
  }

  // 4. Code Block Copy Buttons
  document.querySelectorAll('pre').forEach(pre => {
    const btn = document.createElement('button');
    btn.className = 'code-copy-btn';
    btn.textContent = 'Copy';
    btn.style.position = 'absolute';
    btn.style.top = '8px';
    btn.style.right = '8px';
    btn.style.background = 'var(--bg-panel)';
    btn.style.border = '1px solid var(--border-color)';
    btn.style.borderRadius = '4px';
    btn.style.color = 'var(--text-muted)';
    btn.style.fontSize = '11px';
    btn.style.padding = '2px 8px';
    btn.style.cursor = 'pointer';

    pre.style.position = 'relative';
    pre.appendChild(btn);

    btn.addEventListener('click', () => {
      const code = pre.querySelector('code') ? pre.querySelector('code').innerText : pre.innerText;
      navigator.clipboard.writeText(code.replace(/Copy$/, '').trim());
      btn.textContent = 'Copied!';
      setTimeout(() => { btn.textContent = 'Copy'; }, 2000);
    });
  });

  // 5. Search Modal & Index
  const searchTrigger = document.getElementById('searchTrigger');
  const searchModal = document.getElementById('searchModal');
  const searchClose = document.getElementById('searchCloseBtn');
  const searchInput = document.getElementById('searchInput');
  const searchResults = document.getElementById('searchResults');

  const DOCS_INDEX = [
    { title: 'Introduction', cat: 'Getting Started', url: 'index.html', desc: 'Overview of Meridian Shell terminal platform.' },
    { title: 'Installation & Build', cat: 'Getting Started', url: 'build.html', desc: 'Compiling from source, DNF RPM, APT DEB, Arch packages.' },
    { title: 'Configuration', cat: 'Getting Started', url: 'configuration.html', desc: 'config.json options, anime themes, font settings.' },
    { title: 'Terminal Emulation', cat: 'Terminal', url: 'terminal-emulation.html', desc: 'VT100, XTerm escapes, alternate screens, TrueColor.' },
    { title: 'Shell & AST Engine', cat: 'Terminal', url: 'shell.html', desc: 'Standalone POSIX AST executor, pipelines, job control.' },
    { title: 'PTY Multiplexer', cat: 'Terminal', url: 'pty.html', desc: 'POSIX openpty master/slave descriptor management.' },
    { title: 'Renderer & Direct Images', cat: 'Graphics', url: 'renderer.html', desc: 'Direct 32-bit RGBA inline image decoding and GPU pipeline.' },
    { title: 'GUI & Canvas Architecture', cat: 'Interface', url: 'gui.html', desc: 'Qt6, Wayland, X11 window management and canvas.' },
    { title: 'System Architecture', cat: 'Developer', url: 'architecture.html', desc: 'Complete breakdown of layers, PTY, VT parser, and renderer.' },
    { title: 'Security & Credentials', cat: 'Developer', url: 'security.html', desc: 'Risk classification interceptor and credential redactor.' },
    { title: 'AI Architecture', cat: 'Developer', url: 'ai.html', desc: 'Local AI intent engine, error diagnostics, and coding agent.' },
    { title: 'Fedora RPM & Linux Packages', cat: 'Packaging', url: 'packaging.html', desc: 'DNF repository, RPM spec file, DEB, and PKGBUILD.' },
    { title: 'Component Status Matrix', cat: 'Status', url: 'status.html', desc: 'Current honest implementation state across all subsystems.' }
  ];

  function openSearch() {
    if (searchModal) {
      searchModal.style.display = 'flex';
      if (searchInput) {
        searchInput.focus();
        searchInput.value = '';
        renderResults('');
      }
    }
  }

  function closeSearch() {
    if (searchModal) searchModal.style.display = 'none';
  }

  function renderResults(q) {
    if (!searchResults) return;
    if (!q) {
      searchResults.innerHTML = '<div style="font-size: 13px; color: var(--text-subtle); padding: 12px;">Type to search Meridian Shell documentation...</div>';
      return;
    }
    const filtered = DOCS_INDEX.filter(item => 
      item.title.toLowerCase().includes(q.toLowerCase()) || 
      item.desc.toLowerCase().includes(q.toLowerCase()) ||
      item.cat.toLowerCase().includes(q.toLowerCase())
    );

    if (filtered.length === 0) {
      searchResults.innerHTML = '<div style="font-size: 13px; color: var(--text-subtle); padding: 12px;">No matching documentation pages found.</div>';
      return;
    }

    searchResults.innerHTML = filtered.map(item => `
      <a href="${item.url}" class="search-item">
        <div class="search-item-cat">${item.cat}</div>
        <div class="search-item-title">${item.title}</div>
        <div style="font-size: 12px; color: var(--text-muted);">${item.desc}</div>
      </a>
    `).join('');
  }

  if (searchTrigger) searchTrigger.addEventListener('click', openSearch);
  if (searchClose) searchClose.addEventListener('click', closeSearch);
  if (searchModal) searchModal.addEventListener('click', (e) => { if (e.target === searchModal) closeSearch(); });

  if (searchInput) {
    searchInput.addEventListener('input', (e) => renderResults(e.target.value));
  }

  // Keyboard shortcut '/' or 'Ctrl+K'
  document.addEventListener('keydown', (e) => {
    if ((e.key === '/' || (e.ctrlKey && e.key === 'k')) && document.activeElement !== searchInput) {
      e.preventDefault();
      openSearch();
    }
    if (e.key === 'Escape') closeSearch();
  });
});
