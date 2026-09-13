#include "HeaderSection.hpp"

#include <QFile>

namespace meridian::gui {

HeaderSection::HeaderSection(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 8);
    mainLayout->setSpacing(12);

    // Top Row: Image (Left) + System Info (Right)
    auto* topRowLayout = new QHBoxLayout();
    topRowLayout->setContentsMargins(0, 0, 0, 0);
    topRowLayout->setSpacing(24);

    image_renderer_ = new ImageRenderer(this);
    image_renderer_->setFixedSize(270, 270);
    image_renderer_->setCornerRadius(8);

    // Load custom artwork if set, otherwise fallback to bundled artwork
    bool loaded = false;
    const char* env_art = std::getenv("MERIDIAN_ARTWORK");
    if (env_art && image_renderer_->loadImage(QString::fromUtf8(env_art))) {
        loaded = true;
    }
    if (!loaded) {
        const char* home = std::getenv("HOME");
        QString config_art = QString::fromUtf8(home ? home : ".") + "/.config/meridian/artwork.png";
        if (image_renderer_->loadImage(config_art)) {
            loaded = true;
        }
    }
    if (!loaded) {
        if (!image_renderer_->loadImage("resources/images/artwork.png")) {
            image_renderer_->loadImage(":/resources/images/artwork.png");
        }
    }

    sys_info_ = new SystemInfoWidget(this);
    sys_info_->setFixedHeight(270);

    topRowLayout->addWidget(image_renderer_, 0, Qt::AlignLeft | Qt::AlignVCenter);
    topRowLayout->addWidget(sys_info_, 1);

    // Bottom Row: Clock & Powerline Badges
    clock_widget_ = new ClockWidget(this);

    mainLayout->addLayout(topRowLayout);
    mainLayout->addWidget(clock_widget_);
}

void HeaderSection::updateWorkingDir(const QString& dir) {
    if (clock_widget_) {
        clock_widget_->setCurrentDirectory(dir);
    }
}

} // namespace meridian::gui

