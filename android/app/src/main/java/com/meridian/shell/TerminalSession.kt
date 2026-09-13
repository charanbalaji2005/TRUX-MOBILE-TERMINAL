package com.meridian.shell

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import java.util.concurrent.atomic.AtomicLong

/**
 * One live terminal. Owns a native handle; owns no UI.
 *
 * A session outlives the Activity. It is destroyed only by an explicit
 * [close], never by a configuration change.
 *
 * PERSISTENT state (readable from this object, mirrored to Room):
 *   id, name, shell, environmentName, createdAt, lastKnownCwd, cols, rows
 *
 * LIVE state (exists only while process is alive):
 *   handle (native PTY fd + parser), exited, exitStatus
 */
class TerminalSession(
    val id: String,
    initialName: String,
    val shell: String,
    val environmentName: String,
    private val handle: Long,
    val createdAt: Long = System.currentTimeMillis()
) {
    var name: String by mutableStateOf(initialName)
        internal set

    var cols: Int = 80
        private set
    var rows: Int = 24
        private set

    /** Last known working directory (best-effort, updated via OSC 7 or user action). */
    var lastKnownCwd: String = ""
        internal set

    var exited: Boolean by mutableStateOf(false)
        internal set
    var exitStatus: Int = -1
        internal set

    @Suppress("unused")
    private val lastRendered = AtomicLong(0)

    val isAlive: Boolean get() = !exited && PtyBridge.nativeIsAlive(handle)

    fun write(text: String) = write(text.toByteArray(Charsets.UTF_8))

    fun write(bytes: ByteArray) {
        if (exited) return
        PtyBridge.nativeWrite(handle, bytes, bytes.size)
    }

    fun feedOutput(text: String) = feedOutput(text.toByteArray(Charsets.UTF_8))

    fun feedOutput(bytes: ByteArray) {
        if (exited) return
        PtyBridge.nativeFeedOutput(handle, bytes, bytes.size)
    }

    fun resize(newCols: Int, newRows: Int, force: Boolean = false) {
        if (newCols <= 0 || newRows <= 0) return
        if (!force && newCols == cols && newRows == rows) return
        cols = newCols
        rows = newRows
        PtyBridge.nativeResize(handle, newCols, newRows)
    }

    fun interrupt() = PtyBridge.nativeSignal(handle, PtyBridge.SIGINT)

    fun generation(): Long = PtyBridge.nativeGeneration(handle)

    /** Blocks the calling (render) thread until the screen changes. */
    fun awaitChange(since: Long, timeoutMs: Int): Boolean =
        PtyBridge.nativeAwaitChange(handle, since, timeoutMs)

    /**
     * Fills caller-owned arrays. [cells] must hold rows*cols*3 ints.
     * Returns ints written, or -1 if the array is too small (grow and retry).
     */
    fun snapshot(cells: IntArray, meta: IntArray, scrollOffset: Int = 0): Int =
        PtyBridge.nativeSnapshot(handle, cells, meta, scrollOffset)

    fun visibleText(): String = PtyBridge.nativeDumpText(handle)

    internal fun markExited() {
        if (exited) return
        exitStatus = PtyBridge.nativeExitStatus(handle)
        exited = true
    }

    internal fun close() {
        PtyBridge.nativeDestroySession(handle)
        exited = true
    }
}
