# TRUX Linux Users & Groups System

TRUX implements a fully functional POSIX-compliant userspace identity and group management system.

---

## Important Distinction: Userspace vs Android Kernel Root

> [!IMPORTANT]
> **TRUX Root is a Linux Userspace Identity, NOT Android Kernel Root.**
> TRUX provides a multi-user POSIX environment inside its application sandbox.
> The `root` user (UID 0) within TRUX manages files, processes, and packages within the TRUX userspace.
> It **does not** exploit or escalate privileges into the Android host kernel, nor does it access other applications' private storage on non-rooted devices.

---

## User Database Structure

User and group records are persistently stored on-device in standard Linux configuration paths:

| File | Path | Description | Access Permissions |
|------|------|-------------|--------------------|
| `/etc/passwd` | `$PREFIX/etc/passwd` | User accounts, UIDs, default homes, and shells | Read by all users (644) |
| `/etc/group` | `$PREFIX/etc/group` | Group definitions, GIDs, and member lists | Read by all users (644) |
| `/etc/shadow` | `$PREFIX/etc/shadow` | Salted SHA-256 password hashes | Owner-only read/write (600) |
| `/etc/hostname` | `$PREFIX/etc/hostname` | Device hostname | Read by all users (644) |
| `/etc/shells` | `$PREFIX/etc/shells` | Permitted login shells | Read by all users (644) |

### Default Accounts
```text
root:x:0:0:root:/home/root:/bin/bash
trux:x:1000:1000:TRUX User:/home/trux:/bin/bash
```

---

## User Management Commands

### 1. Identify Current User (`whoami` & `id`)
```bash
$ whoami
trux

$ id
uid=1000(trux) gid=1000(trux) groups=1000(trux),10(wheel),27(sudo),1001(developers)

$ id root
uid=0(root) gid=0(root) groups=0(root)
```

### 2. Adding Users (`useradd` / `adduser`)
```bash
$ useradd alice
useradd: created user 'alice' (UID 1001, GID 1001) with home directory '/data/data/com.meridian.shell/files/home/alice'.

# Options:
useradd -u 1050 bob                   # Specify custom UID
useradd -G developers,sudo alice      # Supplementary groups
useradd -s /bin/sh charlie            # Specific login shell
useradd --system trux-daemon          # System user (UID in 100-999 range)
```

### 3. Setting Passwords (`passwd`)
```bash
$ passwd alice
Changing password for alice.
New password: 
Retype new password: 
passwd: password updated successfully.
```
*Passwords are hashed with cryptographic salt + SHA-256 and stored in `/etc/shadow` with mode 600.*

### 4. Modifying Users (`usermod`)
```bash
# Add user to supplementary group
$ usermod -aG developers alice

# Change shell or home
$ usermod -s /bin/sh alice
$ usermod -d /home/alice_dev alice
```

### 5. Deleting Users (`userdel` / `deluser`)
```bash
$ userdel alice          # Deletes user record
$ userdel -r alice       # Deletes user record and home directory
```

---

## Group Management Commands

### 1. Listing Groups (`groups`)
```bash
$ groups
trux : trux wheel sudo developers

$ groups alice
alice : alice developers
```

### 2. Creating Groups (`groupadd`)
```bash
$ groupadd testers
groupadd: group 'testers' (GID 1002) created.
```

### 3. Modifying & Deleting Groups (`groupmod` & `groupdel`)
```bash
$ groupmod -n qa testers     # Rename group
$ groupdel qa                # Remove group
```

### 4. Group Member Management (`gpasswd`)
```bash
$ gpasswd -a charan developers   # Add user to group
$ gpasswd -d charan developers   # Remove user from group
```

---

## Switching Users (`su` & `sudo`)

### Userspace `su`
```bash
$ su alice
alice@android:~$ whoami
alice
alice@android:~$ exit
trux@android:~$
```

### Userspace `root`
```bash
$ su root
==========================================================
[TRUX] Switched to TRUX userspace root (UID 0).
Notice: This applies inside the TRUX Linux sandbox environment.
It does NOT provide Android kernel root privileges.
==========================================================
root@android:~# whoami
root
root@android:~# id
uid=0(root) gid=0(root) groups=0(root)
```

### Userspace `sudo`
Execute single commands with userspace UID 0:
```bash
$ sudo useradd david
$ sudo passwd david
```
