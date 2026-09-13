#pragma once
// src/gui/TabManager.hpp
//
// Manages active terminal tabs and splits, hosting the unified TerminalView canvas.

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <memory>

#include "TerminalView.hpp"
#include "PtyBridge.hpp"

namespace meridian::gui {

class TerminalTab : public QWidget {
    Q_OBJECT

public:
    explicit TerminalTab(QWidget* parent = nullptr);
    ~TerminalTab() override = default;

    TerminalView* terminalView() { return terminal_view_; }
    std::shared_ptr<PtyBridge> ptyBridge() { return bridge_; }

private:
    TerminalView* terminal_view_ = nullptr;
    std::shared_ptr<PtyBridge> bridge_;
};

class TabManager : public QTabWidget {
    Q_OBJECT

public:
    explicit TabManager(QWidget* parent = nullptr);
    ~TabManager() override = default;

    TerminalTab* createNewTab(const QString& title = "Terminal");
    TerminalTab* currentTerminalTab();
};

} // namespace meridian::gui
