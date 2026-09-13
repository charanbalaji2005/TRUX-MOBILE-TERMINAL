#include "SystemInfoWidget.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QProcess>
#include <fstream>
#include <pwd.h>
#include <sstream>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace meridian::gui {

SystemInfoWidget::SystemInfoWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Vivid neon dots matching reference theme
    details_.palette_colors = {
        QColor("#00E5FF"), // Neon Cyan
        QColor("#3B82F6"), // Electric Blue
        QColor("#A855F7"), // Neon Purple
        QColor("#F43F5E"), // Hot Pink
        QColor("#22C55E"), // Neon Green
        QColor("#F59E0B"), // Amber
        QColor("#FDE047"), // Yellow
        QColor("#FB923C"), // Orange
        QColor("#EF4444")  // Red
    };

    detectStaticDetails();
    detectDynamicMetrics();

    connect(&refresh_timer_, &QTimer::timeout, this, &SystemInfoWidget::refreshMetrics);
    refresh_timer_.start(3000); // refresh memory and uptime every 3s
}

void SystemInfoWidget::detectStaticDetails() {
    // 1. Username & Hostname
    char hostname[256] = "linux";
    gethostname(hostname, sizeof(hostname));
    QString user = "user";
    struct passwd* pw = getpwuid(geteuid());
    if (pw && pw->pw_name) {
        user = QString::fromUtf8(pw->pw_name);
    }
    details_.user_host = QString("%1@%2").arg(user, QString::fromUtf8(hostname));

    // 2. OS & Kernel
    struct utsname uts{};
    if (uname(&uts) == 0) {
        details_.os_kernel = QString("%1 %2").arg(QString::fromUtf8(uts.sysname), QString::fromUtf8(uts.release));
    } else {
        details_.os_kernel = "Linux";
    }

    // 3. Desktop Environment / Window Manager
    const char* xdg_desktop = std::getenv("XDG_CURRENT_DESKTOP");
    const char* session_type = std::getenv("XDG_SESSION_TYPE");
    QString wm = xdg_desktop ? QString::fromUtf8(xdg_desktop) : "Desktop";
    QString st = session_type ? QString::fromUtf8(session_type) : "Wayland";
    st[0] = st[0].toUpper();

    if (std::getenv("HYPRLAND_INSTANCE_SIGNATURE")) {
        wm = "Hyprland";
    } else if (std::getenv("SWAYSOCK")) {
        wm = "Sway";
    }
    details_.desktop_wm = QString("%1 (%2)").arg(wm, st);

    // 4. Shell (dynamic version detection)
    const char* shell_env = std::getenv("SHELL");
    QString sh_bin = shell_env ? QString::fromUtf8(shell_env) : "/bin/bash";
    QString sh_name = sh_bin.section('/', -1);

    QString ver_cmd = QString("%1 --version 2>/dev/null").arg(sh_bin);
    FILE* pipe = popen(ver_cmd.toUtf8().constData(), "r");
    QString full_ver;
    if (pipe) {
        char buf[256];
        if (fgets(buf, sizeof(buf), pipe)) {
            full_ver = QString::fromUtf8(buf).trimmed();
        }
        pclose(pipe);
    }

    QString ver_str;
    for (const QString& part : full_ver.split(' ', Qt::SkipEmptyParts)) {
        if (!part.isEmpty() && (part[0].isDigit() || (part.size() > 1 && part[0] == 'v' && part[1].isDigit()))) {
            ver_str = part;
            if (ver_str.startsWith('v')) ver_str = ver_str.mid(1);
            if (ver_str.contains('(')) ver_str = ver_str.section('(', 0, 0);
            if (ver_str.contains(',')) ver_str = ver_str.section(',', 0, 0);
            break;
        }
    }
    if (ver_str.isEmpty()) ver_str = "latest";
    details_.shell_version = QString("%1 %2").arg(sh_name, ver_str);

    // 5. Terminal Version
    details_.terminal_version = "meridian 2.1";
}

