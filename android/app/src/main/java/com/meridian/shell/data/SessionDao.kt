package com.meridian.shell.data

import androidx.room.*
import kotlinx.coroutines.flow.Flow

@Dao
interface SessionDao {

    /** Observe all non-closed sessions, ordered by last used descending. */
    @Query("SELECT * FROM sessions WHERE closed = 0 ORDER BY lastUsedAt DESC")
    fun observeActiveSessions(): Flow<List<SessionEntity>>

    /** One-shot query used during startup to restore session list. */
    @Query("SELECT * FROM sessions WHERE closed = 0 ORDER BY lastUsedAt DESC")
    suspend fun getActiveSessions(): List<SessionEntity>

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertOrReplace(session: SessionEntity)

    @Query("UPDATE sessions SET lastUsedAt = :ts WHERE id = :id")
    suspend fun touchSession(id: String, ts: Long = System.currentTimeMillis())

    @Query("UPDATE sessions SET name = :name WHERE id = :id")
    suspend fun rename(id: String, name: String)

    @Query("UPDATE sessions SET workingDirectory = :dir WHERE id = :id")
    suspend fun updateWorkingDirectory(id: String, dir: String)

    @Query("UPDATE sessions SET columns = :cols, rows = :rows WHERE id = :id")
    suspend fun updateSize(id: String, cols: Int, rows: Int)

    /** Soft-delete: marks the session closed so it stops appearing but keeps history. */
    @Query("UPDATE sessions SET closed = 1 WHERE id = :id")
    suspend fun markClosed(id: String)

    /** Hard delete, used by "Clear Meridian data". */
    @Query("DELETE FROM sessions")
    suspend fun deleteAll()
}
