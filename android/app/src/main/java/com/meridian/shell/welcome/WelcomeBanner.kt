package com.meridian.shell.welcome

import android.content.Context
import java.io.File

object WelcomeBanner {

    private const val PREFS_NAME = "trux_settings"
    private const val PREF_SHOW_WELCOME = "show_welcome_on_new_session"

    fun isWelcomeEnabled(context: Context): Boolean {
        val prefs = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        val defaultVal = true
        val confFile = File(context.filesDir, "usr/etc/trux/welcome.conf")
        if (confFile.exists()) {
            val content = runCatching { confFile.readText() }.getOrDefault("").trim()
            if (content.contains("enabled=0")) return false
            if (content.contains("enabled=1")) return true
        }
        return prefs.getBoolean(PREF_SHOW_WELCOME, defaultVal)
    }

    fun setWelcomeEnabled(context: Context, enabled: Boolean) {
        val prefs = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        prefs.edit().putBoolean(PREF_SHOW_WELCOME, enabled).apply()

        val confDir = File(context.filesDir, "usr/etc/trux").also { it.mkdirs() }
        val confFile = File(confDir, "welcome.conf")
        runCatching {
            confFile.writeText("enabled=${if (enabled) "1" else "0"}\n")
        }
    }

    /**
     * Builds the complete ANSI dashboard as a string ready to write to terminal output.
     */
    fun generateBanner(cols: Int = 36, rows: Int = 24, color: Boolean = true): String {
        val sb = StringBuilder()

        val rst = if (color) "\u001B[0m" else ""
        val bold = if (color) "\u001B[1m" else ""
        val dim = if (color) "\u001B[2m" else ""

        val wht = if (color) "\u001B[1;37m" else ""
        val slv = if (color) "\u001B[38;5;252m" else ""
        val gry = if (color) "\u001B[38;5;245m" else ""
        val drk = if (color) "\u001B[38;5;240m" else ""
        val acc = if (color) "\u001B[1;38;5;255m" else ""

        fun padCenter(text: String, width: Int, rawLen: Int = text.length): String {
            if (rawLen >= width) return text
            val totalPad = width - rawLen
            val left = totalPad / 2
            return " ".repeat(left) + text
        }

        val effectiveCols = if (cols < 28) 28 else cols
        val isCompressed = rows <= 18 || cols < 32

        sb.append("\r\n")

        // 1. Header & TRUX Logo (Crisp, solid block art - completely unambiguous TRUX)
        val l1 = "█████   ████    █   █   █   █"
        val l2 = "  █     █   █   █   █    █ █ "
        val l3 = "  █     ████    █   █     █  "
        val l4 = "  █     █  █    █   █    █ █ "
        val l5 = "  █     █   █    ███    █   █"

        if (isCompressed) {
            // Compressed layout: Fits comfortably in 10-14 rows when virtual keyboard is open
            sb.append(padCenter("$acc$bold$l1$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$acc$bold$l2$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$slv$bold$l3$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$slv$bold$l4$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$gry$bold$l5$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$slv$bold ANDROID TERMINAL • BEYOND $rst", effectiveCols, 27)).append("\r\n")
            sb.append(padCenter("$wht Type 'help' | 'trux' | 'pkg' $rst", effectiveCols, 28)).append("\r\n")
        } else {
            // Standard layout: Full height viewport (keyboard closed)
            sb.append(padCenter("$dim${drk}WELCOME TO$rst", effectiveCols, 10)).append("\r\n")
            sb.append(padCenter("$acc$bold$l1$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$acc$bold$l2$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$slv$bold$l3$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$slv$bold$l4$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$gry$bold$l5$rst", effectiveCols, 29)).append("\r\n")
            sb.append(padCenter("$slv$bold A N D R O I D   T E R M I N A L $rst", effectiveCols, 31)).append("\r\n")
            sb.append(padCenter("$gry$dim BUILD  ›  EXPLORE  ›  BEYOND $rst", effectiveCols, 28)).append("\r\n")

            sb.append("\r\n")

            // Feature Section (Curated bullet items, <= 30 chars wide, zero wrapping)
            val r1 = " • Linux Env     • Secure Lock"
            val r2 = " • Packages      • File Viewer"
            val r3 = " • AI Assistant  • High Perf  "
            sb.append(padCenter("$slv$r1$rst", effectiveCols, r1.length)).append("\r\n")
            sb.append(padCenter("$slv$r2$rst", effectiveCols, r2.length)).append("\r\n")
            sb.append(padCenter("$slv$r3$rst", effectiveCols, r3.length)).append("\r\n")

            sb.append("\r\n")

            // Command hints (compact, <= 34 chars wide, zero wrapping)
            sb.append(" $gry$dim" + "Type " + "$wht'help'$gry$dim" + "     → Show commands$rst\r\n")
            sb.append(" $gry$dim" + "Type " + "$wht'trux'$gry$dim" + "     → Explore TRUX$rst\r\n")
            sb.append(" $gry$dim" + "Type " + "$wht'pkg'$gry$dim" + "      → Manage packages$rst\r\n")
            sb.append(" $gry$dim" + "Type " + "$wht'trux ai'$gry$dim" + "  → AI assistant$rst\r\n")
        }

        sb.append("\r\n")

        return sb.toString()
    }
}
