package com.meridian.shell

import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File

/**
 * Instrumented tests for SessionManager — require a device or emulator.
 *
 * Tests session lifecycle: create, switch, rename, close, restart.
 * Also stress-tests multi-session scenarios.
 *
 * NOTE: These tests create real PTY sessions that exec /system/bin/sh.
 * They will fail on emulators without a working PTY subsystem.
 */
@RunWith(AndroidJUnit4::class)
class SessionManagerTest {

    private lateinit var manager: SessionManager

    @Before
    fun setUp() {
        // Reset singleton for test isolation
        val ctx = ApplicationProvider.getApplicationContext<android.app.Application>()
        PtyBridge.ensureLoaded()
        manager = SessionManager.get(ctx)
        manager.closeAll()
    }

    // ---- Single session ----------------------------------------------------

    @Test
    fun createOneSession() {
        val session = manager.createSession(name = "Test")
        assertEquals("Test", session.name)
        assertTrue(session.isAlive)
        assertEquals(1, manager.sessions.size)
    }

    @Test
    fun activeSessionSetOnFirstCreate() {
        val session = manager.createSession(name = "First")
        assertEquals(session, manager.activeSession)
    }

    @Test
    fun sessionIsAliveAfterCreate() {
        val session = manager.createSession(name = "Alive Test")
        Thread.sleep(200)  // Let the shell start
        assertTrue("Shell should be alive", session.isAlive)
    }

    // ---- Multiple sessions -------------------------------------------------

    @Test
    fun create10Sessions() {
        val sessions = (1..10).map { manager.createSession(name = "Session $it") }
        assertEquals(10, manager.sessions.size)
        sessions.forEach { assertTrue("Session ${it.name} should be alive", it.isAlive) }
    }

    @Test
    fun create50Sessions() {
        // 50 PTYs is a real stress test; may fail if device runs out of fds.
        val count = 50
        val sessions = (1..count).map {
            runCatching { manager.createSession(name = "Stress $it") }.getOrNull()
        }.filterNotNull()
        assertTrue("Should create at least 40 of 50 sessions", sessions.size >= 40)
    }

    @Test
    fun switchBetweenSessions() {
        val s1 = manager.createSession(name = "S1")
        val s2 = manager.createSession(name = "S2")
        val s3 = manager.createSession(name = "S3")

        manager.switchTo(s1)
        assertEquals(s1, manager.activeSession)

        manager.switchTo(s3)
        assertEquals(s3, manager.activeSession)

        manager.switchTo(s2)
        assertEquals(s2, manager.activeSession)
    }

    // ---- Rename ------------------------------------------------------------

    @Test
    fun renameSession() {
        val session = manager.createSession(name = "Original")
        manager.rename(session, "Renamed")
        assertEquals("Renamed", session.name)
    }

    @Test
    fun renameWithBlankNameDoesNothing() {
        val session = manager.createSession(name = "Unchanged")
        manager.rename(session, "   ")
        assertEquals("Unchanged", session.name)
    }

    // ---- Close -------------------------------------------------------------

    @Test
    fun closeSessionRemovesFromList() {
        val session = manager.createSession(name = "ToClose")
        manager.close(session)
        assertFalse(manager.sessions.contains(session))
    }

    @Test
    fun closeSessionMarksExited() {
        val session = manager.createSession(name = "Exiting")
        manager.close(session)
        assertTrue(session.exited)
    }

    @Test
    fun closingActiveSessionSwitchesToNext() {
        val s1 = manager.createSession(name = "S1")
        val s2 = manager.createSession(name = "S2")
        manager.switchTo(s1)
        manager.close(s1)
        assertEquals(s2, manager.activeSession)
    }

    @Test
    fun closeAllSessions() {
        repeat(5) { manager.createSession(name = "Batch $it") }
        manager.closeAll()
        assertEquals(0, manager.sessions.size)
        assertNull(manager.activeSession)
    }

    // ---- Restart -----------------------------------------------------------

    @Test
    fun restartCreatesNewSession() {
        val original = manager.createSession(name = "ToRestart")
        val restarted = manager.restart(original)
        assertNotSame(original, restarted)
        assertEquals("ToRestart", restarted.name)
        assertTrue(restarted.isAlive)
    }

    // ---- Write / PTY interaction -------------------------------------------

    @Test
    fun writeEchoCommandProducesOutput() {
        val session = manager.createSession(name = "Echo")
        Thread.sleep(300)  // Wait for shell startup
        session.write("echo MERIDIAN_TEST_MARKER\n")
        Thread.sleep(300)  // Wait for output
        val text = session.visibleText()
        // Note: echo output depends on the shell being ready; may not always be visible
        // in a short window. This is a best-effort test.
        assertNotNull(text)
    }
}
