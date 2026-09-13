---
layout: default
title: "Security & Credentials"
category: "Developer"
---

# Security

Spec §82's requirements, checked against what's actually true of this
codebase — not asserted, checked:

| Requirement | Status |
|---|---|
| Not require root | **True.** Nothing in this repo calls `setuid`/checks for `geteuid()==0`/requires elevated privileges. `sudo` only appears as a pattern the risk classifier recognizes in *other people's* commands. |
| No telemetry | **True.** No analytics, crash reporting, or usage-tracking code exists anywhere in this repo. |
| No hidden network requests | **True, verifiably** — grep for it yourself: |

```bash
grep -rn "connect\|socket\|getaddrinfo\|CURLOPT\|http://\|https://" src/ --include="*.cpp" --include="*.hpp"
```

Run today, this returns a handful of matches — all noise, not network
code: the AST's `Connector`/`Connector::And` etc. (Meridian Shell's
`;`/`&&`/`||` sequencing concept, nothing to do with sockets), a doc
comment mentioning "provider connectivity," and a CLI help string
containing the word "connectivity." No `connect()`/`socket()`/
`getaddrinfo()` call or `http(s)://` URL used in an actual request
exists anywhere in `src/`. (`docs/providers.md` and
`ai_provider.hpp`'s comments do name real provider URLs — as
documentation of where a future implementation would point, never as
code that dials out.)

| Requirement | Status |
|---|---|
| No command history uploaded automatically | **True.** `Executor::history()` is purely in-memory/local; nothing serializes or transmits it. |
| No API keys exposed | **True, vacuously** — no API key is ever read, stored, or handled anywhere yet, since no provider exists. Once one does, it must go through `CredentialStore`, never a config file or log line. |
| No AI command silently executes | **True.** `AIController::analyze_command()` only ever returns text describing what it found — it has no code path that executes anything. There is no "auto-fix" or "auto-run" anywhere in `src/ai/`. |
| Secure credential storage when available | **Partial — see below.** |

## Credential storage (`src/security/credential_store.hpp`)

The spec asks for Linux Secret Service / desktop keyring storage when
available (§57). Checked directly in this environment:

```
$ pkg-config --exists libsecret-1 && echo available || echo unavailable
unavailable
$ find / -iname "libsecret*.h" 2>/dev/null
(nothing found)
```

The Secret Service **runtime** library is actually present on this
machine (`/usr/lib/x86_64-linux-gnu/libsecret-1.so.0`), but the
**development headers** (`libsecret-1-dev`) are not, and there's no
network access to install them. So nothing here has been compiled
against real Secret Service — that would mean writing code against an
API this build can't even syntax-check.

What exists instead: `FileCredentialStore`, an honestly-labeled
fallback — `backend_name()` literally returns `"file (0600) — NOT
OS-keyring-backed"` so a caller can't mistake it for the real thing.
Implementation details that were actually gotten right (tested, not
assumed):

- Files are created via `open()` with mode `0600` passed directly to
  the syscall, not created-then-`chmod()`'d — the latter leaves a real
  (if brief) window where the file exists with default, broader
  permissions before the `chmod` lands. `open()`'s mode argument can
  only have bits *cleared* by `umask`, never added, so requesting
  `0600` guarantees no group/other bits are ever set, with no window.
- `key_name` is sanitized (`[A-Za-z0-9_.-]` only, everything else
  becomes `_`) before being used to build a file path — a key name
  containing `/` or `..` can't escape the credentials directory. Tested
  directly: `tests/test_security.cpp`'s
  `credential_store_sanitizes_path_traversal_attempts` passes
  `"../../etc/passwd_like_name"` as a key and confirms both that
  retrieval still works *and* that the file that was actually written
  landed inside the sandboxed test directory, not wherever the raw
  traversal would have pointed.
- The actual on-disk file mode is verified with `stat()` in a test, not
  just assumed from reading the `open()` call — see
  `credential_store_file_has_owner_only_permissions`.

**Before this fallback ships credentials for real users**, it should
be replaced with a real Secret Service backend on a machine that has
`libsecret-1-dev` — the `CredentialStore` interface is designed so that
swap doesn't touch any calling code.

## Secret redaction (`src/ai/secret_redactor.hpp`)

Covered in detail in `docs/ai.md`. Summary for this document: pattern-
based (regex), catches common named formats (KEY=VALUE env vars,
Bearer tokens, AWS/GitHub/Slack/OpenAI-style key prefixes, PEM private
key blocks), does NOT do generic high-entropy-string detection. This is
a real safeguard, not a guarantee — the spec's own context-minimization
principle (§66: "default to minimum necessary context") matters more
than redaction as the primary defense, since redaction can only catch
patterns it knows about.

## Execution safety

Both Meridian Shell (`docs/shell.md`) and the PTY layer (`docs/pty.md`)
spawn processes via direct `fork()`+`execve()`/`execvp()` — never
`system()`, `popen()`, or building a string for `/bin/sh -c` on
user-controlled input. This is checkable the same way as the network
claim above:

```bash
grep -rn "system(\|popen(" src/ --include="*.cpp"
```

The only `system()` calls that exist anywhere in this repo are inside
**test files** (`tests/test_ai.cpp`, `tests/test_command_context.cpp`),
used to set up throwaway fixture directories/files with fixed,
hardcoded arguments — never on data derived from what's being tested.
No production code path (`src/`) uses `system()` or `popen()` anywhere.

## What's NOT yet addressed

- No sandboxing/seccomp for spawned processes — a spawned shell command
  or (eventually) an AI-generated command has the same privileges as
  the user running Meridian, same as any terminal.
- No code signing / release-build hardening — out of scope until
  there's a packaging story (`docs/status.md`'s M20).
- Job control's signal-disposition changes (`docs/shell.md`) are
  process-global by nature (that's what POSIX job control *is*) — see
  the comment on `Shell::enable_job_control()` for why it's an explicit
  opt-in rather than automatic, specifically so it can't leak into
  contexts (like a shared test process) that didn't ask for it.
