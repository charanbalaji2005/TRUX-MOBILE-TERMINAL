#pragma once
// src/gui/MainWindow.hpp
//
// Main Window for Meridian Terminal graphical application.
// Renders the translucent dark rounded shell, TabManager, and global shortcuts.

#include <QMainWindow>
#include <memory>

#include "TabManager.hpp"

namespace meridian::gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    TabManager* tab_manager_ = nullptr;
    int corner_radius_ = 14;
};

} // namespace meridian::gui