void SystemInfoWidget::detectDynamicMetrics() {
    // 1. RAM Usage
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        uint64_t total_kb = 0, avail_kb = 0;
        while (std::getline(meminfo, line)) {
            if (line.rfind("MemTotal:", 0) == 0) {
                std::istringstream ss(line.substr(9));
                ss >> total_kb;
            } else if (line.rfind("MemAvailable:", 0) == 0) {
                std::istringstream ss(line.substr(13));
                ss >> avail_kb;
            }
        }
        if (total_kb > 0) {
            uint64_t used_kb = (total_kb > avail_kb) ? (total_kb - avail_kb) : 0;
            double used_gib = static_cast<double>(used_kb) / (1024.0 * 1024.0);
            double total_gib = static_cast<double>(total_kb) / (1024.0 * 1024.0);
            details_.ram_usage = QString("%1 GiB / %2 GiB")
                                    .arg(QString::number(used_gib, 'f', 2))
                                    .arg(QString::number(total_gib, 'f', 2));
        }
    }

    // 2. Uptime
    std::ifstream uptime_file("/proc/uptime");
    if (uptime_file.is_open()) {
        double seconds = 0;
        uptime_file >> seconds;
        uint64_t total_sec = static_cast<uint64_t>(seconds);
        uint64_t days = total_sec / 86400;
        uint64_t hours = (total_sec % 86400) / 3600;
        uint64_t mins = (total_sec % 3600) / 60;

        QStringList parts;
        if (days > 0) parts << QString("%1 %2").arg(days).arg(days == 1 ? "day" : "days");
        if (hours > 0 || days > 0) parts << QString("%1 %2").arg(hours).arg(hours == 1 ? "hour" : "hours");
        parts << QString("%1 %2").arg(mins).arg(mins == 1 ? "min" : "mins");
        details_.uptime = parts.join(", ");
    }
}

void SystemInfoWidget::refreshMetrics() {
    detectDynamicMetrics();
    update();
}

QSize SystemInfoWidget::sizeHint() const {
    return QSize(380, 260);
}

void SystemInfoWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QFont monoFont("JetBrains Mono", 10);
    monoFont.setStyleHint(QFont::Monospace);
    painter.setFont(monoFont);

    int w = width();
    int y = 22;

    // 1. User @ Host Title (Centered)
    painter.setPen(QColor("#d8dee9"));
    QFont titleFont = monoFont;
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, y, w, 20), Qt::AlignCenter, details_.user_host);
    y += 28;

    // Helper: draw decorative divider with center circle node
    auto drawDivider = [&](int divY) {
        painter.setPen(QPen(QColor(180, 190, 205, 120), 1.0));
        int midX = w / 2;
        int lineLen = std::min(w - 40, 260);
        int startX = (w - lineLen) / 2;
        int endX = startX + lineLen;

        painter.drawLine(startX, divY, midX - 8, divY);
        painter.drawLine(midX + 8, divY, midX + lineLen, divY);

        painter.setBrush(QBrush(QColor("#18191c"))); // dark fill inside circle
        painter.drawEllipse(QPoint(midX, divY), 4, 4);
        painter.setBrush(Qt::NoBrush);
    };

    // 2. Top Divider Line
    drawDivider(y);
    y += 18;

    // Helper: draw single metadata item row
    auto drawItem = [&](const QString& icon, const QColor& iconColor, const QString& text) {
        painter.setFont(monoFont);
        int leftMargin = std::max(20, (w - 280) / 2);
        painter.setPen(iconColor);
        painter.drawText(leftMargin, y, icon);
        painter.setPen(QColor("#a0aec0"));
        painter.drawText(leftMargin + 24, y, "→");
        painter.setPen(QColor("#d8dee9"));
        painter.drawText(leftMargin + 46, y, text);
        y += 22;
    };

    // 3. System Items List (sleek Nerd Font glyphs matching reference theme)
    drawItem("󰌽", QColor("#8ca388"), details_.os_kernel);
    drawItem("󰨇", QColor("#6ea5b9"), details_.desktop_wm);
    drawItem("󰞷", QColor("#9b87af"), details_.shell_version);
    drawItem("", QColor("#5aafa8"), details_.terminal_version);
    drawItem("󰘚", QColor("#d7b987"), details_.ram_usage);
    drawItem("󱑂", QColor("#cd8791"), details_.uptime);

    y += 4;
    // 4. Bottom Divider Line
    drawDivider(y);
    y += 20;

    // 5. Palette Color Dots Row (Centered)
    int dotCount = static_cast<int>(details_.palette_colors.size());
    int dotRadius = 5;
    int dotSpacing = 16;
    int totalDotWidth = (dotCount - 1) * dotSpacing + dotRadius * 2;
    int startDotX = (w - totalDotWidth) / 2;

    for (int i = 0; i < dotCount; ++i) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(details_.palette_colors[i]));
        painter.drawEllipse(QPoint(startDotX + i * dotSpacing + dotRadius, y), dotRadius, dotRadius);
    }
}

} // namespace meridian::gui

