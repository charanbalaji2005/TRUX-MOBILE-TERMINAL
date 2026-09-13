package com.meridian.shell.viewer

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.widget.Toast
import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ContentCopy
import androidx.compose.material.icons.filled.Search
import androidx.compose.material.icons.filled.WrapText
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.File
import java.io.FileReader

@Composable
fun TextViewer(file: File, isCode: Boolean = false) {
    val context = LocalContext.current
    var lines by remember { mutableStateOf<List<String>>(emptyList()) }
    var isTruncated by remember { mutableStateOf(false) }
    var totalLineCount by remember { mutableIntStateOf(0) }
    var searchQuery by remember { mutableStateOf("") }
    var isSearchOpen by remember { mutableStateOf(false) }
    var wrapLines by remember { mutableStateOf(false) }
    var isLoading by remember { mutableStateOf(true) }

    val maxLinesToLoad = 5000

    LaunchedEffect(file) {
        isLoading = true
        withContext(Dispatchers.IO) {
            val loaded = mutableListOf<String>()
            var count = 0
            try {
                BufferedReader(FileReader(file)).use { reader ->
                    var line: String? = reader.readLine()
                    while (line != null) {
                        count++
                        if (loaded.size < maxLinesToLoad) {
                            loaded.add(line)
                        }
                        line = reader.readLine()
                    }
                }
                withContext(Dispatchers.Main) {
                    lines = loaded
                    totalLineCount = count
                    isTruncated = count > maxLinesToLoad
                    isLoading = false
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    lines = listOf("Error reading file: ${e.message}")
                    isLoading = false
                }
            }
        }
    }

    Column(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0F1117))
    ) {
        // Toolbar: Search, Wrap, Copy
        Row(
            Modifier
                .fillMaxWidth()
                .background(Color(0xFF161B22))
                .padding(horizontal = 12.dp, vertical = 6.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            if (isSearchOpen) {
                TextField(
                    value = searchQuery,
                    onValueChange = { searchQuery = it },
                    placeholder = { Text("Search...", fontSize = 12.sp, color = Color(0xFF64748B)) },
                    singleLine = true,
                    colors = TextFieldDefaults.colors(
                        focusedContainerColor = Color(0xFF21262D),
                        unfocusedContainerColor = Color(0xFF21262D),
                        focusedTextColor = Color.White,
                        unfocusedTextColor = Color.White
                    ),
                    modifier = Modifier
                        .weight(1f)
                        .height(42.dp)
                        .clip(RoundedCornerShape(6.dp))
                )
            } else {
                Text(
                    text = "${lines.size} lines" + if (isTruncated) " (truncated from $totalLineCount)" else "",
                    color = Color(0xFF64748B),
                    fontSize = 11.sp,
                    fontFamily = FontFamily.Monospace,
                    modifier = Modifier.padding(start = 8.dp)
                )
            }

            Row(verticalAlignment = Alignment.CenterVertically) {
                IconButton(onClick = { isSearchOpen = !isSearchOpen }) {
                    Icon(
                        imageVector = Icons.Default.Search,
                        contentDescription = "Search",
                        tint = if (isSearchOpen) Color(0xFF38BDF8) else Color(0xFF94A3B8),
                        modifier = Modifier.size(18.dp)
                    )
                }

                IconButton(onClick = { wrapLines = !wrapLines }) {
                    Icon(
                        imageVector = Icons.Default.WrapText,
                        contentDescription = "Toggle Wrap",
                        tint = if (wrapLines) Color(0xFF38BDF8) else Color(0xFF94A3B8),
                        modifier = Modifier.size(18.dp)
                    )
                }

                IconButton(
                    onClick = {
                        val fullText = lines.joinToString("\n")
                        val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                        clipboard.setPrimaryClip(ClipData.newPlainText("File content", fullText))
                        Toast.makeText(context, "Copied ${lines.size} lines", Toast.LENGTH_SHORT).show()
                    }
                ) {
                    Icon(
                        imageVector = Icons.Default.ContentCopy,
                        contentDescription = "Copy text",
                        tint = Color(0xFF94A3B8),
                        modifier = Modifier.size(18.dp)
                    )
                }
            }
        }

        if (isLoading) {
            Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                CircularProgressIndicator(color = Color.White, strokeWidth = 2.dp)
            }
        } else {
            SelectionContainer {
                val horizontalScroll = rememberScrollState()
                val contentModifier = if (wrapLines) Modifier.fillMaxWidth() else Modifier.horizontalScroll(horizontalScroll)

                LazyColumn(
                    modifier = Modifier
                        .fillMaxSize()
                        .then(contentModifier)
                        .padding(vertical = 4.dp)
                ) {
                    itemsIndexed(lines) { index, lineText ->
                        Row(
                            Modifier.fillMaxWidth().padding(horizontal = 8.dp, vertical = 1.dp),
                            verticalAlignment = Alignment.Top
                        ) {
                            // Line Number
                            Text(
                                text = (index + 1).toString().padStart(4, ' '),
                                color = Color(0xFF475569),
                                fontSize = 11.sp,
                                fontFamily = FontFamily.Monospace,
                                modifier = Modifier.padding(end = 12.dp)
                            )

                            // Formatted text
                            val annotated = buildAnnotatedString {
                                val text = lineText
                                if (searchQuery.isNotBlank() && text.contains(searchQuery, ignoreCase = true)) {
                                    var start = 0
                                    val q = searchQuery.lowercase()
                                    val lower = text.lowercase()
                                    while (start < text.length) {
                                        val idx = lower.indexOf(q, start)
                                        if (idx == -1) {
                                            append(text.substring(start))
                                            break
                                        }
                                        append(text.substring(start, idx))
                                        withStyle(SpanStyle(background = Color(0xFFEAB308), color = Color.Black)) {
                                            append(text.substring(idx, idx + q.length))
                                        }
                                        start = idx + q.length
                                    }
                                } else if (isCode) {
                                    // Basic code syntax coloring
                                    appendCodeHighlighted(text)
                                } else {
                                    append(text)
                                }
                            }

                            Text(
                                text = annotated,
                                color = Color(0xFFE2E8F0),
                                fontSize = 12.sp,
                                fontFamily = FontFamily.Monospace,
                                lineHeight = 16.sp
                            )
                        }
                    }
                }
            }
        }
    }
}

