package com.meridian.shell

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.graphics.*
import android.text.InputType
import android.view.*
import android.view.inputmethod.*
import kotlin.concurrent.thread
import kotlin.math.*

/**
 * Draws the terminal grid directly onto a Canvas.
 *
 * One View for the whole terminal — not one TextView per line, and no WebView.
 * The cell and metadata arrays are allocated once per geometry change and
 * reused, so a steady-state frame allocates nothing.
 *
 * Features:
 *   - Real ANSI / UTF-8 / Unicode rendering
 *   - Long-press → text selection with drag handles
 *   - Copy via Android ClipboardManager (never logged or uploaded)
 *   - Paste from clipboard into PTY
 *   - Vertical scroll through terminal scrollback
 *   - Keyboard show/hide triggers PTY resize
 */
class TerminalView(context: Context) : View(context) {

    companion object {
        private const val DEFAULT_FG = 0xFFD0D0D0.toInt()
        private const val DEFAULT_BG = 0xFF000000.toInt()
        private const val CURSOR_COLOR = 0xFFE0E0E0.toInt()
        private const val SEL_COLOR = 0x447FD1FF
        private const val META_SIZE = 8
        private const val SCROLL_SENSITIVITY = 1.0f
        const val DEFAULT_FONT_SIZE_SP = 13f
        const val MIN_FONT_SIZE = 8f
        const val MAX_FONT_SIZE = 28f
    }

    var scrollOffsetRows: Int = 0
        private set
    private var maxScrollbackRows: Int = 0
    private var scrollAccumulatorDy: Float = 0f

    var session: TerminalSession? = null
        set(value) {
            field = value
            lastGeneration = 0
            clearSelection()
            requestGeometryPush(force = true)
            postInvalidate()
        }

    /** Invoked after the grid size is recomputed, e.g. to persist dimensions. */
    var onGeometryChanged: ((cols: Int, rows: Int) -> Unit)? = null

    /** Invoked when the session process terminates so it can be cleanly closed. */
    var onSessionExited: ((TerminalSession) -> Unit)? = null

