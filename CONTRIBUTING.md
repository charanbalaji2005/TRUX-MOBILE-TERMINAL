# Contributing

The rule this whole repo follows, from the original spec: **never claim
something works without testing it.** In practice:

1. Read `docs/status.md` first — it's the source of truth for what's
   real versus planned, and it's kept honest on purpose. If you fix
   something or implement something new, update it in the same change.
2. Before writing code for a milestone, check whether it needs
   something this build environment doesn't have (network, Qt6, a
   display server, `cmake`). If so, that's fine — build it on a machine
   that does have those things, but hold yourself to the same standard:
   compile it, run it, test it against something real (a real API
   response, a real rendered window) before considering it done.
3. `make test` (or `ctest`, if you're on the `cmake` path) before every
   commit. All 60+ existing tests should stay green; add new ones for
   new behavior rather than only manually eyeballing it.
4. Prefer integration tests that exercise real processes/files over
   mocks where practical — see `tests/test_shell_executor.cpp` and
   `tests/test_pty_manager.cpp` for the pattern: real `fork`/`exec`,
   verified by reading back what the real child process actually did.
5. If a sanitizer build is available to you (`-fsanitize=address,
   undefined`), run the suite through it before merging anything that
   touches memory, fork/exec, or file descriptors.
6. Keep the "do not shell out" rule intact: no `system()`, `popen()`,
   or building a command string for `/bin/sh -c` on user-controlled
   input, in the terminal core, the shell, or (once implemented) the AI
   layer's command execution path.
7. Small, real, verified increments over large, speculative ones. If a
   change is big enough that you can't describe exactly how you tested
   it, it's probably too big for one commit.

## Milestone order

`docs/status.md`'s "if you're picking this up next" section has the
current recommended order (cmake verification -> one AI provider,
fully -> minimal Qt6 window). Follow it unless you have a good reason
not to, and if you deviate, say why in `docs/status.md`.