private fun androidx.compose.ui.text.AnnotatedString.Builder.appendCodeHighlighted(line: String) {
    val trimmed = line.trimStart()
    if (trimmed.startsWith("//") || trimmed.startsWith("#") || trimmed.startsWith("/*")) {
        withStyle(SpanStyle(color = Color(0xFF64748B))) {
            append(line)
        }
        return
    }

    val keywords = setOf(
        "fun", "val", "var", "class", "interface", "def", "function", "import", "package",
        "return", "if", "else", "for", "while", "do", "when", "switch", "case", "break",
        "continue", "try", "catch", "finally", "throw", "public", "private", "protected",
        "static", "final", "const", "let", "async", "await", "fn", "mut", "struct", "enum",
        "impl", "type", "select", "from", "where", "insert", "update", "delete", "create"
    )

    val tokens = line.split(Regex("(?<=[^a-zA-Z0-9_])|(?=[^a-zA-Z0-9_])"))
    for (token in tokens) {
        when {
            token in keywords -> {
                withStyle(SpanStyle(color = Color(0xFFF43F5E), fontWeight = FontWeight.Bold)) {
                    append(token)
                }
            }
            token.startsWith("\"") || token.endsWith("\"") || token.startsWith("'") || token.endsWith("'") -> {
                withStyle(SpanStyle(color = Color(0xFF38BDF8))) {
                    append(token)
                }
            }
            token.toIntOrNull() != null -> {
                withStyle(SpanStyle(color = Color(0xFFF59E0B))) {
                    append(token)
                }
            }
            else -> append(token)
        }
    }
}
