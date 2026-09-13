# TRUX Android Testing Guide

This guide describes the automated test suite and manual verification procedures for TRUX.

---

## 1. Automated Test Suite

TRUX includes 32 comprehensive unit tests running via JUnit 4 on the JVM.

### Executing Tests

```bash
cd android
./gradlew testDebugUnitTest
```

### Test Coverage Areas

1. **`SessionManagerTest`**:
   - Multiple concurrent session creation and destruction
   - Session switching and active session tracking
   - Session renaming and geometry updates
   - Automatic termination cleanup when shell process exits
   - Graceful recovery when all sessions are closed (creates fresh "Main" session)
2. **`KeyMapperTest`**:
   - Soft keyboard mapping to ANSI/VT escape sequences
   - Extra key row handling (ESC, TAB, CTRL, ALT, cursor keys, HOME, END, PGUP, PGDN)
   - Latching state for CTRL and ALT keys
3. **`StorageManagerTest`**:
   - Dynamic path resolution for `$HOME`, `$PREFIX`, `$TMPDIR`
   - Quota and disk usage checks
4. **`SessionEntity & TruxDatabase`**:
   - Room DAO queries: insert, query active, update size, mark closed, touch session

---

## 2. Manual Command Verification (Section 50)

Execute the following commands inside a live TRUX terminal:

```sh
# 1. Working directory
$ pwd
/data/data/com.meridian.shell/files/home

# 2. Directory listing
$ ls

# 3. Directory creation & traversal
$ mkdir test
$ cd test
$ pwd
/data/data/com.meridian.shell/files/home/test

# 4. File creation & reading
$ touch hello.txt
$ echo "hello"
hello
$ echo "hello" > hello.txt
$ cat hello.txt
hello

# 5. Copy, move, remove
$ cp hello.txt copy.txt
$ mv copy.txt moved.txt
$ rm moved.txt

# 6. Find, grep, sed
$ find .
.
./hello.txt
$ grep hello hello.txt
hello
$ sed 's/hello/TRUX/' hello.txt
TRUX

# 7. Shell version
$ bash --version || echo $SHELL

# 8. Archive handling
$ tar --version || toybox tar --version
```

---

## 3. Package Management Verification (Section 51)

```sh
# Step 1: Update repository index
$ pkg update

# Step 2: Search for git
$ pkg search git

# Step 3: Install Git
$ pkg install git

# Step 4: Verify git executable
$ git --version
git version 2.45.0

# Step 5: Install Python
$ pkg install python

# Step 6: Verify python executable
$ python3 --version
Python 3.11.8

# Step 7: Check installed packages
$ pkg list-installed
trux-core             2.0.1
toybox                0.8.9
git                   2.45.0
python                3.11.8

# Step 8: Uninstall git
$ pkg uninstall git
Package git uninstalled.

# Step 9: Verify git is legitimately unavailable
$ git --version
sh: git: not found
```

---

## 4. Concurrency & Stress Testing (Section 32)

1. Open Session Drawer via left-edge swipe.
2. Tap "New Session" to create 5 concurrent sessions.
3. Switch between sessions.
4. Verify each session maintains its own independent PTY, reader thread, and scrollback.
5. Exit any session with `exit` or Ctrl+D; verify it is cleanly removed without affecting sibling sessions.
