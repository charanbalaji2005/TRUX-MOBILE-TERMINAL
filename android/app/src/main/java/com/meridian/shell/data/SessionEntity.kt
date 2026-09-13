package com.meridian.shell.data

import androidx.room.Entity
import androidx.room.PrimaryKey

/**
 * Persisted representation of a terminal session.
 *
 * Distinguishes between PERSISTENT data (stored here) and LIVE data (PTY/process).
 *
 * PERSISTENT — stored in Room:
 *   session metadata, working directory, preferences, environment name
 *
 * LIVE — NOT serializable to SQLite:
 *   the PTY file descriptor, process PID, process RAM, shell read buffer
 *
 * When the app process is killed and restarted, we restore session metadata from
 * this table but cannot restore the live process. The user sees named sessions
 * that can be re-launched.
 */
@Entity(tableName = "sessions")
data class SessionEntity(
    @PrimaryKey val id: String,

    /** User-visible name, e.g. "Main", "Python dev", "SSH to prod" */
    var name: String,

    /** Resolved shell path, e.g. /data/data/com.meridian.shell/files/usr/bin/bash */
    val shell: String,

    /** Environment name: "meridian", "debian", "ubuntu" */
    val environmentName: String,

    /** Last known working directory; restored as cwd on re-launch */
    var workingDirectory: String,

    /** Unix millis when this session was first created */
    val createdAt: Long,

    /** Unix millis of last user interaction; used for sorting */
    var lastUsedAt: Long,

    /** Last known terminal width */
    var columns: Int = 80,

    /** Last known terminal height */
    var rows: Int = 24,

    /** Whether this session was explicitly closed by the user (tombstone) */
    var closed: Boolean = false,

    /** Optional user notes attached to a session */
    var notes: String = ""
)
