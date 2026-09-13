// tests / test_pty_manager.cpp
//
// Real integration tests: an actual PTY pair is opened, a real process
// is forked and exec'd onto it, and we read its actual output back
// through the master fd.

#include "mini_test.hpp"
#include "../src/core/pty/pty_manager.hpp"

#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

using namespace meridian::pty;

MTEST(pty_spawns_real_process_and_reads_its_output) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/echo";
    opts.args = {"hello-from-real-process"};
    bool ok = pty.spawn(opts);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(pty.is_running());

    std::string out;
    char buf[256];
    // Drain until EOF (child exits and closes its end of the pty).
    ssize_t n;
    while ((n = pty.read(buf, sizeof(buf))) > 0) out.append(buf, static_cast<std::size_t>(n));

    ASSERT_TRUE(out.find("hello-from-real-process") != std::string::npos);

    int status = pty.wait_for_exit();
    ASSERT_EQ(status, 0);
}

MTEST(pty_reports_nonzero_exit_status) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/bash";
    opts.args = {"-c", "exit 3"};
    ASSERT_TRUE(pty.spawn(opts));

    char buf[64];
    while (pty.read(buf, sizeof(buf)) > 0) { /* drain */ }

    int status = pty.wait_for_exit();
    ASSERT_EQ(status, 3);
}

MTEST(pty_resize_updates_kernel_window_size) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/bash";
    opts.args = {"-c", "sleep 0.2"};
    opts.rows = 24;
    opts.cols = 80;
    ASSERT_TRUE(pty.spawn(opts));

    pty.resize(40, 120);

    struct winsize ws{};
    ioctl(pty.master_fd(), TIOCGWINSZ, &ws);
    ASSERT_EQ(ws.ws_row, (unsigned short)40);
    ASSERT_EQ(ws.ws_col, (unsigned short)120);

    char buf[64];
    while (pty.read(buf, sizeof(buf)) > 0) { /* drain */ }
    pty.wait_for_exit();
}

MTEST(pty_write_reaches_child_stdin) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = "/bin/bash";
    // cat with no args echoes stdin to stdout until EOF; we write a line
    // then close by exiting the shell via the written command instead
    // (simpler and avoids needing to close master mid-test).
    opts.args = {"-c", "read line; echo \"got:$line\""};
    ASSERT_TRUE(pty.spawn(opts));

    pty.write("ping\n");

    std::string out;
    char buf[256];
    ssize_t n;
    while ((n = pty.read(buf, sizeof(buf))) > 0) out.append(buf, static_cast<std::size_t>(n));

    ASSERT_TRUE(out.find("got:ping") != std::string::npos);
    pty.wait_for_exit();
}
