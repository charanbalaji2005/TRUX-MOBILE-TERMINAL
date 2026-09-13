package com.meridian.shell.viewer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.rememberScrollState
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.File
import java.io.FileReader

@Composable
fun CsvViewer(file: File) {
    var rows by remember { mutableStateOf<List<List<String>>>(emptyList()) }
    var isLoading by remember { mutableStateOf(true) }

    LaunchedEffect(file) {
        isLoading = true
        withContext(Dispatchers.IO) {
            val parsed = mutableListOf<List<String>>()
            try {
                BufferedReader(FileReader(file)).use { reader ->
                    var count = 0
                    var line = reader.readLine()
                    while (line != null && count < 2000) {
                        parsed.add(parseCsvLine(line))
                        count++
                        line = reader.readLine()
                    }
                }
                withContext(Dispatchers.Main) {
                    rows = parsed
                    isLoading = false
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    rows = listOf(listOf("Error loading CSV: ${e.message}"))
                    isLoading = false
                }
            }
        }
    }

    if (isLoading) {
        Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            CircularProgressIndicator(color = Color.White, strokeWidth = 2.dp)
        }
        return
    }

    val headers = rows.firstOrNull() ?: emptyList()
    val dataRows = if (rows.size > 1) rows.subList(1, rows.size) else emptyList()
    val hScroll = rememberScrollState()

    Column(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0D1117))
    ) {
        // Table info
        Box(
            Modifier
                .fillMaxWidth()
                .background(Color(0xFF161B22))
                .padding(horizontal = 16.dp, vertical = 6.dp)
        ) {
            Text(
                text = "${rows.size} rows • ${headers.size} columns",
                color = Color(0xFF94A3B8),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.SemiBold
            )
        }

        Box(
            Modifier
                .weight(1f)
                .fillMaxWidth()
                .horizontalScroll(hScroll)
        ) {
            LazyColumn(Modifier.fillMaxHeight()) {
                // Header row
                item {
                    Row(
                        Modifier
                            .background(Color(0xFF1E293B))
                            .border(0.5.dp, Color(0xFF334155))
                    ) {
                        headers.forEachIndexed { i, col ->
                            Box(
                                Modifier
                                    .width(140.dp)
                                    .border(0.5.dp, Color(0xFF334155))
                                    .padding(horizontal = 10.dp, vertical = 8.dp)
                            ) {
                                Text(
                                    text = col.ifEmpty { "Col ${i + 1}" },
                                    color = Color.White,
                                    fontSize = 12.sp,
                                    fontWeight = FontWeight.Bold,
                                    fontFamily = FontFamily.Monospace
                                )
                            }
                        }
                    }
                }

                // Data rows
                itemsIndexed(dataRows) { rowIndex, row ->
                    val bg = if (rowIndex % 2 == 0) Color(0xFF0F141C) else Color(0xFF141923)
                    Row(
                        Modifier
                            .background(bg)
                            .border(0.5.dp, Color(0xFF1E293B))
                    ) {
                        headers.indices.forEach { colIndex ->
                            val cell = row.getOrElse(colIndex) { "" }
                            Box(
                                Modifier
                                    .width(140.dp)
                                    .border(0.5.dp, Color(0xFF1E293B))
                                    .padding(horizontal = 10.dp, vertical = 6.dp)
                            ) {
                                Text(
                                    text = cell,
                                    color = Color(0xFFCBD5E1),
                                    fontSize = 11.sp,
                                    fontFamily = FontFamily.Monospace
                                )
                            }
                        }
                    }
                }
            }
        }
    }
}

private fun parseCsvLine(line: String): List<String> {
    val result = mutableListOf<String>()
    val current = StringBuilder()
    var inQuotes = false

    for (char in line) {
        when (char) {
            '"' -> inQuotes = !inQuotes
            ',' -> {
                if (inQuotes) {
                    current.append(char)
                } else {
                    result.add(current.toString().trim())
                    current.clear()
                }
            }
            else -> current.append(char)
        }
    }
    result.add(current.toString().trim())
    return result
}
