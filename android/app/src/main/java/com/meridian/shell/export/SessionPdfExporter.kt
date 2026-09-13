package com.meridian.shell.export

import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Typeface
import android.graphics.pdf.PdfDocument
import android.os.Environment
import android.widget.Toast
import androidx.core.content.FileProvider
import com.meridian.shell.TerminalSession
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileOutputStream
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

object SessionPdfExporter {

    private const val PAGE_WIDTH = 595 // A4 standard width (pt)
    private const val PAGE_HEIGHT = 842 // A4 standard height (pt)
    private const val MARGIN_LEFT = 36f
    private const val MARGIN_RIGHT = 36f
    private const val MARGIN_TOP = 40f
    private const val MARGIN_BOTTOM = 40f
    private const val LINE_HEIGHT = 13f

    /**
     * Exports the given session's terminal content to a formatted PDF.
     * Returns the generated File, or null if export failed.
     */
    suspend fun exportSessionToPdf(
        context: Context,
        session: TerminalSession,
        customText: String? = null
    ): File? = withContext(Dispatchers.IO) {
        try {
            // Retrieve session text
            val rawText = customText ?: session.visibleText()
            val text = if (rawText.isBlank()) {
                val logFile = File(context.filesDir, "sessions/${session.id}.log")
                if (logFile.exists()) logFile.readText() else "No terminal activity recorded for this session."
            } else {
                rawText
            }

            val doc = PdfDocument()
            val dateFormat = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault())
            val exportTime = dateFormat.format(Date())

            // Clean ANSI escape sequences for crisp printing
            val cleanText = text.replace(Regex("\u001B\\[[;?0-9]*[a-zA-Z]"), "")

            // Paint configurations
            val bgPaint = Paint().apply { color = Color.parseColor("#090D16") }
            val headerBgPaint = Paint().apply { color = Color.parseColor("#131926") }
            val borderPaint = Paint().apply {
                color = Color.parseColor("#1E293B")
                strokeWidth = 1f
                style = Paint.Style.STROKE
            }
            val titlePaint = Paint().apply {
                color = Color.parseColor("#FFFFFF")
                textSize = 15f
                typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD)
            }
            val metaPaint = Paint().apply {
                color = Color.parseColor("#94A3B8")
                textSize = 8.5f
                typeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL)
            }
            val bodyPaint = Paint().apply {
                color = Color.parseColor("#E2E8F0")
                textSize = 8.5f
                typeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL)
            }
            val promptPaint = Paint().apply {
                color = Color.parseColor("#38BDF8")
                textSize = 8.5f
                typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD)
            }
            val footerPaint = Paint().apply {
                color = Color.parseColor("#64748B")
                textSize = 7.5f
                typeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL)
            }

            val maxContentWidth = PAGE_WIDTH - MARGIN_LEFT - MARGIN_RIGHT
            val maxCharsPerLine = (maxContentWidth / (bodyPaint.measureText("M"))).toInt().coerceAtLeast(40)

            // Wrap lines
            val wrappedLines = mutableListOf<String>()
            cleanText.lines().forEach { rawLine ->
                if (rawLine.length <= maxCharsPerLine) {
                    wrappedLines.add(rawLine)
                } else {
                    var remaining = rawLine
                    while (remaining.length > maxCharsPerLine) {
                        wrappedLines.add(remaining.substring(0, maxCharsPerLine))
                        remaining = remaining.substring(maxCharsPerLine)
                    }
                    if (remaining.isNotEmpty()) wrappedLines.add(remaining)
                }
            }

            val contentTop = MARGIN_TOP + 80f
            val contentBottom = PAGE_HEIGHT - MARGIN_BOTTOM - 20f
            val availableHeight = contentBottom - contentTop
            val linesPerPage = (availableHeight / LINE_HEIGHT).toInt().coerceAtLeast(10)
            val totalPages = ((wrappedLines.size + linesPerPage - 1) / linesPerPage).coerceAtLeast(1)

            var lineIdx = 0
            for (pageNum in 1..totalPages) {
                val pageInfo = PdfDocument.PageInfo.Builder(PAGE_WIDTH, PAGE_HEIGHT, pageNum).create()
                val page = doc.startPage(pageInfo)
                val canvas = page.canvas

                // Background
                canvas.drawRect(0f, 0f, PAGE_WIDTH.toFloat(), PAGE_HEIGHT.toFloat(), bgPaint)

                // Header Card
                val headerRect = android.graphics.RectF(MARGIN_LEFT, MARGIN_TOP, PAGE_WIDTH - MARGIN_RIGHT, MARGIN_TOP + 65f)
                canvas.drawRoundRect(headerRect, 8f, 8f, headerBgPaint)
                canvas.drawRoundRect(headerRect, 8f, 8f, borderPaint)

                canvas.drawText("TRUX TERMINAL SESSION EXPORT", MARGIN_LEFT + 14f, MARGIN_TOP + 22f, titlePaint)
                val metaLine1 = "Session: ${session.name}  •  Shell: ${session.shell}  •  Cols/Rows: ${session.cols}x${session.rows}"
                val metaLine2 = "Exported: $exportTime  •  Developer: Charan Balaji  •  TRUX OS"
                canvas.drawText(metaLine1, MARGIN_LEFT + 14f, MARGIN_TOP + 40f, metaPaint)
                canvas.drawText(metaLine2, MARGIN_LEFT + 14f, MARGIN_TOP + 54f, metaPaint)

                // Render terminal content lines
                var y = contentTop + 14f
                var linesRendered = 0
                while (lineIdx < wrappedLines.size && linesRendered < linesPerPage) {
                    val line = wrappedLines[lineIdx]
                    val paintToUse = if (line.contains("trux@") || line.contains("root@") || line.startsWith("$") || line.startsWith("#")) {
                        promptPaint
                    } else {
                        bodyPaint
                    }
                    canvas.drawText(line, MARGIN_LEFT, y, paintToUse)
                    y += LINE_HEIGHT
                    lineIdx++
                    linesRendered++
                }

                // Footer
                val footerText = "Page $pageNum of $totalPages  •  TRUX-MOBILE-TERMINAL  •  © 2026 TRUX"
                canvas.drawText(footerText, MARGIN_LEFT, PAGE_HEIGHT - MARGIN_BOTTOM + 8f, footerPaint)

                doc.finishPage(page)
            }

            // Save PDF to Downloads
            val safeName = session.name.replace(Regex("[^a-zA-Z0-9_-]"), "_")
            val fileName = "TRUX_Session_${safeName}_${System.currentTimeMillis()}.pdf"
            val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
            downloadsDir.mkdirs()
            val outFile = File(downloadsDir, fileName)

            FileOutputStream(outFile).use { out ->
                doc.writeTo(out)
            }
            doc.close()

            // Also copy to app internal sessions directory
            val internalCopy = File(context.filesDir, "sessions/$fileName").also {
                it.parentFile?.mkdirs()
            }
            runCatching { outFile.copyTo(internalCopy, overwrite = true) }

            withContext(Dispatchers.Main) {
                Toast.makeText(context, "Session exported to Downloads/$fileName", Toast.LENGTH_LONG).show()
            }

            outFile
        } catch (e: Exception) {
            e.printStackTrace()
            withContext(Dispatchers.Main) {
                Toast.makeText(context, "Failed to export PDF: ${e.message}", Toast.LENGTH_SHORT).show()
            }
            null
        }
    }

    fun openOrSharePdf(context: Context, pdfFile: File) {
        try {
            val uri = FileProvider.getUriForFile(
                context,
                "${context.packageName}.provider",
                pdfFile
            )
            val intent = Intent(Intent.ACTION_VIEW).apply {
                setDataAndType(uri, "application/pdf")
                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(Intent.createChooser(intent, "Open TRUX Session PDF"))
        } catch (e: Exception) {
            Toast.makeText(context, "Cannot open PDF: ${e.message}", Toast.LENGTH_SHORT).show()
        }
    }
}
