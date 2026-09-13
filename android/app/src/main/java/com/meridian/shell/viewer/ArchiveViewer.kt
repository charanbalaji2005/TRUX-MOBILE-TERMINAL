package com.meridian.shell.viewer

import android.widget.Toast
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Folder
import androidx.compose.material.icons.filled.InsertDriveFile
import androidx.compose.material.icons.filled.Unarchive
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
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.text.SimpleDateFormat
import java.util.*
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

data class ArchiveEntryInfo(
    val name: String,
    val size: Long,
    val compressedSize: Long,
    val isDirectory: Boolean,
    val time: Long
)

@Composable
fun ArchiveViewer(file: File) {
    val context = LocalContext.current
    val scope = rememberCoroutineScope()
    var entries by remember { mutableStateOf<List<ArchiveEntryInfo>>(emptyList()) }
    var isLoading by remember { mutableStateOf(true) }
    var isExtracting by remember { mutableStateOf(false) }
    var extractMsg by remember { mutableStateOf<String?>(null) }

    LaunchedEffect(file) {
        isLoading = true
        withContext(Dispatchers.IO) {
            val list = mutableListOf<ArchiveEntryInfo>()
            try {
                if (file.name.endsWith(".zip", ignoreCase = true) ||
                    file.name.endsWith(".apk", ignoreCase = true) ||
                    file.name.endsWith(".jar", ignoreCase = true)) {
                    ZipInputStream(FileInputStream(file)).use { zis ->
                        var entry: ZipEntry? = zis.nextEntry
                        while (entry != null) {
                            list.add(
                                ArchiveEntryInfo(
                                    name = entry.name,
                                    size = entry.size,
                                    compressedSize = entry.compressedSize,
                                    isDirectory = entry.isDirectory,
                                    time = entry.time
                                )
                            )
                            zis.closeEntry()
                            entry = zis.nextEntry
                        }
                    }
                } else {
                    list.add(ArchiveEntryInfo(name = file.name, size = file.length(), compressedSize = file.length(), isDirectory = false, time = file.lastModified()))
                }
                withContext(Dispatchers.Main) {
                    entries = list
                    isLoading = false
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    extractMsg = "Error reading archive: ${e.message}"
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
        // Archive Action Header
        Row(
            Modifier
                .fillMaxWidth()
                .background(Color(0xFF161B22))
                .padding(horizontal = 14.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = "${entries.size} files in archive",
                color = Color(0xFF94A3B8),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.SemiBold
            )

            Button(
                onClick = {
                    if (isExtracting) return@Button
                    isExtracting = true
                    extractMsg = null
                    scope.launch(Dispatchers.IO) {
                        try {
                            val targetDir = File(file.parentFile, file.nameWithoutExtension)
                            targetDir.mkdirs()
                            ZipInputStream(FileInputStream(file)).use { zis ->
                                var entry: ZipEntry? = zis.nextEntry
                                val buffer = ByteArray(8192)
                                while (entry != null) {
                                    val newFile = File(targetDir, entry.name)
                                    if (entry.isDirectory) {
                                        newFile.mkdirs()
                                    } else {
                                        newFile.parentFile?.mkdirs()
                                        FileOutputStream(newFile).use { fos ->
                                            var len: Int
                                            while (zis.read(buffer).also { len = it } > 0) {
                                                fos.write(buffer, 0, len)
                                            }
                                        }
                                    }
                                    zis.closeEntry()
                                    entry = zis.nextEntry
                                }
                            }
                            withContext(Dispatchers.Main) {
                                isExtracting = false
                                Toast.makeText(context, "Extracted to ${targetDir.name}/", Toast.LENGTH_LONG).show()
                            }
                        } catch (e: Exception) {
                            withContext(Dispatchers.Main) {
                                isExtracting = false
                                extractMsg = "Extraction failed: ${e.message}"
                            }
                        }
                    }
                },
                colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF334155)),
                shape = RoundedCornerShape(6.dp),
                contentPadding = PaddingValues(horizontal = 12.dp, vertical = 4.dp),
                modifier = Modifier.height(30.dp)
            ) {
                Icon(
                    imageVector = Icons.Default.Unarchive,
                    contentDescription = "Extract",
                    tint = Color.White,
                    modifier = Modifier.size(16.dp)
                )
                Spacer(Modifier.width(6.dp))
                Text(
                    text = if (isExtracting) "Extracting..." else "Extract",
                    color = Color.White,
                    fontSize = 11.sp,
                    fontFamily = FontFamily.Monospace
                )
            }
        }

        if (extractMsg != null) {
            Box(
                Modifier
                    .fillMaxWidth()
                    .background(Color(0xFF7F1D1D))
                    .padding(8.dp)
            ) {
                Text(extractMsg!!, color = Color.White, fontSize = 11.sp, fontFamily = FontFamily.Monospace)
            }
        }

        if (isLoading) {
            Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                CircularProgressIndicator(color = Color.White, strokeWidth = 2.dp)
            }
        } else {
            LazyColumn(
                Modifier
                    .fillMaxSize()
                    .padding(horizontal = 12.dp, vertical = 6.dp)
            ) {
                items(entries) { item ->
                    Row(
                        Modifier
                            .fillMaxWidth()
                            .padding(vertical = 4.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Icon(
                            imageVector = if (item.isDirectory) Icons.Default.Folder else Icons.Default.InsertDriveFile,
                            contentDescription = null,
                            tint = if (item.isDirectory) Color(0xFF38BDF8) else Color(0xFF94A3B8),
                            modifier = Modifier.size(18.dp)
                        )
                        Spacer(Modifier.width(10.dp))
                        Column(Modifier.weight(1f)) {
                            Text(
                                text = item.name,
                                color = Color(0xFFF1F5F9),
                                fontSize = 12.sp,
                                fontFamily = FontFamily.Monospace
                            )
                            if (!item.isDirectory && item.size >= 0) {
                                Text(
                                    text = "${FileViewerManager.formatFileSize(item.size)} (Compressed: ${FileViewerManager.formatFileSize(item.compressedSize)})",
                                    color = Color(0xFF64748B),
                                    fontSize = 10.sp,
                                    fontFamily = FontFamily.Monospace
                                )
                            }
                        }
                    }
                    HorizontalDivider(color = Color(0xFF1E293B), thickness = 0.5.dp)
                }
            }
        }
    }
}
