---
layout: default
title: "AI Architecture"
category: "Developer"
---

# Meridian AI

**Two different statuses in this one system — read carefully, because
the split matters:**

| Piece | Status |
|---|---|
| Local command analyzer (typo detection) | **Real, implemented, tested** |
| Risk classifier (LOW/MEDIUM/HIGH/CRITICAL) | **Real, implemented, tested** |
| Secret redactor | **Real, implemented, tested** |
| `meridian ai on/off/status/detect/privacy/analyze/redact/test` CLI | **Real, implemented, tested** |
| Config persistence (`~/.config/meridian/ai.toml`) | **Real, implemented, tested** |
| Groq / Gemini / Ollama Cloud providers | **Not implemented — see below** |
| `meridian ai explain` (real natural-language explanation) | **Not implemented — reports so honestly** |
| Qt6 AI popup (Ctrl+Space) | **Not implemented — needs Qt6** |

## What's real (`src/ai/`)

- **`secret_redactor.{hpp,cpp}`** — regex-based redaction of common
  secret shapes: `KEY=value`-style env vars (any name containing
  key/token/secret/password), `Authorization: Bearer <token>`, AWS
  access key IDs, GitHub tokens (classic + fine-grained), Slack tokens,
  OpenAI/Anthropic-style `sk-...` keys, and whole PEM private-key
  blocks. Tested against realistic examples of each — see
  `tests/test_ai.cpp`. Known gap: no generic high-entropy-string
  detection, so an unusual secret format that doesn't match any of
  these specific patterns won't be caught. This is pattern matching,
  not a guarantee — never the only safeguard.

- **`risk_classifier.{hpp,cpp}`** — classifies a raw command line into
  LOW/MEDIUM/HIGH/CRITICAL (spec §41/§73) via ordered pattern matching:
  CRITICAL (`rm -rf /`, fork bombs, `dd`/`mkfs` onto a disk device,
  recursive `chmod 777 /`), HIGH (`sudo`, `rm -rf` anywhere, `shutdown`/
  `reboot`, `kill -9`, piping a downloaded script into a shell), MEDIUM
  (plain `rm`/`chmod`/`chown`, global package installs, force-push).
  This is heuristic pattern matching over the command text, not real
  semantic understanding of what a command will do — documented as such
  rather than oversold.

- **`command_analyzer.{hpp,cpp}`** — spec §38's "local analyzer": scans
  real `$PATH` directories (`opendir`/`readdir`/`access`) for
  executables, and when a typed command name isn't a known shell
  builtin or PATH executable, suggests the closest match using Optimal
  String Alignment distance (Levenshtein plus adjacent-transposition
  support — plain Levenshtein charges 2 for "gerp" -> "grep", which is
  the single most common typo shape and would sit right at the edge of
  the suggestion threshold; this was a real bug caught by testing
  against the actual PATH on the build machine, not a hypothetical).
  **Honestly scoped**: this catches misspelled command *names*
  ("gerp" -> "grep"). It does NOT catch subcommand-level mistakes like
  "npm instal express" -> "npm install express" — recognizing that
  "instal" isn't a valid npm subcommand needs either an actual LLM or a
  hand-built dictionary of every program's subcommands, neither of
  which a generic PATH scanner can do. Verified directly: `npm` really
  is on this machine's PATH, so `meridian ai analyze "npm instal
  express"` correctly reports "no issues detected" rather than silently
  failing to work — that's the documented scope boundary, not a bug.

- **`config.{hpp,cpp}`** — a deliberately minimal flat `key = "value"`
  store, NOT a full TOML parser (the spec's example configs look like
  TOML; implementing real TOML — nested tables, arrays, typed values —
  is its own scope, and this doesn't pretend to). Covers exactly what
  the AI state needs: enabled/detect/privacy/provider/model.

- **`ai_controller.{hpp,cpp}`** — ties the above together into the
  state machine spec §36/§37/§59/§61/§64 describe, plus
  `analyze_command()` (typo + risk, returns `""` when there's nothing
  to flag — matching "don't show `✦`/`⚠`/`🔒` unless something's
  actually notable"). `explain_command()` always reports plainly that
  no provider is implemented rather than fabricating a plausible-
  looking canned explanation — faking that would be strictly worse than
  saying nothing, since a wrong "explanation" is actively misleading.

- **`src/app/meridian_main.cpp`** — the `meridian ai ...` CLI. Try it:

  ```bash
  ./build/meridian ai on
  ./build/meridian ai analyze "gerp foo"        # -> possible correction: grep foo
  ./build/meridian ai analyze "sudo rm -rf /"   # -> risk: CRITICAL - ...
  ./build/meridian ai redact 'API_KEY=sk-abc123 Authorization: Bearer xyz'
  ./build/meridian ai test                      # -> honest "NOT IMPLEMENTED" per provider
  ./build/meridian ai status
  ```

  `MERIDIAN_CONFIG_HOME` overrides the config directory (tests use
  this to avoid touching a real `~/.config`).

## What's still not implemented, and why

**Groq, Gemini, Ollama Cloud.** `src/ai/ai_provider.hpp` still defines
only the interface (spec §48's contract) — implementing any of them for
real means making real HTTPS calls and checking real responses, and
this environment has no network access. See `docs/status.md` for the
full reasoning; the short version is the same one that's applied
everywhere else in this project: don't ship networking code that's
never actually talked to a server.

**`meridian ai explain`.** Needs real language generation to produce a
correct explanation of an arbitrary command's purpose/flags/risks —
that's what a provider is for. Without one, it says so.

**Qt6 AI popup (Ctrl+Space, spec §42-43).** Needs Qt6, which isn't
available here. See `docs/gui.md`.

## Concrete next step

Same as before: pick Groq (simplest, OpenAI-compatible), implement
`AIProvider` for it against the real API with real integration tests,
then route `AIController::explain_command()` through it when a provider
is configured and reachable — falling back to today's honest "no
provider" message when it isn't, rather than replacing one failure mode
with another.
