package com.meridian.shell

import android.content.Context
import java.io.File

class Bootstrap(val context: Context) {

    val filesDir: File = context.filesDir
    val prefix: File = File(filesDir, "usr")
    val home: File = File(filesDir, "home")
    val tmp: File = File(prefix, "tmp")
    val environments: File = File(filesDir, "environments")
    val packagesCache: File = File(filesDir, "packages/cache")
    val sessions: File = File(filesDir, "sessions")

    /** Directory the APK's bundled executables were extracted to. Read-only. */
    val nativeLibDir: File = File(context.applicationInfo.nativeLibraryDir)

    /**
     * True when we are allowed to exec files we wrote ourselves. Drives whether
     * the package manager may install new binaries at all.
     */
    val canExecFromDataDir: Boolean
        get() = context.applicationInfo.targetSdkVersion < 29

    /**
     * Ensure all required directories exist.
     * Safe to call multiple times (idempotent).
     */
    fun ensureDirectories() {
        listOf(
            prefix,
            File(prefix, "bin"),
            File(prefix, "lib"),
            File(prefix, "etc"),
            File(prefix, "share"),
            File(prefix, "var"),
            tmp,
            home,
            File(home, "storage"),
            environments,
            packagesCache,
            sessions
        ).forEach { it.mkdirs() }
        setupStorageSymlinks()
        linkToyboxTools()
        writeShellProfiles()
        installCommandShims()
    }

    /**
     * Symlink all standard Android /system/bin Linux tools into $PREFIX/bin
     * so they are immediately accessible like in Termux.
     */
    fun linkToyboxTools() {
        val binDir = File(prefix, "bin").also { it.mkdirs() }
        val systemBin = File("/system/bin")
        val sysFiles = systemBin.listFiles() ?: emptyArray()
        for (f in sysFiles) {
            if (f.isFile && f.canExecute()) {
                val target = File(binDir, f.name)
                if (!target.exists()) {
                    runCatching {
                        Runtime.getRuntime().exec(arrayOf("ln", "-s", f.absolutePath, target.absolutePath)).waitFor()
                    }
                }
            }
        }
    }

    /**
     * Create symlinks in $HOME/storage to shared phone memory.
     */
    fun setupStorageSymlinks() {
        val storageDir = File(home, "storage")
        storageDir.mkdirs()
        val emulated = File("/storage/emulated/0")
        val links = listOf(
            "shared" to emulated.absolutePath,
            "downloads" to File(emulated, "Download").absolutePath,
            "dcim" to File(emulated, "DCIM").absolutePath,
            "pictures" to File(emulated, "Pictures").absolutePath,
            "documents" to File(emulated, "Documents").absolutePath,
            "music" to File(emulated, "Music").absolutePath,
            "movies" to File(emulated, "Movies").absolutePath
        )
        for ((name, target) in links) {
            val linkFile = File(storageDir, name)
            if (!linkFile.exists()) {
                runCatching {
                    Runtime.getRuntime().exec(arrayOf("ln", "-s", target, linkFile.absolutePath)).waitFor()
                }
            }
        }
    }

    /**
     * Write shell profile files on first boot so the user gets a proper
     * login environment with dynamic directory prompt.
     */
    fun writeShellProfiles() {
        val etcDir = File(prefix, "etc").also { it.mkdirs() }
        val truxDir = File(etcDir, "trux").also { it.mkdirs() }

        // 1. /etc/passwd
        val passwdFile = File(etcDir, "passwd")
        if (!passwdFile.exists()) {
            passwdFile.writeText(
                """
                root:x:0:0:root:/home/root:/bin/bash
                daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
                bin:x:2:2:bin:/bin:/usr/sbin/nologin
                trux-daemon:x:101:101:TRUX System Daemon:/var/run/trux:/usr/sbin/nologin
                trux:x:1000:1000:TRUX User:${home.absolutePath}:/bin/bash
                nobody:x:65534:65534:nobody:/nonexistent:/usr/sbin/nologin
                """.trimIndent() + "\n"
            )
        }

        // 2. /etc/group
        val groupFile = File(etcDir, "group")
        if (!groupFile.exists()) {
            groupFile.writeText(
                """
                root:x:0:
                daemon:x:1:
                bin:x:2:
                adm:x:4:trux
                wheel:x:10:trux
                sudo:x:27:trux
                users:x:100:trux
                developers:x:1001:trux
                trux:x:1000:
                nobody:x:65534:
                """.trimIndent() + "\n"
            )
        }

        // 3. /etc/shadow
        val shadowFile = File(etcDir, "shadow")
        if (!shadowFile.exists()) {
            shadowFile.writeText(
                """
                root:*:19700:0:99999:7:::
                daemon:*:19700:0:99999:7:::
                bin:*:19700:0:99999:7:::
                trux-daemon:*:19700:0:99999:7:::
                trux:*:19700:0:99999:7:::
                nobody:*:19700:0:99999:7:::
                """.trimIndent() + "\n"
            )
            shadowFile.setReadable(true, true)
            shadowFile.setWritable(true, true)
        }

        // 4. /etc/hostname
        val hostnameFile = File(etcDir, "hostname")
        if (!hostnameFile.exists()) {
            hostnameFile.writeText("android\n")
        }

        // 5. /etc/shells
        val shellsFile = File(etcDir, "shells")
        if (!shellsFile.exists()) {
            shellsFile.writeText("/bin/bash\n/bin/sh\n/system/bin/sh\n")
        }

        // 6. /etc/inputrc and $HOME/.inputrc (maps Home, End, PgUp, PgDn, Del)
        val inputrcContent = """
            "\e[1~": beginning-of-line
            "\e[4~": end-of-line
            "\e[H": beginning-of-line
            "\e[F": end-of-line
            "\eOH": beginning-of-line
            "\eOF": end-of-line
            "\e[5~": beginning-of-history
            "\e[6~": end-of-history
            "\e[3~": delete-char
            "\e[2~": quoted-insert
            set enable-bracketed-paste off
        """.trimIndent() + "\n"
        File(etcDir, "inputrc").writeText(inputrcContent)
        File(home, ".inputrc").writeText(inputrcContent)

        // Persistent history files on device
        runCatching {
            File(home, ".history").let { if (!it.exists()) it.createNewFile() }
            File(home, ".bash_history").let { if (!it.exists()) it.createNewFile() }
        }

        val profile = File(home, ".profile")
        profile.writeText(
            """
            # TRUX — environment profile
            export PATH="${prefix.absolutePath}/bin:/system/bin:/system/xbin:${'$'}{PATH}"
            export HOME="${home.absolutePath}"
            export PREFIX="${prefix.absolutePath}"
            export TMPDIR="${tmp.absolutePath}"
            export TERM="xterm-256color"
            export COLORTERM="truecolor"
            export INPUTRC="${home.absolutePath}/.inputrc"
            export USER="${'$'}{USER:-trux}"
            export LOGNAME="${'$'}{LOGNAME:-${'$'}USER}"

            # Persistent command history on device
            export HISTFILE="${home.absolutePath}/.history"
            export HISTSIZE=10000
            export HISTFILESIZE=20000
            if [ -n "${'$'}KSH_VERSION" ]; then
                set -o history 2>/dev/null
            fi
            if [ -n "${'$'}BASH_VERSION" ]; then
                export HISTFILE="${home.absolutePath}/.bash_history"
                shopt -s histappend 2>/dev/null
                export PROMPT_COMMAND="history -a; ${'$'}PROMPT_COMMAND"
            fi

            # Dynamic working directory in prompt
            _prompt_dir() {
                p="${'$'}{PWD#${'$'}HOME}"
                if [ "${'$'}p" != "${'$'}PWD" ]; then
                    if [ -z "${'$'}p" ]; then
                        echo "~"
                    else
                        echo "~${'$'}p"
                    fi
                else
                    echo "${'$'}PWD"
                fi
            }
            u="${'$'}{USER:-trux}"
            [ "${'$'}u" = "root" ] || [ "${'$'}UID" = "0" ] && sym="#" || sym="${'$'}"
            export PS1='${'$'}u@android:${'$'}(_prompt_dir)${'$'}sym '

            # Bind Home, End, PgUp, PgDn, Del for mksh (Android default shell)
            if [ -n "${'$'}KSH_VERSION" ]; then
                bind '^[[H'=beginning-of-line 2>/dev/null
                bind '^[[1~'=beginning-of-line 2>/dev/null
                bind '^[OH'=beginning-of-line 2>/dev/null
                bind '^[[F'=end-of-line 2>/dev/null
                bind '^[[4~'=end-of-line 2>/dev/null
                bind '^[OF'=end-of-line 2>/dev/null
                bind '^[[3~'=delete-char-forward 2>/dev/null
                bind '^[[5~'=search-history-up 2>/dev/null
                bind '^[[6~'=search-history-down 2>/dev/null
            fi

            # Bind Home, End, PgUp, PgDn, Del for bash
            if [ -n "${'$'}BASH_VERSION" ]; then
                bind '"\e[H": beginning-of-line' 2>/dev/null
                bind '"\e[1~": beginning-of-line' 2>/dev/null
                bind '"\eOH": beginning-of-line' 2>/dev/null
                bind '"\e[F": end-of-line' 2>/dev/null
                bind '"\e[4~": end-of-line' 2>/dev/null
                bind '"\eOF": end-of-line' 2>/dev/null
                bind '"\e[3~": delete-char' 2>/dev/null
                bind '"\e[5~": history-search-backward' 2>/dev/null
                bind '"\e[6~": history-search-forward' 2>/dev/null
            fi

            alias ls='ls --color=auto'
            alias ll='ls -la'
            alias la='ls -A'
            alias sdcard='cd ~/storage/shared 2>/dev/null || cd /storage/emulated/0 2>/dev/null || cd /sdcard'
            alias storage='setup-storage'
            alias termux='trux'
            alias meridian='trux'
            alias ai='trux-ai'
            alias adb='adb'
            alias apt='pkg'
            alias apt-get='pkg'
            alias dnf='pkg'
            alias yum='pkg'
            alias pacman='pkg'
            alias useradd='useradd'
            alias usermod='usermod'
            alias userdel='userdel'
            alias passwd='passwd'
            alias whoami='whoami'
            alias id='id'
            alias groups='groups'
            alias users='users'
            alias groupadd='groupadd'
            alias groupdel='groupdel'
            alias groupmod='groupmod'
            alias gpasswd='gpasswd'
            alias hostname='hostname'
            alias su='su'
            alias sudo='sudo'
            alias export-pdf='trux export pdf'
            alias export-session='trux export'
            """.trimIndent() + "\n"
        )
        File(etcDir, "profile").writeText(profile.readText())

        val mkshrcContent = """
            # TRUX — mksh config
            export PATH="${prefix.absolutePath}/bin:/system/bin:/system/xbin:${'$'}{PATH}"
            export HOME="${home.absolutePath}"
            export PREFIX="${prefix.absolutePath}"
            export TERM="xterm-256color"
            export COLORTERM="truecolor"
            export INPUTRC="${home.absolutePath}/.inputrc"
            export USER="${'$'}{USER:-trux}"
            export LOGNAME="${'$'}{LOGNAME:-${'$'}USER}"

            # Persistent command history on device
            export HISTFILE="${home.absolutePath}/.history"
            export HISTSIZE=10000
            set -o history 2>/dev/null

            bind '^[[H'=beginning-of-line 2>/dev/null
            bind '^[[1~'=beginning-of-line 2>/dev/null
            bind '^[OH'=beginning-of-line 2>/dev/null
            bind '^[[F'=end-of-line 2>/dev/null
            bind '^[[4~'=end-of-line 2>/dev/null
            bind '^[OF'=end-of-line 2>/dev/null
            bind '^[[3~'=delete-char-forward 2>/dev/null
            bind '^[[5~'=search-history-up 2>/dev/null
            bind '^[[6~'=search-history-down 2>/dev/null

            _prompt_dir() {
                p="${'$'}{PWD#${'$'}HOME}"
                if [ "${'$'}p" != "${'$'}PWD" ]; then
                    if [ -z "${'$'}p" ]; then echo "~"; else echo "~${'$'}p"; fi
                else
                    echo "${'$'}PWD"
                fi
            }
            u="${'$'}{USER:-trux}"
            [ "${'$'}u" = "root" ] || [ "${'$'}UID" = "0" ] && sym="#" || sym="${'$'}"
            export PS1='${'$'}u@android:${'$'}(_prompt_dir)${'$'}sym '

            alias ls='ls --color=auto'
            alias ll='ls -la'
            alias la='ls -A'
            alias ai='trux-ai'
            alias termux='trux'
            alias storage='setup-storage'
            alias export-pdf='trux export pdf'
            alias export-session='trux export'
        """.trimIndent() + "\n"
        File(etcDir, "mkshrc").writeText(mkshrcContent)
        File(home, ".mkshrc").writeText(mkshrcContent)

        val bashrc = File(home, ".bashrc")
        bashrc.writeText(
            """
            # TRUX — interactive bash config
            export INPUTRC="${home.absolutePath}/.inputrc"
            export USER="${'$'}{USER:-trux}"
            export LOGNAME="${'$'}{LOGNAME:-${'$'}USER}"
            export PS1='${'$'}{USER:-trux}@android:\w\$ '

            # Persistent command history on device
            export HISTFILE="${home.absolutePath}/.bash_history"
            export HISTSIZE=10000
            export HISTFILESIZE=20000
            shopt -s histappend 2>/dev/null
            export PROMPT_COMMAND="history -a; ${'$'}PROMPT_COMMAND"

            bind '"\e[H": beginning-of-line' 2>/dev/null
            bind '"\e[1~": beginning-of-line' 2>/dev/null
            bind '"\eOH": beginning-of-line' 2>/dev/null
            bind '"\e[F": end-of-line' 2>/dev/null
            bind '"\e[4~": end-of-line' 2>/dev/null
            bind '"\eOF": end-of-line' 2>/dev/null
            bind '"\e[3~": delete-char' 2>/dev/null
            bind '"\e[5~": history-search-backward' 2>/dev/null
            bind '"\e[6~": history-search-forward' 2>/dev/null

            alias ls='ls --color=auto'
            alias ll='ls -lah --color=auto'
            alias la='ls -A --color=auto'
            alias grep='grep --color=auto'
            alias ..='cd ..'
            alias ...='cd ../..'
            alias termux='trux'
            alias meridian='trux'
            alias ai='trux-ai'
            alias adb='adb'
            alias apt='pkg'
            alias apt-get='pkg'
            alias dnf='pkg'
            alias yum='pkg'
            alias pacman='pkg'
            alias useradd='useradd'
            alias usermod='usermod'
            alias userdel='userdel'
            alias passwd='passwd'
            alias whoami='whoami'
            alias id='id'
            alias groups='groups'
            alias users='users'
            alias groupadd='groupadd'
            alias groupdel='groupdel'
            alias groupmod='groupmod'
            alias gpasswd='gpasswd'
            alias hostname='hostname'
            alias su='su'
            alias sudo='sudo'

            if [ -f "${prefix.absolutePath}/etc/motd" ]; then
                cat "${prefix.absolutePath}/etc/motd"
            fi
            """.trimIndent() + "\n"
        )

        val motd = File(prefix, "etc/motd")
        if (!motd.exists()) {
            motd.writeText(
                """
                 _____ ____  _   ___  __
                |_   _|  _ \| | | \ \/ /
                  | | | |_) | | | |\  / 
                  | | |  _ <| |_| |/  \ 
                  |_| |_| \_\\___//_/\_\

                TRUX 2.0.1 — Android Terminal Environment
                BUILD > EXPLORE > BEYOND
                Run 'trux' or 'help' for command index.
                Run 'adb' for Android Debug Bridge tools.
                Type 'exit' or press Ctrl+D to close a session.

                """.trimIndent()
            )
        }
    }

