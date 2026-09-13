---
layout: default
title: "Installation & Build"
category: "Getting Started"
---

# Build

## Path 1: `make` — actually verified in this repo's build environment

This is the build that was run, repeatedly, while writing this code. It
needs only `g++` (C++20) and `make` — no `cmake`, no `Qt6`.

```bash
make all      # builds meridian_tests, meridian_demo, meridian-shell
make test     # builds (if needed) and runs the test suite
make demo     # builds (if needed) and runs the headless PTY+ANSI demo
make shell    # builds (if needed) and drops you into meridian-shell
make clean
```

Expected `make test` output ends with:

```
60 tests, 139 assertions, 0 failed test(s), 0 failed assertion(s)
```

### Sanitizer build (extra confidence, no valgrind needed)

```bash
g++ -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  src/core/vt/screen_buffer.cpp src/core/vt/ansi_parser.cpp src/core/pty/pty_manager.cpp \
  src/shell/lexer.cpp src/shell/parser.cpp src/shell/executor.cpp src/shell/builtins.cpp \
  tests/test_main.cpp tests/test_screen_buffer.cpp tests/test_ansi_parser.cpp \
  tests/test_pty_manager.cpp tests/test_shell_lexer.cpp tests/test_shell_parser.cpp \
  tests/test_shell_executor.cpp \
  -lutil -o /tmp/meridian_tests_asan
/tmp/meridian_tests_asan
```

## Path 2: `cmake` — written carefully, NOT verified here

`cmake` isn't installed in the environment this repo was built in (no
network to install it), so `CMakeLists.txt` has never actually been
configured or built. It's believed correct — it mirrors the Makefile's
target list — but "believed correct" and "verified" are different
claims, and `docs/status.md` is explicit about which one this is.

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

If this doesn't work cleanly on the first try, that's expected and not
a sign anything else in the repo is suspect — please open an issue (or
just fix it) and it'll get folded back in.

## Adding Qt6 later

`CMakeLists.txt` has a commented-out block for finding Qt6 and pulling
in `src/gui` once that directory exists. Nothing needs to change in
`meridian-core` or `meridian-shell-lib` to support it — that separation
was deliberate from the start (see `docs/architecture.md`).
