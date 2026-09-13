package com.meridian.shell

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

/**
 * Unit tests for StorageManager's data structures and formatting logic.
 * Runs on the JVM without an emulator.
 */
class StorageManagerTest {

    @Test
    fun storageBreakdown_totalCalculationIsCorrect() {
        val breakdown = StorageManager.StorageBreakdown(
            sessionsBytes = 1024L * 1024L * 10L,     // 10 MB
            linuxEnvBytes = 1024L * 1024L * 100L,   // 100 MB
            packagesBytes = 1024L * 1024L * 50L,    // 50 MB
            packageCacheBytes = 1024L * 1024L * 20L,// 20 MB
            userFilesBytes = 1024L * 1024L * 5L     // 5 MB
        )

        // total = sessions + linuxEnv + packageCache + userFiles = 10 + 100 + 20 + 5 = 135 MB
        val expectedTotal = (10L + 100L + 20L + 5L) * 1024L * 1024L
        assertEquals(expectedTotal, breakdown.totalBytes)
    }

    @Test
    fun storageBreakdown_formatOutputsExpectedSections() {
        val breakdown = StorageManager.StorageBreakdown(
            sessionsBytes = 500L,
            linuxEnvBytes = 2L * 1024L * 1024L,
            packagesBytes = 1L * 1024L * 1024L,
            packageCacheBytes = 0L,
            userFilesBytes = 4096L
        )

        val formatted = breakdown.format()
        assertTrue("Contains Sessions header", formatted.contains("Sessions"))
        assertTrue("Contains Linux environment header", formatted.contains("Linux environment"))
        assertTrue("Contains Packages header", formatted.contains("Packages"))
        assertTrue("Contains Package cache header", formatted.contains("Package cache"))
        assertTrue("Contains User files header", formatted.contains("User files"))
        assertTrue("Contains Total header", formatted.contains("Total"))
    }
}