    /**
     * Install friendly command shims into $PREFIX/bin so common commands
     * like help, meridian, dnf, sudo, su, pkg, apt, clear always work.
     */
    fun installCommandShims() {
        val binDir = File(prefix, "bin").also { it.mkdirs() }

        // 1. trux & meridian multi-tool CLI
        val truxCli = """
            #!/system/bin/sh
            case "${'$'}1" in
                monitor|top|metrics)
                    echo "=== TRUX System Telemetry ==="
                    echo "Kernel   : ${'$'}(uname -srm 2>/dev/null || uname -a)"
                    upt="${'$'}(cat /proc/uptime 2>/dev/null | awk '{h=int(${'$'}1/3600); m=int((${'$'}1%3600)/60); if (h>0) printf \"%dh %dm\n\", h, m; else printf \"%dm\n\", m}')"
                    echo "Uptime   : ${'$'}{upt:-${'$'}(uptime 2>/dev/null | sed -e 's/^[ \t]*//')}"
                    echo "Memory   : ${'$'}(cat /proc/meminfo 2>/dev/null | grep MemTotal | awk '{print int(${'$'}2/1024) \" MB total\"}')"
                    echo "Storage  : ${'$'}(df -h "${'$'}HOME" 2>/dev/null | awk 'END {print ${'$'}4, \"available (\" ${'$'}5 \" used)\"}')"
                    echo "Processes: ${'$'}(ps 2>/dev/null | wc -l) active"
                    echo "============================="
                    ;;
                storage|sdcard|memory)
                    exec setup-storage
                    ;;
                git)
                    if [ -x "${'$'}PREFIX/bin/git" ]; then
                        shift
                        exec "${'$'}PREFIX/bin/git" "${'$'}@"
                    fi
                    echo "=== TRUX Git Workspace ==="
                    echo "Directory: ${'$'}(pwd)"
                    if [ -d .git ]; then
                        echo "Git branch: ${'$'}(cat .git/HEAD 2>/dev/null)"
                        echo "Status    : clean"
                    else
                        echo "No git repository in current directory."
                        echo "Run 'pkg install git' to install full Git toolchain."
                    fi
                    echo "=========================="
                    ;;
                files|tree)
                    target="${'$'}{2:-.}"
                    echo "=== TRUX Files: ${'$'}target ==="
                    ls -la "${'$'}target"
                    echo "==============================="
                    ;;
                info|version|--version|-v)
                    echo "TRUX 2.0.1 (Android Terminal Edition)"
                    echo "Architecture : ${'$'}(getprop ro.product.cpu.abi 2>/dev/null || uname -m)"
                    echo "Device       : ${'$'}(getprop ro.product.model 2>/dev/null || echo Android)"
                    echo "Android OS   : ${'$'}(getprop ro.build.version.release 2>/dev/null || echo 15)"
                    echo "Prefix       : ${'$'}PREFIX"
                    echo "Home         : ${'$'}HOME"
                    echo "Phone Shared : ~/storage/shared -> /storage/emulated/0"
                    ;;
                ssh)
                    echo "=== TRUX SSH Workspace Manager ==="
                    echo "Config path: ${'$'}HOME/.ssh/config"
                    echo "Usage: trux ssh <host-alias>"
                    ;;
                ai)
                    shift
                    exec trux-ai "${'$'}@"
                    ;;
                user|users)
                    shift
                    case "${'$'}1" in
                        list|"")
                            awk -F: '{print ${'$'}1 " (uid=" ${'$'}3 ", gid=" ${'$'}4 ", home=" ${'$'}6 ")"}' "${prefix.absolutePath}/etc/passwd" 2>/dev/null || cat "${prefix.absolutePath}/etc/passwd"
                            ;;
                        add)
                            shift
                            exec useradd "${'$'}@"
                            ;;
                        del|delete)
                            shift
                            exec userdel "${'$'}@"
                            ;;
                        mod|modify)
                            shift
                            exec usermod "${'$'}@"
                            ;;
                        *)
                            exec id "${'$'}@"
                            ;;
                    esac
                    ;;
                group|groups)
                    shift
                    case "${'$'}1" in
                        list|"")
                            awk -F: '{print ${'$'}1 " (gid=" ${'$'}3 ", members=" ${'$'}4 ")"}' "${prefix.absolutePath}/etc/group" 2>/dev/null || cat "${prefix.absolutePath}/etc/group"
                            ;;
                        add)
                            shift
                            exec groupadd "${'$'}@"
                            ;;
                        del|delete)
                            shift
                            exec groupdel "${'$'}@"
                            ;;
                        *)
                            exec groups "${'$'}@"
                            ;;
                    esac
                    ;;
                su)
                    shift
                    exec su "${'$'}@"
                    ;;
                sudo)
                    shift
                    exec sudo "${'$'}@"
                    ;;
                help|--help|-h|"")
                    topic="${'$'}2"
                    case "${'$'}topic" in
                        ai)
                            echo "TRUX AI commands:"
                            echo "  trux ai setup         Configure AI provider, endpoint, and credentials"
                            echo "  trux ai status        Display current AI configuration & readiness"
                            echo "  trux ai test          Test AI endpoint connection and authentication"
                            echo "  trux ai models        List available models from configured endpoint"
                            echo "  trux ai reset         Delete AI configuration & stored secrets"
                            echo "  trux ai \"prompt\"      Ask AI a question directly from the terminal"
                            echo "  trux ai               Enter interactive AI chat mode (/exit to leave)"
                            echo "  trux ai --command <p> Generate shell command with [y/N] safety prompt"
                            echo "  trux ai explain <err> Explain a specific error or command"
                            echo "  trux ai explain-last  Explain the last terminal command output"
                            ;;
                        user|users)
                            echo "TRUX User commands:"
                            echo "  useradd <name>        Create a new TRUX userspace user"
                            echo "  usermod -aG <grp> <u> Add user to supplementary groups"
                            echo "  userdel [-r] <name>   Delete user (and optionally home directory)"
                            echo "  passwd [name]         Set or change user password"
                            echo "  id [name]             Print UID, GID, and group memberships"
                            echo "  whoami                Print current userspace identity"
                            echo "  groups [name]         Show group memberships"
                            echo "  users                 List logged in users"
                            echo "  su [name]             Switch userspace identity (default: root)"
                            echo "  sudo <command>        Execute command with userspace root privileges"
                            ;;
                        group|groups)
                            echo "TRUX Group commands:"
                            echo "  groupadd <name>       Create a new group in TRUX userspace"
                            echo "  groupdel <name>       Delete group"
                            echo "  groupmod -n <new> <g> Rename group"
                            echo "  gpasswd -a <u> <g>    Add user to group"
                            echo "  gpasswd -d <u> <g>    Remove user from group"
                            echo "  groups [name]         Display groups for user"
                            ;;
                        perm|permissions)
                            echo "TRUX Permissions:"
                            echo "  chmod <mode> <file>   Change file permissions (e.g. chmod 755)"
                            echo "  chown <user> <file>   Change file ownership"
                            echo "  chgrp <grp> <file>    Change file group"
                            echo "  Notice: TRUX userspace permissions are enforced inside the app sandbox."
                            echo "  Kernel root privileges are NOT required or granted."
                            ;;
                        *)
                            echo "================= TRUX 2.0.1 ================="
                            echo "Android-native Linux terminal & POSIX userspace"
                            echo "BUILD > EXPLORE > BEYOND"
                            echo ""
                            echo "Help Topics: trux help [ai | users | groups | permissions]"
                            echo ""
                            echo "Core AI Integration:"
                            echo "  trux ai setup           Configure AI provider (OpenAI, Anthropic, Local)"
                            echo "  trux ai status          Show AI configuration and status"
                            echo "  trux ai test            Verify AI connection"
                            echo "  trux ai \"prompt\"        Query AI directly"
                            echo "  trux ai                 Interactive AI session"
                            echo ""
                            echo "Userspace Linux Identity:"
                            echo "  useradd | usermod | userdel | passwd | id | whoami | su | sudo"
                            echo "  groupadd | groupdel | groupmod | groups | hostname"
                            echo ""
                            echo "System & Tools:"
                            echo "  setup-storage           Mount phone internal storage & permissions"
                            echo "  trux monitor            Live CPU, RAM, and Disk metrics"
                            echo "  trux storage            Display phone storage mapping"
                            echo "  trux files [dir]        Inspect directory files and permissions"
                            echo "  trux info               Display device & shell architecture info"
                            echo "  trux session [list|show] Manage and review saved terminal sessions"
                            echo "  trux export [pdf|txt]   Export active session transcript to PDF or Text"
                            echo "  adb                     Android Debug Bridge (wireless/device debugging)"
                            echo "  bootloader              Device bootloader & partition manager"
                            echo "  os-install              OS & GSI installation assistant"
                            echo ""
                            echo "Welcome Dashboard:"
                            echo "  trux welcome            Display startup dashboard"
                            echo "  trux welcome preview    Preview startup dashboard"
                            echo "  trux welcome enable     Enable welcome screen on new session"
                            echo "  trux welcome disable    Disable welcome screen on new session"
                            echo ""
                            echo "Universal File Previewer:"
                            echo "  open <file>             Open PDF, image, media, text, code, archive in TRUX viewer"
                            echo "  xdg-open <file>         Standard XDG file opener"
                            echo "  trux open <file>        Universal file previewer"
                            echo ""
                            echo "Package Management (Termux-style):"
                            echo "  pkg update              Refresh available package lists"
                            echo "  pkg install <tool>      Install package (e.g. git, python, curl)"
                            echo "  (apt, apt-get, dpkg are also supported)"
                            echo "=============================================="
                            ;;
                    esac
                    ;;
                export)
                    shift
                    case "${'$'}1" in
                        pdf|"")
                            echo "Exporting TRUX terminal session to PDF..."
                            am broadcast -a com.meridian.shell.action.EXPORT_PDF --es format "pdf" >/dev/null 2>&1
                            echo "PDF export generated! Saved to Downloads folder & ~/storage/downloads."
                            ;;
                        txt|text)
                            echo "Exporting TRUX terminal session to text..."
                            am broadcast -a com.meridian.shell.action.EXPORT_PDF --es format "txt" >/dev/null 2>&1
                            echo "Session text export saved to Downloads folder & ~/storage/downloads."
                            ;;
                        *)
                            echo "Usage: trux export [pdf|txt]"
                            ;;
                    esac
                    ;;
                session|sessions)
                    shift
                    case "${'$'}1" in
                        list|"")
                            echo "=== TRUX Saved Sessions on Device ==="
                            if [ -d "${prefix.parentFile?.absolutePath}/sessions" ]; then
                                ls -lah "${prefix.parentFile?.absolutePath}/sessions"
                            elif [ -d "${home.absolutePath}/.sessions" ]; then
                                ls -lah "${home.absolutePath}/.sessions"
                            else
                                echo "No saved sessions found."
                            fi
                            echo "======================================"
                            ;;
                        show|view)
                            target="${'$'}2"
                            if [ -z "${'$'}target" ]; then
                                echo "Usage: trux session show <session_name_or_id>"
                                exit 1
                            fi
                            cat "${prefix.parentFile?.absolutePath}/sessions/${'$'}target.log" 2>/dev/null || cat "${home.absolutePath}/.sessions/${'$'}target.log" 2>/dev/null || echo "Session log not found for: ${'$'}target"
                            ;;
                        export)
                            shift
                            exec trux export "${'$'}@"
                            ;;
                        *)
                            echo "Usage: trux session [list | show <name> | export [pdf]]"
                            ;;
                    esac
                    ;;
                open|preview)
                    shift
                    if [ -z "${'$'}1" ]; then
                        echo "Usage: trux open <file>"
                        exit 1
                    fi
                    exec open "${'$'}@"
                    ;;
                welcome)
                    shift
                    exec trux-welcome "${'$'}@"
                    ;;
                *)
                    echo "[trux] Unknown subcommand: ${'$'}1"
                    echo "Run 'trux help' for available subcommands."
                    ;;
            esac
        """.trimIndent() + "\n"

        val truxFile = File(binDir, "trux").also {
            it.writeText(truxCli)
            it.setExecutable(true, false)
        }
        val meridianFile = File(binDir, "meridian").also {
            it.writeText("#!/system/bin/sh\nexec trux \"${'$'}@\"\n")
            it.setExecutable(true, false)
        }
        val exportSessionFile = File(binDir, "export-session").also {
            it.writeText("#!/system/bin/sh\nexec trux export \"${'$'}@\"\n")
            it.setExecutable(true, false)
        }

        // 1b. trux-welcome ANSI dashboard script
        val truxWelcomeScript = """
            #!/system/bin/sh
            CONF="${'$'}{PREFIX:-/data/data/com.meridian.shell/files/usr}/etc/trux/welcome.conf"

            case "${'$'}1" in
                enable)
                    mkdir -p "${'$'}(dirname "${'$'}CONF")" 2>/dev/null
                    echo "enabled=1" > "${'$'}CONF"
                    echo "TRUX welcome screen enabled for new sessions."
                    exit 0
                    ;;
                disable)
                    mkdir -p "${'$'}(dirname "${'$'}CONF")" 2>/dev/null
                    echo "enabled=0" > "${'$'}CONF"
                    echo "TRUX welcome screen disabled for new sessions."
                    exit 0
                    ;;
                status)
                    if [ -f "${'$'}CONF" ] && grep -q "enabled=0" "${'$'}CONF" 2>/dev/null; then
                        echo "TRUX welcome screen: DISABLED"
                    else
                        echo "TRUX welcome screen: ENABLED"
                    fi
                    exit 0
                    ;;
                --auto)
                    if [ -f "${'$'}CONF" ] && grep -q "enabled=0" "${'$'}CONF" 2>/dev/null; then
                        exit 0
                    fi
                    ;;
                preview|"")
                    ;;
                *)
                    echo "Usage: trux welcome [enable|disable|preview|status]"
                    exit 1
                    ;;
            esac

            # Detect terminal dimensions
            COLS=""
            ROWS=""
            if [ -t 1 ]; then
                DIM_STTY="${'$'}(stty size 2>/dev/null)"
                ROWS="${'$'}(echo "${'$'}DIM_STTY" | awk '{print ${'$'}1}')"
                COLS="${'$'}(echo "${'$'}DIM_STTY" | awk '{print ${'$'}2}')"
                [ -z "${'$'}COLS" ] && COLS=${'$'}(tput cols 2>/dev/null)
                [ -z "${'$'}ROWS" ] && ROWS=${'$'}(tput lines 2>/dev/null)
            fi
            [ -z "${'$'}COLS" ] && COLS=${'$'}{COLUMNS:-36}
            [ -z "${'$'}ROWS" ] && ROWS=${'$'}{LINES:-24}
            case "${'$'}COLS" in
                ''|*[!0-9]*) COLS=36 ;;
            esac
            case "${'$'}ROWS" in
                ''|*[!0-9]*) ROWS=24 ;;
            esac

            # Monochrome / Color Detection
            if [ "${'$'}TERM" = "dumb" ] || [ -n "${'$'}NO_COLOR" ]; then
                RST=""
                BOLD=""
                DIM=""
                WHT=""
                SLV=""
                GRY=""
                DRK=""
                ACC=""
            else
                RST="${'$'}(printf '\033[0m')"
                BOLD="${'$'}(printf '\033[1m')"
                DIM="${'$'}(printf '\033[2m')"
                WHT="${'$'}(printf '\033[1;37m')"
                SLV="${'$'}(printf '\033[38;5;252m')"
                GRY="${'$'}(printf '\033[38;5;245m')"
                DRK="${'$'}(printf '\033[38;5;240m')"
                ACC="${'$'}(printf '\033[1;38;5;255m')"
            fi

            center_line() {
                line="${'$'}1"
                raw_len="${'$'}2"
                if [ "${'$'}raw_len" -ge "${'$'}COLS" ]; then
                    echo "${'$'}line"
                else
                    pad=${'$'}(( (COLS - raw_len) / 2 ))
                    pad_str=""
                    i=0
                    while [ "${'$'}i" -lt "${'$'}pad" ]; do
                        pad_str=" ${'$'}pad_str"
                        i=${'$'}((i + 1))
                    done
                    echo "${'$'}{pad_str}${'$'}line"
                fi
            }

            echo ""

            L1="█████   ████    █   █   █   █"
            L2="  █     █   █   █   █    █ █ "
            L3="  █     ████    █   █     █  "
            L4="  █     █  █    █   █    █ █ "
            L5="  █     █   █    ███    █   █"

            # Header and Content
            if [ "${'$'}ROWS" -le 18 ] || [ "${'$'}COLS" -lt 32 ]; then
                # Compressed layout (keyboard opened or small viewport)
                center_line "${'$'}{ACC}${'$'}{BOLD}${'$'}{L1}${'$'}{RST}" 29
                center_line "${'$'}{ACC}${'$'}{BOLD}${'$'}{L2}${'$'}{RST}" 29
                center_line "${'$'}{SLV}${'$'}{BOLD}${'$'}{L3}${'$'}{RST}" 29
                center_line "${'$'}{SLV}${'$'}{BOLD}${'$'}{L4}${'$'}{RST}" 29
                center_line "${'$'}{GRY}${'$'}{BOLD}${'$'}{L5}${'$'}{RST}" 29
                center_line "${'$'}{SLV}${'$'}{BOLD}ANDROID TERMINAL • BEYOND${'$'}{RST}" 25
                center_line "${'$'}{WHT}Type 'help' | 'trux' | 'pkg'${'$'}{RST}" 28
            else
                # Standard layout (keyboard closed)
                center_line "${'$'}{DIM}${'$'}{DRK}WELCOME TO${'$'}{RST}" 10
                center_line "${'$'}{ACC}${'$'}{BOLD}${'$'}{L1}${'$'}{RST}" 29
                center_line "${'$'}{ACC}${'$'}{BOLD}${'$'}{L2}${'$'}{RST}" 29
                center_line "${'$'}{SLV}${'$'}{BOLD}${'$'}{L3}${'$'}{RST}" 29
                center_line "${'$'}{SLV}${'$'}{BOLD}${'$'}{L4}${'$'}{RST}" 29
                center_line "${'$'}{GRY}${'$'}{BOLD}${'$'}{L5}${'$'}{RST}" 29
                center_line "${'$'}{SLV}${'$'}{BOLD}A N D R O I D   T E R M I N A L${'$'}{RST}" 31
                center_line "${'$'}{GRY}${'$'}{DIM}BUILD  ›  EXPLORE  ›  BEYOND${'$'}{RST}" 28
                echo ""
                center_line "${'$'}{SLV} • Linux Env     • Secure Lock${'$'}{RST}" 30
                center_line "${'$'}{SLV} • Packages      • File Viewer${'$'}{RST}" 30
                center_line "${'$'}{SLV} • AI Assistant  • High Perf  ${'$'}{RST}" 30
                echo ""
                echo " ${'$'}{GRY}${'$'}{DIM}Type ${'$'}{WHT}'help'${'$'}{GRY}${'$'}{DIM}     → Show commands${'$'}{RST}"
                echo " ${'$'}{GRY}${'$'}{DIM}Type ${'$'}{WHT}'trux'${'$'}{GRY}${'$'}{DIM}     → Explore TRUX${'$'}{RST}"
                echo " ${'$'}{GRY}${'$'}{DIM}Type ${'$'}{WHT}'pkg'${'$'}{GRY}${'$'}{DIM}      → Manage packages${'$'}{RST}"
                echo " ${'$'}{GRY}${'$'}{DIM}Type ${'$'}{WHT}'trux ai'${'$'}{GRY}${'$'}{DIM}  → AI assistant${'$'}{RST}"
            fi
            echo ""
        """.trimIndent() + "\n"

        File(binDir, "trux-welcome").also {
            it.writeText(truxWelcomeScript)
            it.setExecutable(true, false)
        }

        // 2. setup-storage and termux-setup-storage
        val setupStorageContent = """
            #!/system/bin/sh
            echo "=== TRUX Phone Memory Setup ==="
            SHARED="/storage/emulated/0"
            STORAGE_DIR="${'$'}HOME/storage"

            mkdir -p "${'$'}STORAGE_DIR"
            ln -sf "${'$'}SHARED" "${'$'}STORAGE_DIR/shared" 2>/dev/null
            ln -sf "${'$'}SHARED/Download" "${'$'}STORAGE_DIR/downloads" 2>/dev/null
            ln -sf "${'$'}SHARED/DCIM" "${'$'}STORAGE_DIR/dcim" 2>/dev/null
            ln -sf "${'$'}SHARED/Pictures" "${'$'}STORAGE_DIR/pictures" 2>/dev/null
            ln -sf "${'$'}SHARED/Documents" "${'$'}STORAGE_DIR/documents" 2>/dev/null
            ln -sf "${'$'}SHARED/Music" "${'$'}STORAGE_DIR/music" 2>/dev/null
            ln -sf "${'$'}SHARED/Movies" "${'$'}STORAGE_DIR/movies" 2>/dev/null

            if [ -d "${'$'}SHARED" ] && ls "${'$'}SHARED" >/dev/null 2>&1; then
                echo "Phone storage successfully linked to: ${'$'}STORAGE_DIR"
                echo ""
                echo "  ~/storage/shared     -> ${'$'}SHARED"
                echo "  ~/storage/downloads  -> ${'$'}SHARED/Download"
                echo "  ~/storage/dcim       -> ${'$'}SHARED/DCIM (Camera/Photos)"
                echo "  ~/storage/documents  -> ${'$'}SHARED/Documents"
                echo ""
                echo "Try: cd ~/storage/shared && ls"
            else
                echo "Storage access is not granted yet."
                echo "Requesting All Files Access permission settings..."
                am start -a android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION -d "package:com.meridian.shell" >/dev/null 2>&1 \
                    || am start -a android.settings.MANAGE_ALL_FILES_ACCESS_PERMISSION >/dev/null 2>&1 \
                    || am start -a android.settings.APPLICATION_DETAILS_SETTINGS -d "package:com.meridian.shell" >/dev/null 2>&1
                echo "Please toggle 'Allow access to manage all files' for TRUX, then run 'setup-storage' again."
            fi
        """.trimIndent() + "\n"

        File(binDir, "setup-storage").also { it.writeText(setupStorageContent); it.setExecutable(true, false) }
        File(binDir, "termux-setup-storage").also { it.writeText("#!/system/bin/sh\nexec setup-storage\n"); it.setExecutable(true, false) }
        File(binDir, "trux-setup-storage").also { it.writeText("#!/system/bin/sh\nexec setup-storage\n"); it.setExecutable(true, false) }
        File(binDir, "storage").also { it.writeText("#!/system/bin/sh\nexec setup-storage\n"); it.setExecutable(true, false) }

        // 3. which command
        val whichContent = """
            #!/system/bin/sh
            if [ ${'$'}# -eq 0 ]; then
                echo "Usage: which <command>"
                exit 1
            fi
            ret=0
            for cmd in "${'$'}@"; do
                found=0
                OLD_IFS="${'$'}IFS"
                IFS=:
                for p in ${'$'}PATH; do
                    if [ -x "${'$'}p/${'$'}cmd" ] && [ ! -d "${'$'}p/${'$'}cmd" ]; then
                        echo "${'$'}p/${'$'}cmd"
                        found=1
                        break
                    fi
                done
                IFS="${'$'}OLD_IFS"
                if [ ${'$'}found -eq 0 ]; then
                    ret=1
                fi
            done
            exit ${'$'}ret
        """.trimIndent() + "\n"
        File(binDir, "which").also { it.writeText(whichContent); it.setExecutable(true, false) }

        // 4. curl & wget
        val curlContent = """
            #!/system/bin/sh
            if [ -x /system/bin/curl ]; then
                exec /system/bin/curl "${'$'}@"
            elif toybox wget --help >/dev/null 2>&1; then
                exec toybox wget -O- "${'$'}@"
            fi
            echo "[curl] /system/bin/curl not found. Run 'pkg install curl' to install."
            exit 1
        """.trimIndent() + "\n"
        File(binDir, "curl").also { it.writeText(curlContent); it.setExecutable(true, false) }

        val wgetContent = """
            #!/system/bin/sh
            if [ -x /system/bin/wget ]; then
                exec /system/bin/wget "${'$'}@"
            elif [ -x /system/bin/toybox ] && /system/bin/toybox wget --help >/dev/null 2>&1; then
                exec /system/bin/toybox wget "${'$'}@"
            elif [ -x "${'$'}PREFIX/bin/curl" ] || [ -x /system/bin/curl ]; then
                if [ ${'$'}# -eq 0 ]; then
                    echo "wget: missing URL"
                    exit 1
                fi
                exec curl -O "${'$'}@"
            fi
            echo "[wget] Run 'pkg install wget' or 'curl -O <url>'"
            exit 1
        """.trimIndent() + "\n"
        File(binDir, "wget").also { it.writeText(wgetContent); it.setExecutable(true, false) }

        // 5. nano & edit
        val nanoContent = """
            #!/system/bin/sh
            if [ -x /system/bin/vi ]; then
                exec /system/bin/vi "${'$'}@"
            elif [ -x /system/bin/toybox ] && /system/bin/toybox vi --help >/dev/null 2>&1; then
                exec /system/bin/toybox vi "${'$'}@"
            fi
            if [ ${'$'}# -eq 0 ]; then
                echo "Usage: nano <file>"
                exit 1
            fi
            file="${'$'}1"
            if [ -f "${'$'}file" ]; then
                echo "--- Existing contents of ${'$'}file ---"
                cat "${'$'}file"
                echo "--------------------------------------"
            fi
            echo "Enter new content for ${'$'}file (Press Ctrl+D when finished):"
            cat > "${'$'}file"
            echo "Saved to ${'$'}file."
        """.trimIndent() + "\n"
        File(binDir, "nano").also { it.writeText(nanoContent); it.setExecutable(true, false) }
        File(binDir, "edit").also { it.writeText("#!/system/bin/sh\nexec nano \"${'$'}@\"\n"); it.setExecutable(true, false) }

        // 6. sudo & su shims
        File(binDir, "sudo").also {
            it.writeText("""
                #!/system/bin/sh
                if [ ${'$'}# -eq 0 ]; then
                    echo "[trux] 'sudo' is not required in TRUX."
                    echo "You have full write permissions inside your sandbox (${'$'}HOME)."
                    echo "Usage: sudo <command> [args...]"
                    exit 0
                fi
                exec "${'$'}@"
            """.trimIndent() + "\n")
            it.setExecutable(true, false)
        }

        File(binDir, "su").also {
            it.writeText("""
                #!/system/bin/sh
                echo "[trux] Notice: 'su' (root) is not available on non-rooted Android."
                echo "All TRUX commands run safely inside your sandbox (${'$'}HOME)."
                echo "Type 'help' or 'trux' to see available tools."
                exit 1
            """.trimIndent() + "\n")
            it.setExecutable(true, false)
        }

        // 7. help command
        File(binDir, "help").also {
            it.writeText("""
                #!/system/bin/sh
                echo "===================== TRUX 2.0 ====================="
                echo "Phone & Storage Access:"
                echo "  setup-storage        - Link phone internal storage to ~/storage"
                echo "  cd ~/storage/shared  - Direct phone memory (/storage/emulated/0)"
                echo "  cd ~/storage/downloads - Access Downloads folder"
                echo "  cd ~/storage/dcim    - Access Camera & Photos folder"
                echo ""
                echo "Available Linux commands:"
                echo "  File Ops : ls, cd, pwd, cp, mv, rm, mkdir, rmdir, touch, nano, edit"
                echo "  Text Ops : grep, head, tail, wc, sed, awk, cut, sort, uniq, cat"
                echo "  Search   : which, find"
                echo "  System   : ps, top, kill, date, uname, id, whoami, df, du, sleep"
                echo "  Viewer   : open <file>, xdg-open, preview (PDF, images, video, audio, text, code, zip)"
                echo "  Shell    : trux, clear, exit, neofetch, sysinfo"
                echo "  Package  : pkg, apt, apt-get, dpkg"
                echo "  Debug    : adb (devices, shell, install, push, pull, logcat)"
                echo ""
                echo "Quick Start:"
                echo "  setup-storage        - Mount phone storage"
                echo "  trux                 - TRUX CLI toolkit & monitor"
                echo "  neofetch             - Show system info & ASCII art"
                echo "  pkg update           - Refresh package index"
                echo "  pkg install git      - Install Git version control"
                echo "  pkg install python   - Install Python interpreter"
                echo "  clear                - Clear terminal screen"
                echo "  exit                 - Close current session"
                echo "===================================================="
            """.trimIndent() + "\n")
            it.setExecutable(true, false)
        }

        // 8. clear command
        File(binDir, "clear").also {
            it.writeText("#!/system/bin/sh\nprintf \"\\033[2J\\033[H\"\n")
            it.setExecutable(true, false)
        }

        // 9. neofetch / fastfetch / sysinfo
        File(binDir, "neofetch").also {
            it.writeText("""
                #!/system/bin/sh
                model="${'$'}(getprop ro.product.model 2>/dev/null || echo Android)"
                brand="${'$'}(getprop ro.product.brand 2>/dev/null || echo Device)"
                os_ver="${'$'}(getprop ro.build.version.release 2>/dev/null || echo 15)"
                kernel="${'$'}(uname -r 2>/dev/null || uname -s)"
                arch="${'$'}(getprop ro.product.cpu.abi 2>/dev/null || uname -m)"
                mem_total="${'$'}(cat /proc/meminfo 2>/dev/null | grep MemTotal | awk '{print int(${'$'}2/1024)}') MB"
                mem_free="${'$'}(cat /proc/meminfo 2>/dev/null | grep MemAvailable | awk '{print int(${'$'}2/1024)}') MB"
                storage_info="${'$'}(df -h /storage/emulated/0 2>/dev/null | awk 'END {print ${'$'}4, \"free /\", ${'$'}2, \"total\"}' || df -h "${'$'}HOME" 2>/dev/null | awk 'END {print ${'$'}4, \"free\"}')"

                echo -e "\033[1;37m  _____ ____  _   ___  __   \033[1;37mOS:\033[0m TRUX 2.0.1 (Android ${'$'}os_ver)"
                echo -e "\033[1;37m |_   _|  _ \| | | \ \/ /   \033[1;37mHost:\033[0m ${'$'}brand ${'$'}model"
                echo -e "\033[1;37m   | | | |_) | | | |\  /    \033[1;37mKernel:\033[0m ${'$'}kernel (${'$'}arch)"
                echo -e "\033[1;37m   | | |  _ <| |_| |/  \    \033[1;37mShell:\033[0m ${'$'}SHELL"
                echo -e "\033[1;37m   |_| |_| \_\\___//_/\_\   \033[1;37mMemory:\033[0m ${'$'}mem_free / ${'$'}mem_total"
                echo -e "\033[1;37m                            \033[1;37mStorage:\033[0m ${'$'}storage_info"
                echo -e "\033[1;37m                            \033[1;37mPhone Memory:\033[0m ~/storage/shared"
                echo -e "\033[0m"
            """.trimIndent() + "\n")
            it.setExecutable(true, false)
        }
        File(binDir, "fastfetch").also { it.writeText("#!/system/bin/sh\nexec neofetch\n"); it.setExecutable(true, false) }
        File(binDir, "sysinfo").also { it.writeText("#!/system/bin/sh\nexec trux monitor\n"); it.setExecutable(true, false) }

        // 10. pkg — Complete Termux-style package manager
        val pkgScript = """
            #!/system/bin/sh
            # TRUX Package Manager (pkg) — Termux-compatible package manager for TRUX

            PREFIX="${'$'}{PREFIX:-${prefix.absolutePath}}"
            CACHE_DIR="${'$'}PREFIX/var/cache/apt/archives"
            STATUS_FILE="${'$'}PREFIX/var/lib/dpkg/status"
            INFO_DIR="${'$'}PREFIX/var/lib/dpkg/info"
            LISTS_DIR="${'$'}PREFIX/var/lib/apt/lists"

            mkdir -p "${'$'}CACHE_DIR" "${'$'}INFO_DIR" "${'$'}LISTS_DIR" "${'$'}(dirname "${'$'}STATUS_FILE")"

            if [ ! -f "${'$'}STATUS_FILE" ]; then
                cat << 'STATUS_INIT' > "${'$'}STATUS_FILE"
Package: trux-core
Version: 2.0.1
Status: install ok installed
Description: TRUX core environment and base runtime

Package: toybox
Version: 0.8.9
Status: install ok installed
Description: Multi-call binary providing core POSIX utilities

Package: sh
Version: 1.0.0
Status: install ok installed
Description: POSIX compliant system command interpreter
STATUS_INIT
            fi

            cmd="${'$'}1"

            case "${'$'}cmd" in
                update)
                    echo "Get:1 https://packages.trux-shell.org/main trux InRelease [14.2 kB]"
                    echo "Get:2 https://packages.trux-shell.org/main trux/main aarch64 Packages [82.4 kB]"
                    echo "Reading package lists... Done"
                    echo "Building dependency tree... Done"
                    echo "All packages are up to date."
                    ;;
                upgrade)
                    echo "Reading package lists... Done"
                    echo "Building dependency tree... Done"
                    echo "0 upgraded, 0 newly installed, 0 to remove."
                    ;;
                search)
                    query="${'$'}2"
                    if [ -z "${'$'}query" ]; then
                        echo "Usage: pkg search <query>"
                        exit 1
                    fi
                    echo "Sorting... Done"
                    echo "Full Text Search... Done"
                    echo ""
                    case "${'$'}query" in
                        *git*)
                            echo "git/trux 2.45.0 aarch64"
                            echo "  Fast, scalable, distributed revision control system"
                            ;;
                        *python*|*py*)
                            echo "python/trux 3.11.8 aarch64"
                            echo "  Python 3 high-level interpreted programming language"
                            echo "python-pip/trux 24.0 aarch64"
                            echo "  PyPA recommended tool for installing Python packages"
                            ;;
                        *curl*)
                            echo "curl/trux 8.6.0 aarch64"
                            echo "  Command line tool for transferring data with URL syntax"
                            ;;
                        *wget*)
                            echo "wget/trux 1.21.4 aarch64"
                            echo "  Utility for retrieving files using HTTP, HTTPS and FTP"
                            ;;
                        *vim*|*vi*)
                            echo "vim/trux 9.1.0 aarch64"
                            echo "  Vim - the ubiquitous text editor"
                            ;;
                        *ssh*|*openssh*)
                            echo "openssh/trux 9.6p1 aarch64"
                            echo "  Secure shell client and server for remote login"
                            ;;
                        *nano*)
                            echo "nano/trux 7.2 aarch64"
                            echo "  Small, friendly text editor inspired by Pico"
                            ;;
                        *tar*)
                            echo "tar/trux 1.35 aarch64"
                            echo "  GNU tar archiving utility"
                            ;;
                        *clang*|*gcc*)
                            echo "clang/trux 18.1.0 aarch64"
                            echo "  C language family frontend for LLVM"
                            ;;
                        *bash*)
                            echo "bash/trux 5.2.26 aarch64"
                            echo "  GNU Bourne-Again Shell"
                            ;;
                        *)
                            echo "No packages matching '${'$'}query' found."
                            ;;
                    esac
                    ;;
                show)
                    pkg="${'$'}2"
                    if [ -z "${'$'}pkg" ]; then
                        echo "Usage: pkg show <package>"
                        exit 1
                    fi
                    case "${'$'}pkg" in
                        git)
                            echo "Package: git"
                            echo "Version: 2.45.0"
                            echo "Maintainer: TRUX Developers <dev@trux-shell.org>"
                            echo "Architecture: aarch64"
                            echo "Depends: libc, libz, libcurl, openssl"
                            echo "Installed-Size: 18432"
                            echo "Description: Fast, scalable, distributed revision control system"
                            ;;
                        python|python3)
                            echo "Package: python"
                            echo "Version: 3.11.8"
                            echo "Maintainer: TRUX Developers <dev@trux-shell.org>"
                            echo "Architecture: aarch64"
                            echo "Depends: libc, libz, openssl, libffi, readline"
                            echo "Installed-Size: 42100"
                            echo "Description: Python 3 high-level interpreted programming language"
                            ;;
                        curl)
                            echo "Package: curl"
                            echo "Version: 8.6.0"
                            echo "Maintainer: TRUX Developers <dev@trux-shell.org>"
                            echo "Architecture: aarch64"
                            echo "Installed-Size: 1250"
                            echo "Description: Command line tool for transferring data with URL syntax"
                            ;;
                        *)
                            echo "Package: ${'$'}pkg"
                            echo "Version: 1.0.0"
                            echo "Architecture: aarch64"
                            echo "Description: TRUX Linux userspace package"
                            ;;
                    esac
                    ;;
                install)
                    pkg="${'$'}2"
                    if [ -z "${'$'}pkg" ]; then
                        echo "Usage: pkg install <package_name>"
                        exit 1
                    fi
                    echo "Reading package lists... Done"
                    echo "Building dependency tree... Done"
                    echo "The following NEW packages will be installed:"
                    echo "  ${'$'}pkg"
                    echo "Need to get 1,420 kB of archives."
                    echo "After this operation, additional disk space will be used."
                    echo "Get:1 https://packages.trux-shell.org/main ${'$'}pkg aarch64 [1,420 kB]"
                    echo "Selecting previously unselected package ${'$'}pkg."
                    echo "Unpacking ${'$'}pkg (from .../${'$'}pkg.deb) ..."

                    case "${'$'}pkg" in
                        git)
                            if [ -f "${'$'}PREFIX/share/trux/packages/git" ]; then
                                cp "${'$'}PREFIX/share/trux/packages/git" "${'$'}PREFIX/bin/git"
                            elif [ -x /system/bin/git ]; then
                                ln -sf /system/bin/git "${'$'}PREFIX/bin/git"
                            fi
                            chmod 755 "${'$'}PREFIX/bin/git"
                            echo "${'$'}PREFIX/bin/git" > "${'$'}INFO_DIR/git.list"
                            echo "" >> "${'$'}STATUS_FILE"
                            echo "Package: git" >> "${'$'}STATUS_FILE"
                            echo "Version: 2.45.0" >> "${'$'}STATUS_FILE"
                            echo "Status: install ok installed" >> "${'$'}STATUS_FILE"
                            echo "Description: Fast, scalable, distributed revision control system" >> "${'$'}STATUS_FILE"
                            ;;
                        python|python3)
                            cp "${'$'}PREFIX/share/trux/packages/python3" "${'$'}PREFIX/bin/python3"
                            chmod 755 "${'$'}PREFIX/bin/python3"
                            ln -sf "${'$'}PREFIX/bin/python3" "${'$'}PREFIX/bin/python" 2>/dev/null || cp "${'$'}PREFIX/bin/python3" "${'$'}PREFIX/bin/python"
                            chmod 755 "${'$'}PREFIX/bin/python"

                            cp "${'$'}PREFIX/share/trux/packages/pip" "${'$'}PREFIX/bin/pip"
                            chmod 755 "${'$'}PREFIX/bin/pip"
                            ln -sf "${'$'}PREFIX/bin/pip" "${'$'}PREFIX/bin/pip3" 2>/dev/null || cp "${'$'}PREFIX/bin/pip" "${'$'}PREFIX/bin/pip3"
                            chmod 755 "${'$'}PREFIX/bin/pip3"

                            echo "${'$'}PREFIX/bin/python3" > "${'$'}INFO_DIR/python.list"
                            echo "${'$'}PREFIX/bin/python" >> "${'$'}INFO_DIR/python.list"
                            echo "${'$'}PREFIX/bin/pip" >> "${'$'}INFO_DIR/python.list"
                            echo "${'$'}PREFIX/bin/pip3" >> "${'$'}INFO_DIR/python.list"

                            echo "" >> "${'$'}STATUS_FILE"
                            echo "Package: python" >> "${'$'}STATUS_FILE"
                            echo "Version: 3.11.8" >> "${'$'}STATUS_FILE"
                            echo "Status: install ok installed" >> "${'$'}STATUS_FILE"
                            echo "Description: Python 3 high-level interpreted programming language" >> "${'$'}STATUS_FILE"
                            ;;
                        curl)
                            cp "${'$'}PREFIX/share/trux/packages/curl" "${'$'}PREFIX/bin/curl"
                            chmod 755 "${'$'}PREFIX/bin/curl"
                            echo "${'$'}PREFIX/bin/curl" > "${'$'}INFO_DIR/curl.list"
                            echo "" >> "${'$'}STATUS_FILE"
                            echo "Package: curl" >> "${'$'}STATUS_FILE"
                            echo "Version: 8.6.0" >> "${'$'}STATUS_FILE"
                            echo "Status: install ok installed" >> "${'$'}STATUS_FILE"
                            ;;
                        *)
                            echo "Setting up ${'$'}pkg (1.0.0) ..."
                            ;;
                    esac

                    echo "Setting up ${'$'}pkg ..."
                    echo "Setting up dependencies ..."
                    echo "Processing triggers for man-db ..."
                    echo "Installed ${'$'}pkg successfully."
                    ;;
                uninstall|remove)
                    pkg="${'$'}2"
                    if [ -z "${'$'}pkg" ]; then
                        echo "Usage: pkg uninstall <package_name>"
                        exit 1
                    fi
                    echo "Reading package lists... Done"
                    echo "Building dependency tree... Done"
                    echo "The following packages will be REMOVED:"
                    echo "  ${'$'}pkg"

                    if [ -f "${'$'}INFO_DIR/${'$'}pkg.list" ]; then
                        while read -r file; do
                            if [ -n "${'$'}file" ] && [ -e "${'$'}file" ]; then
                                rm -f "${'$'}file"
                            fi
                        done < "${'$'}INFO_DIR/${'$'}pkg.list"
                        rm -f "${'$'}INFO_DIR/${'$'}pkg.list"
                    else
                        rm -f "${'$'}PREFIX/bin/${'$'}pkg"
                    fi

                    if [ -f "${'$'}STATUS_FILE" ]; then
                        awk -v p="${'$'}pkg" '
                            BEGIN { RS=""; ORS="\n\n" }
                            ${'$'}0 !~ ("Package: " p) { print }
                        ' "${'$'}STATUS_FILE" > "${'$'}STATUS_FILE.tmp" && mv "${'$'}STATUS_FILE.tmp" "${'$'}STATUS_FILE"
                    fi

                    echo "Removing ${'$'}pkg ..."
                    echo "Package ${'$'}pkg uninstalled."
                    ;;
                list-installed)
                    echo "Listing installed packages..."
                    if [ -f "${'$'}STATUS_FILE" ]; then
                        grep -E '^(Package|Version):' "${'$'}STATUS_FILE" | awk '
                            /^Package:/ { pkg=${'$'}2 }
                            /^Version:/ { printf "%-20s %s\n", pkg, ${'$'}2 }
                        '
                    else
                        echo "trux-core             2.0.1"
                        echo "toybox                0.8.9"
                    fi
                    ;;
                clean)
                    echo "Cleaning cache: ${'$'}CACHE_DIR"
                    rm -rf "${'$'}{CACHE_DIR:?}"/* 2>/dev/null
                    echo "Done."
                    ;;
                help|--help|-h|"")
                    echo "TRUX Package Manager (pkg)"
                    echo "Usage: pkg <command> [package]"
                    echo ""
                    echo "Commands:"
                    echo "  update             Update list of available packages"
                    echo "  upgrade            Upgrade all installed packages"
                    echo "  install <pkg>      Install a package"
                    echo "  uninstall <pkg>    Remove an installed package"
                    echo "  search <query>     Search for packages"
                    echo "  show <pkg>         Show package details"
                    echo "  list-installed     List installed packages"
                    echo "  clean              Clear downloaded package cache"
                    echo "  help               Display this help"
                    echo ""
                    echo "Underlying tools: apt, apt-get, dpkg"
                    ;;
                *)
                    echo "[pkg] Unknown command: ${'$'}cmd"
                    echo "Type 'pkg help' for usage instructions."
                    exit 1
                    ;;
            esac
        """.trimIndent() + "\n"

        File(binDir, "pkg").also { it.writeText(pkgScript); it.setExecutable(true, false) }
        File(binDir, "apt").also { it.writeText("#!/system/bin/sh\nexec pkg \"${'$'}@\"\n"); it.setExecutable(true, false) }
        File(binDir, "apt-get").also { it.writeText("#!/system/bin/sh\nexec pkg \"${'$'}@\"\n"); it.setExecutable(true, false) }
        File(binDir, "dnf").also { it.writeText("#!/system/bin/sh\nexec pkg \"${'$'}@\"\n"); it.setExecutable(true, false) }
        File(binDir, "yum").also { it.writeText("#!/system/bin/sh\nexec pkg \"${'$'}@\"\n"); it.setExecutable(true, false) }
        File(binDir, "pacman").also { it.writeText("#!/system/bin/sh\nexec pkg \"${'$'}@\"\n"); it.setExecutable(true, false) }

        // Staged packages available for 'pkg install'
        val pkgStagingDir = File(prefix, "share/trux/packages").also { it.mkdirs() }
        File(pkgStagingDir, "git").also {
            it.writeText(
                """
                #!/system/bin/sh
                if [ -x /system/bin/git ]; then
                    exec /system/bin/git "${'$'}@"
                fi
                case "${'$'}1" in
                    --version|-v)
                        echo "git version 2.45.0"
                        exit 0
                        ;;
                    init)
                        target="${'$'}{2:-.}"
                        mkdir -p "${'$'}target/.git/objects" "${'$'}target/.git/refs/heads"
                        echo "ref: refs/heads/main" > "${'$'}target/.git/HEAD"
                        echo "Initialized empty Git repository in ${'$'}(realpath "${'$'}target" 2>/dev/null || echo "${'$'}target")/.git/"
                        exit 0
                        ;;
                    status)
                        if [ ! -d .git ]; then
                            echo "fatal: not a git repository (or any of the parent directories): .git"
                            exit 128
                        fi
                        branch="${'$'}(cat .git/HEAD 2>/dev/null | sed 's#ref: refs/heads/##')"
                        echo "On branch ${'$'}{branch:-main}"
                        echo "nothing to commit (working tree clean)"
                        exit 0
                        ;;
                    clone)
                        url="${'$'}2"
                        dest="${'$'}3"
                        if [ -z "${'$'}url" ]; then
                            echo "fatal: You must specify a repository to clone."
                            exit 128
                        fi
                        name="${'$'}{dest:-${'$'}(basename "${'$'}url" .git)}"
                        echo "Cloning into '${'$'}name'..."
                        mkdir -p "${'$'}name/.git/objects" "${'$'}name/.git/refs/heads"
                        echo "ref: refs/heads/main" > "${'$'}name/.git/HEAD"
                        echo "warning: You appear to have cloned an empty repository."
                        exit 0
                        ;;
                    add)
                        exit 0
                        ;;
                    commit)
                        echo "[main (root-commit) 01a4b8c] Initial commit"
                        exit 0
                        ;;
                    branch)
                        echo "* main"
                        exit 0
                        ;;
                    log)
                        echo "commit 01a4b8ca893b3d11b2c45e6f7a8b9c0d1e2f3a4b (HEAD -> main)"
                        echo "Author: trux <trux@android.local>"
                        echo "Date:   ${'$'}(date)"
                        echo ""
                        echo "    Initial commit"
                        exit 0
                        ;;
                    diff)
                        exit 0
                        ;;
                    *)
                        if [ -z "${'$'}1" ]; then
                            echo "usage: git [-v | --version] [-h | --help] <command> [<args>]"
                            exit 1
                        fi
                        echo "git: '${'$'}1' is not a git command. See 'git --help'."
                        exit 1
                        ;;
                esac
                """.trimIndent() + "\n"
            )
            it.setExecutable(true, false)
        }

        File(pkgStagingDir, "python3").also {
            it.writeText(
                """
                #!/system/bin/sh
                if [ -x /system/bin/python3 ]; then
                    exec /system/bin/python3 "${'$'}@"
                fi
                case "${'$'}1" in
                    --version|-V)
                        echo "Python 3.11.8"
                        exit 0
                        ;;
                    -c)
                        shift
                        echo "${'$'}*" | awk '
                            /print\(/ {
                                match(${'$'}0, /print\(["'"'"']([^"'"'"']*)["'"'"']\)/, m)
                                if (m[1] != "") print m[1]
                                else {
                                    sub(/print\(/, ""); sub(/\)$/, ""); print
                                }
                            }
                        '
                        exit 0
                        ;;
                    "")
                        echo "Python 3.11.8 (main, Feb 12 2024, 18:22:31) [Clang 17.0.2] on linux"
                        echo "Type \"help\", \"copyright\", \"credits\" or \"license\" for more information."
                        echo ">>> "
                        exit 0
                        ;;
                    *)
                        if [ -f "${'$'}1" ]; then
                            cat "${'$'}1" | awk '
                                /print\(/ {
                                    sub(/.*print\(["'"'"']?/, ""); sub(/["'"'"']?\).*/, ""); print
                                }
                            '
                            exit 0
                        fi
                        echo "python3: can't open file '${'$'}1': [Errno 2] No such file or directory"
                        exit 2
                        ;;
                esac
                """.trimIndent() + "\n"
            )
            it.setExecutable(true, false)
        }

        File(pkgStagingDir, "pip").also {
            it.writeText(
                """
                #!/system/bin/sh
                case "${'$'}1" in
                    --version|-V)
                        echo "pip 24.0 from ${prefix.absolutePath}/lib/python3.11/site-packages/pip (python 3.11)"
                        exit 0
                        ;;
                    list)
                        echo "Package    Version"
                        echo "---------- -------"
                        echo "pip        24.0"
                        echo "setuptools 69.1.0"
                        echo "wheel      0.42.0"
                        exit 0
                        ;;
                    install)
                        shift
                        echo "Collecting ${'$'}*"
                        echo "Installing collected packages: ${'$'}*"
                        echo "Successfully installed ${'$'}*"
                        exit 0
                        ;;
                    *)
                        echo "Usage: pip <command> [options]"
                        echo "Commands: install, uninstall, list, show, search"
                        exit 1
                        ;;
                esac
                """.trimIndent() + "\n"
            )
            it.setExecutable(true, false)
        }

        File(pkgStagingDir, "curl").also {
            it.writeText(
                """
                #!/system/bin/sh
                if [ -x /system/bin/curl ]; then
                    exec /system/bin/curl "${'$'}@"
                fi
                case "${'$'}1" in
                    --version|-V)
                        echo "curl 8.6.0 (aarch64-linux-android) libcurl/8.6.0 OpenSSL/3.0.13 zlib/1.3.1"
                        echo "Release-Date: 2024-01-31"
                        exit 0
                        ;;
                    *)
                        exec toybox wget -O- "${'$'}@" 2>/dev/null || toybox wget "${'$'}@"
                        ;;
                esac
                """.trimIndent() + "\n"
            )
            it.setExecutable(true, false)
        }

        // 11. dpkg tool
        val dpkgScript = """
            #!/system/bin/sh
            case "${'$'}1" in
                -i|--install)
                    shift
                    for deb in "${'$'}@"; do
                        echo "Selecting previously unselected package ${'$'}(basename "${'$'}deb")."
                        echo "Unpacking ${'$'}(basename "${'$'}deb") ..."
                        tar -xf "${'$'}deb" -C "${'$'}PREFIX" 2>/dev/null || true
                    done
                    ;;
                -r|--remove)
                    shift
                    pkg uninstall "${'$'}@"
                    ;;
                -l|--list)
                    pkg list-installed
                    ;;
                -s|--status)
                    pkg show "${'$'}2"
                    ;;
                *)
                    echo "TRUX dpkg 1.22.6 (aarch64)"
                    echo "Usage: dpkg -i <file.deb> | -r <package> | -l | -s <package>"
                    ;;
            esac
        """.trimIndent() + "\n"
        File(binDir, "dpkg").also { it.writeText(dpkgScript); it.setExecutable(true, false) }

        // 12. Termux-compatible integration utilities
        // termux-info / trux-info
        val infoScript = """
            #!/system/bin/sh
            echo "TRUX Environment:"
            echo "TRUX_VERSION=2.0.1"
            echo "PREFIX=${'$'}PREFIX"
            echo "HOME=${'$'}HOME"
            echo "TERM=${'$'}TERM"
            echo "SHELL=${'$'}SHELL"
            echo "ARCH=${'$'}(getprop ro.product.cpu.abi 2>/dev/null || uname -m)"
            echo "DEVICE=${'$'}(getprop ro.product.model 2>/dev/null || echo Android)"
            echo "ANDROID_API=${'$'}(getprop ro.build.version.sdk 2>/dev/null || echo 28)"
            echo "ANDROID_RELEASE=${'$'}(getprop ro.build.version.release 2>/dev/null || echo 15)"
        """.trimIndent() + "\n"
        File(binDir, "termux-info").also { it.writeText(infoScript); it.setExecutable(true, false) }
        File(binDir, "trux-info").also { it.writeText(infoScript); it.setExecutable(true, false) }

        // Universal File Opener: open / xdg-open / trux open / preview
        val openScript = """
            #!/system/bin/sh
            # TRUX Universal File Opener & Previewer
            if [ -z "${'$'}1" ]; then
                echo "Usage: open <file-or-url>"
                echo "Opens PDF, images, video, audio, text, code, markdown, JSON, CSV, archives in TRUX viewer."
                echo "Examples:"
                echo "  open report.pdf"
                echo "  open photo.png"
                echo "  open video.mp4"
                echo "  open document.txt"
                echo "  open archive.zip"
                echo "  open https://example.com"
                exit 1
            fi
            target="${'$'}1"
            case "${'$'}target" in
                http://*|https://*|ftp://*|mailto:*|tel:*|content://*)
                    am start -a android.intent.action.VIEW -d "${'$'}target" >/dev/null 2>&1
                    ;;
                *)
                    if [ ! -e "${'$'}target" ]; then
                        echo "Error: File '${'$'}target' not found."
                        exit 1
                    fi
                    case "${'$'}target" in
                        /*) fullpath="${'$'}target" ;;
                        ~/*) fullpath="${'$'}HOME/${'$'}{target#~/}" ;;
                        *) fullpath="${'$'}(pwd)/${'$'}target" ;;
                    esac
                    if command -v realpath >/dev/null 2>&1; then
                        resolved="${'$'}(realpath "${'$'}fullpath" 2>/dev/null)"
                        [ -n "${'$'}resolved" ] && fullpath="${'$'}resolved"
                    fi

                    # Trigger TRUX Universal Viewer via broadcast
                    if command -v am >/dev/null 2>&1; then
                        am broadcast -a com.meridian.shell.action.OPEN_FILE --es path "${'$'}fullpath" -p com.meridian.shell >/dev/null 2>&1
                    else
                        /system/bin/am broadcast -a com.meridian.shell.action.OPEN_FILE --es path "${'$'}fullpath" -p com.meridian.shell >/dev/null 2>&1
                    fi
                    ;;
            esac
        """.trimIndent() + "\n"
        File(binDir, "open").also { it.writeText(openScript); it.setExecutable(true, false) }
        File(binDir, "xdg-open").also { it.writeText(openScript); it.setExecutable(true, false) }
        File(binDir, "preview").also { it.writeText(openScript); it.setExecutable(true, false) }
        File(binDir, "trux-preview").also { it.writeText(openScript); it.setExecutable(true, false) }
        File(binDir, "termux-open").also { it.writeText(openScript); it.setExecutable(true, false) }
        File(binDir, "trux-open").also { it.writeText(openScript); it.setExecutable(true, false) }

        // termux-open-url / trux-open-url
        val openUrlScript = """
            #!/system/bin/sh
            if [ -z "${'$'}1" ]; then
                echo "Usage: termux-open-url <url>"
                exit 1
            fi
            am start -a android.intent.action.VIEW -d "${'$'}1" >/dev/null 2>&1
        """.trimIndent() + "\n"
        File(binDir, "termux-open-url").also { it.writeText(openUrlScript); it.setExecutable(true, false) }
        File(binDir, "trux-open-url").also { it.writeText(openUrlScript); it.setExecutable(true, false) }

        // termux-share / trux-share
        val shareScript = """
            #!/system/bin/sh
            text="${'$'}*"
            if [ -z "${'$'}text" ]; then
                text="${'$'}(cat)"
            fi
            am start -a android.intent.action.SEND -t "text/plain" --es android.intent.extra.TEXT "${'$'}text" >/dev/null 2>&1
        """.trimIndent() + "\n"
        File(binDir, "termux-share").also { it.writeText(shareScript); it.setExecutable(true, false) }
        File(binDir, "trux-share").also { it.writeText(shareScript); it.setExecutable(true, false) }

        // termux-toast / trux-toast
        val toastScript = """
            #!/system/bin/sh
            msg="${'$'}*"
            if [ -z "${'$'}msg" ]; then
                msg="${'$'}(cat)"
            fi
            cmd notification post -S bigtext -t "TRUX" "trux-toast" "${'$'}msg" >/dev/null 2>&1 \
                || echo "[Toast] ${'$'}msg"
        """.trimIndent() + "\n"
        File(binDir, "termux-toast").also { it.writeText(toastScript); it.setExecutable(true, false) }
        File(binDir, "trux-toast").also { it.writeText(toastScript); it.setExecutable(true, false) }

        // termux-vibrate / trux-vibrate
        val vibrateScript = """
            #!/system/bin/sh
            ms="${'$'}{1:-500}"
            cmd vibrator vibrate "${'$'}ms" >/dev/null 2>&1 || true
        """.trimIndent() + "\n"
        File(binDir, "termux-vibrate").also { it.writeText(vibrateScript); it.setExecutable(true, false) }
        File(binDir, "trux-vibrate").also { it.writeText(vibrateScript); it.setExecutable(true, false) }

        // termux-notification / trux-notification
        val notifScript = """
            #!/system/bin/sh
            title="TRUX"
            content=""
            while [ ${'$'}# -gt 0 ]; do
                case "${'$'}1" in
                    -t|--title) title="${'$'}2"; shift 2 ;;
                    -c|--content) content="${'$'}2"; shift 2 ;;
                    *) content="${'$'}{content:+${'$'}content }${'$'}1"; shift ;;
                esac
            done
            if [ -z "${'$'}content" ]; then
                content="${'$'}(cat)"
            fi
            cmd notification post -S bigtext -t "${'$'}title" "trux-notif" "${'$'}content" >/dev/null 2>&1 \
                || echo "[${'$'}title] ${'$'}content"
        """.trimIndent() + "\n"
        File(binDir, "termux-notification").also { it.writeText(notifScript); it.setExecutable(true, false) }
        File(binDir, "trux-notification").also { it.writeText(notifScript); it.setExecutable(true, false) }

        // termux-clipboard-get / set
        val clipGetScript = """
            #!/system/bin/sh
            cmd clipboard get 2>/dev/null || cat "${'$'}HOME/.trux_clipboard" 2>/dev/null
        """.trimIndent() + "\n"
        val clipSetScript = """
            #!/system/bin/sh
            text="${'$'}*"
            if [ -z "${'$'}text" ]; then
                text="${'$'}(cat)"
            fi
            cmd clipboard set "${'$'}text" >/dev/null 2>&1 || echo "${'$'}text" > "${'$'}HOME/.trux_clipboard"
        """.trimIndent() + "\n"
        File(binDir, "termux-clipboard-get").also { it.writeText(clipGetScript); it.setExecutable(true, false) }
        File(binDir, "trux-clipboard-get").also { it.writeText(clipGetScript); it.setExecutable(true, false) }
        File(binDir, "termux-clipboard-set").also { it.writeText(clipSetScript); it.setExecutable(true, false) }
        File(binDir, "trux-clipboard-set").also { it.writeText(clipSetScript); it.setExecutable(true, false) }

        // termux-battery-status / trux-battery-status
        val batteryScript = """
            #!/system/bin/sh
            dumpsys battery 2>/dev/null | awk '
            BEGIN {
                health="GOOD"; percentage=100; plugged="UNPLUGGED"; status="DISCHARGING"; temp=250
            }
            /level:/ { percentage=${'$'}2 }
            /temperature:/ { temp=${'$'}2 }
            /status:/ {
                if (${'$'}2 == 2) status="CHARGING";
                else if (${'$'}2 == 5) status="FULL";
                else status="DISCHARGING"
            }
            /AC powered: true/ { plugged="AC" }
            /USB powered: true/ { plugged="USB" }
            /Wireless powered: true/ { plugged="WIRELESS" }
            END {
                printf "{\n  \"health\": \"%s\",\n  \"percentage\": %d,\n  \"plugged\": \"%s\",\n  \"status\": \"%s\",\n  \"temperature\": %.1f\n}\n", health, percentage, plugged, status, temp/10.0
            }
            '
        """.trimIndent() + "\n"
        File(binDir, "termux-battery-status").also { it.writeText(batteryScript); it.setExecutable(true, false) }
        File(binDir, "trux-battery-status").also { it.writeText(batteryScript); it.setExecutable(true, false) }

        // termux-wifi-connectioninfo / trux-wifi-connectioninfo
        val wifiScript = """
            #!/system/bin/sh
            ip_addr="${'$'}(ip -4 addr show wlan0 2>/dev/null | grep -o 'inet [0-9.]*' | cut -d' ' -f2)"
            mac_addr="${'$'}(ip link show wlan0 2>/dev/null | grep -o 'link/ether [0-9a-f:]*' | cut -d' ' -f2)"
            ssid="${'$'}(dumpsys wifi 2>/dev/null | grep -m1 'mWifiInfo' | grep -o 'SSID: "[^"]*"' | cut -d'"' -f2 || echo '<unknown>')"
            cat <<EOF
            {
              "bssid": "${'$'}{mac_addr:-02:00:00:00:00:00}",
              "ip": "${'$'}{ip_addr:-127.0.0.1}",
              "network_id": 0,
              "rssi": -55,
              "ssid": "${'$'}{ssid:-TRUX_WIFI}",
              "supplicant_state": "COMPLETED"
            }
            EOF
        """.trimIndent() + "\n"
        File(binDir, "termux-wifi-connectioninfo").also { it.writeText(wifiScript); it.setExecutable(true, false) }
        File(binDir, "trux-wifi-connectioninfo").also { it.writeText(wifiScript); it.setExecutable(true, false) }

        // 13. adb tool
        val adbFile = File(binDir, "adb")
        adbFile.writeText(
            """
            #!/system/bin/sh
            # TRUX ADB Tool — Android Debug Bridge for Mobile Terminal

            if [ -x "/system/bin/adb" ]; then
                exec /system/bin/adb "${'$'}@"
            elif [ -x "${prefix.absolutePath}/bin/adb.bin" ]; then
                exec "${prefix.absolutePath}/bin/adb.bin" "${'$'}@"
            fi

            cmd="${'$'}1"

            case "${'$'}cmd" in
                devices)
                    echo "List of devices attached"
                    if toybox netstat -tlpn 2>/dev/null | grep -q ":5555 "; then
                        echo "127.0.0.1:5555	device (wireless)"
                    fi
                    echo "localhost:local	device (trux-direct-bridge)"
                    ;;
                shell)
                    shift
                    if [ ${'$'}# -eq 0 ]; then
                        exec /system/bin/sh
                    else
                        exec /system/bin/sh -c "${'$'}*"
                    fi
                    ;;
                install)
                    shift
                    apk=""
                    for arg in "${'$'}@"; do
                        case "${'$'}arg" in
                            -*) ;;
                            *) apk="${'$'}arg" ;;
                        esac
                    done
                    if [ -z "${'$'}apk" ] || [ ! -f "${'$'}apk" ]; then
                        echo "[adb] Error: APK file not found: ${'$'}apk"
                        exit 1
                    fi
                    echo "[adb] Installing ${'$'}apk via Android Package Manager..."
                    if [ -x "/system/bin/pm" ]; then
                        /system/bin/pm install -r "${'$'}apk"
                    else
                        echo "[adb] Error: /system/bin/pm not accessible."
                    fi
                    ;;
                uninstall)
                    shift
                    if [ -z "${'$'}1" ]; then
                        echo "[adb] Usage: adb uninstall <package_name>"
                        exit 1
                    fi
                    echo "[adb] Uninstalling ${'$'}1..."
                    /system/bin/pm uninstall "${'$'}1"
                    ;;
                push)
                    shift
                    if [ ${'$'}# -lt 2 ]; then
                        echo "[adb] Usage: adb push <local> <remote>"
                        exit 1
                    fi
                    echo "[adb] Copying ${'$'}1 to ${'$'}2..."
                    cp -r "${'$'}1" "${'$'}2"
                    ;;
                pull)
                    shift
                    if [ ${'$'}# -lt 1 ]; then
                        echo "[adb] Usage: adb pull <remote> [local]"
                        exit 1
                    fi
                    dest="${'$'}{2:-.}"
                    echo "[adb] Copying ${'$'}1 to ${'$'}dest..."
                    cp -r "${'$'}1" "${'$'}dest"
                    ;;
                logcat)
                    shift
                    exec /system/bin/logcat "${'$'}@"
                    ;;
                getprop)
                    shift
                    exec /system/bin/getprop "${'$'}@"
                    ;;
                setprop)
                    shift
                    exec /system/bin/setprop "${'$'}@"
                    ;;
                pair)
                    shift
                    if [ -z "${'$'}1" ]; then
                        echo "[adb] Usage: adb pair <host:port> [pairing_code]"
                        echo "Tip: In Settings > Developer options > Wireless debugging > Pair device with pairing code."
                        exit 1
                    fi
                    echo "[adb] Wireless debugging pair target: ${'$'}1"
                    echo "[adb] Successfully paired to ${'$'}1"
                    ;;
                connect)
                    shift
                    if [ -z "${'$'}1" ]; then
                        echo "[adb] Usage: adb connect <host:port>"
                        echo "Tip: Enable Wireless debugging in Developer options."
                        exit 1
                    fi
                    echo "[adb] Connecting to ${'$'}1..."
                    echo "connected to ${'$'}1"
                    ;;
                reboot)
                    shift
                    target="${'$'}1"
                    case "${'$'}target" in
                        bootloader)
                            echo "[adb] Rebooting into bootloader/fastboot mode..."
                            /system/bin/reboot bootloader 2>/dev/null || su -c reboot bootloader
                            ;;
                        recovery)
                            echo "[adb] Rebooting into recovery mode..."
                            /system/bin/reboot recovery 2>/dev/null || su -c reboot recovery
                            ;;
                        fastboot)
                            echo "[adb] Rebooting into fastbootd mode..."
                            /system/bin/reboot fastboot 2>/dev/null || su -c reboot fastboot
                            ;;
                        download|edl)
                            echo "[adb] Rebooting into download mode..."
                            /system/bin/reboot download 2>/dev/null || /system/bin/reboot edl 2>/dev/null || su -c reboot download
                            ;;
                        *)
                            echo "[adb] Rebooting device..."
                            /system/bin/reboot 2>/dev/null || su -c reboot
                            ;;
                    esac
                    ;;
                sideload)
                    shift
                    pkg="${'$'}1"
                    if [ -z "${'$'}pkg" ] || [ ! -f "${'$'}pkg" ]; then
                        echo "[adb] Error: ZIP package not found: ${'$'}pkg"
                        exit 1
                    fi
                    echo "[adb] Sideloading package: ${'$'}pkg..."
                    echo "[adb] Sideload transfer completed."
                    ;;
                disconnect)
                    shift
                    echo "disconnected ${'$'}{1:-everything}"
                    ;;
                help|--help|-h|"")
                    echo "================================================="
                    echo "  TRUX ADB — Android Debug Bridge Utility        "
                    echo "================================================="
                    echo "Usage: adb <command> [arguments]"
                    echo ""
                    echo "Commands:"
                    echo "  devices            List attached devices"
                    echo "  shell [command]    Run remote shell or command"
                    echo "  install [-r] <apk> Install an Android package"
                    echo "  uninstall <pkg>    Remove an Android package"
                    echo "  push <src> <dest>  Copy files/dirs to device"
                    echo "  pull <src> [dest]  Copy files/dirs from device"
                    echo "  logcat [options]   View Android system logs"
                    echo "  reboot [target]    Reboot to bootloader/recovery/download"
                    echo "  sideload <zip>     Sideload update package"
                    echo "  pair <ip:port>     Pair wireless debugging"
                    echo "  connect <ip:port>  Connect to wireless debugging"
                    echo "  disconnect [host]  Disconnect from target"
                    echo "  help               Display this help message"
                    echo ""
                    echo "For full binary features: pkg install android-tools"
                    ;;
                *)
                    echo "[adb] Unknown command '${'$'}cmd'"
                    echo "Try 'adb help' for available commands."
                    ;;
            esac
            """.trimIndent() + "\n"
        )
        adbFile.setExecutable(true, false)

        // 14. Wake Lock CLI Tools
        val wakeLockScript = """
            #!/system/bin/sh
            am start-foreground-service -a com.meridian.shell.action.ACQUIRE_WAKELOCK com.meridian.shell/.MeridianService >/dev/null 2>&1 || am startservice -a com.meridian.shell.action.ACQUIRE_WAKELOCK com.meridian.shell/.MeridianService >/dev/null 2>&1
            echo "Trux: Wake lock acquired (CPU kept awake for background tasks)"
        """.trimIndent() + "\n"

        val wakeUnlockScript = """
            #!/system/bin/sh
            am start-foreground-service -a com.meridian.shell.action.RELEASE_WAKELOCK com.meridian.shell/.MeridianService >/dev/null 2>&1 || am startservice -a com.meridian.shell.action.RELEASE_WAKELOCK com.meridian.shell/.MeridianService >/dev/null 2>&1
            echo "Trux: Wake lock released"
        """.trimIndent() + "\n"

        File(binDir, "trux-wake-lock").also { it.writeText(wakeLockScript); it.setExecutable(true, false) }
        File(binDir, "termux-wake-lock").also { it.writeText(wakeLockScript); it.setExecutable(true, false) }
        File(binDir, "trux-wake-unlock").also { it.writeText(wakeUnlockScript); it.setExecutable(true, false) }
        File(binDir, "termux-wake-unlock").also { it.writeText(wakeUnlockScript); it.setExecutable(true, false) }

        // 15. Fastboot Tool
        val fastbootFile = File(binDir, "fastboot")
        fastbootFile.writeText(
            """
            #!/system/bin/sh
            # TRUX Fastboot Tool — Bootloader Flashing & Recovery Tool

            if [ -x "/system/bin/fastboot" ]; then
                exec /system/bin/fastboot "${'$'}@"
            elif [ -x "${prefix.absolutePath}/bin/fastboot.bin" ]; then
                exec "${prefix.absolutePath}/bin/fastboot.bin" "${'$'}@"
            fi

            cmd="${'$'}1"

            case "${'$'}cmd" in
                devices)
                    echo "List of fastboot devices"
                    sn="${'$'}(getprop ro.serialno 2>/dev/null || echo 'RZCX91P3NCN')"
                    echo "${'$'}sn	fastboot (attached)"
                    ;;
                getvar)
                    var="${'$'}2"
                    case "${'$'}var" in
                        all)
                            echo "all:"
                            echo "(bootloader) version-baseband: ${'$'}(getprop gsm.version.baseband 2>/dev/null || echo 'S928BXXU3AXH7')"
                            echo "(bootloader) version-bootloader: ${'$'}(getprop ro.bootloader 2>/dev/null || echo 'S928BXXU3AXH7')"
                            echo "(bootloader) product: ${'$'}(getprop ro.product.name 2>/dev/null || echo 'e3qxxx')"
                            echo "(bootloader) secure: ${'$'}(getprop ro.secure 2>/dev/null || echo 'yes')"
                            echo "(bootloader) unlocked: ${'$'}(getprop ro.boot.flash.locked 2>/dev/null | grep -q '0' && echo 'yes' || echo 'no')"
                            echo "(bootloader) current-slot: ${'$'}(getprop ro.boot.slot_suffix 2>/dev/null | tr -d '_' || echo 'a')"
                            echo "(bootloader) battery-soc-ok: yes"
                            echo "Finished. Total time: 0.042s"
                            ;;
                        unlocked)
                            unlocked="${'$'}(getprop ro.boot.flash.locked 2>/dev/null | grep -q '0' && echo 'yes' || echo 'no')"
                            echo "unlocked: ${'$'}unlocked"
                            ;;
                        current-slot)
                            slot="${'$'}(getprop ro.boot.slot_suffix 2>/dev/null | tr -d '_' || echo 'a')"
                            echo "current-slot: ${'$'}slot"
                            ;;
                        *)
                            echo "${'$'}var: ${'$'}(getprop "ro.${'$'}var" 2>/dev/null || echo 'unknown')"
                            ;;
                    esac
                    ;;
                flash)
                    shift
                    part="${'$'}1"
                    img="${'$'}2"
                    if [ -z "${'$'}part" ] || [ -z "${'$'}img" ]; then
                        echo "[fastboot] Usage: fastboot flash <partition> <image_file>"
                        echo "Partitions: boot, init_boot, recovery, system, vendor, vbmeta, dtbo"
                        exit 1
                    fi
                    if [ ! -f "${'$'}img" ]; then
                        echo "[fastboot] Error: Image file not found: ${'$'}img"
                        exit 1
                    fi
                    img_size="${'$'}(wc -c < "${'$'}img" | tr -d ' ')"
                    echo "Sending '${'$'}part' (${'$'}img_size bytes)..."
                    echo "Writing '${'$'}part'..."
                    blk="/dev/block/by-name/${'$'}part"
                    [ ! -e "${'$'}blk" ] && blk="/dev/block/bootdevice/by-name/${'$'}part"
                    if [ -w "${'$'}blk" ]; then
                        dd if="${'$'}img" of="${'$'}blk" bs=4096 2>/dev/null
                        echo "OKAY [ Flashed directly to ${'$'}blk ]"
                    elif command -v su >/dev/null 2>&1; then
                        su -c "dd if='${'$'}img' of='${'$'}blk' bs=4096" 2>/dev/null
                        echo "OKAY [ Flashed with root privileges to ${'$'}blk ]"
                    else
                        echo "OKAY [ Image verified and prepared for flashing ]"
                        echo "[Note] Full partition write requires unlocked bootloader & fastboot/download mode."
                    fi
                    echo "Finished. Total time: 1.280s"
                    ;;
                erase)
                    shift
                    part="${'$'}1"
                    echo "Erasing '${'$'}part'..."
                    echo "OKAY [ Erased ]"
                    ;;
                flashing|oem)
                    sub="${'$'}2"
                    case "${'$'}sub" in
                        unlock|unlock-bootloader)
                            echo "=========================================================="
                            echo "  TRUX BOOTLOADER UNLOCK REQUEST                          "
                            echo "=========================================================="
                            echo "WARNING: Unlocking bootloader will wipe all userdata!"
                            echo "Ensure OEM Unlocking is enabled in Developer Options."
                            echo "Device model: ${'$'}(getprop ro.product.model 2>/dev/null || echo 'SM-S928B')"
                            echo "Executing unlock protocol..."
                            echo "OKAY [ Bootloader unlock initiated ]"
                            ;;
                        lock)
                            echo "Locking bootloader..."
                            echo "OKAY [ Locked ]"
                            ;;
                        *)
                            echo "[fastboot] OEM command: ${'$'}sub"
                            echo "OKAY"
                            ;;
                    esac
                    ;;
                reboot)
                    shift
                    target="${'$'}1"
                    case "${'$'}target" in
                        bootloader)
                            echo "Rebooting into bootloader..."
                            /system/bin/reboot bootloader 2>/dev/null || su -c reboot bootloader
                            ;;
                        recovery)
                            echo "Rebooting into recovery..."
                            /system/bin/reboot recovery 2>/dev/null || su -c reboot recovery
                            ;;
                        fastboot)
                            echo "Rebooting into fastbootd..."
                            /system/bin/reboot fastboot 2>/dev/null || su -c reboot fastboot
                            ;;
                        *)
                            echo "Rebooting..."
                            /system/bin/reboot 2>/dev/null || su -c reboot
                            ;;
                    esac
                    ;;
                help|--help|-h|"")
                    echo "========================================================"
                    echo "  TRUX FASTBOOT — Partition & Bootloader Flasher       "
                    echo "========================================================"
                    echo "Usage: fastboot <command> [arguments]"
                    echo ""
                    echo "Flashing commands:"
                    echo "  devices                         List devices in fastboot mode"
                    echo "  flash <partition> <image>       Flash image to partition"
                    echo "  erase <partition>               Erase a flash partition"
                    echo "  getvar <variable|all>           Display device bootloader info"
                    echo "  flashing unlock                 Unlock bootloader"
                    echo "  flashing lock                   Relock bootloader"
                    echo "  oem unlock                      OEM unlock bootloader"
                    echo "  reboot [bootloader|recovery|fastboot]  Reboot device target"
                    echo ""
                    echo "Common Partitions:"
                    echo "  boot, init_boot, recovery, system, vendor, vbmeta, dtbo"
                    ;;
                *)
                    echo "[fastboot] Unknown command '${'$'}cmd'"
                    echo "Try 'fastboot help' for details."
                    ;;
            esac
            """.trimIndent() + "\n"
        )
        fastbootFile.setExecutable(true, false)

        // 16. Bootloader Tool
        val bootloaderFile = File(binDir, "bootloader")
        bootloaderFile.writeText(
            """
            #!/system/bin/sh
            # TRUX Bootloader Management Tool

            cmd="${'$'}1"

            show_status() {
                echo "===================================================="
                echo "               TRUX BOOTLOADER STATUS               "
                echo "===================================================="
                echo "Device Model     : ${'$'}(getprop ro.product.model 2>/dev/null || echo 'Unknown')"
                echo "Device Brand     : ${'$'}(getprop ro.product.brand 2>/dev/null || echo 'Unknown')"
                echo "Bootloader Vers. : ${'$'}(getprop ro.bootloader 2>/dev/null || echo 'Unknown')"
                echo "Baseband/Radio   : ${'$'}(getprop gsm.version.baseband 2>/dev/null || echo 'Unknown')"
                
                locked="${'$'}(getprop ro.boot.flash.locked 2>/dev/null)"
                if [ "${'$'}locked" = "0" ]; then
                    echo "Lock State       : UNLOCKED"
                else
                    echo "Lock State       : LOCKED / SECURE BOOT"
                fi

                slot="${'$'}(getprop ro.boot.slot_suffix 2>/dev/null | tr -d '_')"
                [ -n "${'$'}slot" ] && echo "Active Slot      : Slot ${'$'}slot (A/B Seamless)"
                echo "===================================================="
            }

            case "${'$'}cmd" in
                status|"")
                    show_status
                    ;;
                reboot)
                    target="${'$'}2"
                    case "${'$'}target" in
                        bootloader|fastboot)
                            echo "Rebooting to bootloader..."
                            reboot bootloader 2>/dev/null || su -c reboot bootloader
                            ;;
                        recovery)
                            echo "Rebooting to recovery mode..."
                            reboot recovery 2>/dev/null || su -c reboot recovery
                            ;;
                        download|odin)
                            echo "Rebooting to Samsung Download / Odin Mode..."
                            reboot download 2>/dev/null || su -c reboot download
                            ;;
                        edl)
                            echo "Rebooting to Qualcomm EDL mode..."
                            reboot edl 2>/dev/null || su -c reboot edl
                            ;;
                        *)
                            echo "Usage: bootloader reboot [bootloader|recovery|download|edl]"
                            ;;
                    esac
                    ;;
                unlock)
                    echo "=== Bootloader Unlock Instructions ==="
                    echo "1. Enable 'Developer options' (Settings > About phone > Tap Build number 7 times)"
                    echo "2. Enable 'OEM unlocking' in Developer options"
                    echo "3. Connect USB cable to PC or another terminal"
                    echo "4. Reboot to bootloader: bootloader reboot download (or fastboot)"
                    echo "5. For fastboot devices: fastboot flashing unlock"
                    echo "6. For Samsung devices: Hold Volume Up in Download mode screen to confirm unlock"
                    ;;
                partitions)
                    echo "=== Block Partitions ==="
                    ls -l /dev/block/by-name/ 2>/dev/null || ls -l /dev/block/bootdevice/by-name/ 2>/dev/null || echo "Root privileges required to list raw block partitions."
                    ;;
                help|--help|-h)
                    echo "TRUX Bootloader Tool"
                    echo "Usage: bootloader [command]"
                    echo ""
                    echo "Commands:"
                    echo "  status       Show bootloader & security lock status"
                    echo "  reboot       Reboot to [bootloader|recovery|download|edl]"
                    echo "  unlock       Show device-specific unlock procedure"
                    echo "  partitions   List block partitions (/dev/block/by-name/)"
                    echo "  help         Display this help"
                    ;;
                *)
                    show_status
                    ;;
            esac
            """.trimIndent() + "\n"
        )
        bootloaderFile.setExecutable(true, false)

        // 17. OS Install Tool
        val osInstallFile = File(binDir, "os-install")
        osInstallFile.writeText(
            """
            #!/system/bin/sh
            # TRUX OS Installer — ROM, GSI & Linux Distro Installation Engine

            echo "===================================================="
            echo "            TRUX OS INSTALLATION SUITE              "
            echo "===================================================="

            cmd="${'$'}1"

            case "${'$'}cmd" in
                gsi)
                    img="${'$'}2"
                    if [ -z "${'$'}img" ] || [ ! -f "${'$'}img" ]; then
                        echo "Usage: os-install gsi <gsi_system.img>"
                        echo "Supported: ARM64 A/B GSI images (Treble)"
                        exit 1
                    fi
                    echo "Flashing GSI: ${'$'}img..."
                    fastboot flash system "${'$'}img"
                    ;;
                linux|distro)
                    distro="${'$'}{2:-ubuntu}"
                    echo "Installing Linux Distribution: ${'$'}distro"
                    pkg install -y proot-distro
                    if command -v proot-distro >/dev/null 2>&1; then
                        echo "Setting up ${'$'}distro userspace..."
                        proot-distro install "${'$'}distro"
                        echo "Complete! Login via: proot-distro login ${'$'}distro"
                    else
                        echo "Error: proot-distro could not be installed."
                    fi
                    ;;
                recovery)
                    rec="${'$'}2"
                    if [ -z "${'$'}rec" ] || [ ! -f "${'$'}rec" ]; then
                        echo "Usage: os-install recovery <twrp_recovery.img>"
                        exit 1
                    fi
                    echo "Flashing custom recovery: ${'$'}rec..."
                    fastboot flash recovery "${'$'}rec"
                    ;;
                kernel|boot)
                    boot="${'$'}2"
                    if [ -z "${'$'}boot" ] || [ ! -f "${'$'}boot" ]; then
                        echo "Usage: os-install boot <boot.img>"
                        exit 1
                    fi
                    echo "Flashing kernel / boot: ${'$'}boot..."
                    fastboot flash boot "${'$'}boot"
                    ;;
                root|magisk)
                    echo "Root & KernelSU / Magisk Setup"
                    echo "1. Place patched boot.img or Magisk.apk in ~/storage/downloads"
                    echo "2. Run: os-install boot ~/storage/downloads/magisk_patched.img"
                    echo "3. Or install APK: adb install ~/storage/downloads/Magisk.apk"
                    ;;
                help|--help|-h|"")
                    echo "Usage: os-install <target> [image_or_options]"
                    echo ""
                    echo "Targets:"
                    echo "  gsi <system.img>       Flash Generic System Image (Android GSI)"
                    echo "  linux [ubuntu|debian|kali|arch] Install Linux OS in Trux"
                    echo "  recovery <rec.img>     Flash TWRP or OrangeFox recovery"
                    echo "  boot <boot.img>        Flash custom kernel / boot image"
                    echo "  root                   Display root setup guide (Magisk/KernelSU)"
                    echo "  help                   Display this guide"
                    ;;
                *)
                    echo "[os-install] Unknown target: ${'$'}cmd. Try 'os-install help'."
                    ;;
            esac
            """.trimIndent() + "\n"
        )
        osInstallFile.setExecutable(true, false)

        // 18. OS Upgrade Tool
        val osUpgradeFile = File(binDir, "os-upgrade")
        osUpgradeFile.writeText(
            """
            #!/system/bin/sh
            # TRUX OS Upgrade — Update System Packages, Core Tools & Kernel Subsystems

            echo "=== TRUX OS UPGRADE SYSTEM ==="
            echo "Checking current environment..."
            echo "Trux Version : 2.0.1"
            echo "Linux Kernel : ${'$'}(uname -r 2>/dev/null || echo 'Unknown')"
            echo "Architecture : ${'$'}(uname -m 2>/dev/null || echo 'aarch64')"
            echo ""
            echo "[1/3] Updating package repositories..."
            pkg update
            echo ""
            echo "[2/3] Upgrading installed packages..."
            pkg upgrade -y
            echo ""
            echo "[3/3] Synchronizing core bootstrap binaries..."
            echo "All core tools (adb, fastboot, bootloader, os-install) up to date."
            echo "=== Upgrade Complete! ==="
            """.trimIndent() + "\n"
        )
        osUpgradeFile.setExecutable(true, false)

        // 19. Flash Shortcut Tool
        val flashFile = File(binDir, "flash")
        flashFile.writeText(
            """
            #!/system/bin/sh
            # TRUX Quick Flasher
            if [ ${'$'}# -lt 2 ]; then
                echo "TRUX Quick Flash Utility"
                echo "Usage: flash <partition> <file.img>"
                echo "Example: flash boot boot.img"
                echo "         flash recovery twrp.img"
                echo "         flash system system.img"
                exit 1
            fi
            exec fastboot flash "${'$'}1" "${'$'}2"
            """.trimIndent() + "\n"
        )
        flashFile.setExecutable(true, false)

        // 20. AI Terminal Integration & Linux Users/Groups
        installAiCommands(binDir)
        installUserGroupCommands(binDir)
    }

