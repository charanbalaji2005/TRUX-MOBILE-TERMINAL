#pragma once
// src/gui/HeaderSection.hpp
//
// Top header section combining the artwork pixel art image, dynamic Linux
// system metadata, and live powerline clock/prompt badges.

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <memory>

#include "ImageRenderer.hpp"
#include "SystemInfoWidget.hpp"
#include "ClockWidget.hpp"

namespace meridian::gui {

class HeaderSection : public QWidget {
    Q_OBJECT

public:
    explicit HeaderSection(QWidget* parent = nullptr);
    ~HeaderSection() override = default;

    ImageRenderer* imageRenderer() { return image_renderer_; }
    SystemInfoWidget* systemInfoWidget() { return sys_info_; }
    ClockWidget* clockWidget() { return clock_widget_; }

    void updateWorkingDir(const QString& dir);

private:
    ImageRenderer* image_renderer_ = nullptr;
    SystemInfoWidget* sys_info_ = nullptr;
    ClockWidget* clock_widget_ = nullptr;
};

} // namespace meridian::gui

