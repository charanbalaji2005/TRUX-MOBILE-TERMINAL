# TRUX File Permissions & Ownership

TRUX maintains standard POSIX permission models where technically feasible within the Android security architecture.

---

## Permission Bits

Files and directories within TRUX userspace respect standard Unix permission bits:

```text
r (4) : Read
w (2) : Write
x (1) : Execute
```

Represented across:
- **Owner (u)**: The user identity owning the file
- **Group (g)**: The primary or assigned group
- **Others (o)**: Any other user identity in the TRUX userspace

### Permission Commands

```bash
# Set owner read/write, others read
$ chmod 644 config.txt

# Make a script executable
$ chmod +x script.sh
$ chmod 755 script.sh

# Restrict sensitive credential file to owner only
$ chmod 600 ~/.ssh/id_rsa
```

---

## File Ownership (`chown` & `chgrp`)

Within the TRUX application private filesystem (`$PREFIX` and `$HOME`), file ownership can be managed across userspace users:

```bash
# Assign file to alice
$ chown alice script.sh

# Assign file to alice with developers group
$ chown alice:developers script.sh

# Assign group only
$ chgrp developers script.sh
```

---

## Storage Filesystem Differences & Limitations

| Storage Location | Filesystem Type | Permissions & Ownership Support |
|------------------|-----------------|---------------------------------|
| `$PREFIX` (`/data/data/com.meridian.shell/files/usr`) | ext4 / f2fs (App Private) | **Full POSIX permissions, symlinks, execution, chmod, chown** |
| `$HOME` (`/data/data/com.meridian.shell/files/home`) | ext4 / f2fs (App Private) | **Full POSIX permissions, symlinks, execution, chmod, chown** |
| `~/storage/shared` (`/storage/emulated/0`) | FUSE / sdcardfs (Android Shared) | **Partially Supported**: Android OS enforces fixed UID/GID and synthetic permissions on external shared storage. `chmod` and `chown` on `/storage/emulated/0` are constrained by the Android media provider kernel FUSE layer. |

> [!NOTE]
> For compilation, scripts with shebangs (`#!/bin/bash`), and security-sensitive files (SSH private keys, GPG keys, AI configuration tokens), always store them in `$HOME` or `$PREFIX` rather than `/storage/emulated/0` to retain full POSIX execution and permission bits.
