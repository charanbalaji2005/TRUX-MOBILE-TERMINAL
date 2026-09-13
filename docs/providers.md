---
layout: default
title: "AI Providers Configuration"
category: "Developer"
---

# AI providers — planned endpoints, not implemented

See `docs/ai.md` for why nothing here has been built yet. This file
just records the concrete API details for whoever implements each
provider, so that work starts from verified facts rather than
re-researching them.

## Groq Cloud

- Base URL: `https://api.groq.com/openai/v1` — OpenAI-compatible, per
  Groq's own documentation.
- Auth: `Authorization: Bearer <GROQ_API_KEY>`.
- Model list should be fetched from the provider, not hardcoded (Groq's
  available models change over time).

## Google Gemini

- Has its own request/response shape — do not assume it's
  OpenAI-compatible like Groq. Needs its own adapter, not a shared
  OpenAI-style client with a different base URL.
- Auth and endpoint details: check Gemini's current API docs at
  implementation time rather than trusting anything cached here, since
  this file was written without network access to verify against the
  live docs.

## Ollama Cloud

- Base URL: `https://ollama.com/api`.
- Auth: `Authorization: Bearer <OLLAMA_API_KEY>`.
- Model list should be fetched from the provider's model-listing
  endpoint, not hardcoded.

## Shared requirements (apply to all three)

- Never hardcode API keys; never commit them. Prefer the Linux
  Secret Service / desktop keyring for storage; configuration files
  hold provider + model selection only, never the key itself.
- Normalize errors to the categories in `ai_provider.hpp`'s intended
  shape: authentication, rate limit, timeout, network, model
  unavailable, invalid request, provider unavailable. Never let a raw
  API key or `Authorization` header value reach a log line.
- Async/non-blocking — the terminal must stay responsive while a
  request is in flight, with a configurable timeout.
- Redact likely secrets (API keys, passwords, tokens, SSH private keys)
  from any context sent to a provider — see `docs/ai.md`.

## Implementation order recommendation

Groq first (simplest — genuinely OpenAI-compatible, so existing HTTP/
JSON client patterns apply directly), then Gemini, then Ollama Cloud.
Each one should have its own real integration tests against the live
API before starting the next — see `docs/status.md`'s "if you're
picking this up next" section.
