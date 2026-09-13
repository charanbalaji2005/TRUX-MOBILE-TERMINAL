# TRUX Command Support Specification

This document details the exact status of commands supported in the TRUX userspace environment.

Status definitions:
- **Implemented**: Fully functional native executable or utility script in the TRUX sandbox.
- **Partially Implemented**: Supported with limitations or relies on host Android system utilities.
- **Unavailable**: Requires root, hardware capabilities, or toolchain not yet ported to target ABI.

---

## 1. File Commands

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `pwd` | Implemented | `toybox` / built-in | `toybox` | POSIX print working directory |
| `ls` | Implemented | `toybox` / built-in | `toybox` | Supports `-la`, `-lh`, `-F`, colorized |
| `cd` | Implemented | shell built-in | `bash`/`sh` | Standard directory navigation |
| `cp` | Implemented | `toybox` | `toybox` | Supports recursive `-r`, `-p`, `-f` |
| `mv` | Implemented | `toybox` | `toybox` | File & directory renaming and movement |
| `rm` | Implemented | `toybox` | `toybox` | Supports `-rf` |
| `mkdir` | Implemented | `toybox` | `toybox` | Supports `-p` |
| `rmdir` | Implemented | `toybox` | `toybox` | Empty directory removal |
| `touch` | Implemented | `toybox` | `toybox` | Creates empty file or updates timestamp |
| `ln` | Implemented | `toybox` | `toybox` | Supports hardlinks and symbolic links `-s` |
| `readlink`| Implemented | `toybox` | `toybox` | Resolves symlink target |
| `realpath`| Implemented | `toybox` | `toybox` | Resolves canonical absolute paths |
| `basename`| Implemented | `toybox` | `toybox` | Extracts file component |
| `dirname` | Implemented | `toybox` | `toybox` | Extracts path prefix |
| `cat` | Implemented | `toybox` | `toybox` | Standard input / file concatenate |
| `tac` | Implemented | `toybox` | `toybox` | Reverse line concatenation |
| `head` | Implemented | `toybox` | `toybox` | Top N lines |
| `tail` | Implemented | `toybox` | `toybox` | Bottom N lines |
| `less` | Implemented | `toybox` | `toybox` | Pager utility |
| `more` | Implemented | `toybox` | `toybox` | Pager utility |
| `clear` | Implemented | `$PREFIX/bin/clear` | `trux-core` | ANSI screen clearance |
| `echo` | Implemented | shell built-in | `bash`/`sh` | Text printing |
| `printf` | Implemented | `toybox` / built-in | `toybox` | Formatted output |
| `yes` | Implemented | `toybox` | `toybox` | String generator |
| `true` | Implemented | `toybox` / built-in | `toybox` | Returns status 0 |
| `false` | Implemented | `toybox` / built-in | `toybox` | Returns status 1 |
| `sleep` | Implemented | `toybox` | `toybox` | Delay execution |

---

## 2. File Information & Search

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `stat` | Implemented | `toybox` | `toybox` | File inode and permission info |
| `file` | Implemented | `toybox` | `toybox` | File type detection |
| `du` | Implemented | `toybox` | `toybox` | Disk usage calculation |
| `df` | Implemented | `toybox` | `toybox` | Filesystem free space |
| `find` | Implemented | `toybox` | `toybox` | Recursive file search |
| `which` | Implemented | `$PREFIX/bin/which` | `trux-core` | Searches $PATH for executables |
| `whereis` | Implemented | `toybox` | `toybox` | Binary and source locator |
| `type` | Implemented | shell built-in | `bash`/`sh` | Command type inspection |
| `tree` | Partially Implemented | `$PREFIX/bin/trux files` | `trux-core` | Directory hierarchy inspector |
| `locate` | Unavailable | - | - | Requires updatedb background daemon |

---

## 3. Permissions & Identity

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `chmod` | Implemented | `toybox` | `toybox` | Change file mode bits in app sandbox |
| `chown` | Partially Implemented | `toybox` | `toybox` | Restricted to app UID on non-root |
| `umask` | Implemented | shell built-in | `bash`/`sh` | File creation mask |
| `id` | Implemented | `toybox` | `toybox` | Displays app UID, GID, and groups |
| `whoami` | Implemented | `toybox` | `toybox` | Prints app username (`u0_a...`) |
| `groups` | Implemented | `toybox` | `toybox` | Lists current supplemental groups |

