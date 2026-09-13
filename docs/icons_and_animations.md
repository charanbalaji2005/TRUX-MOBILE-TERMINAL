# Meridian Terminal — Icons & Animation Engine Architecture

Meridian Terminal features a dual-subsystem architecture separating **static terminal glyph icons** (via an extensible Nerd Font v3 registry with multi-tier fallbacks) from **true native raster animations** (via an event-driven animation engine supporting GIF/APNG/WebP, Kitty Graphics Protocol, frame composition, and memory limits).

---

## 1. Static Glyph Icon Subsystem

Meridian's icon subsystem (`meridian::icons::IconRegistry`) provides crisp, single-cell aligned icons for over 100+ file extensions, directories, programming languages, Git states, and dev tools.

### 1.1 Font Support & Fallback Tiers

Meridian uses a 3-tier fallback engine:

| Tier | Description | Example Directory | Example Clean Git |
|---|---|---|---|
| **Tier 1: `NERD_FONT`** | 10,000+ Nerd Font v3 glyphs | ` src/` | `✔ clean` |
| **Tier 2: `UNICODE`** | Safe BMP Unicode emoji / symbols | `📁 src/` | `✓ clean` |
| **Tier 3: `ASCII`** | High-density clean ASCII markers | `[src]` | `ok clean` |

> **Zero Replacement Guarantee**: Meridian never outputs unprintable boxes (`□`, ``, `?`) when an icon is unsupported.

### 1.2 Configuring the Icon Tier

You can explicitly force an icon tier via environment variables:

```bash
# Force full Nerd Font icons
export MERIDIAN_ICON_TIER=NERD_FONT

# Force safe Unicode emoji
export MERIDIAN_ICON_TIER=UNICODE

# Force pure ASCII fallback
export MERIDIAN_ICON_TIER=ASCII
```

### 1.3 User Custom Icon Mappings (`~/.config/meridian/icons.toml`)

You can define custom icon mappings in `~/.config/meridian/icons.toml`:

```toml
[extensions]
".cpp" = ""
".py" = ""
".rs" = ""
".custom" = "󰡨"

[filenames]
"README.md" = ""
"Dockerfile" = "󰡨"

[directories]
"src" = "󰙲"
"build" = ""
"assets" = ""

[commands]
"docker" = "󰡨"
"git" = ""
```

---

## 2. Raster Animation Engine

Meridian does not treat animated pictures as font icons. Instead, raster animations are decoded directly into 32-bit RGBA pixel frames and composited using Meridian's native graphics and GPU pipeline.

### 2.1 Supported Formats
- **GIF87a / GIF89a**: Multi-frame GIF with centisecond delay headers and disposal modes.
- **APNG**: Animated Portable Network Graphics.
- **Animated WebP**: Multi-frame WebP container with alpha transparency.
- **Kitty Graphics Protocol**: Direct frame streaming via APC escape codes (`ESC _ G ... ESC \`).

### 2.2 Animated Image CLI (`pic`)

The `pic` command provides full playback control:

```bash
# Display an image
pic image.png

# Display and play an animated GIF
pic animation.gif --animate

# Loop an animation indefinitely
pic animation.gif --loop

# Loop an animation 3 times
pic animation.gif --loop 3

# Set custom playback frame rate (e.g. 30 FPS)
pic animation.gif --fps 30

# Seek to a specific frame
pic animation.gif --frame 10

# Pause or stop playback
pic animation.gif --pause
pic animation.gif --stop

# List all active GPU layers and animation objects
pic --list

# Clear all active image and animation layers
pic --clear
```

### 2.3 Performance & Safety Limits

To protect against malicious or oversized animations (e.g. decompression bombs), Meridian enforces strict bounding limits:

- **Memory Limit**: Default `256 MB` total decoded frame memory. Oldest stopped animations are evicted under memory pressure.
- **Max Canvas Dimensions**: `16384 x 16384` px.
- **Max Frames**: `2048` frames per animation.
- **Max Duration**: `3600` seconds (1 hour).

### 2.4 Kitty Graphics Protocol Animation Support

Meridian fully supports the Kitty Graphics Protocol APC stream:

```
\033_Ga=a,s=800,v=600,c=0,r=0,z=100;<base64_payload>\033\
```

- `a=a`: Animation frame transmission.
- `s=w, v=h`: Frame dimensions in pixels.
- `c=index`: Target frame index.
- `z=delay`: Frame delay in milliseconds.
- `r=loops`: Loop count (0 = infinite).

