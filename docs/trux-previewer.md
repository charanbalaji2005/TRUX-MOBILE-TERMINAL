# TRUX Universal File Previewer

TRUX includes a built-in, native Android universal file previewer that lets you view documents, media, source code, and archives directly from the terminal without leaving TRUX or launching third-party apps for supported formats.

---

## 1. Terminal Commands

From any TRUX terminal session, open files using:

```bash
# General usage
open <file>

# Alternative syntax
trux open <file>
xdg-open <file>
preview <file>
```

### Examples
```bash
# View PDF reports
open report.pdf

# Preview images
open photo.png
open diagram.webp

# Play video & audio
open video.mp4
open recording.wav

# Inspect text & source code
open script.py
open config.json
open dataset.csv
open notes.md

# Inspect archives without extracting
open bundle.zip
```

---

## 2. Supported Formats & Decoders

| Category | Formats Supported | Features |
|---|---|---|
| **PDF** | `.pdf` | Multi-page lazy rendering via `PdfRenderer`, page navigation, zoom in/out, fit percentage |
| **Images** | `.png`, `.jpg`, `.jpeg`, `.webp`, `.gif`, `.bmp`, `.heic`, `.heif`, `.avif` | Pinch-to-zoom, pan, double-tap zoom reset, image dimensions & size indicators |
| **Video** | `.mp4`, `.m4v`, `.webm`, `.mkv`, `.3gp`, `.mov` | Hardware-accelerated playback with Media3 ExoPlayer, seek bar, play/pause, mute |
| **Audio** | `.mp3`, `.wav`, `.aac`, `.m4a`, `.flac`, `.ogg`, `.opus` | Visual player with duration, seek controls, audio playback |
| **Source Code** | `.py`, `.js`, `.ts`, `.kt`, `.java`, `.cpp`, `.c`, `.rs`, `.go`, `.html`, `.css`, `.sql`, `.sh` | Monospace font, line numbers, syntax token highlighting, search in text, wrap toggle, clipboard copy |
| **Markdown** | `.md`, `.markdown` | Headings, lists, code blocks, blockquotes, raw/rendered mode toggle |
| **JSON** | `.json` | Formatted pretty-printing, key/value syntax coloring, search, copy |
| **CSV** | `.csv`, `.tsv` | Scrollable data table grid with headers, zebra striping, horizontal & vertical scrolling |
| **Archives** | `.zip`, `.tar`, `.tgz`, `.apk`, `.jar` | Inspect archive entries without extracting, file sizes, compression ratio, 1-tap extraction to directory |
| **Binary / Office** | `.docx`, `.xlsx`, `.pptx`, `.bin`, unknown | Metadata card (size, MIME, permissions, modified date, SHA-256), hex dump preview, and "Open with another app" fallback |

---

## 3. Architecture & Security

- **Sandbox Enforcement**: Opening `.sh` or binary executables previews the text/metadata and **never** executes the file. Executables must be invoked through the shell (`./program`).
- **Memory Efficiency**: Large text files stream in chunks; large images use bitmap downsampling; PDFs render pages on demand.
- **Privacy & Sharing**: Integrates with Android Sharesheet via `FileProvider` (`${applicationId}.fileprovider`), securely sharing files without exposing private root filesystem paths.
- **Overlay Navigation**: The viewer displays as a full-screen overlay dialog; dismissing it (`←` or Back) returns directly to the running shell session without disturbing the PTY.
