#include "TerminalView.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QFontMetricsF>
#include <algorithm>

namespace meridian::gui {

namespace {

const QColor DEFAULT_BG(24, 25, 28, 235); // Dark Translucent
const QColor DEFAULT_FG(216, 222, 233);   // Soft White / Gray
const QColor SELECTION_BG(59, 130, 246, 110); // Translucent Blue Accent

const QColor ANSI_COLORS[16] = {
    QColor("#15171a"), // 0: Black
    QColor("#ef4444"), // 1: Red
    QColor("#22c55e"), // 2: Green
    QColor("#fde047"), // 3: Yellow
    QColor("#3b82f6"), // 4: Blue
    QColor("#a855f7"), // 5: Magenta
    QColor("#00e5ff"), // 6: Cyan
    QColor("#f5f7fa"), // 7: White
    QColor("#5c6370"), // 8: Bright Black
    QColor("#f43f5e"), // 9: Bright Red / Hot Pink
    QColor("#4ade80"), // 10: Bright Green
    QColor("#fef08a"), // 11: Bright Yellow
    QColor("#60a5fa"), // 12: Bright Blue
    QColor("#c084fc"), // 13: Bright Magenta
    QColor("#22d3ee"), // 14: Bright Cyan
    QColor("#ffffff")  // 15: Bright White
};

} // namespace

TerminalView::TerminalView(std::shared_ptr<PtyBridge> bridge, QWidget* parent)
    : QWidget(parent)
    , bridge_(std::move(bridge))
    , font_("JetBrains Mono", 11)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setMouseTracking(true);

    font_.setStyleHint(QFont::Monospace);
    font_.setFixedPitch(true);
    setFontSize(11.0);

    // Add native full-color startup artwork graphic
    graphic_mgr_.add_graphic(graphics::GraphicManager::create_startup_artwork_graphic(28.0f, 32.0f, 220.0f, 170.0f));

    connect(bridge_.get(), &PtyBridge::screenUpdated, this, [this]() {
        update();
    });

    connect(&cursor_timer_, &QTimer::timeout, this, [this]() {
        cursor_visible_ = !cursor_visible_;
        update();
    });
    cursor_timer_.start(500);

    connect(&anim_timer_, &QTimer::timeout, this, [this]() {
        graphic_mgr_.tick(0.033);
        update();
    });
    anim_timer_.start(33);
}

void TerminalView::setFontSize(qreal pt_size) {
    font_.setPointSizeF(std::clamp(pt_size, 6.0, 36.0));
    QFontMetricsF fm(font_);
    char_width_ = fm.horizontalAdvance(QLatin1Char('M'));
    if (char_width_ <= 0) char_width_ = 8.0;
    char_height_ = fm.height();
    if (char_height_ <= 0) char_height_ = 16.0;
    char_ascent_ = fm.ascent();

    if (width() > 0 && height() > 0) {
        int cols = std::max(10, static_cast<int>(width() / char_width_));
        int rows = std::max(4, static_cast<int>(height() / char_height_));
        bridge_->resize(rows, cols);
    }
    update();
}

QColor TerminalView::resolveColor(const vt::Color& color, bool is_fg) {
    switch (color.kind) {
        case vt::Color::Kind::Default:
            return is_fg ? DEFAULT_FG : DEFAULT_BG;
        case vt::Color::Kind::Indexed: {
            if (color.index < 16) {
                return ANSI_COLORS[color.index];
            }
            // 6x6x6 color cube
            if (color.index >= 16 && color.index <= 231) {
                int idx = color.index - 16;
                int r = (idx / 36) ? (idx / 36 * 40 + 55) : 0;
                int g = ((idx % 36) / 6) ? ((idx % 36) / 6 * 40 + 55) : 0;
                int b = (idx % 6) ? (idx % 6 * 40 + 55) : 0;
                return QColor(r, g, b);
            }
            // Grayscale ramp
            if (color.index >= 232 && color.index <= 255) {
                int gray = 8 + (color.index - 232) * 10;
                return QColor(gray, gray, gray);
            }
            return is_fg ? DEFAULT_FG : DEFAULT_BG;
        }
        case vt::Color::Kind::Rgb:
            return QColor(color.r, color.g, color.b);
    }
    return is_fg ? DEFAULT_FG : DEFAULT_BG;
}

QPoint TerminalView::pixelToCell(const QPoint& pt) const {
    int col = static_cast<int>(pt.x() / char_width_);
    int row = static_cast<int>(pt.y() / char_height_);
    return QPoint(col, row);
}