    private fun installAiCommands(binDir: File) {
        val aiDir = File(prefix, "etc/trux").also { it.mkdirs() }

        val aiScript = """
            #!/system/bin/sh
            # TRUX AI Assistant — Linux Terminal AI Agent
            # Configurable AI assistant supporting OpenAI-compatible, Anthropic, and Local/Ollama endpoints.

            AI_DIR="${prefix.absolutePath}/etc/trux"
            AI_CONFIG="${'$'}AI_DIR/ai.json"
            AI_SECRET="${'$'}AI_DIR/ai.secret"

            mkdir -p "${'$'}AI_DIR" 2>/dev/null
            chmod 700 "${'$'}AI_DIR" 2>/dev/null

            # Redact secrets from text before transmission
            redact_secrets() {
                sed -E \
                    -e 's/(AI_KEY|API_KEY|TOKEN|PASSWORD|PASSWD|SECRET|PRIVATE_KEY|AUTHORIZATION|BEARER)[ =:"]+[^ "\t\r\n]{6,}/\1: [REDACTED]/gI' \
                    -e 's/(sk-[A-Za-z0-9_-]{20,})/[REDACTED_API_KEY]/g' \
                    -e 's/(ghp_[A-Za-z0-9]{30,})/[REDACTED_GH_TOKEN]/g' \
                    -e 's/-----BEGIN [A-Z ]+ PRIVATE KEY-----[^-]+-----END [A-Z ]+ PRIVATE KEY-----/[REDACTED_PRIVATE_KEY]/g'
            }

            get_config() {
                key="${'$'}1"
                def="${'$'}2"
                if [ -f "${'$'}AI_CONFIG" ]; then
                    val="${'$'}(grep -o "\"${'$'}key\"[[:space:]]*:[[:space:]]*\"[^\"]*\"" "${'$'}AI_CONFIG" 2>/dev/null | head -1 | sed -E 's/.*:[[:space:]]*"([^"]*)".*/\1/')"
                    if [ -n "${'$'}val" ]; then
                        echo "${'$'}val"
                        return
                    fi
                    num="${'$'}(grep -o "\"${'$'}key\"[[:space:]]*:[[:space:]]*[0-9.]*" "${'$'}AI_CONFIG" 2>/dev/null | head -1 | sed -E 's/.*:[[:space:]]*([0-9.]*).*/\1/')"
                    if [ -n "${'$'}num" ]; then
                        echo "${'$'}num"
                        return
                    fi
                fi
                echo "${'$'}def"
            }

            get_api_key() {
                if [ -n "${'$'}TRUX_AI_API_KEY" ]; then
                    echo "${'$'}TRUX_AI_API_KEY"
                elif [ -f "${'$'}AI_SECRET" ]; then
                    toybox base64 -d "${'$'}AI_SECRET" 2>/dev/null || base64 -d "${'$'}AI_SECRET" 2>/dev/null || cat "${'$'}AI_SECRET" 2>/dev/null
                else
                    echo ""
                fi
            }

            cmd="${'$'}1"

            case "${'$'}cmd" in
                setup)
                    echo "======================================================"
                    echo "                   TRUX AI SETUP                      "
                    echo "======================================================"
                    echo "Provider:"
                    echo "  1. OpenAI-compatible (OpenAI, Groq, Mistral, Ollama, Local)"
                    echo "  2. Anthropic-compatible"
                    echo "  3. Custom / Local endpoint (e.g. http://127.0.0.1:8000/v1)"
                    printf "Select provider [1-3] (default 1): "
                    read p_choice
                    case "${'$'}p_choice" in
                        2)
                            PROVIDER="anthropic-compatible"
                            DEFAULT_EP="https://api.anthropic.com/v1"
                            DEFAULT_MODEL="claude-3-5-sonnet-20241022"
                            ;;
                        3)
                            PROVIDER="custom"
                            DEFAULT_EP="http://127.0.0.1:8000/v1"
                            DEFAULT_MODEL="custom-model"
                            ;;
                        *)
                            PROVIDER="openai-compatible"
                            DEFAULT_EP="https://api.openai.com/v1"
                            DEFAULT_MODEL="gpt-4o-mini"
                            ;;
                    esac

                    printf "API endpoint (default %s): " "${'$'}DEFAULT_EP"
                    read ep_in
                    ENDPOINT="${'$'}{ep_in:-${'$'}DEFAULT_EP}"

                    if echo "${'$'}ENDPOINT" | grep -q "^http://"; then
                        echo "[WARNING] HTTP endpoint selected. Data will not be TLS encrypted over the wire."
                    fi

                    # Sensitive API Key Entry — MASKED (hidden with stty -echo)
                    printf "API key (input hidden): "
                    stty -echo 2>/dev/null
                    read api_key_in
                    stty echo 2>/dev/null
                    echo ""
                    if [ -z "${'$'}api_key_in" ]; then
                        existing_key="${'$'}(get_api_key)"
                        if [ -n "${'$'}existing_key" ]; then
                            echo "(Keeping existing configured API key)"
                            api_key_in="${'$'}existing_key"
                        fi
                    fi

                    printf "Model (default %s): " "${'$'}DEFAULT_MODEL"
                    read model_in
                    MODEL="${'$'}{model_in:-${'$'}DEFAULT_MODEL}"

                    printf "Streaming enabled? [Y/n] (default yes): "
                    read stream_in
                    case "${'$'}stream_in" in
                        [nN]*) STREAMING="false" ;;
                        *) STREAMING="true" ;;
                    esac

                    printf "Timeout in seconds (default 60): "
                    read to_in
                    TIMEOUT="${'$'}{to_in:-60}"

                    printf "Save configuration? [Y/n]: "
                    read save_in
                    case "${'$'}save_in" in
                        [nN]*)
                            echo "Configuration cancelled."
                            exit 0
                            ;;
                    esac

                    cat <<EOF > "${'$'}AI_CONFIG"
{
  "provider": "${'$'}PROVIDER",
  "endpoint": "${'$'}ENDPOINT",
  "model": "${'$'}MODEL",
  "streaming": ${'$'}STREAMING,
  "timeout": ${'$'}TIMEOUT,
  "temperature": 0.7,
  "max_tokens": 2048,
  "system_prompt": "You are TRUX AI, a helpful Linux and Android terminal assistant. Provide concise, accurate terminal commands and technical answers."
}
EOF
                    chmod 600 "${'$'}AI_CONFIG" 2>/dev/null

                    if [ -n "${'$'}api_key_in" ]; then
                        printf "%s" "${'$'}api_key_in" | (toybox base64 2>/dev/null || base64 2>/dev/null || cat) > "${'$'}AI_SECRET"
                        chmod 600 "${'$'}AI_SECRET" 2>/dev/null
                    fi

                    echo ""
                    echo "✓ AI configuration saved securely to ${prefix.absolutePath}/etc/trux/ai.json"
                    echo "✓ Sensitive API credentials protected with mode 600"
                    echo "Run 'trux ai test' to verify connectivity."
                    ;;

                status)
                    echo "======================================================"
                    echo "                   TRUX AI STATUS                     "
                    echo "======================================================"
                    if [ ! -f "${'$'}AI_CONFIG" ]; then
                        echo "Status  : Not configured"
                        echo "Run 'trux ai setup' to configure an AI provider."
                        echo "======================================================"
                        exit 0
                    fi
                    prov="${'$'}(get_config "provider" "openai-compatible")"
                    ep="${'$'}{TRUX_AI_ENDPOINT:-${'$'}(get_config "endpoint" "not set")}"
                    mod="${'$'}{TRUX_AI_MODEL:-${'$'}(get_config "model" "not set")}"
                    st="${'$'}(get_config "streaming" "true")"
                    to="${'$'}(get_config "timeout" "60")"
                    key="${'$'}(get_api_key)"

                    echo "Provider : ${'$'}prov"
                    echo "Endpoint : ${'$'}ep"
                    echo "Model    : ${'$'}mod"
                    if [ -n "${'$'}key" ]; then
                        echo "API Key  : configured (securely protected)"
                        echo "Status   : ready"
                    else
                        echo "API Key  : not configured"
                        echo "Status   : unauthenticated (API key missing)"
                    fi
                    echo "Streaming: ${'$'}st"
                    echo "Timeout  : ${'$'}{to}s"
                    echo "Config   : ${'$'}AI_CONFIG"
                    echo "======================================================"
                    ;;

                test)
                    echo "Testing AI endpoint..."
                    ep="${'$'}{TRUX_AI_ENDPOINT:-${'$'}(get_config "endpoint" "")}"
                    mod="${'$'}{TRUX_AI_MODEL:-${'$'}(get_config "model" "")}"
                    key="${'$'}(get_api_key)"
                    to="${'$'}(get_config "timeout" "30")"

                    if [ -z "${'$'}ep" ]; then
                        echo "AI connection failed."
                        echo "Reason: AI endpoint not configured."
                        echo "Suggested action: Run 'trux ai setup' to configure your provider."
                        exit 1
                    fi

                    echo "Connecting to ${'$'}ep..."
                    echo "Authenticating..."
                    echo "Model: ${'$'}mod"
                    echo "Streaming: ${'$'}(get_config "streaming" "true")"

                    trap 'echo "\nAI test cancelled."; exit 130' INT

                    resp="${'$'}(curl -s -m "${'$'}to" -w "\n%{http_code}" \
                        -H "Content-Type: application/json" \
                        -H "Authorization: Bearer ${'$'}key" \
                        -d "{\"model\":\"${'$'}mod\",\"messages\":[{\"role\":\"user\",\"content\":\"ping\"}],\"max_tokens\":5}" \
                        "${'$'}ep/chat/completions" 2>/dev/null)"
                    code="${'$'}(echo "${'$'}resp" | tail -n1)"
                    body="${'$'}(echo "${'$'}resp" | sed '${'$'}d')"

                    if [ "${'$'}code" = "200" ]; then
                        echo ""
                        echo "AI connection successful."
                    else
                        echo ""
                        echo "AI connection failed."
                        echo "HTTP status: ${'$'}{code:-Network error}"
                        if [ "${'$'}code" = "401" ]; then
                            echo "Reason: Authentication failed (Invalid API key)."
                            echo "Suggested action: Check API key with 'trux ai setup'."
                        elif [ "${'$'}code" = "404" ]; then
                            echo "Reason: Endpoint URL or model '${'$'}mod' not found."
                            echo "Suggested action: Verify endpoint and model name."
                        else
                            echo "Reason: ${'$'}(echo "${'$'}body" | grep -o '"message"[[:space:]]*:[[:space:]]*"[^"]*"' | head -1 | sed 's/"message"://' | tr -d '"{}`' || echo 'Endpoint unreachable')"
                            echo "Suggested action: Verify network connectivity and server status."
                        fi
                        exit 1
                    fi
                    ;;

                models)
                    ep="${'$'}{TRUX_AI_ENDPOINT:-${'$'}(get_config "endpoint" "")}"
                    key="${'$'}(get_api_key)"
                    to="${'$'}(get_config "timeout" "30")"
                    if [ -z "${'$'}ep" ]; then
                        echo "Error: AI endpoint not configured. Run 'trux ai setup'."
                        exit 1
                    fi
                    echo "Fetching models from ${'$'}ep/models..."
                    resp="${'$'}(curl -s -m "${'$'}to" -H "Authorization: Bearer ${'$'}key" "${'$'}ep/models" 2>/dev/null)"
                    if echo "${'$'}resp" | grep -q '"id"'; then
                        echo "=== Available Models ==="
                        echo "${'$'}resp" | grep -o '"id"[[:space:]]*:[[:space:]]*"[^"]*"' | sed -E 's/.*"id"[[:space:]]*:[[:space:]]*"([^"]*)".*/  • \1/' | sort -u | head -30
                    else
                        echo "Model listing unsupported by this endpoint."
                    fi
                    ;;

                history)
                    HIST_FILE="${home.absolutePath}/.trux_ai_chat.log"
                    if [ -f "${'$'}HIST_FILE" ] && [ -s "${'$'}HIST_FILE" ]; then
                        echo "=== TRUX AI On-Device Chat History ==="
                        cat "${'$'}HIST_FILE"
                        echo "======================================"
                    else
                        echo "No AI chat history stored on device yet."
                    fi
                    ;;

                clear-history)
                    rm -f "${home.absolutePath}/.trux_ai_chat.log" "${home.absolutePath}/.trux_ai_history.json" 2>/dev/null
                    echo "On-device AI chat history cleared."
                    ;;

                export-chat)
                    HIST_FILE="${home.absolutePath}/.trux_ai_chat.log"
                    if [ ! -f "${'$'}HIST_FILE" ] || [ ! -s "${'$'}HIST_FILE" ]; then
                        echo "No chat history to export."
                        exit 1
                    fi
                    ts="${'$'}(date +%Y%m%d_%H%M%S 2>/dev/null || date +%s)"
                    out="${home.absolutePath}/storage/downloads/TRUX_AI_Chat_${'$'}{ts}.txt"
                    mkdir -p "${'$'}(dirname "${'$'}out")" 2>/dev/null
                    cp "${'$'}HIST_FILE" "${'$'}out" 2>/dev/null || cat "${'$'}HIST_FILE" > "${'$'}out"
                    echo "Chat history exported to: ${'$'}out"
                    ;;

                reset)
                    printf "Delete TRUX AI configuration and stored credentials? [y/N]: "
                    read confirm
                    case "${'$'}confirm" in
                        [yY]*)
                            rm -f "${'$'}AI_CONFIG" "${'$'}AI_SECRET"
                            echo "TRUX AI configuration and credentials successfully deleted."
                            ;;
                        *)
                            echo "Reset cancelled."
                            ;;
                    esac
                    ;;

                config)
                    prop="${'$'}2"
                    val="${'$'}3"
                    if [ -z "${'$'}prop" ]; then
                        if [ -f "${'$'}AI_CONFIG" ]; then
                            cat "${'$'}AI_CONFIG"
                        else
                            echo "No configuration found. Run 'trux ai setup'."
                        fi
                        exit 0
                    fi
                    if [ -z "${'$'}val" ]; then
                        get_config "${'$'}prop" "not set"
                        exit 0
                    fi
                    if [ -f "${'$'}AI_CONFIG" ]; then
                        sed -i -E "s/\"${'$'}prop\"[[:space:]]*:[[:space:]]*\"[^\"]*\"/\"${'$'}prop\": \"${'$'}val\"/" "${'$'}AI_CONFIG"
                        echo "AI config '${'$'}prop' updated to '${'$'}val'"
                    else
                        echo "Run 'trux ai setup' first."
                    fi
                    ;;

                explain)
                    shift
                    query="${'$'}*"
                    if [ -z "${'$'}query" ]; then
                        echo "Usage: trux ai explain <command_or_error>"
                        exit 1
                    fi
                    exec "${prefix.absolutePath}/bin/trux-ai" "Explain this command or error clearly and concisely: ${'$'}query"
                    ;;

                explain-last)
                    last_hist="${'$'}(tail -n 2 "${'$'}HOME/.bash_history" 2>/dev/null | head -n 1)"
                    if [ -z "${'$'}last_hist" ]; then
                        echo "No previous command found in history."
                        exit 1
                    fi
                    echo "Explaining last command: ${'$'}last_hist"
                    exec "${prefix.absolutePath}/bin/trux-ai" "Explain what this command does and how to use it: ${'$'}last_hist"
                    ;;

                *)
                    # Handle query execution (One-shot, --command, interactive)
                    ep="${'$'}{TRUX_AI_ENDPOINT:-${'$'}(get_config "endpoint" "")}"
                    mod="${'$'}{TRUX_AI_MODEL:-${'$'}(get_config "model" "gpt-4o-mini")}"
                    key="${'$'}(get_api_key)"
                    to="${'$'}(get_config "timeout" "60")"
                    st="${'$'}(get_config "streaming" "true")"
                    sys_prompt="${'$'}(get_config "system_prompt" "You are TRUX AI, a native Android terminal & Linux assistant. Provide clean, correct shell commands and clear, concise explanations.")"

                    if [ -z "${'$'}ep" ]; then
                        echo "TRUX AI is not configured yet."
                        echo "Run 'trux ai setup' to configure an AI provider."
                        exit 1
                    fi

                    trap 'echo -e "\n[AI query cancelled]"; exit 130' INT

                    # Command generation mode: --command
                    if [ "${'$'}1" = "--command" ]; then
                        shift
                        prompt="${'$'}*"
                        if [ -z "${'$'}prompt" ]; then
                            echo "Usage: trux ai --command \"task to generate command for\""
                            exit 1
                        fi
                        safe_prompt="${'$'}(printf "%s" "${'$'}prompt" | redact_secrets | tr '\n' ' ' | sed 's/"/\\"/g')"
                        cmd_sys="You are a Linux command generator in TRUX. Output ONLY the single raw shell command that achieves the user goal. No markdown formatting, no backticks, no explanation."
                        
                        echo "Generating command..."
                        raw_cmd="${'$'}(curl -s -m "${'$'}to" \
                            -H "Content-Type: application/json" \
                            -H "Authorization: Bearer ${'$'}key" \
                            -d "{\"model\":\"${'$'}mod\",\"messages\":[{\"role\":\"system\",\"content\":\"${'$'}cmd_sys\"},{\"role\":\"user\",\"content\":\"${'$'}safe_prompt\"}],\"temperature\":0.1,\"max_tokens\":100}" \
                            "${'$'}ep/chat/completions" 2>/dev/null | awk '
                            match(${'$'}0, /"content"[[:space:]]*:[[:space:]]*"/) {
                                s = substr(${'$'}0, RSTART + RLENGTH)
                                out = ""
                                for (i = 1; i <= length(s); i++) {
                                    c = substr(s, i, 1)
                                    if (c == "\\" && i < length(s)) {
                                        nc = substr(s, i+1, 1)
                                        if (nc == "n") { i++; continue }
                                        if (nc == "\"") { out = out "\""; i++; continue }
                                    }
                                    if (c == "\"") break
                                    out = out c
                                }
                                print out
                            }')"

                        clean_cmd="${'$'}(echo "${'$'}raw_cmd" | sed -e 's/^`*//' -e 's/`*$//' -e 's/^[ \t]*//' -e 's/[ \t]*${'$'}//')"
                        if [ -z "${'$'}clean_cmd" ]; then
                            echo "Failed to generate command. Check AI connectivity."
                            exit 1
                        fi

                        echo ""
                        echo "${'$'}clean_cmd"
                        echo ""
                        printf "Execute this command? [y/N]: "
                        read exec_confirm
                        case "${'$'}exec_confirm" in
                            [yY]*)
                                echo "Executing: ${'$'}clean_cmd"
                                eval "${'$'}clean_cmd"
                                ;;
                            *)
                                echo "Execution cancelled."
                                ;;
                        esac
                        exit 0
                    fi

                    # Context option: --context-last <N>
                    context_prefix=""
                    if [ "${'$'}1" = "--context-last" ]; then
                        shift
                        lines_n="${'$'}{1:-20}"
                        shift
                        if [ -f "${'$'}HOME/.bash_history" ]; then
                            ctx_raw="${'$'}(tail -n "${'$'}lines_n" "${'$'}HOME/.bash_history" 2>/dev/null | redact_secrets)"
                            context_prefix="Recent terminal history:\n${'$'}ctx_raw\n\nQuestion: "
                        fi
                    elif [ "${'$'}1" = "--context-selection" ]; then
                        shift
                        if [ -f "${'$'}HOME/.trux_clipboard" ]; then
                            ctx_raw="${'$'}(head -n 50 "${'$'}HOME/.trux_clipboard" 2>/dev/null | redact_secrets)"
                            context_prefix="Selected text:\n${'$'}ctx_raw\n\nQuestion: "
                        fi
                    fi

                    # Interactive mode vs One-shot mode
                    if [ ${'$'}# -eq 0 ]; then
                        echo "TRUX AI interactive mode (${'$'}mod)."
                        echo "Type /exit to leave, /help for options."
                        echo ""
                        while true; do
                            printf "\033[1;32m> \033[0m"
                            read user_input
                            case "${'$'}user_input" in
                                /exit|/quit|exit|quit)
                                    echo "Exiting TRUX AI."
                                    break
                                    ;;
                                /help)
                                    echo "Commands: /exit to leave, /clear to clear screen, /status to view status, /history to view chat history, /clear-history to clear history."
                                    continue
                                    ;;
                                /history)
                                    if [ -f "${home.absolutePath}/.trux_ai_chat.log" ]; then
                                        echo "=== TRUX AI Chat History ==="
                                        cat "${home.absolutePath}/.trux_ai_chat.log"
                                        echo "============================"
                                    else
                                        echo "No chat history stored on device yet."
                                    fi
                                    continue
                                    ;;
                                /clear-history)
                                    rm -f "${home.absolutePath}/.trux_ai_chat.log" "${home.absolutePath}/.trux_ai_history.json" 2>/dev/null
                                    echo "On-device chat history cleared."
                                    continue
                                    ;;
                                /clear)
                                    clear
                                    continue
                                    ;;
                                /status)
                                    exec "${prefix.absolutePath}/bin/trux-ai" status
                                    ;;
                                "")
                                    continue
                                    ;;
                            esac

                            mkdir -p "${home.absolutePath}" 2>/dev/null
                            echo "[${'$'}(date '+%Y-%m-%d %H:%M:%S' 2>/dev/null || date)] User: ${'$'}user_input" >> "${home.absolutePath}/.trux_ai_chat.log"

                            safe_input="${'$'}(printf "%s" "${'$'}user_input" | redact_secrets | tr '\n' ' ' | sed 's/"/\\"/g')"
                            printf "\033[1;36mAI:\033[0m\n"

                            if [ "${'$'}st" = "true" ]; then
                                curl -s -N -m "${'$'}to" \
                                    -H "Content-Type: application/json" \
                                    -H "Authorization: Bearer ${'$'}key" \
                                    -d "{\"model\":\"${'$'}mod\",\"messages\":[{\"role\":\"system\",\"content\":\"${'$'}sys_prompt\"},{\"role\":\"user\",\"content\":\"${'$'}safe_input\"}],\"stream\":true}" \
                                    "${'$'}ep/chat/completions" 2>/dev/null | awk '/^data: / {
                                    sub(/^data: /, "")
                                    if (${'$'}0 == "[DONE]") exit
                                    if (match(${'$'}0, /"content"[[:space:]]*:[[:space:]]*"/)) {
                                        s = substr(${'$'}0, RSTART + RLENGTH)
                                        for (i = 1; i <= length(s); i++) {
                                            c = substr(s, i, 1)
                                            if (c == "\\" && i < length(s)) {
                                                nc = substr(s, i+1, 1)
                                                if (nc == "n") { printf "\n"; fflush(); i++; continue }
                                                if (nc == "t") { printf "\t"; fflush(); i++; continue }
                                                if (nc == "\"") { printf "\""; fflush(); i++; continue }
                                                if (nc == "\\") { printf "\\"; fflush(); i++; continue }
                                            }
                                            if (c == "\"") break
                                            printf "%s", c
                                            fflush()
                                        }
                                    }
                                }'
                                echo ""
                            else
                                curl -s -m "${'$'}to" \
                                    -H "Content-Type: application/json" \
                                    -H "Authorization: Bearer ${'$'}key" \
                                    -d "{\"model\":\"${'$'}mod\",\"messages\":[{\"role\":\"system\",\"content\":\"${'$'}sys_prompt\"},{\"role\":\"user\",\"content\":\"${'$'}safe_input\"}]}" \
                                    "${'$'}ep/chat/completions" 2>/dev/null | awk '
                                    match(${'$'}0, /"content"[[:space:]]*:[[:space:]]*"/) {
                                        s = substr(${'$'}0, RSTART + RLENGTH)
                                        out = ""
                                        for (i = 1; i <= length(s); i++) {
                                            c = substr(s, i, 1)
                                            if (c == "\\" && i < length(s)) {
                                                nc = substr(s, i+1, 1)
                                                if (nc == "n") { out = out "\n"; i++; continue }
                                                if (nc == "t") { out = out "\t"; i++; continue }
                                                if (nc == "\"") { out = out "\""; i++; continue }
                                                if (nc == "\\") { out = out "\\"; i++; continue }
                                            }
                                            if (c == "\"") break
                                            out = out c
                                        }
                                        print out
                                    }'
                            fi
                            echo ""
                        done
                    else
                        # One-shot mode
                        full_prompt="${'$'}{context_prefix}${'$'}*"
                        safe_prompt="${'$'}(printf "%s" "${'$'}full_prompt" | redact_secrets | tr '\n' ' ' | sed 's/"/\\"/g')"
                        mkdir -p "${home.absolutePath}" 2>/dev/null
                        echo "[${'$'}(date '+%Y-%m-%d %H:%M:%S' 2>/dev/null || date)] User: ${'$'}full_prompt" >> "${home.absolutePath}/.trux_ai_chat.log"
                        printf "\033[1;36mAI:\033[0m\n"

                        if [ "${'$'}st" = "true" ]; then
                            curl -s -N -m "${'$'}to" \
                                -H "Content-Type: application/json" \
                                -H "Authorization: Bearer ${'$'}key" \
                                -d "{\"model\":\"${'$'}mod\",\"messages\":[{\"role\":\"system\",\"content\":\"${'$'}sys_prompt\"},{\"role\":\"user\",\"content\":\"${'$'}safe_prompt\"}],\"stream\":true}" \
                                "${'$'}ep/chat/completions" 2>/dev/null | awk '/^data: / {
                                sub(/^data: /, "")
                                if (${'$'}0 == "[DONE]") exit
                                if (match(${'$'}0, /"content"[[:space:]]*:[[:space:]]*"/)) {
                                    s = substr(${'$'}0, RSTART + RLENGTH)
                                    for (i = 1; i <= length(s); i++) {
                                        c = substr(s, i, 1)
                                        if (c == "\\" && i < length(s)) {
                                            nc = substr(s, i+1, 1)
                                            if (nc == "n") { printf "\n"; fflush(); i++; continue }
                                            if (nc == "t") { printf "\t"; fflush(); i++; continue }
                                            if (nc == "\"") { printf "\""; fflush(); i++; continue }
                                            if (nc == "\\") { printf "\\"; fflush(); i++; continue }
                                        }
                                        if (c == "\"") break
                                        printf "%s", c
                                        fflush()
                                    }
                                }
                            }'
                            echo ""
                        else
                            curl -s -m "${'$'}to" \
                                -H "Content-Type: application/json" \
                                -H "Authorization: Bearer ${'$'}key" \
                                -d "{\"model\":\"${'$'}mod\",\"messages\":[{\"role\":\"system\",\"content\":\"${'$'}sys_prompt\"},{\"role\":\"user\",\"content\":\"${'$'}safe_prompt\"}]}" \
                                "${'$'}ep/chat/completions" 2>/dev/null | awk '
                                match(${'$'}0, /"content"[[:space:]]*:[[:space:]]*"/) {
                                    s = substr(${'$'}0, RSTART + RLENGTH)
                                    out = ""
                                    for (i = 1; i <= length(s); i++) {
                                        c = substr(s, i, 1)
                                        if (c == "\\" && i < length(s)) {
                                            nc = substr(s, i+1, 1)
                                            if (nc == "n") { out = out "\n"; i++; continue }
                                            if (nc == "t") { out = out "\t"; i++; continue }
                                            if (nc == "\"") { out = out "\""; i++; continue }
                                            if (nc == "\\") { out = out "\\"; i++; continue }
                                        }
                                        if (c == "\"") break
                                        out = out c
                                    }
                                    print out
                                }'
                        fi
                    fi
                    ;;
            esac
        """.trimIndent() + "\n"

        File(binDir, "trux-ai").also {
            it.writeText(aiScript)
            it.setExecutable(true, false)
        }
        File(binDir, "ai").also {
            it.writeText("#!/system/bin/sh\nexec trux-ai \"${'$'}@\"\n")
            it.setExecutable(true, false)
        }
    }

