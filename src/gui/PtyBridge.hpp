#pragma once
// src/gui/PtyBridge.hpp
//
// Bridges Meridian's C++ PTY manager, ANSI state machine, and ScreenBuffer
// into Qt's asynchronous event loop with real Linux process execution.

#include <QObject>
#include <QThread>
#include <QSocketNotifier>
#include <QByteArray>
#include <QString>
#include <memory>
#include <mutex>

#include "../core/pty/pty_manager.hpp"
#include "../core/vt/ansi_parser.hpp"
#include "../core/vt/screen_buffer.hpp"

namespace meridian::gui {

class PtyBridge : public QObject {
    Q_OBJECT

public:
    explicit PtyBridge(int rows = 24, int cols = 80, QObject* parent = nullptr);
    ~PtyBridge() override;

    bool startSession(const QString& program = "");
    void writeInput(const QByteArray& data);
    void resize(int rows, int cols);

    vt::ScreenBuffer& screenBuffer() { return screen_; }
    const vt::ScreenBuffer& screenBuffer() const { return screen_; }
    std::mutex& screenMutex() { return screen_mutex_; }

    QString currentWorkingDir() const;
    bool isAlive() const;

signals:
    void screenUpdated();
    void titleChanged(const QString& title);
    void directoryChanged(const QString& cwd);
    void sessionEnded(int exit_code);

private slots:
    void handlePtyRead();

private:
    pty::PtyManager pty_mgr_;
    vt::AnsiParser parser_;
    vt::ScreenBuffer screen_;
    std::unique_ptr<QSocketNotifier> notifier_;
    mutable std::mutex screen_mutex_;
    int pty_master_fd_ = -1;
    bool running_ = false;
};

} // namespace meridian::gui

