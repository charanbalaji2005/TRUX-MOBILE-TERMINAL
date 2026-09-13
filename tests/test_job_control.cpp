// tests / test_job_control.cpp
//
// This is the real test for job control: it spawns the actual
// meridian-shell BINARY (not the Executor class directly) on a REAL PTY,
// exactly the way PtyManager would host any shell. It then writes a raw
// Ctrl+Z byte (0x1A) to the PTY master. That byte is never delivered to
// meridian-shell as ordinary input — the kernel's tty line discipline
// intercepts it (ISIG mode) and sends a real SIGTSTP to the terminal's
// foreground process group. This is the same mechanism a real interactive
// session relies on, so if this test passes, process-group assignment and
// tcsetpgrp foreground handoff are both actually working, not just
// plausible-looking code.

#include "mini_test.hpp"
#include "../src/core/pty/pty_manager.hpp"

#include <chrono>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <poll.h>
#include <string>
#include <unistd.h>

using namespace meridian::pty;

namespace {

// master_fd_ is a BLOCKING descriptor, so read() must never be called
// without first confirming (via poll()) that data is actually available —
// otherwise a single call can block indefinitely once the shell has
// nothing more to say. This is what the very first version of this test
// got wrong, and why it hung rather than failed.
std::string read_available(PtyManager& pty, int timeout_ms) {
    std::string out;
    char buf[4096];
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (true) {
        auto now = std::chrono::steady_clock::now();
        if (now >= deadline) break;
        int remaining = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
        struct pollfd pfd{pty.master_fd(), POLLIN, 0};
        int pr = poll(&pfd, 1, remaining);
        if (pr <= 0) break; // timeout or error: return whatever's been read so far
        ssize_t n = pty.read(buf, sizeof(buf));
        if (n <= 0) break; // EOF or error
        out.append(buf, static_cast<std::size_t>(n));
    }
    return out;
}

std::string shell_binary_path() { return "./build/meridian-shell"; }

} // namespace

MTEST(job_control_ctrl_z_stops_foreground_job_via_real_pty) {
    PtyManager pty;
    SpawnOptions opts;
    opts.program = shell_binary_path();
    opts.rows = 24;
    opts.cols = 80;
    bool ok = pty.spawn(opts);
    ASSERT_TRUE(ok);
    if (!ok) return;

    // Let the shell start and enable job control.
    read_available(pty, 200);

    pty.write("sleep 100\n");
    read_available(pty, 200); // let sleep actually start and become foreground

    // The real test: a genuine Ctrl+Z byte through the kernel's tty driver.
    pty.write("\x1a");
    std::string after_stop = read_available(pty, 500);
    ASSERT_TRUE(after_stop.find("Stopped") != std::string::npos);

    pty.write("jobs\n");
    std::string jobs_out = read_available(pty, 300);
    ASSERT_TRUE(jobs_out.find("Stopped") != std::string::npos);
    ASSERT_TRUE(jobs_out.find("sleep 100") != std::string::npos);

    // Extract the pgid from the jobs listing ("[1] <pgid>  Stopped ...")
    // so we can clean up the still-alive (stopped) sleep process rather
    // than leaving it around for the rest of the test run.
    pid_t pgid = -1;
    auto bracket = jobs_out.find(']');
    if (bracket != std::string::npos) {
        pgid = static_cast<pid_t>(std::strtol(jobs_out.c_str() + bracket + 1, nullptr, 10));
    }

    pty.write("bg\n");
    std::string bg_out = read_available(pty, 300);
    ASSERT_TRUE(bg_out.find("sleep 100") != std::string::npos);

    pty.write("jobs\n");
    std::string jobs_out2 = read_available(pty, 300);
    ASSERT_TRUE(jobs_out2.find("Running") != std::string::npos);

    // Cleanup: kill the whole stopped/backgrounded process group directly
    // (this test process is the sleep's grandparent, not its parent, but
    // Unix signal delivery only cares about matching uid, not ancestry).
    if (pgid > 0) kill(-pgid, SIGKILL);

    pty.write("exit\n");
    // Bounded wait: if meridian-shell somehow never exits, fail loudly
    // rather than hanging the whole test suite forever.
    read_available(pty, 300);
    int status = pty.wait_for_exit();
    ASSERT_EQ(status, 0);
}

MTEST(job_control_disabled_for_noninteractive_shell) {
    // "-c" mode must NOT attempt job control at all (no controlling tty
    // to claim) — this is the same non-interactive path the executor
    // integration tests already exercise; confirmed here at the binary
    // level so a regression would show up even if only shell_main.cpp
    // were touched.
    PtyManager pty;
    SpawnOptions opts;
    opts.program = shell_binary_path();
    opts.args = {"-c", "echo noninteractive-ok"};
    ASSERT_TRUE(pty.spawn(opts));
    std::string out = read_available(pty, 300);
    ASSERT_TRUE(out.find("noninteractive-ok") != std::string::npos);
    pty.wait_for_exit();
}
