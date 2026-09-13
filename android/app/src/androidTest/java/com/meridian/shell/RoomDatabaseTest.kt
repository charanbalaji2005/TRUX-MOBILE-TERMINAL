package com.meridian.shell

import androidx.room.Room
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.meridian.shell.data.MeridianDatabase
import com.meridian.shell.data.SessionDao
import com.meridian.shell.data.SessionEntity
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Instrumented tests for Room database — require a device or emulator.
 *
 * Tests the SessionEntity/SessionDao contract to ensure persistence
 * behaves correctly for the key operations that SessionManager relies on.
 */
@RunWith(AndroidJUnit4::class)
class RoomDatabaseTest {

    private lateinit var db: MeridianDatabase
    private lateinit var dao: SessionDao

    @Before
    fun setUp() {
        db = Room.inMemoryDatabaseBuilder(
            ApplicationProvider.getApplicationContext(),
            MeridianDatabase::class.java
        ).build()
        dao = db.sessionDao()
    }

    @After
    fun tearDown() {
        db.close()
    }

    private fun makeEntity(id: String, name: String): SessionEntity = SessionEntity(
        id = id,
        name = name,
        shell = "/system/bin/sh",
        environmentName = "meridian",
        workingDirectory = "/data/data/com.meridian.shell/files/home",
        createdAt = System.currentTimeMillis(),
        lastUsedAt = System.currentTimeMillis()
    )

    // ---- Insert and query --------------------------------------------------

    @Test
    fun insertAndRetrieve() = runBlocking {
        val entity = makeEntity("id-1", "Main")
        dao.insertOrReplace(entity)
        val sessions = dao.getActiveSessions()
        assertEquals(1, sessions.size)
        assertEquals("Main", sessions[0].name)
    }

    @Test
    fun insertMultipleAndRetrieveAll() = runBlocking {
        for (i in 1..10) {
            dao.insertOrReplace(makeEntity("id-$i", "Session $i"))
        }
        val sessions = dao.getActiveSessions()
        assertEquals(10, sessions.size)
    }

    // ---- Update operations -------------------------------------------------

    @Test
    fun renameUpdatesName() = runBlocking {
        dao.insertOrReplace(makeEntity("id-rename", "OldName"))
        dao.rename("id-rename", "NewName")
        val sessions = dao.getActiveSessions()
        assertEquals("NewName", sessions.first { it.id == "id-rename" }.name)
    }

    @Test
    fun touchUpdatesLastUsed() = runBlocking {
        val before = System.currentTimeMillis() - 1000
        dao.insertOrReplace(makeEntity("id-touch", "Touch Me").copy(lastUsedAt = before))
        dao.touchSession("id-touch")
        val sessions = dao.getActiveSessions()
        assertTrue(sessions.first { it.id == "id-touch" }.lastUsedAt > before)
    }

    @Test
    fun updateSizeStored() = runBlocking {
        dao.insertOrReplace(makeEntity("id-size", "Resize Test"))
        dao.updateSize("id-size", 120, 40)
        val sessions = dao.getActiveSessions()
        val s = sessions.first { it.id == "id-size" }
        assertEquals(120, s.columns)
        assertEquals(40, s.rows)
    }

    // ---- Soft delete -------------------------------------------------------

    @Test
    fun markClosedExcludesFromActiveSessions() = runBlocking {
        dao.insertOrReplace(makeEntity("id-close", "To Close"))
        dao.markClosed("id-close")
        val sessions = dao.getActiveSessions()
        assertTrue(sessions.none { it.id == "id-close" })
    }

    @Test
    fun closedSessionNotReturnedInActiveList() = runBlocking {
        dao.insertOrReplace(makeEntity("id-keep", "Keep"))
        dao.insertOrReplace(makeEntity("id-close2", "Close"))
        dao.markClosed("id-close2")
        val sessions = dao.getActiveSessions()
        assertEquals(1, sessions.size)
        assertEquals("Keep", sessions[0].name)
    }

    // ---- Hard delete -------------------------------------------------------

    @Test
    fun deleteAllClearsTable() = runBlocking {
        repeat(5) { dao.insertOrReplace(makeEntity("bulk-$it", "Bulk $it")) }
        dao.deleteAll()
        assertEquals(0, dao.getActiveSessions().size)
    }

    // ---- Idempotent upsert -------------------------------------------------

    @Test
    fun insertOrReplaceUpdatesExisting() = runBlocking {
        val original = makeEntity("id-upsert", "Original")
        dao.insertOrReplace(original)
        dao.insertOrReplace(original.copy(name = "Updated"))
        val sessions = dao.getActiveSessions()
        assertEquals(1, sessions.size)
        assertEquals("Updated", sessions[0].name)
    }
}
