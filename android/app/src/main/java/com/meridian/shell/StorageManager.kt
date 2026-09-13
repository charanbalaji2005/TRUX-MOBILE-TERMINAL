package com.meridian.shell

import android.content.Context
import com.meridian.shell.data.MeridianDatabase
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

/**
 * Calculates and manages filesystem storage for the Meridian userspace.
 *
 * All numbers come from real filesystem stat() calls — no fake values.
 *
 * Storage breakdown shown to users:
 *   Sessions             — filesDir/sessions/
 *   Linux environment    — filesDir/usr/ (the $PREFIX tree)
 *   Packages             — filesDir/usr/ (subset; same as above)
 *   Package cache        — filesDir/packages/cache/ (downloaded archives)
 *   User files           — filesDir/home/ (~)
 *   Total                — sum of all above
 */
class StorageManager(
    private val context: Context,
    private val bootstrap: Bootstrap,
    private val db: MeridianDatabase
) {
    data class StorageBreakdown(
        val sessionsBytes: Long,
        val linuxEnvBytes: Long,
        val packagesBytes: Long,
        val packageCacheBytes: Long,
        val userFilesBytes: Long
    ) {
        val totalBytes: Long get() =
            sessionsBytes + linuxEnvBytes + packageCacheBytes + userFilesBytes

        fun format(): String = buildString {
            appendLine("Storage")
            appendLine("")
            appendLine("Sessions          ${formatSize(sessionsBytes)}")
            appendLine("Linux environment ${formatSize(linuxEnvBytes)}")
            appendLine("Packages          ${formatSize(packagesBytes)}")
            appendLine("Package cache     ${formatSize(packageCacheBytes)}")
            appendLine("User files        ${formatSize(userFilesBytes)}")
            appendLine("─".repeat(40))
            appendLine("Total             ${formatSize(totalBytes)}")
        }

        private fun formatSize(bytes: Long): String = when {
            bytes < 1024 -> "$bytes B"
            bytes < 1024 * 1024 -> "%.1f KB".format(bytes / 1024.0)
            bytes < 1024 * 1024 * 1024 -> "%.1f MB".format(bytes / (1024.0 * 1024))
            else -> "%.2f GB".format(bytes / (1024.0 * 1024 * 1024))
        }
    }

    /**
     * Calculate storage usage. Runs on IO dispatcher; may take a second for
     * large installations.
     */
    suspend fun calculate(): StorageBreakdown = withContext(Dispatchers.IO) {
        StorageBreakdown(
            sessionsBytes = dirSize(File(context.filesDir, "sessions")),
            linuxEnvBytes = dirSize(bootstrap.prefix) + dirSize(bootstrap.environments),
            packagesBytes = dirSize(File(bootstrap.prefix, "bin")) +
                            dirSize(File(bootstrap.prefix, "lib")),
            packageCacheBytes = dirSize(bootstrap.packagesCache),
            userFilesBytes = dirSize(bootstrap.home)
        )
    }

    /**
     * Delete downloaded package archives (.tar.xz files in packages/cache/).
     * Does NOT delete installed packages or user files.
     */
    suspend fun clearPackageCache(): Long = withContext(Dispatchers.IO) {
        val cacheDir = bootstrap.packagesCache
        var freed = 0L
        cacheDir.listFiles()?.forEach { file ->
            freed += file.length()
            file.delete()
        }
        freed
    }

    /**
     * Wipe ALL Meridian data: sessions, linux environment, packages, settings, user files.
     *
     * This is a destructive, irreversible operation. Caller must confirm with the user
     * TWICE before calling this.
     *
     * After this call the app will restart as if freshly installed.
     */
    suspend fun clearAllData(): Unit = withContext(Dispatchers.IO) {
        // Close all live sessions first
        runCatching { SessionManager.get(context).closeAll() }

        // Delete filesystem data
        listOf(
            File(context.filesDir, "sessions"),
            bootstrap.prefix,
            bootstrap.environments,
            bootstrap.packagesCache,
            bootstrap.home
        ).forEach { it.deleteRecursively() }

        // Clear Room databases
        db.sessionDao().deleteAll()
        db.packageDao().deleteAll()

        // Clear DataStore preferences
        context.filesDir.resolve("datastore").deleteRecursively()
    }

    private fun dirSize(dir: File): Long {
        if (!dir.exists()) return 0L
        return dir.walk().filter { it.isFile }.sumOf { it.length() }
    }

    companion object {
        @Volatile private var instance: StorageManager? = null

        fun get(context: Context, bootstrap: Bootstrap): StorageManager =
            instance ?: synchronized(this) {
                instance ?: StorageManager(
                    context.applicationContext, bootstrap,
                    MeridianDatabase.get(context.applicationContext)
                ).also { instance = it }
            }
    }
}