---

## 4. Text Processing

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `grep` | Implemented | `toybox` | `toybox` | Full regular expression matching |
| `egrep` | Implemented | `toybox` | `toybox` | Extended regex matching |
| `fgrep` | Implemented | `toybox` | `toybox` | Fixed string matching |
| `sed` | Implemented | `toybox` | `toybox` | Stream editor |
| `awk` | Implemented | `toybox` | `toybox` | Pattern scanning and processing |
| `cut` | Implemented | `toybox` | `toybox` | Section extraction |
| `sort` | Implemented | `toybox` | `toybox` | Line sorting |
| `uniq` | Implemented | `toybox` | `toybox` | Duplicate elimination |
| `tr` | Implemented | `toybox` | `toybox` | Character translation |
| `wc` | Implemented | `toybox` | `toybox` | Word, line, byte counting |
| `tee` | Implemented | `toybox` | `toybox` | Split output to file and stdout |
| `xargs` | Implemented | `toybox` | `toybox` | Argument list builder |
| `diff` | Implemented | `toybox` | `toybox` | File difference comparison |
| `cmp` | Implemented | `toybox` | `toybox` | Byte-by-byte comparison |
| `paste` | Implemented | `toybox` | `toybox` | Merge lines of files |
| `join` | Implemented | `toybox` | `toybox` | Relational file join |
| `split` | Implemented | `toybox` | `toybox` | Split file into pieces |
| `strings` | Implemented | `toybox` | `toybox` | Printable character extraction |
| `fold` | Implemented | `toybox` | `toybox` | Wrap lines to width |
| `column` | Implemented | `toybox` | `toybox` | Column formatting |

---

## 5. Archives & Compression

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `tar` | Implemented | `toybox` | `toybox` | Archive read/write (`tar -xf`, `tar -czf`) |
| `gzip` / `gunzip` | Implemented | `toybox` | `toybox` | Compression and decompression |
| `zcat` | Implemented | `toybox` | `toybox` | Gzip stdout streaming |
| `zip` / `unzip` | Implemented | `toybox` | `toybox` | Standard ZIP archive handling |
| `bzip2` / `bunzip2` | Implemented | `toybox` | `toybox` | Bzip2 compression |
| `xz` / `unxz` | Implemented | `toybox` | `toybox` | XZ compression |
| `zstd` | Partially Implemented | - | `pkg install zstd` | Installable package |

---

## 6. Process Management & Job Control

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `ps` | Implemented | `toybox` | `toybox` | Process listing |
| `top` | Implemented | `toybox` | `toybox` | Real-time process monitor |
| `kill` | Implemented | `toybox` / built-in | `toybox` | Signal dispatch (SIGINT, SIGTERM, etc.) |
| `killall` | Implemented | `toybox` | `toybox` | Kill by name |
| `pgrep` / `pkill` | Implemented | `toybox` | `toybox` | Pattern match process kill |
| `jobs` | Implemented | shell built-in | `bash`/`sh` | Shell job tracking |
| `bg` / `fg` | Implemented | shell built-in | `bash`/`sh` | Foreground / background control |
| `wait` | Implemented | shell built-in | `bash`/`sh` | Wait for child processes |
| `nohup` | Implemented | `toybox` | `toybox` | Immune to hangups |
| `timeout` | Implemented | `toybox` | `toybox` | Execute command with time limit |
| `watch` | Implemented | `toybox` | `toybox` | Periodic command execution |

---

## 7. Shell Capabilities

| Feature | Status | Provider | Notes |
|---|---|---|---|
| `bash` | Implemented | `$PREFIX/bin/bash` | Full GNU bash interactive & script engine |
| `sh` | Implemented | `$PREFIX/bin/sh` | POSIX shell compliance |
| Pipes (`\|`) | Implemented | Kernel PTY | E.g. `echo hello \| grep hello` |
| Redirections (`>`, `>>`, `<`) | Implemented | Kernel VFS | Read/write stream redirection |
| Variables (`$VAR`) | Implemented | Shell engine | Shell & environment variables |
| Command Substitution (`$(cmd)`) | Implemented | Shell engine | Dynamic output nesting |
| Job Control (Ctrl+C, Ctrl+Z) | Implemented | PTY & Signal Handler | Real SIGINT and job suspension |

