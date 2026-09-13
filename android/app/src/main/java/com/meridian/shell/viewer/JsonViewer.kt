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
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ContentCopy
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
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
import org.json.JSONArray
import org.json.JSONObject
import java.io.File

@Composable
fun JsonViewer(file: File) {
    val context = LocalContext.current
    var formattedLines by remember { mutableStateOf<List<String>>(emptyList()) }
    var isLoading by remember { mutableStateOf(true) }

    LaunchedEffect(file) {
        isLoading = true
        withContext(Dispatchers.IO) {
            try {
                val raw = file.readText().trim()
                val pretty = if (raw.startsWith("{")) {
                    JSONObject(raw).toString(2)
                } else if (raw.startsWith("[")) {
                    JSONArray(raw).toString(2)
                } else {
                    raw
                }
                withContext(Dispatchers.Main) {
                    formattedLines = pretty.lines()
                    isLoading = false
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    formattedLines = file.readLines()
                    isLoading = false
                }
            }
        }
    }

    Column(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0D1117))
    ) {
        // Header
        Row(
            Modifier
                .fillMaxWidth()
                .background(Color(0xFF161B22))
                .padding(horizontal = 14.dp, vertical = 6.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = "FORMATTED JSON (${formattedLines.size} lines)",
                color = Color(0xFF94A3B8),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.SemiBold
            )

            IconButton(
                onClick = {
                    val full = formattedLines.joinToString("\n")
                    val cm = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                    cm.setPrimaryClip(ClipData.newPlainText("JSON", full))
                    Toast.makeText(context, "JSON copied to clipboard", Toast.LENGTH_SHORT).show()
                }
            ) {
                Icon(
                    imageVector = Icons.Default.ContentCopy,
                    contentDescription = "Copy JSON",
                    tint = Color(0xFF94A3B8),
                    modifier = Modifier.size(18.dp)
                )
            }
        }

        if (isLoading) {
            Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                CircularProgressIndicator(color = Color.White, strokeWidth = 2.dp)
            }
        } else {
            val hScroll = rememberScrollState()
            LazyColumn(
                Modifier
                    .fillMaxSize()
                    .horizontalScroll(hScroll)
                    .padding(8.dp)
            ) {
                itemsIndexed(formattedLines) { index, line ->
                    Row(Modifier.padding(vertical = 1.dp)) {
                        Text(
                            text = (index + 1).toString().padStart(4, ' '),
                            color = Color(0xFF475569),
                            fontSize = 11.sp,
                            fontFamily = FontFamily.Monospace,
                            modifier = Modifier.padding(end = 12.dp)
                        )

                        val annotated = buildAnnotatedString {
                            val colonIndex = line.indexOf(':')
                            if (colonIndex != -1 && line.trimStart().startsWith("\"")) {
                                val keyPart = line.substring(0, colonIndex)
                                val valPart = line.substring(colonIndex)
                                withStyle(SpanStyle(color = Color(0xFF38BDF8), fontWeight = FontWeight.Medium)) {
                                    append(keyPart)
                                }
                                withStyle(SpanStyle(color = Color(0xFFE2E8F0))) {
                                    append(":")
                                }
                                appendJsonValue(valPart.removePrefix(":"))
                            } else {
                                appendJsonValue(line)
                            }
                        }

                        Text(
                            text = annotated,
                            fontSize = 12.sp,
                            fontFamily = FontFamily.Monospace
                        )
                    }
                }
            }
        }
    }
}

private fun androidx.compose.ui.text.AnnotatedString.Builder.appendJsonValue(value: String) {
    val trimmed = value.trim()
    when {
        trimmed.startsWith("\"") && trimmed.endsWith("\"") -> {
            withStyle(SpanStyle(color = Color(0xFF4ADE80))) {
                append(value)
            }
        }
        trimmed == "true" || trimmed == "false" -> {
            withStyle(SpanStyle(color = Color(0xFFF43F5E), fontWeight = FontWeight.Bold)) {
                append(value)
            }
        }
        trimmed == "null" -> {
            withStyle(SpanStyle(color = Color(0xFF94A3B8), fontStyle = androidx.compose.ui.text.font.FontStyle.Italic)) {
                append(value)
            }
        }
        trimmed.toDoubleOrNull() != null -> {
            withStyle(SpanStyle(color = Color(0xFFF59E0B))) {
                append(value)
            }
        }
        else -> {
            withStyle(SpanStyle(color = Color(0xFFE2E8F0))) {
                append(value)
            }
        }
    }
}
