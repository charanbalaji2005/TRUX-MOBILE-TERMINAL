#pragma once
// src/gui/SystemInfoWidget.hpp
//
// Live Linux system information widget. Dynamically discovers username,
// hostname, kernel, desktop environment, shell, memory, uptime, and palette.

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QVector>
#include <QColor>

namespace meridian::gui {

struct SystemDetails {
    QString user_host;
    QString os_kernel;
    QString desktop_wm;
    QString shell_version;
    QString terminal_version;
    QString ram_usage;
    QString uptime;
    QVector<QColor> palette_colors;
};

class SystemInfoWidget : public QWidget {
    Q_OBJECT

public:
    explicit SystemInfoWidget(QWidget* parent = nullptr);
    ~SystemInfoWidget() override = default;

public slots:
    void refreshMetrics();

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    SystemDetails details_;
    QTimer refresh_timer_;

    void detectStaticDetails();
    void detectDynamicMetrics();
};

} // namespace meridian::gui

