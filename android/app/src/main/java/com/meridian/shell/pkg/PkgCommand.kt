package com.meridian.shell.pkg

import android.content.Context
import com.meridian.shell.Bootstrap
import com.meridian.shell.data.MeridianDatabase

/**
 * `pkg` command integration.
 *
 * The `pkg` command is a shell script installed at $PREFIX/bin/pkg that
 * sends an Android broadcast intent to MeridianService. The service then
 * calls PackageInstaller and streams output back to the PTY via a named pipe.
 *
 * Shell script template (installed to $PREFIX/bin/pkg):
 * -------------------------------------------------------
 * #!/system/bin/sh
 * am broadcast -a com.meridian.shell.action.PKG \
 *   --es args "$*" \
 *   -n com.meridian.shell/.MeridianService \
 *   > /dev/null 2>&1
 * -------------------------------------------------------
 *
 * This writes output to a named FIFO at /tmp/meridian_pkg_out that the
 * shell script reads: cat /tmp/meridian_pkg_out
 *
 * For simplicity in the initial implementation, pkg runs as an in-process
 * coroutine and writes to the PTY through a registered output callback.
 */
class PkgCommand(
    private val context: Context,
    private val installer: PackageInstaller,
    private val repo: PackageRepository
) {
    /**
     * Execute a pkg command parsed from the shell.
     *
     * Usage:
     *   pkg update
     *   pkg search <query>
     *   pkg install <name> <name2...>
     *   pkg remove <name>
     *   pkg upgrade
     *   pkg list-installed
     *   pkg info <name>
     *
     * @param args The command arguments (first element is the subcommand)
     * @param output Callback that writes lines to the PTY
     */
    suspend fun execute(args: List<String>, output: (String) -> Unit) {
        if (args.isEmpty()) {
            printUsage(output)
            return
        }

        when (val subcmd = args[0]) {
            "update", "upgrade" -> {
                if (subcmd == "update") {
                    output("[pkg] Refreshing package index...")
                    val result = repo.getManifest(forceRefresh = true)
                    result.onSuccess { m ->
                        output("[pkg] Package index updated. ${m.packages.size} packages available.")
                    }.onFailure { e ->
                        output("[pkg] ERROR: ${e.message}")
                        output("[pkg] Check your network connection.")
                    }
                } else {
                    installer.upgrade(output)
                }
            }

            "search" -> {
                val query = args.drop(1).joinToString(" ")
                if (query.isBlank()) {
                    output("[pkg] Usage: pkg search <query>")
                    return
                }
                output("[pkg] Searching for '$query'...")
                val results = repo.search(query)
                if (results.isEmpty()) {
                    output("[pkg] No packages found matching '$query'.")
                } else {
                    output("[pkg] Found ${results.size} package(s):")
                    output("")
                    results.forEach { p ->
                        output("  %-20s %s".format(p.name, p.description))
                    }
                }
            }

            "install" -> {
                val names = args.drop(1)
                if (names.isEmpty()) {
                    output("[pkg] Usage: pkg install <package> [package2...]")
                    return
                }
                for (name in names) {
                    val ok = installer.install(name, output)
                    if (!ok) output("[pkg] Failed to install $name.")
                }
            }

            "remove", "uninstall" -> {
                val name = args.getOrNull(1)
                if (name == null) {
                    output("[pkg] Usage: pkg remove <package>")
                    return
                }
                installer.remove(name, output)
            }

            "list-installed", "list" -> {
                val installed = installer.listInstalled()
                if (installed.isEmpty()) {
                    output("[pkg] No packages installed.")
                    output("[pkg] Run 'pkg update && pkg install bash' to get started.")
                } else {
                    output("[pkg] Installed packages (${installed.size}):")
                    output("")
                    installed.forEach { p ->
                        output("  %-20s %s".format(p.name, p.version))
                    }
                }
            }

            "info" -> {
                val name = args.getOrNull(1)
                if (name == null) {
                    output("[pkg] Usage: pkg info <package>")
                    return
                }
                val entry = repo.find(name)
                if (entry == null) {
                    output("[pkg] Package '$name' not found.")
                    return
                }
                val installed = installer.listInstalled().firstOrNull { it.name == name }
                output("")
                output("Name:         ${entry.name}")
                output("Version:      ${entry.version}")
                output("Description:  ${entry.description}")
                output("Installed:    ${if (installed != null) "Yes (${installed.version})" else "No"}")
                output("Size:         ${entry.installed_size / 1024} KB")
                output("Depends:      ${entry.depends.joinToString(", ").ifEmpty { "(none)" }}")
                output("URL:          ${entry.url}")
                output("")
            }

            else -> {
                output("[pkg] Unknown command: '$subcmd'")
                printUsage(output)
            }
        }
    }

    private fun printUsage(output: (String) -> Unit) {
        output("")
        output("Usage: pkg <command> [options]")
        output("")
        output("Commands:")
        output("  update              Refresh package index")
        output("  upgrade             Upgrade all installed packages")
        output("  search <query>      Search available packages")
        output("  install <name>      Install a package")
        output("  remove <name>       Remove a package")
        output("  list-installed      List installed packages")
        output("  info <name>         Show package details")
        output("")
    }

    companion object {
        /**
         * Write the `pkg` shell wrapper script to $PREFIX/bin/pkg.
         * This script is what the user actually types in the terminal.
         */
        fun installShellWrapper(bootstrap: Bootstrap) {
            val binDir = bootstrap.prefix.resolve("bin")
            binDir.mkdirs()
            val pkgScript = binDir.resolve("pkg")

            val script = """
                #!/system/bin/sh
                # Meridian pkg — package manager
                # Routes commands through MeridianService via Android intent.
                # The service runs PackageInstaller in-process and pipes output back.
                am broadcast -a com.meridian.shell.action.PKG \
                  --esa pkg_args "${"\$"}@" \
                  -n com.meridian.shell/.MeridianService \
                  > /dev/null 2>&1
                # Wait for output pipe and cat it
                FIFO="${"\$TMPDIR"}/meridian_pkg_out"
                if [ -p "${'$'}FIFO" ]; then
                    cat "${'$'}FIFO"
                fi
            """.trimIndent()

            pkgScript.writeText(script)
            pkgScript.setExecutable(true, false)
        }
    }
}
