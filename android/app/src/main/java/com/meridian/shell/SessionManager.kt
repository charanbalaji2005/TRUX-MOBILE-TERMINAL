package com.meridian.shell

import android.content.Context
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import com.meridian.shell.data.MeridianDatabase
import com.meridian.shell.data.SessionEntity
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import java.io.File
import java.util.UUID

class SessionManager private constructor(
    private val bootstrap: Bootstrap,
    private val db: MeridianDatabase
) {
    class SessionLimitReached : RuntimeException(
        "Unable to create another session because device resources are currently insufficient."
    )

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    private val _sessions = mutableStateListOf<TerminalSession>()
    val sessions: List<TerminalSession> get() = _sessions

    var activeSession: TerminalSession? by mutableStateOf(null)
        private set


    fun createSession(
        name: String? = null,
        shell: String? = null,
        environmentName: String = "meridian",
        cwd: String? = null,
        cols: Int = 0,
        rows: Int = 0,
        scrollback: Int = 10_000
    ): TerminalSession {
        PtyBridge.ensureLoaded()
        bootstrap.ensureDirectories()

        val dm = bootstrap.context.resources.displayMetrics
        val isPortrait = dm.widthPixels < dm.heightPixels
        val defaultCols = if (cols > 0) cols else if (isPortrait) 44 else 80
        val defaultRows = if (rows > 0) rows else if (isPortrait) 28 else 24

        val resolvedShell = bootstrap.resolveShell(shell)
        val resolvedCwd = cwd ?: bootstrap.home.absolutePath
        val argv = arrayOf("-l")
        val env = bootstrap.buildEnvironment()

        val handle = PtyBridge.nativeCreateSession(
            resolvedShell, argv, env,
            resolvedCwd, defaultCols, defaultRows, scrollback
        )
        if (handle == 0L) throw SessionLimitReached()

        val id = UUID.randomUUID().toString()
        val sessionName = name ?: defaultName()
        val now = System.currentTimeMillis()

        val session = TerminalSession(
            id = id,
            initialName = sessionName,
            shell = resolvedShell,
            environmentName = environmentName,
            handle = handle,
            createdAt = now
        )
        session.resize(defaultCols, defaultRows)
        _sessions.add(session)
        if (activeSession == null) activeSession = session

        scope.launch {
            db.sessionDao().insertOrReplace(
                SessionEntity(
                    id = id,
                    name = sessionName,
                    shell = resolvedShell,
                    environmentName = environmentName,
                    workingDirectory = resolvedCwd,
                    createdAt = now,
                    lastUsedAt = now,
                    columns = defaultCols,
                    rows = defaultRows
                )
            )
        }

        return session
    }

    fun switchTo(session: TerminalSession) {
        if (_sessions.contains(session)) {
            activeSession = session
            scope.launch { db.sessionDao().touchSession(session.id) }
        }
    }

    fun rename(session: TerminalSession, newName: String) {
        if (newName.isBlank()) return
        session.name = newName.trim()
        scope.launch { db.sessionDao().rename(session.id, newName.trim()) }
    }

    fun persistSessionLog(session: TerminalSession) {
        scope.launch(Dispatchers.IO) {
            runCatching {
                val text = session.visibleText()
                if (text.isNotBlank()) {
                    val dir = File(bootstrap.filesDir, "sessions").also { it.mkdirs() }
                    File(dir, "${session.id}.log").writeText(text)
                    val safeName = session.name.replace(Regex("[^a-zA-Z0-9_-]"), "_")
                    File(dir, "$safeName.log").writeText(text)

                    // Also store in user's home .sessions/ directory for CLI access
                    val userDir = File(bootstrap.home, ".sessions").also { it.mkdirs() }
                    File(userDir, "$safeName.log").writeText(text)
                }
            }
        }
    }

    fun saveAllActiveSessions() {
        _sessions.forEach { persistSessionLog(it) }
    }

    fun close(session: TerminalSession) {
        val index = _sessions.indexOf(session)
        if (index < 0) return
        persistSessionLog(session)
        session.close()
        _sessions.removeAt(index)
        if (activeSession === session) {
            activeSession = _sessions.getOrNull(index) ?: _sessions.lastOrNull()
        }
        scope.launch { db.sessionDao().markClosed(session.id) }
    }

    /**
     * Automatically called when a shell process exits (e.g. user typed 'exit' or Ctrl+D).
     * Stops and removes the session immediately so dead sessions do not linger.
     */
    fun onSessionExited(session: TerminalSession) {
        val index = _sessions.indexOf(session)
        if (index < 0) return
        persistSessionLog(session)
        session.close()
        _sessions.removeAt(index)
        scope.launch { db.sessionDao().markClosed(session.id) }
        if (activeSession === session) {
            activeSession = _sessions.getOrNull(index) ?: _sessions.lastOrNull()
        }
        // If all sessions closed, immediately create a fresh new session
        if (_sessions.isEmpty()) {
            runCatching { createSession(name = "Main") }
        }
    }

    /**
     * Purges all stopped/dead sessions from memory and database.
     */
    fun cleanupStoppedSessions() {
        val dead = _sessions.filter { it.exited || !it.isAlive }
        dead.forEach { close(it) }
        scope.launch {
            runCatching {
                db.sessionDao().getActiveSessions().forEach { entity ->
                    db.sessionDao().markClosed(entity.id)
                }
            }
        }
    }

    fun restart(session: TerminalSession): TerminalSession {
        val name = session.name
        val cols = session.cols
        val rows = session.rows
        val cwd = session.lastKnownCwd
        val env = session.environmentName
        val index = _sessions.indexOf(session)
        close(session)
        val fresh = createSession(name = name, environmentName = env,
            cwd = cwd, cols = cols, rows = rows)
        if (index in 0.._sessions.size - 1) {
            _sessions.remove(fresh)
            _sessions.add(index, fresh)
        }
        activeSession = fresh
        return fresh
    }

    fun updateSessionSize(session: TerminalSession, cols: Int, rows: Int) {
        scope.launch { db.sessionDao().updateSize(session.id, cols, rows) }
    }

    fun closeAll() {
        _sessions.toList().forEach { close(it) }
    }

    suspend fun restoreFromDatabase() {
        val saved = db.sessionDao().getActiveSessions()
        if (saved.isEmpty()) {
            createSession(name = "Main")
            return
        }
        for (entity in saved) {
            runCatching {
                createSessionFromEntity(entity)
            }.onFailure {
                // If restore fails, mark it closed in DB
                scope.launch { db.sessionDao().markClosed(entity.id) }
            }
        }
        if (_sessions.isEmpty()) {
            createSession(name = "Main")
        }
    }

    private fun createSessionFromEntity(entity: SessionEntity): TerminalSession {
        PtyBridge.ensureLoaded()
        bootstrap.ensureDirectories()

        val resolvedShell = bootstrap.resolveShell(entity.shell)
        val env = bootstrap.buildEnvironment()

        val handle = PtyBridge.nativeCreateSession(
            resolvedShell, arrayOf("-l"), env,
            entity.workingDirectory,
            entity.columns, entity.rows, 10_000
        )
        if (handle == 0L) throw SessionLimitReached()

        val session = TerminalSession(
            id = entity.id,
            initialName = entity.name,
            shell = resolvedShell,
            environmentName = entity.environmentName,
            handle = handle,
            createdAt = entity.createdAt
        )
        session.resize(entity.columns, entity.rows)
        _sessions.add(session)
        if (activeSession == null) activeSession = session

        scope.launch { db.sessionDao().touchSession(entity.id) }
        return session
    }

    private fun defaultName(): String =
        if (_sessions.isEmpty()) "Main" else "Session ${_sessions.size + 1}"


    companion object {
        @Volatile
        private var instance: SessionManager? = null

        fun get(context: Context): SessionManager =
            instance ?: synchronized(this) {
                instance ?: SessionManager(
                    Bootstrap(context.applicationContext),
                    MeridianDatabase.get(context.applicationContext)
                ).also { instance = it }
            }
    }
}
