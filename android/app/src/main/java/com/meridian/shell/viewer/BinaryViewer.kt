package com.meridian.shell.viewer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.OpenInNew
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileInputStream

@Composable
fun BinaryViewer(
    file: File,
    fileViewerManager: FileViewerManager,
    isExecutable: Boolean = false
) {
    val context = LocalContext.current
    var hexDump by remember { mutableStateOf("") }
    val details = remember(file) { fileViewerManager.getFileDetails(file) }

    LaunchedEffect(file) {
        withContext(Dispatchers.IO) {
            try {
                FileInputStream(file).use { fis ->
                    val buffer = ByteArray(256)
                    val read = fis.read(buffer)
                    if (read > 0) {
                        val sb = StringBuilder()
                        for (i in 0 until read step 16) {
                            val chunk = buffer.sliceArray(i until minOf(i + 16, read))
                            val hex = chunk.joinToString(" ") { "%02X".format(it) }.padEnd(48, ' ')
                            val ascii = chunk.map { if (it in 32..126) it.toInt().toChar() else '.' }.joinToString("")
                            sb.append("%04X  %s  |%s|\n".format(i, hex, ascii))
                        }
                        withContext(Dispatchers.Main) {
                            hexDump = sb.toString()
                        }
                    }
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    hexDump = "Error reading binary: ${e.message}"
                }
            }
        }
    }

    Column(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0D1117))
            .verticalScroll(rememberScrollState())
            .padding(20.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        if (isExecutable) {
            // Executable Warning Banner per specification
            Row(
                Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(8.dp))
                    .background(Color(0xFF451A03))
                    .border(1.dp, Color(0xFFB45309), RoundedCornerShape(8.dp))
                    .padding(14.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Icon(
                    imageVector = Icons.Default.Warning,
                    contentDescription = "Warning",
                    tint = Color(0xFFF59E0B),
                    modifier = Modifier.size(24.dp)
                )
                Spacer(Modifier.width(12.dp))
                Column {
                    Text(
                        text = "Executable File",
                        color = Color(0xFFFDE68A),
                        fontSize = 13.sp,
                        fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace
                    )
                    Text(
                        text = "TRUX does not execute files from the file previewer for security.\nTo run this program, use the terminal:\n./${file.name}",
                        color = Color(0xFFFCD34D),
                        fontSize = 11.sp,
                        fontFamily = FontFamily.Monospace,
                        modifier = Modifier.padding(top = 4.dp)
                    )
                }
            }
            Spacer(Modifier.height(20.dp))
        }

        // File Details Card
        Column(
            Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(10.dp))
                .background(Color(0xFF161B22))
                .border(1.dp, Color(0xFF30363D), RoundedCornerShape(10.dp))
                .padding(16.dp)
        ) {
            Text(
                text = "FILE INFORMATION",
                color = Color(0xFF94A3B8),
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold,
                fontFamily = FontFamily.Monospace,
                letterSpacing = 1.sp
            )
            Spacer(Modifier.height(12.dp))

            InfoRow("Name", details.name)
            InfoRow("Size", "${details.sizeString} (${details.sizeBytes} bytes)")
            InfoRow("MIME", details.mimeType)
            InfoRow("Permissions", details.permissions)
            InfoRow("Modified", details.lastModified)
            InfoRow("Path", details.path)
            InfoRow("SHA-256", details.sha256)
        }

        Spacer(Modifier.height(20.dp))

        // Open with external app button
        Button(
            onClick = { fileViewerManager.openWithExternalApp(context, file) },
            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF1E293B)),
            border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF334155)),
            shape = RoundedCornerShape(8.dp),
            modifier = Modifier.fillMaxWidth().height(44.dp)
        ) {
            Icon(
                imageVector = Icons.Default.OpenInNew,
                contentDescription = "Open with another app",
                tint = Color(0xFF38BDF8),
                modifier = Modifier.size(18.dp)
            )
            Spacer(Modifier.width(8.dp))
            Text(
                "Open with another app",
                color = Color.White,
                fontSize = 13.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.SemiBold
            )
        }

        Spacer(Modifier.height(24.dp))

        // Hex Dump View
        if (hexDump.isNotEmpty()) {
            Column(
                Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(8.dp))
                    .background(Color(0xFF080B10))
                    .border(1.dp, Color(0xFF1F242C), RoundedCornerShape(8.dp))
                    .padding(12.dp)
            ) {
                Text(
                    text = "HEX DUMP (FIRST 256 BYTES)",
                    color = Color(0xFF64748B),
                    fontSize = 10.sp,
                    fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace
                )
                Spacer(Modifier.height(8.dp))
                Text(
                    text = hexDump,
                    color = Color(0xFF94A3B8),
                    fontSize = 10.sp,
                    fontFamily = FontFamily.Monospace,
                    lineHeight = 14.sp
                )
            }
        }
    }
}

@Composable
fun InfoRow(label: String, value: String) {
    Row(
        Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            text = label,
            color = Color(0xFF64748B),
            fontSize = 11.sp,
            fontFamily = FontFamily.Monospace,
            modifier = Modifier.weight(0.35f)
        )
        Text(
            text = value,
            color = Color(0xFFE2E8F0),
            fontSize = 11.sp,
            fontFamily = FontFamily.Monospace,
            modifier = Modifier.weight(0.65f)
        )
    }
}
