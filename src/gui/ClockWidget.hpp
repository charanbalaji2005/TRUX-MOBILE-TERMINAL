#pragma once
// src/gui/ClockWidget.hpp
//
// Live clock and powerline status badge widget matching reference design.
// Updates date/time every second and tracks current working directory.

#include <QWidget>
#include <QString>
#include <QTimer>

namespace meridian::gui {

class ClockWidget : public QWidget {
    Q_OBJECT

public:
    explicit ClockWidget(QWidget* parent = nullptr);
    ~ClockWidget() override = default;

    void setCurrentDirectory(const QString& dir);
    void setUsername(const QString& user);
    void setCommandIndex(int idx);

public slots:
    void updateTime();

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QString date_time_str_;
    QString current_dir_ = "~";
    QString username_ = "user";
    int command_idx_ = 3610;
    QTimer timer_;
};

} // namespace meridian::gui

