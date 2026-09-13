package com.meridian.shell

/**
 * The one and only place that talks to native code.
 *
 * Everything above this file deals in [Long] handles. No Kotlin class outside
 * this object knows that a C++ TerminalCore exists.
 */
object PtyBridge {

    @Volatile
    private var loaded = false

    /** Thrown instead of UnsatisfiedLinkError so the UI can show a real message. */
    class NativeUnavailable(cause: Throwable) :
        RuntimeException("Meridian native library failed to load", cause)

    fun ensureLoaded() {
        if (loaded) return
        synchronized(this) {
            if (loaded) return
            try {
                System.loadLibrary("meridian_native")
                loaded = true
            } catch (t: Throwable) {
                throw NativeUnavailable(t)
            }
        }
    }

    /**
     * @param env flat `[k0, v0, k1, v1, ...]` pairs
     * @return an opaque handle, or 0 if forkpty failed
     */
    external fun nativeCreateSession(
        shell: String,
        argv: Array<String>,
        env: Array<String>,
        cwd: String,
        cols: Int,
        rows: Int,
        scrollback: Int
    ): Long

    external fun nativeDestroySession(handle: Long)
    external fun nativeWrite(handle: Long, data: ByteArray, length: Int)
    external fun nativeResize(handle: Long, cols: Int, rows: Int)
    external fun nativeSignal(handle: Long, sig: Int)
    external fun nativeIsAlive(handle: Long): Boolean
    external fun nativeExitStatus(handle: Long): Int
    external fun nativeGeneration(handle: Long): Long
    external fun nativeAwaitChange(handle: Long, since: Long, timeoutMs: Int): Boolean
    external fun nativeSnapshot(handle: Long, cells: IntArray, meta: IntArray, scrollOffset: Int): Int
    external fun nativeDumpText(handle: Long): String

    const val SIGINT = 2
    const val SIGKILL = 9
    const val SIGTERM = 15
}
