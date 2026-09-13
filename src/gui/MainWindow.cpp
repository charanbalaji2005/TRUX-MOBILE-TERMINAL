#include "MainWindow.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>

namespace meridian::gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Meridian Terminal 2.0");
    resize(980, 960);
    setMinimumSize(640, 500);

    // Center window on screen
    if (auto* screen = QApplication::primaryScreen()) {
        QRect scrGeom = screen->geometry();
        move((scrGeom.width() - width()) / 2, (scrGeom.height() - height()) / 2);
    }

    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QMainWindow { background-color: transparent; }");

    tab_manager_ = new TabManager(this);
    setCentralWidget(tab_manager_);
}

void MainWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Translucent dark window surface with rounded outer border
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), corner_radius_, corner_radius_);

    // Dark charcoal translucent fill (matching reference #16171a)
    painter.fillPath(path, QBrush(QColor(22, 23, 26, 246)));

    // Outer subtle border
    painter.setPen(QPen(QColor(255, 255, 255, 25), 1.2));
    painter.drawPath(path);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    Qt::KeyboardModifiers mods = event->modifiers();
    int key = event->key();

    // 1. Fullscreen toggle: F11
    if (key == Qt::Key_F11) {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
        event->accept();
        return;
    }

    // 2. New Tab: Ctrl+Shift+T
    if ((mods == (Qt::ControlModifier | Qt::ShiftModifier)) && key == Qt::Key_T) {
        if (tab_manager_) {
            tab_manager_->createNewTab(QString("tab %1").arg(tab_manager_->count() + 1));
        }
        event->accept();
        return;
    }

    // 3. Close Tab: Ctrl+Shift+W
    if ((mods == (Qt::ControlModifier | Qt::ShiftModifier)) && key == Qt::Key_W) {
        if (tab_manager_ && tab_manager_->count() > 1) {
            tab_manager_->removeTab(tab_manager_->currentIndex());
        }
        event->accept();
        return;
    }

    // 4. Next/Prev Tab: Ctrl+PageDown / Ctrl+PageUp
    if (mods == Qt::ControlModifier && key == Qt::Key_PageDown) {
        if (tab_manager_) {
            int next = (tab_manager_->currentIndex() + 1) % tab_manager_->count();
            tab_manager_->setCurrentIndex(next);
        }
        event->accept();
        return;
    }
    if (mods == Qt::ControlModifier && key == Qt::Key_PageUp) {
        if (tab_manager_) {
            int prev = (tab_manager_->currentIndex() - 1 + tab_manager_->count()) % tab_manager_->count();
            tab_manager_->setCurrentIndex(prev);
        }
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

} // namespace meridian::gui

