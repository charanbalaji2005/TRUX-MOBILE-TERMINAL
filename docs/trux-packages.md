# TRUX Package Management System

TRUX provides a Termux-compatible command-line package management interface powered by `pkg`, `apt`, `apt-get`, and `dpkg`.

---

## 1. Package Management Commands

| Command | Syntax | Description |
|---|---|---|
| `pkg update` | `pkg update` | Refreshes repository indices and package metadata |
| `pkg upgrade` | `pkg upgrade` | Upgrades installed packages to their latest versions |
| `pkg search` | `pkg search <query>` | Searches the package index for matches in names and descriptions |
| `pkg install` | `pkg install <pkg>` | Downloads, verifies, unpacks, and configures a package |
| `pkg uninstall` | `pkg uninstall <pkg>` | Removes binary and library files, updating database status |
| `pkg show` | `pkg show <pkg>` | Displays detailed package metadata, version, size, dependencies |
| `pkg list-installed` | `pkg list-installed` | Lists all packages currently installed on the device |
| `pkg clean` | `pkg clean` | Clears downloaded `.deb` / `.tar.xz` archives from local cache |

---

## 2. Underlying Tools & Aliases

The standard Debian commands are fully aliased and integrated:
- `apt install <pkg>` -> forwards to `pkg install <pkg>`
- `apt update` -> forwards to `pkg update`
- `apt-get install <pkg>` -> forwards to `pkg install <pkg>`
- `dpkg -i <archive.deb>` -> extracts package into `$PREFIX` and triggers configuration
- `dpkg -r <pkg>` -> calls uninstallation flow
- `dpkg -l` -> lists installed packages

---

## 3. Local On-Device Cache & Tracking

- **Package Cache**:
  Stored at `$PREFIX/var/cache/apt/archives/`. Downloaded archives are cached locally to avoid redundant network transfers.
- **Status Database**:
  Stored at `$PREFIX/var/lib/dpkg/status`. Records package name, installed version, installation status (`install ok installed`), and description.
- **File Manifests**:
  Stored at `$PREFIX/var/lib/dpkg/info/<package>.list`. Contains the exact list of files belonging to each package. When `pkg uninstall <package>` is executed, every file registered in this manifest is deleted from disk.

---

## 4. Verification & Testing

Example manual verification test:

```sh
# 1. Update package database
$ pkg update

# 2. Search for Git
$ pkg search git

# 3. Install Git
$ pkg install git

# 4. Check version
$ git --version
git version 2.45.0

# 5. List installed packages
$ pkg list-installed
trux-core             2.0.1
toybox                0.8.9
git                   2.45.0

# 6. Uninstall Git
$ pkg uninstall git
Package git uninstalled.

# 7. Verify binary was actually deleted
$ git --version
sh: git: not found
```
