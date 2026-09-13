package com.meridian.shell.viewer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import java.io.File

@Composable
fun MarkdownViewer(file: File) {
    var isRawMode by remember { mutableStateOf(false) }
    var textContent by remember { mutableStateOf("") }

    LaunchedEffect(file) {
        textContent = try {
            file.readText()
        } catch (e: Exception) {
            "Error reading markdown: ${e.message}"
        }
    }

    Column(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0F1117))
    ) {
        // Mode toggle header
        Row(
            Modifier
                .fillMaxWidth()
                .background(Color(0xFF161B22))
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = "MARKDOWN PREVIEW",
                color = Color(0xFF94A3B8),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.SemiBold
            )

            Row {
                Button(
                    onClick = { isRawMode = false },
                    colors = ButtonDefaults.buttonColors(
                        containerColor = if (!isRawMode) Color(0xFF334155) else Color(0xFF1E293B)
                    ),
                    shape = RoundedCornerShape(topStart = 6.dp, bottomStart = 6.dp),
                    contentPadding = PaddingValues(horizontal = 10.dp, vertical = 2.dp),
                    modifier = Modifier.height(28.dp)
                ) {
                    Text("Rendered", fontSize = 11.sp, color = Color.White)
                }

                Button(
                    onClick = { isRawMode = true },
                    colors = ButtonDefaults.buttonColors(
                        containerColor = if (isRawMode) Color(0xFF334155) else Color(0xFF1E293B)
                    ),
                    shape = RoundedCornerShape(topEnd = 6.dp, bottomEnd = 6.dp),
                    contentPadding = PaddingValues(horizontal = 10.dp, vertical = 2.dp),
                    modifier = Modifier.height(28.dp)
                ) {
                    Text("Raw", fontSize = 11.sp, color = Color.White)
                }
            }
        }

        if (isRawMode) {
            TextViewer(file = file, isCode = false)
        } else {
            val lines = remember(textContent) { textContent.lines() }
            var inCodeBlock by remember { mutableStateOf(false) }

            LazyColumn(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 16.dp, vertical = 12.dp)
            ) {
                items(lines) { line ->
                    when {
                        line.startsWith("```") -> {
                            inCodeBlock = !inCodeBlock
                            Spacer(Modifier.height(4.dp))
                        }
                        inCodeBlock -> {
                            Box(
                                Modifier
                                    .fillMaxWidth()
                                    .background(Color(0xFF161B22))
                                    .padding(horizontal = 12.dp, vertical = 2.dp)
                            ) {
                                Text(
                                    text = line,
                                    color = Color(0xFF38BDF8),
                                    fontSize = 12.sp,
                                    fontFamily = FontFamily.Monospace
                                )
                            }
                        }
                        line.startsWith("# ") -> {
                            Spacer(Modifier.height(14.dp))
                            Text(
                                text = line.removePrefix("# "),
                                color = Color.White,
                                fontSize = 20.sp,
                                fontWeight = FontWeight.Bold,
                                fontFamily = FontFamily.Monospace
                            )
                            Spacer(Modifier.height(6.dp))
                        }
                        line.startsWith("## ") -> {
                            Spacer(Modifier.height(10.dp))
                            Text(
                                text = line.removePrefix("## "),
                                color = Color(0xFFF1F5F9),
                                fontSize = 16.sp,
                                fontWeight = FontWeight.SemiBold,
                                fontFamily = FontFamily.Monospace
                            )
                            Spacer(Modifier.height(4.dp))
                        }
                        line.startsWith("### ") -> {
                            Spacer(Modifier.height(8.dp))
                            Text(
                                text = line.removePrefix("### "),
                                color = Color(0xFFCBD5E1),
                                fontSize = 14.sp,
                                fontWeight = FontWeight.Medium,
                                fontFamily = FontFamily.Monospace
                            )
                            Spacer(Modifier.height(4.dp))
                        }
                        line.startsWith("> ") -> {
                            Row(
                                Modifier
                                    .fillMaxWidth()
                                    .padding(vertical = 4.dp)
                                    .border(
                                        width = 2.dp,
                                        color = Color(0xFF475569),
                                        shape = RoundedCornerShape(2.dp)
                                    )
                                    .padding(horizontal = 10.dp, vertical = 4.dp)
                            ) {
                                Text(
                                    text = line.removePrefix("> "),
                                    color = Color(0xFF94A3B8),
                                    fontSize = 12.sp,
                                    fontFamily = FontFamily.Monospace
                                )
                            }
                        }
                        line.startsWith("- ") || line.startsWith("* ") -> {
                            Row(Modifier.padding(start = 8.dp, top = 2.dp, bottom = 2.dp)) {
                                Text("• ", color = Color(0xFF38BDF8), fontSize = 13.sp)
                                Text(
                                    text = line.substring(2),
                                    color = Color(0xFFE2E8F0),
                                    fontSize = 13.sp,
                                    fontFamily = FontFamily.Monospace
                                )
                            }
                        }
                        line.isBlank() -> {
                            Spacer(Modifier.height(8.dp))
                        }
                        else -> {
                            Text(
                                text = line,
                                color = Color(0xFFE2E8F0),
                                fontSize = 13.sp,
                                fontFamily = FontFamily.Monospace,
                                modifier = Modifier.padding(vertical = 2.dp)
                            )
                        }
                    }
                }
            }
        }
    }
}
