package com.meridian.shell

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

/**
 * Manages Linux-compatible environments within Meridian.
 *
 * Each environment is a rootfs directory under:
 *   <filesDir>/environments/<name>/rootfs/
 *
 * Architecture:
 *   Android kernel
 *       │
 *       ▼
 *   Meridian userspace (always present, the default)
 *       │
 *       ├── Debian rootfs  (optional, downloaded on demand)
 *       │
 *       └── Ubuntu rootfs  (optional, downloaded on demand)
 *
 * Technical note on Debian/Ubuntu:
 *   Full chroot requires either root (not available) or proot (userspace
 *   ptrace-based chroot). proot is a real binary (~500 KB) that we would
 *   ship as a bundled .so and execute. This is architecture is wired but
 *   proot binary bundling is P2 — right now enter() returns a Meridian
 *   session with the appropriate cwd/env set.
 */
class EnvironmentManager(
    private val context: Context,
    private val bootstrap: Bootstrap
) {

    enum class EnvironmentType(val id: String, val displayName: String) {
        MERIDIAN("trux", "TRUX"),
        DEBIAN("debian", "Debian"),
        UBUNTU("ubuntu", "Ubuntu")
    }

    data class InstalledEnvironment(
        val type: EnvironmentType,
        val version: String,
        val sizeBytes: Long,
        val rootfsPath: String
    )

    private val envBaseDir = bootstrap.environments

    // ---- query -------------------------------------------------------------

    fun list(): List<InstalledEnvironment> {
        return EnvironmentType.values().mapNotNull { type ->
            val rootfs = rootfsDir(type)
            if (type == EnvironmentType.MERIDIAN || rootfs.exists()) {
                InstalledEnvironment(
                    type = type,
                    version = readVersion(type),
                    sizeBytes = if (type == EnvironmentType.MERIDIAN)
                        bootstrap.prefix.walk().sumOf { it.length() }
                    else
                        rootfs.walk().sumOf { it.length() },
                    rootfsPath = if (type == EnvironmentType.MERIDIAN)
                        bootstrap.prefix.absolutePath
                    else
                        rootfs.absolutePath
                )
            } else null
        }
    }

    fun isInstalled(type: EnvironmentType): Boolean =
        type == EnvironmentType.MERIDIAN || rootfsDir(type).exists()

    /**
     * Build the PtySpawnSpec for entering an environment.
     * For Meridian: normal shell in home directory.
     * For Debian/Ubuntu: would use proot to enter rootfs (P2 — currently
     * falls back to Meridian with env var set to identify the environment).
     */
    fun buildLaunchSpec(type: EnvironmentType): EnvironmentLaunchSpec {
        val rootfs = rootfsDir(type)

        return when (type) {
            EnvironmentType.MERIDIAN -> EnvironmentLaunchSpec(
                shell = bootstrap.resolveShell(null),
                cwd = bootstrap.home.absolutePath,
                extraEnv = mapOf("MERIDIAN_ENV" to "meridian"),
                prootArgs = null
            )
            EnvironmentType.DEBIAN, EnvironmentType.UBUNTU -> {
                if (!rootfs.exists()) {
                    // Not installed — return Meridian as fallback
                    return EnvironmentLaunchSpec(
                        shell = bootstrap.resolveShell(null),
                        cwd = bootstrap.home.absolutePath,
                        extraEnv = mapOf("MERIDIAN_ENV" to "meridian"),
                        prootArgs = null
                    )
                }
                // P2: when proot is bundled, this builds the proot invocation:
                // proot -r <rootfs> -w / /bin/bash -l
                val prootBin = File(bootstrap.nativeLibDir, "libproot.so")
                if (prootBin.canExecute()) {
                    EnvironmentLaunchSpec(
                        shell = prootBin.absolutePath,
                        cwd = bootstrap.home.absolutePath,
                        extraEnv = mapOf(
                            "MERIDIAN_ENV" to type.id,
                            "PROOT_TMP_DIR" to bootstrap.tmp.absolutePath
                        ),
                        prootArgs = listOf(
                            "-r", rootfs.absolutePath,
                            "-b", "/dev", "-b", "/proc", "-b", "/sys",
                            "-w", "/root",
                            "/bin/bash", "-l"
                        )
                    )
                } else {
                    // proot not bundled — fall back to Meridian with a warning message
                    EnvironmentLaunchSpec(
                        shell = bootstrap.resolveShell(null),
                        cwd = bootstrap.home.absolutePath,
                        extraEnv = mapOf(
                            "MERIDIAN_ENV" to type.id,
                            "MERIDIAN_ENV_ROOTFS" to rootfs.absolutePath
                        ),
                        prootArgs = null
                    )
                }
            }
        }
    }

    /**
     * Install a Debian or Ubuntu rootfs by downloading from the Meridian
     * environment repository.
     *
     * The rootfs is a .tar.xz of a minimal debootstrap image (~100-300 MB).
     *
     * @param type Environment to install
     * @param output Progress callback (written to PTY)
     */
    suspend fun install(type: EnvironmentType, output: (String) -> Unit): Boolean =
        withContext(Dispatchers.IO) {
            if (type == EnvironmentType.MERIDIAN) {
                output("[env] TRUX is the base environment and is always present.")
                return@withContext true
            }

            val distroUrl = distroUrl(type)
            output("[env] Downloading ${type.displayName} rootfs from $distroUrl ...")
            output("[env] This is a large download (~150 MB). Please wait.")

            // Distro download placeholder for future proot distribution
            output("[env] ERROR: ${type.displayName} rootfs download is not yet available.")
            output("[env] Please check TRUX releases for rootfs archives.")
            false
        }

    /**
     * Remove an installed environment (does NOT remove the base TRUX env).
     */
    suspend fun remove(type: EnvironmentType, output: (String) -> Unit): Boolean =
        withContext(Dispatchers.IO) {
            if (type == EnvironmentType.MERIDIAN) {
                output("[env] Cannot remove the base TRUX environment.")
                return@withContext false
            }
            val rootfs = rootfsDir(type)
            if (!rootfs.exists()) {
                output("[env] ${type.displayName} is not installed.")
                return@withContext false
            }
            output("[env] Removing ${type.displayName}...")
            rootfs.deleteRecursively()
            output("[env] ${type.displayName} removed.")
            true
        }

    /** Calculate storage used by an environment. */
    suspend fun storageUsage(type: EnvironmentType): Long =
        withContext(Dispatchers.IO) {
            if (type == EnvironmentType.MERIDIAN) {
                bootstrap.prefix.walk().sumOf { it.length() }
            } else {
                rootfsDir(type).walk().sumOf { it.length() }
            }
        }

    // ---- helpers -----------------------------------------------------------

    private fun rootfsDir(type: EnvironmentType): File =
        File(envBaseDir, "${type.id}/rootfs")

    private fun readVersion(type: EnvironmentType): String {
        if (type == EnvironmentType.MERIDIAN) return "2.0.0"
        val versionFile = File(envBaseDir, "${type.id}/VERSION")
        return if (versionFile.exists()) versionFile.readText().trim() else "unknown"
    }

    private fun distroUrl(type: EnvironmentType): String {
        val base = "https://raw.githubusercontent.com/meridianshell/distros/main"
        return when (type) {
            EnvironmentType.DEBIAN -> "$base/debian-bookworm-aarch64-rootfs.tar.xz"
            EnvironmentType.UBUNTU -> "$base/ubuntu-noble-aarch64-rootfs.tar.xz"
            EnvironmentType.MERIDIAN -> ""
        }
    }

    companion object {
        /** Singleton to avoid repeated instantiation. */
        @Volatile
        private var instance: EnvironmentManager? = null

        fun get(context: Context, bootstrap: Bootstrap): EnvironmentManager =
            instance ?: synchronized(this) {
                instance ?: EnvironmentManager(context.applicationContext, bootstrap)
                    .also { instance = it }
            }
    }
}

/** Spec for launching a shell in a given environment. */
data class EnvironmentLaunchSpec(
    val shell: String,
    val cwd: String,
    val extraEnv: Map<String, String>,
    /** If non-null, these args are prepended before the shell (proot usage). */
    val prootArgs: List<String>?
)
