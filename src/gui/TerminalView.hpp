#pragma once
// src/gui/TerminalView.hpp
//
// High-performance custom Terminal rendering widget backed by Meridian's
// ScreenBuffer, ANSI state machine, and PTY manager.

#include <QWidget>
#include <QFont>
#include <QFontMetricsF>
#include <QTimer>
#include <QPoint>
#include <QMenu>
#include <memory>

#include "PtyBridge.hpp"
#include "../core/renderer/render_pipeline.hpp"

#include "../core/graphics/terminal_graphic.hpp"

namespace meridian::gui {

struct GridSelection {
    bool active = false;
    int start_row = 0;
    int start_col = 0;
    int end_row = 0;
    int end_col = 0;

    bool contains(int row, int col) const {
        if (!active) return false;
        int r0 = std::min(start_row, end_row);
        int r1 = std::max(start_row, end_row);
        if (row < r0 || row > r1) return false;
        if (r0 == r1) {
            int c0 = std::min(start_col, end_col);
            int c1 = std::max(start_col, end_col);
            return col >= c0 && col <= c1;
        }
        if (row == r0) return col >= (start_row < end_row ? start_col : end_col);
        if (row == r1) return col <= (start_row < end_row ? end_col : start_col);
        return true;
    }
};

class TerminalView : public QWidget {
    Q_OBJECT

public:
    explicit TerminalView(std::shared_ptr<PtyBridge> bridge, QWidget* parent = nullptr);
    ~TerminalView() override = default;

    void setFontSize(qreal pt_size);
    void copySelection();
    void pasteClipboard();
    void selectAll();
    void clearScrollback();
    graphics::GraphicManager& graphicManager() { return graphic_mgr_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    std::shared_ptr<PtyBridge> bridge_;
    QFont font_;
    qreal char_width_ = 8.0;
    qreal char_height_ = 16.0;
    qreal char_ascent_ = 12.0;

    QTimer cursor_timer_;
    QTimer anim_timer_;
    bool cursor_visible_ = true;
    GridSelection selection_;
    bool selecting_ = false;

    graphics::GraphicManager graphic_mgr_;

    QColor resolveColor(const vt::Color& color, bool is_fg);
    QPoint pixelToCell(const QPoint& pt) const;
    QString selectedText() const;
};

} // namespace meridian::gui

