#include "PtyBridge.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

namespace meridian::gui {

PtyBridge::PtyBridge(int rows, int cols, QObject* parent)
    : QObject(parent)
    , screen_(rows, cols)
    , parser_(screen_)
{
}

PtyBridge::~PtyBridge() {
    if (notifier_) {
        notifier_->setEnabled(false);
    }
    if (pty_mgr_.is_running()) {
        pty_mgr_.close_master();
    }
}

bool PtyBridge::startSession(const QString& program) {
    pty::PtyOptions opts;
    opts.rows = screen_.rows();
    opts.cols = screen_.cols();

    if (!program.isEmpty()) {
        opts.program = program.toStdString();
    } else {
        const char* shell_env = std::getenv("SHELL");
        opts.program = shell_env ? shell_env : "/bin/bash";
    }

    if (!pty_mgr_.spawn(opts)) {
        return false;
    }

    pty_master_fd_ = pty_mgr_.master_fd();
    // Set non-blocking on master fd
    int flags = fcntl(pty_master_fd_, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(pty_master_fd_, F_SETFL, flags | O_NONBLOCK);
    }

    notifier_ = std::make_unique<QSocketNotifier>(pty_master_fd_, QSocketNotifier::Read, this);
    connect(notifier_.get(), &QSocketNotifier::activated, this, &PtyBridge::handlePtyRead);
    notifier_->setEnabled(true);

    running_ = true;
    return true;
}

void PtyBridge::handlePtyRead() {
    if (pty_master_fd_ < 0) return;

    char buf[4096];
    ssize_t bytes_read = 0;
    std::string accumulated;

    while ((bytes_read = read(pty_master_fd_, buf, sizeof(buf))) > 0) {
        accumulated.append(buf, bytes_read);
    }

    if (bytes_read == 0 || (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
        // Child exited or EOF
        running_ = false;
        if (notifier_) notifier_->setEnabled(false);
        int status = pty_mgr_.wait_for_child();
        emit sessionEnded(status);
        return;
    }

    if (!accumulated.empty()) {
        {
            std::lock_guard<std::mutex> lock(screen_mutex_);
            parser_.feed(accumulated);
        }
        if (!parser_.window_title().empty()) {
            emit titleChanged(QString::fromStdString(parser_.window_title()));
        }
        emit screenUpdated();
    }
}

void PtyBridge::writeInput(const QByteArray& data) {
    if (pty_master_fd_ >= 0 && !data.isEmpty()) {
        pty_mgr_.write(data.constData(), data.size());
    }
}

void PtyBridge::resize(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return;

    {
        std::lock_guard<std::mutex> lock(screen_mutex_);
        screen_.resize(rows, cols);
    }
    if (pty_mgr_.is_running()) {
        pty_mgr_.resize(rows, cols);
    }
    emit screenUpdated();
}

QString PtyBridge::currentWorkingDir() const {
    if (!pty_mgr_.is_running()) return "~";

    // Inspect /proc/<pid>/cwd on Linux
    pid_t child_pid = pty_mgr_.child_pid();
    if (child_pid > 0) {
        char path_buf[1024];
        std::string proc_cwd = "/proc/" + std::to_string(child_pid) + "/cwd";
        ssize_t len = readlink(proc_cwd.c_str(), path_buf, sizeof(path_buf) - 1);
        if (len > 0) {
            path_buf[len] = '\0';
            QString cwd = QString::fromUtf8(path_buf);
            QString home = QString::fromUtf8(std::getenv("HOME"));
            if (!home.isEmpty() && cwd.startsWith(home)) {
                return "~" + cwd.mid(home.length());
            }
            return cwd;
        }
    }
    return "~";
}

bool PtyBridge::isAlive() const {
    return running_ && pty_mgr_.is_running();
}

} // namespace meridian::gui

