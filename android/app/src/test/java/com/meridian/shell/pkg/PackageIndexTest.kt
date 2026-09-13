package com.meridian.shell.pkg

import org.junit.Assert.*
import org.junit.Test

/**
 * Unit tests for PackageIndex data models — version comparison logic and
 * JSON field parsing. Run on JVM without emulator.
 */
class PackageIndexTest {

    private fun entry(name: String, version: String) = PackageEntry(
        name = name,
        version = version,
        description = "Test package",
        url = "https://example.com/$name.tar.xz",
        sha256 = "abc123"
    )

    // ---- Version comparison ------------------------------------------------

    @Test fun `newer patch version is newer`() {
        val pkg = entry("bash", "5.2.21")
        assertTrue(pkg.isNewerThan("5.2.20"))
    }

    @Test fun `newer minor version is newer`() {
        val pkg = entry("python3", "3.12.0")
        assertTrue(pkg.isNewerThan("3.11.9"))
    }

    @Test fun `newer major version is newer`() {
        val pkg = entry("git", "3.0.0")
        assertTrue(pkg.isNewerThan("2.45.0"))
    }

    @Test fun `same version is not newer`() {
        val pkg = entry("curl", "8.5.0")
        assertFalse(pkg.isNewerThan("8.5.0"))
    }

    @Test fun `older version is not newer`() {
        val pkg = entry("bash", "5.1.0")
        assertFalse(pkg.isNewerThan("5.2.0"))
    }

    @Test fun `version with fewer dots handled correctly`() {
        val pkg = entry("sh", "1.0")
        assertTrue(pkg.isNewerThan("0.9"))
        assertFalse(pkg.isNewerThan("1.1"))
    }

    // ---- PackageEntry fields -----------------------------------------------

    @Test fun `entry has correct defaults`() {
        val e = entry("vim", "9.0")
        assertEquals("vim", e.name)
        assertEquals("9.0", e.version)
        assertEquals("aarch64", e.arch)
        assertEquals(0L, e.installed_size)
        assertTrue(e.depends.isEmpty())
    }

    @Test fun `manifest wraps package list`() {
        val manifest = PackageManifest(
            version = 1,
            packages = listOf(entry("bash", "5.2"), entry("python3", "3.12"))
        )
        assertEquals(2, manifest.packages.size)
        assertEquals("bash", manifest.packages[0].name)
    }

    // ---- Dependency list ---------------------------------------------------

    @Test fun `dependencies preserved`() {
        val e = PackageEntry(
            name = "git",
            version = "2.45.0",
            description = "Git VCS",
            url = "https://example.com/git.tar.xz",
            sha256 = "def456",
            depends = listOf("curl", "openssl")
        )
        assertEquals(listOf("curl", "openssl"), e.depends)
    }
}