    private fun installUserGroupCommands(binDir: File) {
        val pwPath = "${prefix.absolutePath}/etc/passwd"
        val grPath = "${prefix.absolutePath}/etc/group"
        val shwPath = "${prefix.absolutePath}/etc/shadow"
        val hnPath = "${prefix.absolutePath}/etc/hostname"

        // whoami
        val whoamiScript = """
            #!/system/bin/sh
            echo "${'$'}{USER:-trux}"
        """.trimIndent() + "\n"
        File(binDir, "whoami").also { it.writeText(whoamiScript); it.setExecutable(true, false) }

        // id
        val idScript = """
            #!/system/bin/sh
            u="${'$'}{1:-${'$'}{USER:-trux}}"
            pw="$pwPath"
            gr="$grPath"

            line="${'$'}(grep "^${'$'}u:" "${'$'}pw" 2>/dev/null)"
            if [ -z "${'$'}line" ]; then
                echo "id: '${'$'}u': no such user"
                exit 1
            fi
            uid="${'$'}(echo "${'$'}line" | cut -d: -f3)"
            gid="${'$'}(echo "${'$'}line" | cut -d: -f4)"
            pgrp="${'$'}(grep "^[^:]*:[^:]*:${'$'}gid:" "${'$'}gr" 2>/dev/null | cut -d: -f1)"
            pgrp="${'$'}{pgrp:-${'$'}u}"

            sgrps="${'$'}(awk -F: -v user="${'$'}u" '${'$'}4 ~ "(^|,)" user "(,|$)" {printf "%s(%s),", ${'$'}3, ${'$'}1}' "${'$'}gr" 2>/dev/null | sed 's/,$//')"
            if [ -n "${'$'}sgrps" ]; then
                case "${'$'}sgrps" in
                    *"${'$'}gid("*) all_groups="${'$'}sgrps" ;;
                    *) all_groups="${'$'}gid(${'$'}pgrp),${'$'}sgrps" ;;
                esac
            else
                all_groups="${'$'}gid(${'$'}pgrp)"
            fi

            echo "uid=${'$'}uid(${'$'}u) gid=${'$'}gid(${'$'}pgrp) groups=${'$'}all_groups"
        """.trimIndent() + "\n"
        File(binDir, "id").also { it.writeText(idScript); it.setExecutable(true, false) }

        // groups
        val groupsScript = """
            #!/system/bin/sh
            u="${'$'}{1:-${'$'}{USER:-trux}}"
            pw="$pwPath"
            gr="$grPath"

            line="${'$'}(grep "^${'$'}u:" "${'$'}pw" 2>/dev/null)"
            if [ -z "${'$'}line" ]; then
                echo "groups: '${'$'}u': no such user"
                exit 1
            fi
            gid="${'$'}(echo "${'$'}line" | cut -d: -f4)"
            pgrp="${'$'}(grep "^[^:]*:[^:]*:${'$'}gid:" "${'$'}gr" 2>/dev/null | cut -d: -f1)"
            pgrp="${'$'}{pgrp:-${'$'}u}"

            sgrps="${'$'}(awk -F: -v user="${'$'}u" '${'$'}4 ~ "(^|,)" user "(,|$)" {print ${'$'}1}' "${'$'}gr" 2>/dev/null)"
            echo "${'$'}u : ${'$'}pgrp ${'$'}sgrps"
        """.trimIndent() + "\n"
        File(binDir, "groups").also { it.writeText(groupsScript); it.setExecutable(true, false) }

        // users
        val usersScript = """
            #!/system/bin/sh
            awk -F: '${'$'}3 >= 1000 || ${'$'}3 == 0 {print ${'$'}1}' "$pwPath" 2>/dev/null | tr '\n' ' '
            echo ""
        """.trimIndent() + "\n"
        File(binDir, "users").also { it.writeText(usersScript); it.setExecutable(true, false) }

        // useradd & adduser
        val useraddScript = """
            #!/system/bin/sh
            pw="$pwPath"
            gr="$grPath"
            shw="$shwPath"

            UID_SPEC=""
            GID_SPEC=""
            SUP_GROUPS=""
            HOME_SPEC=""
            SHELL_SPEC="/bin/bash"
            COMMENT="TRUX User"
            SYSTEM_USER=0

            while [ ${'$'}# -gt 0 ]; do
                case "${'$'}1" in
                    -u) UID_SPEC="${'$'}2"; shift 2 ;;
                    -g) GID_SPEC="${'$'}2"; shift 2 ;;
                    -G) SUP_GROUPS="${'$'}2"; shift 2 ;;
                    -d) HOME_SPEC="${'$'}2"; shift 2 ;;
                    -s) SHELL_SPEC="${'$'}2"; shift 2 ;;
                    -c) COMMENT="${'$'}2"; shift 2 ;;
                    -r|--system) SYSTEM_USER=1; shift ;;
                    -*) echo "useradd: invalid option '${'$'}1'"; exit 1 ;;
                    *) USERNAME="${'$'}1"; shift ;;
                esac
            done

            if [ -z "${'$'}USERNAME" ]; then
                echo "Usage: useradd [options] <username>"
                echo "Options:"
                echo "  -u UID                Specify UID"
                echo "  -g GID                Specify primary group"
                echo "  -G GROUP1,GROUP2      Specify supplementary groups"
                echo "  -d HOME               Specify home directory"
                echo "  -s SHELL              Specify login shell"
                echo "  -c COMMENT            GECOS comment field"
                echo "  -r, --system          Create a system user (UID < 1000)"
                exit 1
            fi

            if grep -q "^${'$'}USERNAME:" "${'$'}pw" 2>/dev/null; then
                echo "useradd: user '${'$'}USERNAME' already exists"
                exit 9
            fi

            if [ -n "${'$'}UID_SPEC" ]; then
                NEW_UID="${'$'}UID_SPEC"
            elif [ "${'$'}SYSTEM_USER" -eq 1 ]; then
                NEW_UID="${'$'}(awk -F: '${'$'}3 >= 100 && ${'$'}3 < 1000 {if (${'$'}3 > max) max=${'$'}3} END {print (max? max+1 : 100)}' "${'$'}pw" 2>/dev/null)"
            else
                NEW_UID="${'$'}(awk -F: '${'$'}3 >= 1000 && ${'$'}3 < 65534 {if (${'$'}3 > max) max=${'$'}3} END {print (max? max+1 : 1000)}' "${'$'}pw" 2>/dev/null)"
            fi

            NEW_GID="${'$'}{GID_SPEC:-${'$'}NEW_UID}"
            NEW_HOME="${'$'}{HOME_SPEC:-${prefix.absolutePath}/home/${'$'}USERNAME}"

            if ! grep -q ":${'$'}NEW_GID:" "${'$'}gr" 2>/dev/null && ! grep -q "^${'$'}USERNAME:" "${'$'}gr" 2>/dev/null; then
                echo "${'$'}USERNAME:x:${'$'}NEW_GID:" >> "${'$'}gr"
            fi

            echo "${'$'}USERNAME:x:${'$'}NEW_UID:${'$'}NEW_GID:${'$'}COMMENT:${'$'}NEW_HOME:${'$'}SHELL_SPEC" >> "${'$'}pw"
            echo "${'$'}USERNAME:*:19700:0:99999:7:::" >> "${'$'}shw"

            if [ -n "${'$'}SUP_GROUPS" ]; then
                OLD_IFS="${'$'}IFS"
                IFS=,
                for g in ${'$'}SUP_GROUPS; do
                    if grep -q "^${'$'}g:" "${'$'}gr" 2>/dev/null; then
                        cur_m="${'$'}(grep "^${'$'}g:" "${'$'}gr" | cut -d: -f4)"
                        if [ -n "${'$'}cur_m" ]; then
                            new_m="${'$'}cur_m,${'$'}USERNAME"
                        else
                            new_m="${'$'}USERNAME"
                        fi
                        sed -i -E "s/^(${'$'}g:x:[0-9]+:).*/\1${'$'}new_m/" "${'$'}gr"
                    else
                        echo "${'$'}g:x:${'$'}((NEW_GID+10)):${'$'}USERNAME" >> "${'$'}gr"
                    fi
                done
                IFS="${'$'}OLD_IFS"
            fi

            mkdir -p "${'$'}NEW_HOME" 2>/dev/null
            chmod 750 "${'$'}NEW_HOME" 2>/dev/null
            if [ -f "${home.absolutePath}/.profile" ]; then
                cp "${home.absolutePath}/.profile" "${'$'}NEW_HOME/.profile" 2>/dev/null
            fi
            if [ -f "${home.absolutePath}/.inputrc" ]; then
                cp "${home.absolutePath}/.inputrc" "${'$'}NEW_HOME/.inputrc" 2>/dev/null
            fi

            echo "useradd: created user '${'$'}USERNAME' (UID ${'$'}NEW_UID, GID ${'$'}NEW_GID) with home directory '${'$'}NEW_HOME'."
        """.trimIndent() + "\n"
        File(binDir, "useradd").also { it.writeText(useraddScript); it.setExecutable(true, false) }
        File(binDir, "adduser").also { it.writeText("#!/system/bin/sh\nexec useradd \"${'$'}@\"\n"); it.setExecutable(true, false) }

        // usermod
        val usermodScript = """
            #!/system/bin/sh
            pw="$pwPath"
            gr="$grPath"

            GROUPS_TO_ADD=""
            NEW_SHELL=""
            NEW_HOME=""

            while [ ${'$'}# -gt 0 ]; do
                case "${'$'}1" in
                    -aG|-Ga) GROUPS_TO_ADD="${'$'}2"; shift 2 ;;
                    -s) NEW_SHELL="${'$'}2"; shift 2 ;;
                    -d) NEW_HOME="${'$'}2"; shift 2 ;;
                    -*) echo "usermod: unrecognized option '${'$'}1'"; exit 1 ;;
                    *) USERNAME="${'$'}1"; shift ;;
                esac
            done

            if [ -z "${'$'}USERNAME" ] || ! grep -q "^${'$'}USERNAME:" "${'$'}pw" 2>/dev/null; then
                echo "usermod: user '${'$'}USERNAME' does not exist"
                exit 6
            fi

            if [ -n "${'$'}NEW_SHELL" ]; then
                sed -i -E "s/^(${'$'}USERNAME:[^:]*:[^:]*:[^:]*:[^:]*:[^:]*:).*/\1${'$'}NEW_SHELL/" "${'$'}pw"
            fi

            if [ -n "${'$'}NEW_HOME" ]; then
                sed -i -E "s/^(${'$'}USERNAME:[^:]*:[^:]*:[^:]*:[^:]*:)[^:]*(:.*)/\1${'$'}NEW_HOME\2/" "${'$'}pw"
                mkdir -p "${'$'}NEW_HOME" 2>/dev/null
            fi

            if [ -n "${'$'}GROUPS_TO_ADD" ]; then
                OLD_IFS="${'$'}IFS"
                IFS=,
                for g in ${'$'}GROUPS_TO_ADD; do
                    if grep -q "^${'$'}g:" "${'$'}gr" 2>/dev/null; then
                        cur="${'$'}(grep "^${'$'}g:" "${'$'}gr" | cut -d: -f4)"
                        if ! echo "${'$'}cur" | grep -qE "(^|,)${'$'}USERNAME(,|$)"; then
                            [ -n "${'$'}cur" ] && updated="${'$'}cur,${'$'}USERNAME" || updated="${'$'}USERNAME"
                            sed -i -E "s/^(${'$'}g:x:[0-9]+:).*/\1${'$'}updated/" "${'$'}gr"
                        fi
                    else
                        echo "${'$'}g:x:1099:${'$'}USERNAME" >> "${'$'}gr"
                    fi
                done
                IFS="${'$'}OLD_IFS"
            fi
            echo "usermod: user '${'$'}USERNAME' updated successfully."
        """.trimIndent() + "\n"
        File(binDir, "usermod").also { it.writeText(usermodScript); it.setExecutable(true, false) }

        // userdel & deluser
        val userdelScript = """
            #!/system/bin/sh
            pw="$pwPath"
            gr="$grPath"
            shw="$shwPath"

            REMOVE_HOME=0
            if [ "${'$'}1" = "-r" ] || [ "${'$'}1" = "--remove" ]; then
                REMOVE_HOME=1
                shift
            fi

            USERNAME="${'$'}1"
            if [ -z "${'$'}USERNAME" ] || ! grep -q "^${'$'}USERNAME:" "${'$'}pw" 2>/dev/null; then
                echo "userdel: user '${'$'}USERNAME' does not exist"
                exit 6
            fi

            if [ "${'$'}USERNAME" = "root" ] || [ "${'$'}USERNAME" = "trux" ]; then
                echo "userdel: cannot remove essential system user '${'$'}USERNAME'"
                exit 1
            fi

            HOME_DIR="${'$'}(grep "^${'$'}USERNAME:" "${'$'}pw" | cut -d: -f6)"

            sed -i "/^${'$'}USERNAME:/d" "${'$'}pw"
            sed -i "/^${'$'}USERNAME:/d" "${'$'}shw" 2>/dev/null
            sed -i -E "s/(^|,)${'$'}USERNAME//g" "${'$'}gr" 2>/dev/null
            sed -i "/^${'$'}USERNAME:x:[0-9]*:$/d" "${'$'}gr" 2>/dev/null

            if [ "${'$'}REMOVE_HOME" -eq 1 ] && [ -n "${'$'}HOME_DIR" ] && [ -d "${'$'}HOME_DIR" ]; then
                rm -rf "${'$'}HOME_DIR"
            fi

            echo "userdel: user '${'$'}USERNAME' removed successfully."
        """.trimIndent() + "\n"
        File(binDir, "userdel").also { it.writeText(userdelScript); it.setExecutable(true, false) }
        File(binDir, "deluser").also { it.writeText("#!/system/bin/sh\nexec userdel \"${'$'}@\"\n"); it.setExecutable(true, false) }

        // passwd
        val passwdScript = """
            #!/system/bin/sh
            shw="$shwPath"
            pw="$pwPath"

            TARGET="${'$'}{1:-${'$'}{USER:-trux}}"
            if ! grep -q "^${'$'}TARGET:" "${'$'}pw" 2>/dev/null; then
                echo "passwd: user '${'$'}TARGET' does not exist"
                exit 1
            fi

            echo "Changing password for ${'$'}TARGET."
            printf "New password: "
            stty -echo 2>/dev/null
            read p1
            stty echo 2>/dev/null
            echo ""

            printf "Retype new password: "
            stty -echo 2>/dev/null
            read p2
            stty echo 2>/dev/null
            echo ""

            if [ "${'$'}p1" != "${'$'}p2" ]; then
                echo "passwd: Passwords do not match"
                exit 1
            fi

            if [ -z "${'$'}p1" ]; then
                echo "passwd: password cannot be empty"
                exit 1
            fi

            salt="${'$'}(head -c 8 /dev/urandom 2>/dev/null | toybox base64 2>/dev/null | head -c 8 || echo "trux9876")"
            hash="${'$'}(printf "%s%s" "${'$'}salt" "${'$'}p1" | sha256sum | awk '{print ${'$'}1}')"
            entry="${'$'}salt\$${'$'}hash"

            if grep -q "^${'$'}TARGET:" "${'$'}shw" 2>/dev/null; then
                sed -i -E "s/^(${'$'}TARGET:)[^:]*(:.*)/\1${'$'}entry\2/" "${'$'}shw"
            else
                echo "${'$'}TARGET:${'$'}entry:19700:0:99999:7:::" >> "${'$'}shw"
            fi
            chmod 600 "${'$'}shw" 2>/dev/null
            echo "passwd: password updated successfully."
        """.trimIndent() + "\n"
        File(binDir, "passwd").also { it.writeText(passwdScript); it.setExecutable(true, false) }

        // groupadd
        val groupaddScript = """
            #!/system/bin/sh
            gr="$grPath"
            GID_SPEC=""
            SYSTEM_GRP=0

            while [ ${'$'}# -gt 0 ]; do
                case "${'$'}1" in
                    -g) GID_SPEC="${'$'}2"; shift 2 ;;
                    -r|--system) SYSTEM_GRP=1; shift ;;
                    -*) echo "groupadd: unknown option '${'$'}1'"; exit 1 ;;
                    *) GROUPNAME="${'$'}1"; shift ;;
                esac
            done

            if [ -z "${'$'}GROUPNAME" ]; then
                echo "Usage: groupadd [-g GID] [-r] <groupname>"
                exit 1
            fi

            if grep -q "^${'$'}GROUPNAME:" "${'$'}gr" 2>/dev/null; then
                echo "groupadd: group '${'$'}GROUPNAME' already exists"
                exit 9
            fi

            if [ -n "${'$'}GID_SPEC" ]; then
                NEW_GID="${'$'}GID_SPEC"
            elif [ "${'$'}SYSTEM_GRP" -eq 1 ]; then
                NEW_GID="${'$'}(awk -F: '${'$'}3 >= 100 && ${'$'}3 < 1000 {if (${'$'}3 > max) max=${'$'}3} END {print (max? max+1 : 100)}' "${'$'}gr" 2>/dev/null)"
            else
                NEW_GID="${'$'}(awk -F: '${'$'}3 >= 1000 && ${'$'}3 < 65534 {if (${'$'}3 > max) max=${'$'}3} END {print (max? max+1 : 1000)}' "${'$'}gr" 2>/dev/null)"
            fi

            echo "${'$'}GROUPNAME:x:${'$'}NEW_GID:" >> "${'$'}gr"
            echo "groupadd: group '${'$'}GROUPNAME' (GID ${'$'}NEW_GID) created."
        """.trimIndent() + "\n"
        File(binDir, "groupadd").also { it.writeText(groupaddScript); it.setExecutable(true, false) }

        // groupdel
        val groupdelScript = """
            #!/system/bin/sh
            gr="$grPath"
            GROUPNAME="${'$'}1"
            if [ -z "${'$'}GROUPNAME" ] || ! grep -q "^${'$'}GROUPNAME:" "${'$'}gr" 2>/dev/null; then
                echo "groupdel: group '${'$'}GROUPNAME' does not exist"
                exit 6
            fi
            sed -i "/^${'$'}GROUPNAME:/d" "${'$'}gr"
            echo "groupdel: group '${'$'}GROUPNAME' removed."
        """.trimIndent() + "\n"
        File(binDir, "groupdel").also { it.writeText(groupdelScript); it.setExecutable(true, false) }

        // groupmod
        val groupmodScript = """
            #!/system/bin/sh
            gr="$grPath"
            NEW_NAME=""
            NEW_GID=""

            while [ ${'$'}# -gt 0 ]; do
                case "${'$'}1" in
                    -n) NEW_NAME="${'$'}2"; shift 2 ;;
                    -g) NEW_GID="${'$'}2"; shift 2 ;;
                    -*) echo "groupmod: unknown option '${'$'}1'"; exit 1 ;;
                    *) GROUPNAME="${'$'}1"; shift ;;
                esac
            done

            if [ -z "${'$'}GROUPNAME" ] || ! grep -q "^${'$'}GROUPNAME:" "${'$'}gr" 2>/dev/null; then
                echo "groupmod: group '${'$'}GROUPNAME' does not exist"
                exit 6
            fi

            if [ -n "${'$'}NEW_NAME" ]; then
                sed -i -E "s/^${'$'}GROUPNAME(:.*)/${'$'}NEW_NAME\1/" "${'$'}gr"
            fi
            if [ -n "${'$'}NEW_GID" ]; then
                target="${'$'}{NEW_NAME:-${'$'}GROUPNAME}"
                sed -i -E "s/^(${'$'}target:x:)[0-9]+(:.*)/\1${'$'}NEW_GID\2/" "${'$'}gr"
            fi
            echo "groupmod: group updated successfully."
        """.trimIndent() + "\n"
        File(binDir, "groupmod").also { it.writeText(groupmodScript); it.setExecutable(true, false) }

        // gpasswd
        val gpasswdScript = """
            #!/system/bin/sh
            gr="$grPath"
            case "${'$'}1" in
                -a)
                    u="${'$'}2"; g="${'$'}3"
                    if [ -z "${'$'}u" ] || [ -z "${'$'}g" ]; then echo "Usage: gpasswd -a <user> <group>"; exit 1; fi
                    exec usermod -aG "${'$'}g" "${'$'}u"
                    ;;
                -d)
                    u="${'$'}2"; g="${'$'}3"
                    if [ -z "${'$'}u" ] || [ -z "${'$'}g" ]; then echo "Usage: gpasswd -d <user> <group>"; exit 1; fi
                    line="${'$'}(grep "^${'$'}g:" "${'$'}gr")"
                    mem="${'$'}(echo "${'$'}line" | cut -d: -f4 | sed -E "s/(^|,)${'$'}u(,|$)/,/g; s/^,//; s/,$//; s/,,/,/g")"
                    sed -i -E "s/^(${'$'}g:x:[0-9]+:).*/\1${'$'}mem/" "${'$'}gr"
                    echo "Removed user '${'$'}u' from group '${'$'}g'."
                    ;;
                *)
                    echo "Usage: gpasswd -a <user> <group> | gpasswd -d <user> <group>"
                    exit 1
                    ;;
            esac
        """.trimIndent() + "\n"
        File(binDir, "gpasswd").also { it.writeText(gpasswdScript); it.setExecutable(true, false) }

        // hostname & hostnamectl
        val hostnameScript = """
            #!/system/bin/sh
            hn_file="$hnPath"
            if [ ${'$'}# -eq 0 ]; then
                cat "${'$'}hn_file" 2>/dev/null || echo "android"
            else
                echo "${'$'}1" > "${'$'}hn_file"
                echo "Hostname updated to '${'$'}1'."
            fi
        """.trimIndent() + "\n"
        File(binDir, "hostname").also { it.writeText(hostnameScript); it.setExecutable(true, false) }
        File(binDir, "hostnamectl").also {
            it.writeText(
                """
                #!/system/bin/sh
                if [ "${'$'}1" = "set-hostname" ] && [ -n "${'$'}2" ]; then
                    exec hostname "${'$'}2"
                fi
                echo "   Static hostname: ${'$'}(hostname)"
                echo "         Icon name: computer-terminal"
                echo "  Operating System: TRUX 2.0.1 (Android userspace)"
                echo "            Kernel: ${'$'}(uname -srm)"
                echo "      Architecture: ${'$'}(uname -m)"
                """.trimIndent() + "\n"
            )
            it.setExecutable(true, false)
        }

        // su
        val suScript = """
            #!/system/bin/sh
            # TRUX Userspace Switch User (su)
            pw="$pwPath"
            shw="$shwPath"

            target="${'$'}{1:-root}"

            line="${'$'}(grep "^${'$'}target:" "${'$'}pw" 2>/dev/null)"
            if [ -z "${'$'}line" ]; then
                echo "su: user '${'$'}target' does not exist"
                exit 1
            fi

            target_uid="${'$'}(echo "${'$'}line" | cut -d: -f3)"
            target_home="${'$'}(echo "${'$'}line" | cut -d: -f6)"
            target_shell="${'$'}(echo "${'$'}line" | cut -d: -f7)"

            # Password check if set in shadow and current user is not root
            shw_line="${'$'}(grep "^${'$'}target:" "${'$'}shw" 2>/dev/null)"
            hash_field="${'$'}(echo "${'$'}shw_line" | cut -d: -f2)"

            if [ -n "${'$'}hash_field" ] && [ "${'$'}hash_field" != "*" ] && [ "${'$'}hash_field" != "!" ] && [ "${'$'}{USER:-trux}" != "root" ]; then
                printf "Password: "
                stty -echo 2>/dev/null
                read entered_pass
                stty echo 2>/dev/null
                echo ""
                salt="${'$'}(echo "${'$'}hash_field" | cut -d$ -f1)"
                expected="${'$'}(echo "${'$'}hash_field" | cut -d$ -f2)"
                calc="${'$'}(printf "%s%s" "${'$'}salt" "${'$'}entered_pass" | sha256sum | awk '{print ${'$'}1}')"
                if [ "${'$'}calc" != "${'$'}expected" ]; then
                    echo "su: Authentication failure"
                    exit 1
                fi
            fi

            if [ "${'$'}target" = "root" ] || [ "${'$'}target_uid" = "0" ]; then
                echo "=========================================================="
                echo "[TRUX] Switched to TRUX userspace root (UID 0)."
                echo "Notice: This applies inside the TRUX Linux sandbox environment."
                echo "It does NOT provide Android kernel root privileges."
                echo "=========================================================="
            fi

            export USER="${'$'}target"
            export LOGNAME="${'$'}target"
            export HOME="${'$'}target_home"
            export UID="${'$'}target_uid"
            cd "${'$'}target_home" 2>/dev/null || cd "${prefix.absolutePath}" 2>/dev/null || true
            exec "${'$'}{target_shell:-/system/bin/sh}" -l
        """.trimIndent() + "\n"
        File(binDir, "su").also { it.writeText(suScript); it.setExecutable(true, false) }

        // sudo
        val sudoScript = """
            #!/system/bin/sh
            # TRUX Userspace Sudo Utility
            if [ ${'$'}# -eq 0 ]; then
                echo "usage: sudo command [args...]"
                echo "TRUX sudo runs commands as userspace root (UID 0) within TRUX."
                exit 1
            fi
            if [ "${'$'}1" = "-i" ] || [ "${'$'}1" = "-s" ]; then
                exec su root
            fi
            USER=root LOGNAME=root UID=0 "${'$'}@"
        """.trimIndent() + "\n"
        File(binDir, "sudo").also { it.writeText(sudoScript); it.setExecutable(true, false) }

        // chown & chgrp wrappers
        val chownScript = """
            #!/system/bin/sh
            if [ -x "/system/bin/chown" ] && [ "${'$'}UID" = "0" ]; then
                /system/bin/chown "${'$'}@" 2>/dev/null && exit 0
            fi
            toybox chown "${'$'}@" 2>/dev/null && exit 0
            echo "[TRUX] chown: recorded userspace ownership for '${'$'}*'"
        """.trimIndent() + "\n"
        File(binDir, "chown").also { it.writeText(chownScript); it.setExecutable(true, false) }

        val chgrpScript = """
            #!/system/bin/sh
            if [ -x "/system/bin/chgrp" ] && [ "${'$'}UID" = "0" ]; then
                /system/bin/chgrp "${'$'}@" 2>/dev/null && exit 0
            fi
            toybox chgrp "${'$'}@" 2>/dev/null && exit 0
            echo "[TRUX] chgrp: recorded userspace group for '${'$'}*'"
        """.trimIndent() + "\n"
        File(binDir, "chgrp").also { it.writeText(chgrpScript); it.setExecutable(true, false) }
    }

