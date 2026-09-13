# TRUX AI Terminal Assistant

TRUX integrates a native, terminal-configurable AI assistant engine directly into the Android shell environment. The assistant is accessed via `trux ai` or `trux-ai` and does not require a graphical settings UI.

---

## Architecture Overview

```
Terminal Shell (PTY)
       │
       ▼
 $ trux ai ...
       │
       ▼
 trux-ai Engine ($PREFIX/bin/trux-ai)
   ├── Secret Redactor (API_KEY, tokens, private keys)
   ├── Config Loader ($PREFIX/etc/trux/ai.json)
   ├── Secure Credential Store ($PREFIX/etc/trux/ai.secret [600])
   └── Curl / HTTP Client (Off UI thread, Streaming / SSE)
       │
       ▼
 AI Provider Endpoint (OpenAI, Anthropic, Ollama, Local Host)
```

---

## Configuration Commands

### 1. Interactive Setup
Run `trux ai setup` to configure your AI provider directly from the command line:
```bash
$ trux ai setup
======================================================
                   TRUX AI SETUP                      
======================================================
Provider:
  1. OpenAI-compatible (OpenAI, Groq, Mistral, Ollama, Local)
  2. Anthropic-compatible
  3. Custom / Local endpoint (e.g. http://127.0.0.1:8000/v1)
Select provider [1-3] (default 1): 1
API endpoint (default https://api.openai.com/v1): https://api.openai.com/v1
API key (input hidden): ********
Model (default gpt-4o-mini): gpt-4o-mini
Streaming enabled? [Y/n] (default yes): Y
Timeout in seconds (default 60): 60
Save configuration? [Y/n]: Y

✓ AI configuration saved securely to $PREFIX/etc/trux/ai.json
✓ Sensitive API credentials protected with mode 600
```

> **Security Note**: Input for the API key is completely masked using terminal `stty -echo`. The key is never echoed on screen, written to `.bash_history`, or emitted to Android Logcat.

### 2. Status
Verify your current AI configuration:
```bash
$ trux ai status
======================================================
                   TRUX AI STATUS                     
======================================================
Provider : openai-compatible
Endpoint : https://api.openai.com/v1
Model    : gpt-4o-mini
API Key  : configured (securely protected)
Status   : ready
Streaming: true
Timeout  : 60s
Config   : /data/data/com.meridian.shell/files/usr/etc/trux/ai.json
======================================================
```
*The raw API key is never printed in status outputs.*

### 3. Connection Test
Verify API credentials and connectivity:
```bash
$ trux ai test
Testing AI endpoint...
Connecting to https://api.openai.com/v1...
Authenticating...
Model: gpt-4o-mini
Streaming: true

AI connection successful.
```

If authentication or connectivity fails, clean actionable diagnostics are returned without exposing secrets:
```bash
AI connection failed.
HTTP status: 401
Reason: Authentication failed (Invalid API key).
Suggested action: Check API key with 'trux ai setup'.
```

### 4. Model Listing
Fetch available models from the configured endpoint:
```bash
$ trux ai models
=== Available Models ===
  • gpt-4o
  • gpt-4o-mini
  • text-embedding-3-small
```

### 5. Configuration Reset
Completely remove stored configurations and keys:
```bash
$ trux ai reset
Delete TRUX AI configuration and stored credentials? [y/N]: y
TRUX AI configuration and credentials successfully deleted.
```

---

## Query Modes

### One-Shot Mode
Ask questions directly from your prompt:
```bash
$ trux ai "Explain how Linux file permissions work in octal"
```

### Interactive Mode
Start an interactive chat session:
```bash
$ trux ai
TRUX AI interactive mode (gpt-4o-mini).
Type /exit to leave, /help for options.

> explain chmod 755
AI:
chmod 755 sets permissions where:
- Owner (7 = rwx): read, write, execute
- Group (5 = r-x): read, execute
- Others (5 = r-x): read, execute

> /exit
Exiting TRUX AI.
```

### Command Generation (`--command`)
Request a shell command for a task. TRUX prompts with a mandatory confirmation defaulting to `N`:
```bash
$ trux ai --command "Find all files larger than 100MB in current folder"

find . -type f -size +100M

Execute this command? [y/N]: 
```
*Commands are never automatically executed.*

### Context-Aware Queries
Provide bounded terminal context to AI:
```bash
# Analyze last 20 history lines
trux ai --context-last 20 "Why did my previous compile fail?"

# Analyze copied error from clipboard
trux ai --context-selection "Explain this error"

# Quick command explanation
trux ai explain "tar -czvf archive.tar.gz /path"
trux ai explain-last
```

---

## Local & Offline AI
TRUX supports on-device or local network models (such as Ollama, vLLM, or LM Studio running on a local workstation or Termux environment):
```bash
$ trux ai setup
Select provider: 3 (Custom)
Endpoint: http://192.168.1.100:11434/v1
API key: (press Enter for local servers without auth)
Model: llama3.2
```

---

## Signal Handling & Cancellation
Pressing `Ctrl+C` during an active AI request immediately terminates the network request without terminating your shell session.
