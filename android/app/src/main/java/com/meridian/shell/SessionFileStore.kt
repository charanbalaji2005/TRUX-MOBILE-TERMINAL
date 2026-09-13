package com.meridian.shell

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

/**
 * Manages the per-session filesystem directory.
 *
 * Each session gets a stable local identity at:
 *   <filesDir>/sessions/<session-id>/
 *
 * Structure:
 *   sessions/
 *   ├── <uuid-1>/
 *   │   ├── metadata.json     (name, shell, env, cwd, timestamps)
 *   │   └── files/            (session-private working area, optional)
 *   ├── <uuid-2>/
 *   │   └── ...
 *   └── ...
 *
 * This is PERSISTENT data. The session directory survives:
 *   - Activity recreation (rotation)
 *   - App backgrounding
 *   - Android process kill (OOM)
 *
 * It does NOT survive:
 *   - User clearing app data
 *   - "Clear Meridian data" in Settings → Storage
 */
class SessionFileStore(private val sessionsRoot: File) {

    init {
        sessionsRoot.mkdirs()
    }

    /** Create the directory for a new session. */
    suspend fun createSessionDir(sessionId: String, name: String): File =
        withContext(Dispatchers.IO) {
            val dir = sessionDir(sessionId)
            dir.mkdirs()
            File(dir, "files").mkdirs()
            // Write minimal metadata
            File(dir, "metadata.json").writeText(
                """{"id":"$sessionId","name":"${name.replace("\"", "\\\"")}","created":${System.currentTimeMillis()}}"""
            )
            dir
        }

    /** Delete the session directory on close. */
    suspend fun deleteSessionDir(sessionId: String) = withContext(Dispatchers.IO) {
        sessionDir(sessionId).deleteRecursively()
    }

    /** List all session IDs that have directories (for recovery). */
    suspend fun listSessionIds(): List<String> = withContext(Dispatchers.IO) {
        sessionsRoot.listFiles()
            ?.filter { it.isDirectory }
            ?.map { it.name }
            ?: emptyList()
    }

    /** Get the files/ subdirectory for a session (user's working area). */
    fun sessionFilesDir(sessionId: String): File =
        File(sessionDir(sessionId), "files").also { it.mkdirs() }

    /** Calculate storage used by a single session. */
    suspend fun sessionSize(sessionId: String): Long = withContext(Dispatchers.IO) {
        sessionDir(sessionId).walk().sumOf { it.length() }
    }

    private fun sessionDir(sessionId: String) = File(sessionsRoot, sessionId)
}