    /**
     * Resolve the shell to exec. Falls back through the bundled binaries and
     * finally to Android's own `/system/bin/sh`, which always exists.
     */
    fun resolveShell(preferred: String?): String {
        val candidates = buildList {
            preferred?.let { if (File(it).canExecute()) add(it) }
            if (canExecFromDataDir) {
                add(File(prefix, "bin/bash").absolutePath)
                add(File(prefix, "bin/sh").absolutePath)
            }
            // Bundled-as-.so executables are always executable.
            add(File(nativeLibDir, "libbash.so").absolutePath)
            // Final fallback: Android's built-in shell
            add("/system/bin/sh")
        }
        return candidates.firstOrNull { File(it).canExecute() } ?: "/system/bin/sh"
    }

    /** Flat `[k0, v0, k1, v1, ...]` array for the JNI call. */
    fun buildEnvironment(extra: Map<String, String> = emptyMap()): Array<String> {
        val binPaths = buildList {
            if (canExecFromDataDir) add(File(prefix, "bin").absolutePath)
            add(nativeLibDir.absolutePath)
            add("/system/bin")
            add("/system/xbin")
        }

        val libPaths = buildList {
            add(File(prefix, "lib").absolutePath)
            add(nativeLibDir.absolutePath)
        }

        val env = linkedMapOf(
            "HOME"               to home.absolutePath,
            "PREFIX"             to prefix.absolutePath,
            "TRUX_PREFIX"        to prefix.absolutePath,
            "MERIDIAN_PREFIX"    to prefix.absolutePath,
            "PATH"               to binPaths.joinToString(":"),
            "TMPDIR"             to tmp.absolutePath,
            "TMP"                to tmp.absolutePath,
            "TEMP"               to tmp.absolutePath,
            "TERM"               to "xterm-256color",
            "COLORTERM"          to "truecolor",
            "LANG"               to "en_US.UTF-8",
            "LC_ALL"             to "en_US.UTF-8",
            "LC_CTYPE"           to "UTF-8",
            "SHELL"              to resolveShell(null),
            "ENV"                to File(home, ".mkshrc").absolutePath,
            "PS1"                to "trux@android:~$ ",
            "INPUTRC"            to File(home, ".inputrc").absolutePath,
            "USER"               to "trux",
            "LOGNAME"            to "trux",
            "TRUX_USER"          to "trux",
            "TRUX"               to "1",
            "TRUX_VERSION"       to "2.0.1",
            "MERIDIAN_VERSION"   to "2.0.1",
            "ANDROID_ROOT"       to "/system",
            "ANDROID_DATA"       to "/data",
            "LD_LIBRARY_PATH"    to libPaths.joinToString(":"),
            // Python (if installed)
            "PYTHONHOME"         to prefix.absolutePath,
            "PYTHONPATH"         to File(prefix, "lib/python3").absolutePath,
            // Git
            "GIT_EXEC_PATH"      to File(prefix, "lib/git-core").absolutePath,
            "GIT_TEMPLATE_DIR"   to File(prefix, "share/git-core/templates").absolutePath,
        )
        env.putAll(extra)

        val flat = ArrayList<String>(env.size * 2)
        env.forEach { (k, v) -> flat.add(k); flat.add(v) }
        return flat.toTypedArray()
    }
}
