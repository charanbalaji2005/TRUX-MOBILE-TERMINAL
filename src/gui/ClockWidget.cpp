#include "ClockWidget.hpp"
#include "../dev/git_intel.hpp"

#include <QDateTime>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <pwd.h>
#include <unistd.h>

namespace meridian::gui {

ClockWidget::ClockWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(60);

    // Initial username detection
    struct passwd* pw = getpwuid(geteuid());
    if (pw && pw->pw_name) {
        username_ = QString::fromUtf8(pw->pw_name);
    }
    command_idx_ = 0;

    // Detect CWD
    char cwd_buf[1024];
    if (getcwd(cwd_buf, sizeof(cwd_buf))) {
        current_dir_ = QString::fromUtf8(cwd_buf);
        const char* home = std::getenv("HOME");
        if (home && current_dir_.startsWith(home)) {
            current_dir_ = "~" + current_dir_.mid(static_cast<int>(strlen(home)));
        }
    }

    updateTime();
    connect(&timer_, &QTimer::timeout, this, &ClockWidget::updateTime);
    timer_.start(1000);
}

void ClockWidget::updateTime() {
    QDateTime now = QDateTime::currentDateTime();
    date_time_str_ = now.toString("ddd d MMM - hh:mm");
    update();
}

void ClockWidget::setCurrentDirectory(const QString& dir) {
    current_dir_ = dir;
    const char* home = std::getenv("HOME");
    if (home && current_dir_.startsWith(home)) {
        current_dir_ = "~" + current_dir_.mid(static_cast<int>(strlen(home)));
    }
    update();
}

void ClockWidget::setUsername(const QString& user) {
    username_ = user;
    update();
}

void ClockWidget::setCommandIndex(int idx) {
    command_idx_ = idx;
    update();
}

QSize ClockWidget::sizeHint() const {
    return QSize(500, 60);
}

void ClockWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QFont font("JetBrains Mono", 10);
    font.setStyleHint(QFont::Monospace);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics fm(font);

    int y1 = 4;
    int h1 = 24;
    int arrowW = 12;
    int padX = 12;

    // --- ROW 1: Date/Time (Cobalt Blue) + Dir (Ocean Cyan) + Git Status (Crimson Red) ---
    int textW1 = fm.horizontalAdvance(date_time_str_) + padX * 2;

    // Segment 1 (Date/Time: Cobalt Blue #1a6cda)
    QPainterPath p1;
    p1.moveTo(0, y1);
    p1.lineTo(textW1, y1);
    p1.lineTo(textW1 + arrowW, y1 + h1 / 2.0);
    p1.lineTo(textW1, y1 + h1);
    p1.lineTo(0, y1 + h1);
    p1.closeSubpath();

    painter.fillPath(p1, QBrush(QColor("#1a6cda")));
    painter.setPen(QColor("#ffffff"));
    painter.drawText(QRect(padX, y1, textW1 - padX * 2, h1), Qt::AlignVCenter | Qt::AlignLeft, date_time_str_);

    // Segment 2 (Current Directory: Ocean Cyan #189cb8)
    int textW2 = fm.horizontalAdvance(current_dir_) + padX * 2;
    int startX2 = textW1;

    QPainterPath p2;
    p2.moveTo(startX2, y1);
    p2.lineTo(startX2 + arrowW, y1 + h1 / 2.0);
    p2.lineTo(startX2, y1 + h1);
    p2.lineTo(startX2 + textW2, y1 + h1);
    p2.lineTo(startX2 + textW2 + arrowW, y1 + h1 / 2.0);
    p2.lineTo(startX2 + textW2, y1);
    p2.closeSubpath();

    painter.fillPath(p2, QBrush(QColor("#189cb8")));
    painter.setPen(QColor("#ffffff"));
    painter.drawText(QRect(startX2 + arrowW + padX / 2, y1, textW2, h1), Qt::AlignVCenter | Qt::AlignLeft, current_dir_);

    // Segment 3 (Live Git Intel: Emerald Green #10b981)
    char cwd_raw[1024];
    std::string raw_dir = getcwd(cwd_raw, sizeof(cwd_raw)) ? cwd_raw : ".";
    auto git = meridian::dev::GitIntel::inspect_directory(raw_dir);

    int startX3 = startX2 + textW2;
    if (git.is_git_repo) {
        QString git_text = QString("󰘬 origin ☊ %1").arg(QString::fromStdString(git.branch_name));
        if (git.ahead_count > 0) git_text += QString(" ↑%1").arg(git.ahead_count);
        if (git.behind_count > 0) git_text += QString(" ↓%1").arg(git.behind_count);
        if (git.unstaged_count > 0) git_text += QString(" %1✸").arg(git.unstaged_count);
        if (git.staged_count > 0) git_text += QString(" %1●").arg(git.staged_count);
        if (git.is_clean) git_text += " ✔";

        int textW3 = fm.horizontalAdvance(git_text) + padX * 2;
        QPainterPath p3;
        p3.moveTo(startX3, y1);
        p3.lineTo(startX3 + arrowW, y1 + h1 / 2.0);
        p3.lineTo(startX3, y1 + h1);
        p3.lineTo(startX3 + textW3, y1 + h1);
        p3.lineTo(startX3 + textW3 + arrowW, y1 + h1 / 2.0);
        p3.lineTo(startX3 + textW3, y1);
        p3.closeSubpath();

        painter.fillPath(p3, QBrush(QColor("#10b981"))); // Emerald Green matching user reference
        painter.setPen(QColor("#14231c")); // Deep dark green/charcoal text
        painter.drawText(QRect(startX3 + arrowW + padX / 2, y1, textW3, h1), Qt::AlignVCenter | Qt::AlignLeft, git_text);
    }

    // --- ROW 2: Username (Golden Yellow #d4b41b) + Prompt symbol ❯ ---
    int y2 = y1 + h1 + 6;
    int h2 = 24;

    QString userPrompt = "@" + username_;
    int textW4 = fm.horizontalAdvance(userPrompt) + padX * 2;
    QPainterPath p4;
    p4.moveTo(0, y2);
    p4.lineTo(textW4, y2);
    p4.lineTo(textW4 + arrowW, y2 + h2 / 2.0);
    p4.lineTo(textW4, y2 + h2);
    p4.lineTo(0, y2 + h2);
    p4.closeSubpath();

    painter.fillPath(p4, QBrush(QColor("#d4b41b")));
    painter.setPen(QColor("#2d1832")); // Deep plum/black text for maximum contrast
    painter.drawText(QRect(padX, y2, textW4 - padX * 2, h2), Qt::AlignVCenter | Qt::AlignLeft, userPrompt);

    // Prompt arrow symbol
    painter.setPen(QColor("#ffffff"));
    painter.drawText(textW4 + arrowW + 8, y2 + h2 - 6, "❯");
}

} // namespace meridian::gui
