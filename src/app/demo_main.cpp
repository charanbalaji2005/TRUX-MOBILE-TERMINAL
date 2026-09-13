// src/app/demo_main.cpp
//
// Headless integration demo. Spawns a REAL /bin/bash process on a REAL
// Linux PTY, captures its raw output, and feeds it through the same
// ANSI parser + screen buffer a GUI frontend would use for rendering.
// This is the concrete proof that the PTY layer and the VT engine work
// together end-to-end — no display server required to verify it.

#include "../core/pty/pty_manager.hpp"
#include "../core/vt/ansi_parser.hpp"
#include "../core/vt/screen_buffer.hpp"

#include <iostream>
#include <string>

using namespace meridian;

int main() {
    std::cout << "=== Meridian Terminal Core -- headless PTY + ANSI demo ===\n\n";

    pty::PtyManager pty;
    pty::SpawnOptions opts;
    opts.program = "/bin/bash";
    opts.args = {
        "--norc", "--noprofile", "-c",
        "printf 'Hello \\033[1mBOLD\\033[0m and \\033[31mRED\\033[0m\\n'; "
        "printf 'Second line, \\033[4munderlined\\033[0m.\\n'"
    };
    opts.rows = 24;
    opts.cols = 80;

    if (!pty.spawn(opts)) {
        std::cerr << "spawn failed: " << pty.last_error() << "\n";
        return 1;
    }
    std::cout << "spawned real child process, pid=" << pty.child_pid() << "\n";

    vt::ScreenBuffer screen(opts.rows, opts.cols);
    vt::AnsiParser parser(screen);

    std::string raw;
    char buf[4096];
    ssize_t n;
    while ((n = pty.read(buf, sizeof(buf))) > 0) {
        raw.append(buf, static_cast<std::size_t>(n));
        parser.feed(buf, static_cast<std::size_t>(n));
    }

    int status = pty.wait_for_exit();
    std::cout << "child exited with status " << status << "\n\n";

    std::cout << "--- raw bytes received from the PTY (" << raw.size() << " bytes) ---\n";
    for (unsigned char c : raw) {
        if (c == '\x1b') std::cout << "\\e";
        else if (c == '\r') std::cout << "\\r";
        else if (c == '\n') std::cout << "\\n\n";
        else std::cout << c;
    }
    std::cout << "\n\n";

    std::cout << "--- decoded screen, after real ANSI parsing ---\n";
    std::cout << screen.dump_text() << "\n\n";

    std::cout << "--- proving SGR attribute decoding actually happened ---\n";
    // "Hello " is 6 characters (columns 0-5), so column 6 is the 'B' of BOLD.
    const auto& bold_cell = screen.cell_at(0, 6);
    std::cout << "cell(0,6) = '" << static_cast<char>(bold_cell.codepoint)
              << "'  bold=" << (bold_cell.attrs.bold ? "true" : "false") << "\n";

    return status;
}