void TerminalView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    int cols = std::max(10, static_cast<int>(width() / char_width_));
    int rows = std::max(4, static_cast<int>(height() / char_height_));
    bridge_->resize(rows, cols);
}

void TerminalView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true); // Real full-color smooth image rendering
    painter.setFont(font_);

    std::lock_guard<std::mutex> lock(bridge_->screenMutex());
    const auto& screen = bridge_->screenBuffer();

    int rows = screen.rows();
    int cols = screen.cols();

    auto draw_images = [&](bool under_text) {
        // 1. Draw GraphicManager native raster graphics (Real Full-Color RGBA Images & GIFs)
        for (const auto& g : graphic_mgr_.graphics()) {
            if (!g.visible || (g.z_index < 0) != under_text) continue;
            const auto& frame = g.active_frame();
            if (!frame.is_valid()) continue;

            QImage qimg(frame.rgba.data(), frame.width, frame.height, frame.width * 4, QImage::Format_RGBA8888);
            auto fit = graphics::ImageDecoder::calculate_fit(frame.width, frame.height, g.width, g.height, g.fit_mode, g.x, g.y);

            painter.setOpacity(g.opacity);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, g.filter_mode == graphics::ImageScaleFilter::Smooth);
            painter.drawImage(QRectF(fit.x, fit.y, fit.width, fit.height), qimg);
            painter.setOpacity(1.0f);
        }

        // 2. Draw Kitty Graphics Protocol images & Sixel rasters
        for (const auto& pl : screen.graphics().placements()) {
            if ((pl.z_index < 0) != under_text) continue;
            const auto* img = screen.graphics().find_image(pl.image_id);
            if (!img || img->rgba_data.empty() || img->width <= 0 || img->height <= 0) continue;

            QImage qimg(img->rgba_data.data(), img->width, img->height, img->width * 4, QImage::Format_RGBA8888);
            qreal gx = pl.col * char_width_ + pl.x_offset;
            qreal gy = pl.row * char_height_ + pl.y_offset;
            qreal gw = pl.cols_spanned * char_width_;
            qreal gh = pl.rows_spanned * char_height_;

            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.drawImage(QRectF(gx, gy, gw, gh), qimg);
        }
    };

    // 1. Draw Graphics under text (z < 0)
    draw_images(true);

    // 2. Draw Cell Backgrounds & Batched Glyphs
    for (int r = 0; r < rows; ++r) {
        qreal y = r * char_height_;

        int span_start = -1;
        QString span_text;
        QColor span_fg;
        QFont span_font = font_;

        for (int c = 0; c < cols; ++c) {
            qreal x = c * char_width_;
            const auto& cell = screen.cell_at(r, c);

            bool is_selected = selection_.contains(r, c);
            QColor bg = resolveColor(cell.attrs.bg, false);
            QColor fg = resolveColor(cell.attrs.fg, true);

            if (cell.attrs.reverse) {
                std::swap(bg, fg);
            }

            // Draw Background rect if not default
            if (bg != DEFAULT_BG) {
                painter.fillRect(QRectF(x, y, char_width_ + 0.5, char_height_), bg);
            }

            // Draw Selection Highlight
            if (is_selected) {
                painter.fillRect(QRectF(x, y, char_width_ + 0.5, char_height_), SELECTION_BG);
            }

            // Batched Text Spans
            if (cell.codepoint != 0 && cell.codepoint != ' ') {
                QFont cellFont = font_;
                if (cell.attrs.bold) cellFont.setBold(true);
                if (cell.attrs.italic) cellFont.setItalic(true);
                if (cell.attrs.underline) cellFont.setUnderline(true);

                if (span_start == -1) {
                    span_start = c;
                    span_fg = fg;
                    span_font = cellFont;
                    span_text = QString::fromUcs4(&cell.codepoint, 1);
                } else if (span_fg == fg && span_font == cellFont) {
                    span_text.append(QString::fromUcs4(&cell.codepoint, 1));
                } else {
                    // Flush prior span
                    painter.setFont(span_font);
                    painter.setPen(span_fg);
                    painter.drawText(QPointF(span_start * char_width_, y + char_ascent_), span_text);

                    span_start = c;
                    span_fg = fg;
                    span_font = cellFont;
                    span_text = QString::fromUcs4(&cell.codepoint, 1);
                }
            } else if (span_start != -1) {
                // Flush open span on space / empty cell
                painter.setFont(span_font);
                painter.setPen(span_fg);
                painter.drawText(QPointF(span_start * char_width_, y + char_ascent_), span_text);
                span_start = -1;
                span_text.clear();
            }
        }

        // Flush any remaining span at line end
        if (span_start != -1 && !span_text.isEmpty()) {
            painter.setFont(span_font);
            painter.setPen(span_fg);
            painter.drawText(QPointF(span_start * char_width_, y + char_ascent_), span_text);
        }
    }

    // 3. Draw Graphics over text (z >= 0)
    draw_images(false);

    // 4. Draw Cursor
    if (cursor_visible_ && hasFocus()) {
        int cur_r = screen.cursor_row();
        int cur_c = screen.cursor_col();

        if (cur_r >= 0 && cur_r < rows && cur_c >= 0 && cur_c < cols) {
            qreal cx = cur_c * char_width_;
            qreal cy = cur_r * char_height_;

            painter.fillRect(QRectF(cx, cy, char_width_, char_height_), QColor(255, 255, 255, 180));
            const auto& cell = screen.cell_at(cur_r, cur_c);
            if (cell.codepoint != 0 && cell.codepoint != ' ') {
                painter.setPen(QColor(0, 0, 0));
                QString ch = QString::fromUcs4(&cell.codepoint, 1);
                painter.drawText(QPointF(cx, cy + char_ascent_), ch);
            }
        }
    }
}

