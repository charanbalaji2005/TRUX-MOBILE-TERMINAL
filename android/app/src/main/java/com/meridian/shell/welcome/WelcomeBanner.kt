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
    fun generateBanner(cols: Int = 80, color: Boolean = true): String {
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

        val effectiveCols = if (cols < 30) 30 else cols

        sb.append("\n")

        // 1. Header & TRUX Branding
        if (effectiveCols >= 56) {
            // Medium to Wide
            sb.append(padCenter("$dim${drk}WELCOME TO$rst", effectiveCols, 10)).append("\n")
            sb.append(padCenter("$acc${bold}████████╗██████╗ ██╗   ██╗██╗  ██╗$rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$acc${bold}╚══██╔══╝██╔══██╗██║   ██║╚██╗██╔╝$rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$slv${bold}   ██║   ██████╔╝██║   ██║ ╚███╔╝ $rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$slv${bold}   ██║   ██╔══██╗██║   ██║ ██╔██╗ $rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$gry${bold}   ██║   ██║  ██║╚██████╔╝██╔╝ ██╗$rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$drk   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝$rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$slv$bold A N D R O I D   T E R M I N A L $rst", effectiveCols, 31)).append("\n\n")
            sb.append(padCenter("$gry$dim$bold B U I L D   ›   E X P L O R E   ›   B E Y O N D $rst", effectiveCols, 43)).append("\n\n")
        } else if (effectiveCols >= 42) {
            // Compact Header
            sb.append(padCenter("$dim${drk}WELCOME TO$rst", effectiveCols, 10)).append("\n")
            sb.append(padCenter("$acc${bold}████████╗██████╗ ██╗   ██╗██╗  ██╗$rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$slv${bold}╚══██╔══╝██╔══██╗██║   ██║╚██╗██╔╝$rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$gry${bold}   ██║   ██████╔╝╚██████╔╝██╔╝ ██╗$rst", effectiveCols, 34)).append("\n")
            sb.append(padCenter("$slv$bold A N D R O I D   T E R M I N A L $rst", effectiveCols, 31)).append("\n")
            sb.append(padCenter("$gry$dim B U I L D  ›  E X P L O R E  ›  B E Y O N D $rst", effectiveCols, 41)).append("\n\n")
        } else {
            // Minimal Header
            sb.append(padCenter("$dim${drk}WELCOME TO$rst", effectiveCols, 10)).append("\n")
            sb.append(padCenter("$acc${bold}T R U X$rst", effectiveCols, 7)).append("\n")
            sb.append(padCenter("$slv$bold ANDROID TERMINAL $rst", effectiveCols, 18)).append("\n")
            sb.append(padCenter("$gry$dim BUILD › EXPLORE › BEYOND $rst", effectiveCols, 24)).append("\n\n")
        }

        // 2. Feature Section
        if (effectiveCols >= 76) {
            // 3 Columns box
            val colW = 23
            val lineTop = "┌" + "─".repeat(colW + 2) + "┬" + "─".repeat(colW + 2) + "┬" + "─".repeat(colW + 2) + "┐"
            val lineMid = "├" + "─".repeat(colW + 2) + "┼" + "─".repeat(colW + 2) + "┼" + "─".repeat(colW + 2) + "┤"
            val lineBot = "└" + "─".repeat(colW + 2) + "┴" + "─".repeat(colW + 2) + "┴" + "─".repeat(colW + 2) + "┘"

            fun fmtCell(label: String): String {
                val pad = (colW - label.length).coerceAtLeast(0)
                return " $label" + " ".repeat(pad + 1)
            }

            sb.append(padCenter("$drk$lineTop$rst", effectiveCols, lineTop.length)).append("\n")
            val row1 = "$drk│$slv${fmtCell(">_ Powerful Linux Env")}$drk│$slv${fmtCell("📦 Full Package Ecosys")}$drk│$slv${fmtCell("🤖 Integrated AI Assist")}$drk│$rst"
            sb.append(padCenter(row1, effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter("$drk$lineMid$rst", effectiveCols, lineMid.length)).append("\n")
            val row2 = "$drk│$slv${fmtCell("🔒 Secure App Lock")}$drk│$slv${fmtCell("🖼️ Universal Viewer")}$drk│$slv${fmtCell("⚡ Lightweight High Perf")}$drk│$rst"
            sb.append(padCenter(row2, effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter("$drk$lineBot$rst", effectiveCols, lineBot.length)).append("\n\n")

        } else if (effectiveCols >= 50) {
            // 2 Columns box
            val colW = 22
            val lineTop = "┌" + "─".repeat(colW + 2) + "┬" + "─".repeat(colW + 2) + "┐"
            val lineMid = "├" + "─".repeat(colW + 2) + "┼" + "─".repeat(colW + 2) + "┤"
            val lineBot = "└" + "─".repeat(colW + 2) + "┴" + "─".repeat(colW + 2) + "┘"

            fun fmtCell(label: String): String {
                val pad = (colW - label.length).coerceAtLeast(0)
                return " $label" + " ".repeat(pad + 1)
            }

            sb.append(padCenter("$drk$lineTop$rst", effectiveCols, lineTop.length)).append("\n")
            val row1 = "$drk│$slv${fmtCell(">_ Linux Environment")}$drk│$slv${fmtCell("📦 Package Ecosystem")}$drk│$rst"
            sb.append(padCenter(row1, effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter("$drk$lineMid$rst", effectiveCols, lineMid.length)).append("\n")
            val row2 = "$drk│$slv${fmtCell("🤖 AI Assistant")}$drk│$slv${fmtCell("🔒 Secure App Lock")}$drk│$rst"
            sb.append(padCenter(row2, effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter("$drk$lineMid$rst", effectiveCols, lineMid.length)).append("\n")
            val row3 = "$drk│$slv${fmtCell("🖼️ Universal Viewer")}$drk│$slv${fmtCell("⚡ High Performance")}$drk│$rst"
            sb.append(padCenter(row3, effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter("$drk$lineBot$rst", effectiveCols, lineBot.length)).append("\n\n")

        } else {
            // 1 Column box or list
            val innerW = (effectiveCols - 4).coerceAtLeast(28).coerceAtMost(38)
            val lineTop = "┌" + "─".repeat(innerW + 2) + "┐"
            val lineBot = "└" + "─".repeat(innerW + 2) + "┘"

            fun fmtRow(label: String): String {
                val pad = (innerW - label.length).coerceAtLeast(0)
                return "$drk│$slv $label" + " ".repeat(pad + 1) + "$drk│$rst"
            }

            sb.append(padCenter("$drk$lineTop$rst", effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter(fmtRow(">_ Powerful Linux Env"), effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter(fmtRow("📦 Full Package Ecosystem"), effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter(fmtRow("🤖 Integrated AI Assistant"), effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter(fmtRow("🔒 Secure App Lock"), effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter(fmtRow("🖼️ Universal File Viewer"), effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter(fmtRow("⚡ High Performance"), effectiveCols, lineTop.length)).append("\n")
            sb.append(padCenter("$drk$lineBot$rst", effectiveCols, lineBot.length)).append("\n\n")
        }

        // 3. Command Hints
        val hintIndent = if (effectiveCols > 50) "  " else " "
        sb.append("$hintIndent$gry$dim" + "Type " + "$wht'help'$gry$dim" + "       → Show available commands$rst\n")
        sb.append("$hintIndent$gry$dim" + "Type " + "$wht'trux'$gry$dim" + "       → Explore TRUX tools$rst\n")
        sb.append("$hintIndent$gry$dim" + "Type " + "$wht'pkg'$gry$dim" + "        → Manage packages$rst\n")
        sb.append("$hintIndent$gry$dim" + "Type " + "$wht'trux ai'$gry$dim" + "    → Use AI assistant$rst\n")
        sb.append("\n")

        return sb.toString()
    }
}