    // ---- paints ------------------------------------------------------------

    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        typeface = Typeface.MONOSPACE
        color = DEFAULT_FG
    }
    private val bgPaint = Paint()
    private val cursorPaint = Paint().apply { color = CURSOR_COLOR }
    private val selectionPaint = Paint().apply {
        color = SEL_COLOR
        style = Paint.Style.FILL
    }

    // ---- metrics -----------------------------------------------------------

    private var charWidth = 0f
    private var lineHeight = 0f
    private var baselineOffset = 0f

    private var cols = 0
    private var rows = 0

    private var cells = IntArray(0)
    private val meta = IntArray(META_SIZE)
    private val glyph = CharArray(2)

    @Volatile private var lastGeneration = 0L
    @Volatile private var renderRunning = false
    private var renderThread: Thread? = null

    var fontSizeSp: Float = DEFAULT_FONT_SIZE_SP
        set(value) {
            field = value
            applyFontMetrics()
            requestGeometryPush(force = true)
            postInvalidate()
        }

    fun zoomIn() {
        fontSizeSp = (fontSizeSp + 1.5f).coerceIn(MIN_FONT_SIZE, MAX_FONT_SIZE)
    }

    fun zoomOut() {
        fontSizeSp = (fontSizeSp - 1.5f).coerceIn(MIN_FONT_SIZE, MAX_FONT_SIZE)
    }

    fun resetZoom() {
        fontSizeSp = DEFAULT_FONT_SIZE_SP
    }

    fun pageUp() {
        val delta = if (rows > 2) rows - 2 else 10
        val newOffset = (scrollOffsetRows + delta).coerceIn(0, maxScrollbackRows)
        if (newOffset != scrollOffsetRows) {
            scrollOffsetRows = newOffset
            invalidate()
        }
    }

    fun pageDown() {
        val delta = if (rows > 2) rows - 2 else 10
        val newOffset = (scrollOffsetRows - delta).coerceIn(0, maxScrollbackRows)
        if (newOffset != scrollOffsetRows) {
            scrollOffsetRows = newOffset
            invalidate()
        }
    }

    // ---- selection state ---------------------------------------------------

    private var selStartCol = -1
    private var selStartRow = -1
    private var selEndCol = -1
    private var selEndRow = -1
    private var selecting = false
    private var actionMode: ActionMode? = null

    private fun clearSelection() {
        selStartCol = -1; selStartRow = -1; selEndCol = -1; selEndRow = -1
        selecting = false
        actionMode?.finish()
        actionMode = null
    }

    private fun hasSelection() = selStartCol >= 0 && selEndCol >= 0

    private val scaleDetector = ScaleGestureDetector(context, object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
        override fun onScale(detector: ScaleGestureDetector): Boolean {
            val newSize = (fontSizeSp * detector.scaleFactor).coerceIn(MIN_FONT_SIZE, MAX_FONT_SIZE)
            if (abs(newSize - fontSizeSp) >= 0.1f) {
                fontSizeSp = newSize
            }
            return true
        }
    })

    private val gestureDetector = GestureDetector(context, object : GestureDetector.SimpleOnGestureListener() {
        override fun onLongPress(e: MotionEvent) {
            // Start selection at the tapped cell
            val col = (e.x / charWidth).toInt().coerceIn(0, cols - 1)
            val row = (e.y / lineHeight).toInt().coerceIn(0, rows - 1)
            selStartCol = col; selStartRow = row
            selEndCol = col; selEndRow = row
            selecting = true
            performHapticFeedback(HapticFeedbackConstants.LONG_PRESS)
            showSelectionActionMode()
            invalidate()
        }

        override fun onSingleTapUp(e: MotionEvent): Boolean {
            if (hasSelection()) {
                clearSelection()
                invalidate()
            } else {
                showKeyboard()
            }
            return true
        }

        override fun onScroll(
            e1: MotionEvent?, e2: MotionEvent, dx: Float, dy: Float
        ): Boolean {
            if (abs(dy) > abs(dx) && lineHeight > 0f) {
                parent?.requestDisallowInterceptTouchEvent(true)
                scrollAccumulatorDy += dy
                val deltaRows = (scrollAccumulatorDy / lineHeight).toInt()
                if (deltaRows != 0) {
                    scrollAccumulatorDy -= deltaRows * lineHeight
                    // Dragging downward (dy < 0): scroll UP into history (increase offset)
                    // Dragging upward (dy > 0): scroll DOWN towards bottom (decrease offset)
                    val newOffset = (scrollOffsetRows - deltaRows).coerceIn(0, maxScrollbackRows)
                    if (newOffset != scrollOffsetRows) {
                        scrollOffsetRows = newOffset
                        invalidate()
                    }
                }
                return true
            }
            return false
        }

        override fun onFling(
            e1: MotionEvent?, e2: MotionEvent, velocityX: Float, velocityY: Float
        ): Boolean {
            if (abs(velocityY) > abs(velocityX) && lineHeight > 0f) {
                val flingRows = (velocityY / (lineHeight * 6f)).toInt()
                val newOffset = (scrollOffsetRows - flingRows).coerceIn(0, maxScrollbackRows)
                if (newOffset != scrollOffsetRows) {
                    scrollOffsetRows = newOffset
                    invalidate()
                    return true
                }
            }
            return false
        }
    })

    init {
        isFocusable = true
        isFocusableInTouchMode = true
        setBackgroundColor(DEFAULT_BG)
        applyFontMetrics()
    }

    private fun applyFontMetrics() {
        textPaint.textSize = fontSizeSp * resources.displayMetrics.scaledDensity
        charWidth = textPaint.measureText("M")
        val fm = textPaint.fontMetrics
        lineHeight = (fm.bottom - fm.top) * 1.05f
        baselineOffset = -fm.top
    }

    // ---- geometry ----------------------------------------------------------

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        requestGeometryPush()
    }

    /**
     * Recomputes cols/rows from the current viewport and pushes them to PTY.
     * Must be called on: resize, rotation, keyboard show/hide.
     */
    fun requestGeometryPush(force: Boolean = false) {
        if (width <= 0 || height <= 0 || charWidth <= 0f || lineHeight <= 0f) return
        val newCols = max(1, (width / charWidth).toInt())
        val newRows = max(1, (height / lineHeight).toInt())
        if (!force && newCols == cols && newRows == rows) return

        cols = newCols
        rows = newRows
        val needed = cols * rows * 3
        if (cells.size < needed) cells = IntArray(needed * 2) // pre-allocate extra

        session?.resize(cols, rows, force)
        onGeometryChanged?.invoke(cols, rows)
        postInvalidate()
    }

    // ---- render loop -------------------------------------------------------

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        renderRunning = true
        renderThread = thread(name = "meridian-render", isDaemon = true) {
            while (renderRunning) {
                val s = session
                if (s == null) { Thread.sleep(50); continue }
                // Blocks natively until output arrives; no 60 Hz busy poll.
                s.awaitChange(lastGeneration, 100)
                val gen = s.generation()
                if (gen != lastGeneration) {
                    lastGeneration = gen
                    postInvalidateOnAnimation()
                }
                if (!s.isAlive) {
                    s.markExited()
                    post { onSessionExited?.invoke(s) }
                }
            }
        }
    }

    override fun onDetachedFromWindow() {
        renderRunning = false
        renderThread?.interrupt()
        renderThread = null
        super.onDetachedFromWindow()
    }

    // ---- drawing -----------------------------------------------------------

    override fun onDraw(canvas: Canvas) {
        val s = session ?: return
        if (cols == 0 || rows == 0) return

        canvas.drawColor(DEFAULT_BG)

        val written = s.snapshot(cells, meta, scrollOffsetRows)
        if (written < 0) {
            // Native grid grew since our last geometry push; resize and retry.
            cells = IntArray(max(abs(written), cols * rows * 3) * 2)
            postInvalidate()
            return
        }

        val gridRows = meta[2]
        val gridCols = meta[3]
        maxScrollbackRows = meta[5]
        if (scrollOffsetRows > maxScrollbackRows) {
            scrollOffsetRows = maxScrollbackRows
        }

        // Draw selection background first (under text)
        if (hasSelection()) {
            drawSelectionHighlight(canvas, gridRows, gridCols)
        }

        var i = 0
        for (y in 0 until gridRows) {
            val top = y * lineHeight
            val baseline = top + baselineOffset
            var x = 0
            while (x < gridCols) {
                val packed = cells[i]
                var fg = cells[i + 1]
                var bg = cells[i + 2]
                i += 3

                val cp = packed and 0x1FFFFF
                val cellWidth = (packed ushr 21) and 0x3
                val reverse = (packed and (1 shl 27)) != 0
                val hidden = (packed and (1 shl 29)) != 0

                if (fg ushr 24 == 0) fg = DEFAULT_FG
                if (bg ushr 24 == 0) bg = DEFAULT_BG
                if (reverse) { val t = fg; fg = bg; bg = t }

                val left = x * charWidth
                val span = if (cellWidth == 2) 2 else 1

                if (bg != DEFAULT_BG) {
                    bgPaint.color = bg
                    canvas.drawRect(left, top, left + charWidth * span, top + lineHeight, bgPaint)
                }

                if (cellWidth != 0 && cp != 32 && cp != 0 && !hidden) {
                    textPaint.color = if ((packed and (1 shl 28)) != 0) dim(fg) else fg
                    textPaint.isFakeBoldText = (packed and (1 shl 23)) != 0
                    textPaint.isUnderlineText = (packed and (1 shl 25)) != 0
                    textPaint.isStrikeThruText = (packed and (1 shl 26)) != 0
                    textPaint.textSkewX = if ((packed and (1 shl 24)) != 0) -0.25f else 0f

                    val len = Character.toChars(cp, glyph, 0)
                    canvas.drawText(glyph, 0, len, left, baseline, textPaint)
                }
                x += if (cellWidth == 0) 1 else span
            }
            // A zero-width leading cell can leave `i` behind; realign per row.
            i = (y + 1) * gridCols * 3
        }

        // Draw cursor
        if (meta[4] != 0 && !s.exited && meta[0] in 0 until gridRows && meta[1] in 0 until gridCols) {
            val cy = meta[0] * lineHeight
            val cx = meta[1] * charWidth
            canvas.drawRect(cx, cy, cx + charWidth, cy + lineHeight, cursorPaint)
        }

        // Cursor drawn above. Scrollbar intentionally hidden to avoid clutter.
    }

    private fun drawSelectionHighlight(canvas: Canvas, gridRows: Int, gridCols: Int) {
        val r1 = min(selStartRow, selEndRow).coerceIn(0, gridRows - 1)
        val r2 = max(selStartRow, selEndRow).coerceIn(0, gridRows - 1)
        val c1 = if (selStartRow <= selEndRow) selStartCol else selEndCol
        val c2 = if (selStartRow <= selEndRow) selEndCol else selStartCol

        for (row in r1..r2) {
            val colFrom = if (row == r1) c1.coerceIn(0, gridCols - 1) else 0
            val colTo = if (row == r2) c2.coerceIn(0, gridCols) else gridCols
            val left = colFrom * charWidth
            val right = colTo * charWidth
            val top = row * lineHeight
            canvas.drawRect(left, top, right, top + lineHeight, selectionPaint)
        }
    }

    private fun dim(color: Int): Int = Color.argb(
        Color.alpha(color),
        (Color.red(color) * 0.6f).roundToInt(),
        (Color.green(color) * 0.6f).roundToInt(),
        (Color.blue(color) * 0.6f).roundToInt()
    )

    // ---- selection action mode ---------------------------------------------

    private fun showSelectionActionMode() {
        actionMode?.finish()
        actionMode = startActionMode(object : ActionMode.Callback {
            override fun onCreateActionMode(mode: ActionMode, menu: Menu): Boolean {
                menu.add(Menu.NONE, android.R.id.copy, 0, android.R.string.copy)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS)
                menu.add(Menu.NONE, android.R.id.paste, 1, android.R.string.paste)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM)
                menu.add(Menu.NONE, android.R.id.selectAll, 2, android.R.string.selectAll)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM)
                return true
            }

            override fun onPrepareActionMode(mode: ActionMode, menu: Menu) = false

            override fun onActionItemClicked(mode: ActionMode, item: MenuItem): Boolean {
                return when (item.itemId) {
                    android.R.id.copy -> { copySelection(); mode.finish(); true }
                    android.R.id.paste -> { pasteFromClipboard(); mode.finish(); true }
                    android.R.id.selectAll -> { selectAll(); true }
                    else -> false
                }
            }

            override fun onDestroyActionMode(mode: ActionMode) {
                if (actionMode === mode) {
                    clearSelection()
                    invalidate()
                    actionMode = null
                }
            }
        })
    }

    private fun copySelection() {
        val s = session ?: return
        val fullText = s.visibleText()
        val lines = fullText.split("\n")
        val r1 = min(selStartRow, selEndRow).coerceIn(0, lines.size - 1)
        val r2 = max(selStartRow, selEndRow).coerceIn(0, lines.size - 1)

        val selected = buildString {
            for (row in r1..r2) {
                val line = lines.getOrElse(row) { "" }
                val c1 = if (row == r1) min(selStartCol, selEndCol).coerceIn(0, line.length) else 0
                val c2 = if (row == r2) max(selStartCol, selEndCol).coerceIn(0, line.length) else line.length
                append(line.substring(c1, c2))
                if (row < r2) append("\n")
            }
        }

        if (selected.isNotEmpty()) {
            val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            // Do NOT log clipboard contents — privacy requirement
            clipboard.setPrimaryClip(ClipData.newPlainText("terminal", selected))
        }
    }

    private fun pasteFromClipboard() {
        val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        val clip = clipboard.primaryClip ?: return
        if (clip.itemCount == 0) return
        val text = clip.getItemAt(0).coerceToText(context).toString()
        if (text.isNotEmpty()) {
            // Write directly to PTY as raw bytes — no logging
            session?.write(text)
        }
    }

    private fun selectAll() {
        val fullText = session?.visibleText() ?: return
        val lines = fullText.split("\n")
        selStartRow = 0; selStartCol = 0
        selEndRow = (lines.size - 1).coerceAtLeast(0)
        selEndCol = lines.lastOrNull()?.length ?: 0
        invalidate()
    }

    // ---- input -------------------------------------------------------------

    override fun onCheckIsTextEditor(): Boolean = true

    override fun onCreateInputConnection(outAttrs: EditorInfo): InputConnection {
        outAttrs.inputType = InputType.TYPE_NULL
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI or
                EditorInfo.IME_FLAG_NO_FULLSCREEN or
                EditorInfo.IME_ACTION_NONE
        return object : BaseInputConnection(this, true) {
            override fun commitText(text: CharSequence?, newCursorPosition: Int): Boolean {
                scrollToBottom()
                if (text == null || text.isEmpty()) return true
                val s = session ?: return true

                if (ctrlLatched) {
                    ctrlLatched = false
                    onCtrlConsumed?.invoke()
                    for (ch in text) {
                        val code = when (ch) {
                            in 'a'..'z' -> (ch - 'a' + 1).toByte()
                            in 'A'..'Z' -> (ch - 'A' + 1).toByte()
                            '@', ' '    -> 0.toByte()
                            '['         -> 27.toByte()
                            '\\'        -> 28.toByte()
                            ']'         -> 29.toByte()
                            '^'         -> 30.toByte()
                            '_'         -> 31.toByte()
                            '?'         -> 127.toByte()
                            else        -> ch.code.toByte()
                        }
                        s.write(byteArrayOf(code))
                    }
                    return true
                }

                if (altLatched) {
                    altLatched = false
                    onAltConsumed?.invoke()
                    val esc = byteArrayOf(0x1B)
                    s.write(esc + text.toString().toByteArray(Charsets.UTF_8))
                    return true
                }

                s.write(text.toString())
                return true
            }

            override fun deleteSurroundingText(before: Int, after: Int): Boolean {
                scrollToBottom()
                repeat(before) { session?.write(byteArrayOf(0x7F)) }
                return true
            }

            override fun sendKeyEvent(event: KeyEvent?): Boolean {
                if (event != null && event.action == KeyEvent.ACTION_DOWN) {
                    return this@TerminalView.onKeyDown(event.keyCode, event)
                }
                return true
            }
        }
    }

    fun scrollToBottom() {
        if (scrollOffsetRows != 0) {
            scrollOffsetRows = 0
            invalidate()
        }
    }

    fun showKeyboard() {
        requestFocus()
        val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.showSoftInput(this, InputMethodManager.SHOW_IMPLICIT)
    }

    fun hideKeyboard() {
        val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(windowToken, 0)
    }

    fun toggleKeyboard() {
        requestFocus()
        val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0)
    }

    /** Latched by the extra-key row; consumed by the next printable key. */
    var ctrlLatched = false
    var altLatched = false
    var onCtrlConsumed: (() -> Unit)? = null
    var onAltConsumed: (() -> Unit)? = null

    override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
        // Clear selection on any keypress
        if (hasSelection()) {
            clearSelection()
            invalidate()
        }
        scrollToBottom()

        val s = session ?: return super.onKeyDown(keyCode, event)

        val ctrl = ctrlLatched || event.isCtrlPressed
        val alt = altLatched || event.isAltPressed

        val seq = KeyMapper.map(keyCode, event, ctrl, alt)
        if (seq != null) {
            s.write(seq)
            if (ctrlLatched) {
                ctrlLatched = false
                onCtrlConsumed?.invoke()
            }
            if (altLatched) {
                altLatched = false
                onAltConsumed?.invoke()
            }
            return true
        }
        return super.onKeyDown(keyCode, event)
    }

    // ---- touch -------------------------------------------------------------

    override fun onTouchEvent(event: MotionEvent): Boolean {
        // Allow left-edge swipe (<= 12dp) to bubble up to parent ModalNavigationDrawer
        val edgeMarginPx = 12 * resources.displayMetrics.density
        if (event.actionMasked == MotionEvent.ACTION_DOWN && event.x < edgeMarginPx) {
            return false
        }

        scaleDetector.onTouchEvent(event)
        if (!scaleDetector.isInProgress) {
            gestureDetector.onTouchEvent(event)
        }

        // Update selection end on drag while selecting
        if (selecting && !scaleDetector.isInProgress && event.actionMasked == MotionEvent.ACTION_MOVE) {
            val col = (event.x / charWidth).toInt().coerceIn(0, cols)
            val row = (event.y / lineHeight).toInt().coerceIn(0, rows - 1)
            if (col != selEndCol || row != selEndRow) {
                selEndCol = col
                selEndRow = row
                invalidate()
            }
        }

        if (event.actionMasked == MotionEvent.ACTION_UP ||
            event.actionMasked == MotionEvent.ACTION_CANCEL) {
            parent?.requestDisallowInterceptTouchEvent(false)
            if (selecting && hasSelection()) {
                selecting = false
                // Keep selection visible; action mode already shown
            }
        }

        return true
    }

    override fun performClick(): Boolean = super.performClick()
}