void TerminalView::keyPressEvent(QKeyEvent* event) {
    Qt::KeyboardModifiers mods = event->modifiers();
    int key = event->key();

    // 1. Linux Standard Clipboard Copy: Ctrl+Shift+C (or Ctrl+C if text is selected)
    if ((mods == (Qt::ControlModifier | Qt::ShiftModifier) && key == Qt::Key_C) ||
        (mods == Qt::ControlModifier && key == Qt::Key_C && selection_.active)) {
        copySelection();
        event->accept();
        return;
    }

    // 2. Linux Standard Clipboard Paste: Ctrl+Shift+V (or Ctrl+V in normal mode)
    if ((mods == (Qt::ControlModifier | Qt::ShiftModifier) && key == Qt::Key_V) ||
        (mods == Qt::ControlModifier && key == Qt::Key_V)) {
        pasteClipboard();
        event->accept();
        return;
    }

    // 3. Zoom In/Out Shortcuts
    if (mods == Qt::ControlModifier && (key == Qt::Key_Plus || key == Qt::Key_Equal)) {
        setFontSize(font_.pointSizeF() + 1.0);
        event->accept();
        return;
    }
    if (mods == Qt::ControlModifier && (key == Qt::Key_Minus || key == Qt::Key_Underscore)) {
        setFontSize(font_.pointSizeF() - 1.0);
        event->accept();
        return;
    }
    if (mods == Qt::ControlModifier && key == Qt::Key_0) {
        setFontSize(11.0);
        event->accept();
        return;
    }

    // Clear selection on new input
    if (selection_.active) {
        selection_.active = false;
        update();
    }

    // 4. Special Keys to PTY Escape Sequences
    QByteArray payload;
    if (mods & Qt::ControlModifier) {
        if (key >= Qt::Key_A && key <= Qt::Key_Z) {
            char ctrl_char = static_cast<char>(key - Qt::Key_A + 1);
            payload.append(ctrl_char);
        } else if (key == Qt::Key_Space) {
            payload.append('\0');
        }
    } else {
        switch (key) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                payload = "\r";
                break;
            case Qt::Key_Backspace:
                payload = "\x7f";
                break;
            case Qt::Key_Tab:
                payload = "\t";
                break;
            case Qt::Key_Escape:
                payload = "\x1b";
                break;
            case Qt::Key_Up:
                payload = "\x1b[A";
                break;
            case Qt::Key_Down:
                payload = "\x1b[B";
                break;
            case Qt::Key_Right:
                payload = "\x1b[C";
                break;
            case Qt::Key_Left:
                payload = "\x1b[D";
                break;
            case Qt::Key_Home:
                payload = "\x1b[H";
                break;
            case Qt::Key_End:
                payload = "\x1b[F";
                break;
            case Qt::Key_PageUp:
                payload = "\x1b[5~";
                break;
            case Qt::Key_PageDown:
                payload = "\x1b[6~";
                break;
            case Qt::Key_Delete:
                payload = "\x1b[3~";
                break;
            case Qt::Key_Insert:
                payload = "\x1b[2~";
                break;
            default:
                if (!event->text().isEmpty()) {
                    payload = event->text().toUtf8();
                }
                break;
        }
    }

    if (!payload.isEmpty()) {
        bridge_->writeInput(payload);
        cursor_visible_ = true;
        cursor_timer_.start(500);
        event->accept();
    }
}

