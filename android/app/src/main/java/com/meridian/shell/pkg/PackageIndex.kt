package com.meridian.shell.pkg

import kotlinx.serialization.Serializable

/**
 * Data models for the Meridian package repository.
 *
 * Repository format: packages.json at REPO_BASE_URL
 *
 * {
 *   "version": 1,
 *   "packages": [
 *     {
 *       "name": "bash",
 *       "version": "5.2.21",
 *       "description": "The GNU Bourne Again shell",
 *       "arch": "aarch64",
 *       "url": "https://repo.meridianshell.app/packages/bash_5.2.21_aarch64.tar.xz",
 *       "sha256": "abc123...",
 *       "installed_size": 2097152,
 *       "depends": []
 *     }
 *   ]
 * }
 */

@Serializable
data class PackageManifest(
    val version: Int = 1,
    val packages: List<PackageEntry> = emptyList()
)

@Serializable
data class PackageEntry(
    val name: String,
    val version: String,
    val description: String,
    val arch: String = "aarch64",
    /** Download URL for the .tar.xz archive */
    val url: String,
    /** SHA-256 hex digest of the archive */
    val sha256: String,
    /** Installed size in bytes (uncompressed) */
    val installed_size: Long = 0L,
    /** Runtime dependencies (must be installed first) */
    val depends: List<String> = emptyList()
) {
    /** Compare versions using simple dot-separated integer tuples. */
    fun isNewerThan(other: String): Boolean {
        return parseVersion(version) > parseVersion(other)
    }

    private fun parseVersion(v: String): List<Int> =
        v.split("[._-]".toRegex()).mapNotNull { it.toIntOrNull() }

    private operator fun List<Int>.compareTo(other: List<Int>): Int {
        val max = maxOf(size, other.size)
        for (i in 0 until max) {
            val a = getOrElse(i) { 0 }
            val b = other.getOrElse(i) { 0 }
            if (a != b) return a.compareTo(b)
        }
        return 0
    }
}
