// src/app/shell_main.cpp
//
// Standalone entry point for Meridian Shell. Runs as an interactive
// shell process with full POSIX job control, pipeline execution, and
// line editor.

#include "../shell/shell.hpp"

#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "-c") {
        meridian::shell::Shell shell(/*interactive=*/false);
        return shell.run_command(argv[2], std::cerr);
    }

    bool interactive = isatty(STDIN_FILENO);
    meridian::shell::Shell shell(interactive);
    if (interactive) shell.enable_job_control();
    return shell.run_interactive(std::cin, std::cout, std::cerr);
}
