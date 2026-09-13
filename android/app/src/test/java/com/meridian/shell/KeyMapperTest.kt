package com.meridian.shell

import android.view.KeyEvent
import org.junit.Assert.*
import org.junit.Test

/**
 * Escape sequences are the easiest thing in a terminal to get subtly wrong, and
 * the hardest to notice. These run on the JVM, no emulator needed.
 */
class KeyMapperTest {

    private fun down(code: Int) = KeyEvent(KeyEvent.ACTION_DOWN, code)

    @Test fun enterSendsCarriageReturn() {
        assertArrayEquals(
            byteArrayOf(0x0D),
            KeyMapper.map(KeyEvent.KEYCODE_ENTER, down(KeyEvent.KEYCODE_ENTER), false, false)
        )
    }

    @Test fun backspaceSendsDel() {
        assertArrayEquals(
            byteArrayOf(0x7F),
            KeyMapper.map(KeyEvent.KEYCODE_DEL, down(KeyEvent.KEYCODE_DEL), false, false)
        )
    }

    @Test fun tabSends0x09() {
        assertArrayEquals(
            byteArrayOf(0x09),
            KeyMapper.map(KeyEvent.KEYCODE_TAB, down(KeyEvent.KEYCODE_TAB), false, false)
        )
    }

    @Test fun escapeSends0x1B() {
        assertArrayEquals(
            byteArrayOf(0x1B),
            KeyMapper.map(KeyEvent.KEYCODE_ESCAPE, down(KeyEvent.KEYCODE_ESCAPE), false, false)
        )
    }

    // ---- Arrow keys --------------------------------------------------------

    @Test fun arrowUpSendsCsiA() {
        assertArrayEquals(
            "\u001B[A".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_DPAD_UP, down(KeyEvent.KEYCODE_DPAD_UP), false, false)
        )
    }

    @Test fun arrowDownSendsCsiB() {
        assertArrayEquals(
            "\u001B[B".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_DPAD_DOWN, down(KeyEvent.KEYCODE_DPAD_DOWN), false, false)
        )
    }

    @Test fun arrowRightSendsCsiC() {
        assertArrayEquals(
            "\u001B[C".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_DPAD_RIGHT, down(KeyEvent.KEYCODE_DPAD_RIGHT), false, false)
        )
    }

    @Test fun arrowLeftSendsCsiD() {
        assertArrayEquals(
            "\u001B[D".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_DPAD_LEFT, down(KeyEvent.KEYCODE_DPAD_LEFT), false, false)
        )
    }

    // ---- Navigation --------------------------------------------------------

    @Test fun homeSendsCsiH() {
        assertArrayEquals(
            "\u001B[H".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_MOVE_HOME, down(KeyEvent.KEYCODE_MOVE_HOME), false, false)
        )
    }

    @Test fun endSendsCsiF() {
        assertArrayEquals(
            "\u001B[F".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_MOVE_END, down(KeyEvent.KEYCODE_MOVE_END), false, false)
        )
    }

    @Test fun pageUpSendsCsi5Tilde() {
        assertArrayEquals(
            "\u001B[5~".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_PAGE_UP, down(KeyEvent.KEYCODE_PAGE_UP), false, false)
        )
    }

    @Test fun pageDownSendsCsi6Tilde() {
        assertArrayEquals(
            "\u001B[6~".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_PAGE_DOWN, down(KeyEvent.KEYCODE_PAGE_DOWN), false, false)
        )
    }

    @Test fun deleteSendsCsi3Tilde() {
        assertArrayEquals(
            "\u001B[3~".toByteArray(),
            KeyMapper.map(KeyEvent.KEYCODE_FORWARD_DEL, down(KeyEvent.KEYCODE_FORWARD_DEL), false, false)
        )
    }

    // ---- Ctrl sequences ----------------------------------------------------

    @Test fun ctrlCSendsEtx() {
        assertArrayEquals(
            byteArrayOf(0x03),
            KeyMapper.map(KeyEvent.KEYCODE_C, down(KeyEvent.KEYCODE_C), true, false)
        )
    }

    @Test fun ctrlDSendsEot() {
        assertArrayEquals(
            byteArrayOf(0x04),
            KeyMapper.map(KeyEvent.KEYCODE_D, down(KeyEvent.KEYCODE_D), true, false)
        )
    }

    @Test fun ctrlZSendsSub() {
        assertArrayEquals(
            byteArrayOf(0x1A),
            KeyMapper.map(KeyEvent.KEYCODE_Z, down(KeyEvent.KEYCODE_Z), true, false)
        )
    }

    @Test fun ctrlLSendsFF() {
        assertArrayEquals(
            byteArrayOf(0x0C),
            KeyMapper.map(KeyEvent.KEYCODE_L, down(KeyEvent.KEYCODE_L), true, false)
        )
    }

    @Test fun ctrlASendsSOH() {
        assertArrayEquals(
            byteArrayOf(0x01),
            KeyMapper.map(KeyEvent.KEYCODE_A, down(KeyEvent.KEYCODE_A), true, false)
        )
    }

    @Test fun ctrlESendsENQ() {
        assertArrayEquals(
            byteArrayOf(0x05),
            KeyMapper.map(KeyEvent.KEYCODE_E, down(KeyEvent.KEYCODE_E), true, false)
        )
    }

    // ---- Alt prefix --------------------------------------------------------

    @Test fun altPrefixesWithEscape() {
        assertArrayEquals(
            byteArrayOf(0x1B, 'b'.code.toByte()),
            KeyMapper.map(KeyEvent.KEYCODE_B, down(KeyEvent.KEYCODE_B), false, true)
        )
    }

    // ---- No match ----------------------------------------------------------

    @Test fun unmappedKeyReturnsNull() {
        assertNull(KeyMapper.map(KeyEvent.KEYCODE_CAMERA, down(KeyEvent.KEYCODE_CAMERA), false, false))
    }
}