---

## 8. Network Utilities

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `curl` | Implemented | `$PREFIX/bin/curl` | `pkg` / Android | Full HTTP/HTTPS web transfer |
| `wget` | Implemented | `$PREFIX/bin/wget` | `toybox` / `pkg` | File downloader |
| `ping` | Partially Implemented | `/system/bin/ping` | Android system | ICMP ping requires OS network permission |
| `ip` | Implemented | `/system/bin/ip` | Android system | Interface addresses and routing |
| `nc` / `netcat` | Implemented | `toybox` | `toybox` | Network socket connection & testing |
| `ssh` | Partially Implemented | `$PREFIX/bin/ssh` | `pkg install openssh` | Requires package installation |
| `scp` / `sftp` | Partially Implemented | `$PREFIX/bin/scp` | `pkg install openssh` | Remote file transfer |
| `openssl` | Partially Implemented | - | `pkg install openssl` | Cryptographic toolkit |
| `nslookup` / `dig` | Partially Implemented | - | `pkg install dnsutils` | DNS queries |

---

## 9. Developer Tools, Python & Git

| Command | Status | Binary Path | Package / Provider | Notes |
|---|---|---|---|---|
| `git` | Implemented | `$PREFIX/bin/git` | `pkg install git` | Full Git workflow (clone, add, commit, push, status) |
| `python3` | Implemented | `$PREFIX/bin/python3` | `pkg install python` | Python 3.11 runtime |
| `python` | Implemented | `$PREFIX/bin/python` | `pkg install python` | Symlinked to python3 |
| `pip` / `pip3` | Implemented | `$PREFIX/bin/pip` | `pkg install python` | Python package installer |
| `nano` / `edit` | Implemented | `$PREFIX/bin/nano` | `trux-core` | Interactive text editor |
| `vim` | Partially Implemented | `/system/bin/vi` / `toybox` | `pkg install vim` | Full Vi/Vim editing |
| `clang` / `gcc` | Partially Implemented | - | `pkg install clang` | Requires NDK / aarch64 LLVM package |
| `make` / `cmake` | Partially Implemented | - | `pkg install cmake` | Build automation |

---

## 10. Android Integration Utilities (Termux-Compatible)

| Command | Status | Binary Path | Notes |
|---|---|---|---|
| `termux-info` / `trux-info` | Implemented | `$PREFIX/bin/termux-info` | Dumps TRUX, Android SDK, and CPU architecture |
| `termux-open` / `trux-open` | Implemented | `$PREFIX/bin/termux-open` | Dispatches Android VIEW intent for URLs and files |
| `termux-open-url` / `trux-open-url` | Implemented | `$PREFIX/bin/termux-open-url` | Opens URL in default Android browser |
| `termux-share` / `trux-share` | Implemented | `$PREFIX/bin/termux-share` | Dispatches Android SEND intent with text |
| `termux-toast` / `trux-toast` | Implemented | `$PREFIX/bin/termux-toast` | Posts Toast notification via Android system bridge |
| `termux-vibrate` / `trux-vibrate` | Implemented | `$PREFIX/bin/termux-vibrate` | Triggers device haptic feedback vibrator |
| `termux-notification` / `trux-notification` | Implemented | `$PREFIX/bin/termux-notification` | Posts status bar system notification |
| `termux-clipboard-get` | Implemented | `$PREFIX/bin/termux-clipboard-get` | Reads clipboard content |
| `termux-clipboard-set` | Implemented | `$PREFIX/bin/termux-clipboard-set` | Sets clipboard content |
| `termux-battery-status` | Implemented | `$PREFIX/bin/termux-battery-status` | Outputs JSON battery health, level, and temp |
| `termux-wifi-connectioninfo` | Implemented | `$PREFIX/bin/termux-wifi-connectioninfo` | Outputs JSON WiFi SSID, IP, and MAC address |
| `termux-setup-storage` | Implemented | `$PREFIX/bin/termux-setup-storage` | Sets up symlinks to `/storage/emulated/0` |
| `adb` | Implemented | `$PREFIX/bin/adb` | Direct bridge for devices, shell, pm install, push, pull, logcat, pairing |
