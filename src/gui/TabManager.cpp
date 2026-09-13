#include "TabManager.hpp"
#include "../app/meridian_gui.hpp"

namespace meridian::gui {

TerminalTab::TerminalTab(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(0);

    // Unified Terminal View (Single Canvas containing pixel-art, sysinfo, and shell)
    bridge_ = std::make_shared<PtyBridge>(28, 90, this);
    bridge_->startSession();

    terminal_view_ = new TerminalView(bridge_, this);
    terminal_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(terminal_view_, 1);
}

TabManager::TabManager(QWidget* parent)
    : QTabWidget(parent)
{
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);

    setStyleSheet(
        "QTabWidget::pane { border: none; background: transparent; } "
        "QTabBar::tab { background: #18191c; color: #7f848e; padding: 6px 16px; border-radius: 4px; margin-right: 4px; } "
        "QTabBar::tab:selected { background: #23272e; color: #abb2bf; font-weight: bold; border-bottom: 2px solid #3b82f6; } "
        "QTabBar::tab:hover { background: #1e2227; color: #d8dee9; }"
    );

    connect(this, &QTabWidget::tabCloseRequested, this, [this](int idx) {
        if (count() > 1) {
            removeTab(idx);
        }
    });

    createNewTab("main");
}

TerminalTab* TabManager::createNewTab(const QString& title) {
    auto* tab = new TerminalTab(this);
    int idx = addTab(tab, title);
    setCurrentIndex(idx);
    return tab;
}

TerminalTab* TabManager::currentTerminalTab() {
    return qobject_cast<TerminalTab*>(currentWidget());
}

} // namespace meridian::gui
