package com.meridian.shell

import android.view.KeyEvent

/**
 * Android KeyEvent -> the bytes a VT-style terminal expects.
 *
 * Kept separate from TerminalView so it can be unit tested on the JVM without
 * an emulator: the escape sequences are the easiest thing in a terminal to get
 * subtly wrong.
 */
object KeyMapper {

    private val ESC = byteArrayOf(0x1B)

    private fun csi(final: String): ByteArray = "\u001B[$final".toByteArray()

    fun map(keyCode: Int, event: KeyEvent, ctrl: Boolean, alt: Boolean): ByteArray? {
        val base: ByteArray? = when (keyCode) {
            KeyEvent.KEYCODE_ENTER          -> byteArrayOf(0x0D)
            KeyEvent.KEYCODE_DEL            -> byteArrayOf(0x7F)
            KeyEvent.KEYCODE_FORWARD_DEL    -> csi("3~")
            KeyEvent.KEYCODE_TAB            -> byteArrayOf(0x09)
            KeyEvent.KEYCODE_ESCAPE         -> ESC
            KeyEvent.KEYCODE_DPAD_UP        -> csi("A")
            KeyEvent.KEYCODE_DPAD_DOWN      -> csi("B")
            KeyEvent.KEYCODE_DPAD_RIGHT     -> csi("C")
            KeyEvent.KEYCODE_DPAD_LEFT      -> csi("D")
            KeyEvent.KEYCODE_MOVE_HOME      -> csi("H")
            KeyEvent.KEYCODE_MOVE_END       -> csi("F")
            KeyEvent.KEYCODE_PAGE_UP        -> csi("5~")
            KeyEvent.KEYCODE_PAGE_DOWN      -> csi("6~")
            KeyEvent.KEYCODE_INSERT         -> csi("2~")
            else -> null
        }
        if (base != null) return base

        val unicode = event.getUnicodeChar(if (event.isShiftPressed) KeyEvent.META_SHIFT_ON else 0)
        val ch = if (unicode != 0) {
            unicode.toChar()
        } else {
            when (keyCode) {
                in KeyEvent.KEYCODE_A..KeyEvent.KEYCODE_Z -> ('a'.code + (keyCode - KeyEvent.KEYCODE_A)).toChar()
                in KeyEvent.KEYCODE_0..KeyEvent.KEYCODE_9 -> ('0'.code + (keyCode - KeyEvent.KEYCODE_0)).toChar()
                KeyEvent.KEYCODE_SPACE -> ' '
                else -> return null
            }
        }

        if (ctrl) {
            // Ctrl+A..Ctrl+Z -> 0x01..0x1A, plus the usual punctuation controls.
            val code = when (ch.lowercaseChar()) {
                in 'a'..'z' -> ch.lowercaseChar() - 'a' + 1
                '@', ' '    -> 0
                '['         -> 27
                '\\'        -> 28
                ']'         -> 29
                '^'         -> 30
                '_', '?'    -> 31
                else        -> return byteArrayOf(ch.code.toByte())
            }
            val byte = byteArrayOf(code.toByte())
            return if (alt) ESC + byte else byte
        }

        val bytes = ch.toString().toByteArray(Charsets.UTF_8)
        return if (alt) ESC + bytes else bytes
    }
}