void TerminalView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QPoint cell = pixelToCell(event->pos());
        selection_.active = true;
        selection_.start_row = cell.y();
        selection_.start_col = cell.x();
        selection_.end_row = cell.y();
        selection_.end_col = cell.x();
        selecting_ = true;
        update();
    } else if (event->button() == Qt::MiddleButton) {
        // Middle Click Paste
        pasteClipboard();
    }
}

void TerminalView::mouseMoveEvent(QMouseEvent* event) {
    if (selecting_) {
        QPoint cell = pixelToCell(event->pos());
        selection_.end_row = cell.y();
        selection_.end_col = cell.x();
        update();
    }
}

void TerminalView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && selecting_) {
        selecting_ = false;
        if (selection_.start_row == selection_.end_row && selection_.start_col == selection_.end_col) {
            selection_.active = false;
        }
        update();
    }
}

void TerminalView::wheelEvent(QWheelEvent* event) {
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    if (numSteps != 0) {
        std::lock_guard<std::mutex> lock(bridge_->screenMutex());
        bridge_->screenBuffer().scroll_view(-numSteps * 3);
        update();
    }
}

QString TerminalView::selectedText() const {
    if (!selection_.active) return "";

    std::lock_guard<std::mutex> lock(bridge_->screenMutex());
    const auto& screen = bridge_->screenBuffer();

    int r0 = std::clamp(std::min(selection_.start_row, selection_.end_row), 0, screen.rows() - 1);
    int r1 = std::clamp(std::max(selection_.start_row, selection_.end_row), 0, screen.rows() - 1);

    QString text;
    for (int r = r0; r <= r1; ++r) {
        int c0 = (r == r0) ? std::min(selection_.start_col, selection_.end_col) : 0;
        int c1 = (r == r1) ? std::max(selection_.start_col, selection_.end_col) : screen.cols() - 1;

        c0 = std::clamp(c0, 0, screen.cols() - 1);
        c1 = std::clamp(c1, 0, screen.cols() - 1);

        QString rowText;
        for (int c = c0; c <= c1; ++c) {
            const auto& cell = screen.cell(r, c);
            if (cell.codepoint != 0) {
                rowText.append(QString::fromUcs4(&cell.codepoint, 1));
            } else {
                rowText.append(' ');
            }
        }
        text.append(rowText.trimmed());
        if (r < r1) text.append('\n');
    }
    return text;
}

void TerminalView::copySelection() {
    QString text = selectedText();
    if (!text.isEmpty()) {
        QApplication::clipboard()->setText(text);
    }
}

void TerminalView::pasteClipboard() {
    QString text = QApplication::clipboard()->text();
    if (!text.isEmpty()) {
        // Clean single paste
        bridge_->writeInput(text.toUtf8());
    }
}

void TerminalView::selectAll() {
    std::lock_guard<std::mutex> lock(bridge_->screenMutex());
    const auto& screen = bridge_->screenBuffer();
    selection_.active = true;
    selection_.start_row = 0;
    selection_.start_col = 0;
    selection_.end_row = screen.rows() - 1;
    selection_.end_col = screen.cols() - 1;
    update();
}

void TerminalView::clearScrollback() {
    std::lock_guard<std::mutex> lock(bridge_->screenMutex());
    bridge_->screenBuffer().clear_scrollback();
    update();
}

void TerminalView::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    menu.setStyleSheet("QMenu { background-color: #21252b; color: #abb2bf; border: 1px solid #3e4451; padding: 4px; } "
                       "QMenu::item:selected { background-color: #3b82f6; color: #ffffff; }");

    auto* copyAct = menu.addAction("Copy (Ctrl+Shift+C)", this, &TerminalView::copySelection);
    copyAct->setEnabled(selection_.active);

    auto* pasteAct = menu.addAction("Paste (Ctrl+Shift+V)", this, &TerminalView::pasteClipboard);
    pasteAct->setEnabled(!QApplication::clipboard()->text().isEmpty());

    menu.addSeparator();
    menu.addAction("Select All", this, &TerminalView::selectAll);
    menu.addAction("Clear Scrollback", this, &TerminalView::clearScrollback);

    menu.exec(event->globalPos());
}

void TerminalView::focusInEvent(QFocusEvent*) {
    cursor_visible_ = true;
    cursor_timer_.start(500);
    update();
}

void TerminalView::focusOutEvent(QFocusEvent*) {
    cursor_visible_ = false;
    cursor_timer_.stop();
    update();
}

} // namespace meridian::gui

