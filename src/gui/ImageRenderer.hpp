#pragma once
// src/gui/ImageRenderer.hpp
//
// High-DPI image renderer widget supporting PNG, JPEG, WebP, GIF, and animations.
// Preserves aspect ratio, renders smooth rounded corners and clean scaling.

#include <QWidget>
#include <QPixmap>
#include <QMovie>
#include <QString>
#include <memory>

namespace meridian::gui {

class ImageRenderer : public QWidget {
    Q_OBJECT

public:
    explicit ImageRenderer(QWidget* parent = nullptr);
    ~ImageRenderer() override = default;

    bool loadImage(const QString& path);
    void setCornerRadius(int radius);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap pixmap_;
    std::unique_ptr<QMovie> movie_;
    int corner_radius_ = 8;
    bool is_animated_ = false;
};

} // namespace meridian::gui

