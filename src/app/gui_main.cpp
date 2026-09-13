#include "../gui/MainWindow.hpp"
#include <QApplication>
#include <QIcon>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("Meridian Terminal");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Meridian Terminal");
    app.setWindowIcon(QIcon(":/resources/icons/meridian-terminal.svg"));

    meridian::gui::MainWindow window;
    window.show();

    return app.exec();
}

