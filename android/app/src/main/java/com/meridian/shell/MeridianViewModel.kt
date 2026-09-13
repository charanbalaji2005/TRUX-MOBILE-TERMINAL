package com.meridian.shell

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

/**
 * Lifecycle-aware wrapper around [SessionManager].
 *
 * The ViewModel survives configuration changes (rotation, keyboard show/hide),
 * so the Activity can be recreated without touching a single PTY.
 *
 * SessionManager itself is a process-scoped singleton; the ViewModel just
 * exposes it to Compose via the standard ViewModel lifecycle.
 *
 * IMPORTANT: onCleared() does NOT close sessions. Sessions outlive the
 * ViewModel — only a real process death (OOM kill, user force-stop) ends them.
 */
class MeridianViewModel(application: Application) : AndroidViewModel(application) {

    val manager: SessionManager = SessionManager.get(application)

    // Expose as delegated properties for convenient Compose observation
    val sessions get() = manager.sessions
    val activeSession get() = manager.activeSession

    init {
        // On first ViewModel creation, restore sessions from Room.
        // This is a no-op if sessions are already live (e.g., after rotation).
        if (manager.sessions.isEmpty()) {
            viewModelScope.launch {
                runCatching { manager.restoreFromDatabase() }
            }
        }
    }

    fun createSession(
        name: String? = null,
        shell: String? = null,
        environmentName: String = "meridian"
    ) {
        runCatching {
            val session = manager.createSession(name = name, shell = shell,
                environmentName = environmentName)
            manager.switchTo(session)
        }
    }

    fun switchTo(session: TerminalSession) = manager.switchTo(session)

    fun rename(session: TerminalSession, newName: String) = manager.rename(session, newName)

    fun close(session: TerminalSession) = manager.close(session)

    fun onSessionExited(session: TerminalSession) = manager.onSessionExited(session)

    fun cleanupStoppedSessions() = manager.cleanupStoppedSessions()

    fun restart(session: TerminalSession) = manager.restart(session)

    fun closeAll() = manager.closeAll()

    // ViewModel is NOT the session owner — we deliberately do nothing here.
    override fun onCleared() {
        super.onCleared()
        // Sessions live on. Only process death ends them.
    }
}
