# TRUX Security Architecture & Threat Model

TRUX is built strictly within Android's sandboxed application security model. It does not compromise device integrity, attempt kernel privilege escalations, or leak private credentials.

---

## 1. Android Sandbox & Privilege Model

- **No Exploits or Root Escalation**: TRUX does not include, attempt, or distribute any root exploits. It does not bypass Android's SELinux policy or Linux kernel boundaries.
- **Userspace Containment**: When switching to `root` via `su` or executing via `sudo`, privileges apply exclusively within the TRUX Linux userspace environment (managing TRUX packages, `/etc` databases, and local users).
- **Application Isolation**: Other Android applications' private data directories (`/data/data/<other-app>`) remain completely protected and inaccessible to TRUX, respecting Android's UID sandboxing.

---

## 2. Sensitive Credential Protection

### Secret Storage
- API keys, access tokens, and passwords are never stored in plaintext within world-readable paths.
- The AI credential file (`$PREFIX/etc/trux/ai.secret`) and password shadow database (`$PREFIX/etc/shadow`) are enforced with filesystem permission mode `600` (read/write restricted exclusively to the owning UID).

### Terminal & Scrollback Privacy
- Input for sensitive credentials (`trux ai setup`, `passwd`) uses `stty -echo` to completely prevent characters from rendering on screen.
- Secret input is blocked from:
  - Terminal VT scrollback buffers
  - Shell command history (`.bash_history`)
  - Android Logcat logs
  - Crash reports

### Automatic Secret Redaction for AI Context
Before any terminal output or clipboard context is transmitted to an AI endpoint, the text is filtered through an automated redaction engine:

| Detected Secret Pattern | Redacted Output |
|-------------------------|-----------------|
| `API_KEY=...` | `API_KEY: [REDACTED]` |
| `Authorization: Bearer ...` | `Authorization: [REDACTED]` |
| `sk-...` (OpenAI / Service Keys) | `[REDACTED_API_KEY]` |
| `ghp_...` (GitHub Personal Tokens) | `[REDACTED_GH_TOKEN]` |
| `-----BEGIN PRIVATE KEY-----` | `[REDACTED_PRIVATE_KEY]` |

---

## 3. Network Security & Transport

- **TLS / HTTPS by Default**: All remote connections (AI endpoints, package repositories, curl downloads) default to HTTPS with strict TLS verification.
- **Plaintext Warning**: If a user enters an unencrypted `http://` endpoint (e.g. for local LAN servers), TRUX prints an explicit security notice.
- **No Autonomous Execution**: When using `trux ai --command`, generated shell commands are displayed clearly and require explicit interactive confirmation (`[y/N]`, defaulting to `N`). TRUX never executes commands received from AI without user approval.
