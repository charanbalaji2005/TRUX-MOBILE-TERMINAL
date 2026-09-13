package com.meridian.shell.viewer

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.OpenInNew
import androidx.compose.material.icons.filled.Share
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import java.io.File

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun UniversalFileViewerDialog(
    file: File,
    fileType: FileType,
    fileViewerManager: FileViewerManager,
    onDismiss: () -> Unit
) {
    val context = LocalContext.current
    var showInfoDialog by remember { mutableStateOf(false) }

    Dialog(
        onDismissRequest = onDismiss,
        properties = DialogProperties(
            usePlatformDefaultWidth = false,
            decorFitsSystemWindows = false
        )
    ) {
        BackHandler(onBack = onDismiss)

        Scaffold(
            topBar = {
                TopAppBar(
                    title = {
                        Column {
                            Text(
                                text = file.name,
                                color = Color.White,
                                fontSize = 14.sp,
                                fontFamily = FontFamily.Monospace,
                                fontWeight = FontWeight.Bold,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                            Text(
                                text = "${fileType.displayName} • ${FileViewerManager.formatFileSize(file.length())}",
                                color = Color(0xFF94A3B8),
                                fontSize = 10.sp,
                                fontFamily = FontFamily.Monospace
                            )
                        }
                    },
                    navigationIcon = {
                        IconButton(onClick = onDismiss) {
                            Icon(
                                imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                                contentDescription = "Close viewer",
                                tint = Color.White
                            )
                        }
                    },
                    actions = {
                        IconButton(onClick = { fileViewerManager.shareFile(context, file) }) {
                            Icon(
                                imageVector = Icons.Default.Share,
                                contentDescription = "Share",
                                tint = Color(0xFFCBD5E1),
                                modifier = Modifier.size(20.dp)
                            )
                        }
                        IconButton(onClick = { fileViewerManager.openWithExternalApp(context, file) }) {
                            Icon(
                                imageVector = Icons.Default.OpenInNew,
                                contentDescription = "Open with...",
                                tint = Color(0xFFCBD5E1),
                                modifier = Modifier.size(20.dp)
                            )
                        }
                        IconButton(onClick = { showInfoDialog = true }) {
                            Icon(
                                imageVector = Icons.Default.Info,
                                contentDescription = "File information",
                                tint = Color(0xFFCBD5E1),
                                modifier = Modifier.size(20.dp)
                            )
                        }
                    },
                    colors = TopAppBarDefaults.topAppBarColors(
                        containerColor = Color(0xFF10141D)
                    )
                )
            },
            containerColor = Color(0xFF0A0D12)
        ) { innerPadding ->
            Box(
                Modifier
                    .fillMaxSize()
                    .padding(innerPadding)
                    .background(Color(0xFF0A0D12))
            ) {
                when (fileType) {
                    FileType.PDF -> PdfViewer(file = file)
                    FileType.IMAGE -> ImageViewer(file = file)
                    FileType.VIDEO -> MediaViewer(file = file, isAudioOnly = false)
                    FileType.AUDIO -> MediaViewer(file = file, isAudioOnly = true)
                    FileType.MARKDOWN -> MarkdownViewer(file = file)
                    FileType.JSON -> JsonViewer(file = file)
                    FileType.CSV -> CsvViewer(file = file)
                    FileType.CODE -> TextViewer(file = file, isCode = true)
                    FileType.TEXT, FileType.XML -> TextViewer(file = file, isCode = false)
                    FileType.ARCHIVE -> ArchiveViewer(file = file)
                    FileType.EXECUTABLE -> BinaryViewer(file = file, fileViewerManager = fileViewerManager, isExecutable = true)
                    FileType.OFFICE, FileType.BINARY -> BinaryViewer(file = file, fileViewerManager = fileViewerManager, isExecutable = false)
                }
            }
        }

        if (showInfoDialog) {
            val details = remember(file) { fileViewerManager.getFileDetails(file) }
            AlertDialog(
                onDismissRequest = { showInfoDialog = false },
                confirmButton = {
                    TextButton(onClick = { showInfoDialog = false }) {
                        Text("Close", color = Color(0xFF38BDF8), fontFamily = FontFamily.Monospace)
                    }
                },
                title = {
                    Text(
                        "File Information",
                        color = Color.White,
                        fontFamily = FontFamily.Monospace,
                        fontSize = 15.sp,
                        fontWeight = FontWeight.Bold
                    )
                },
                text = {
                    Column(Modifier.padding(top = 8.dp)) {
                        InfoRow("Name", details.name)
                        InfoRow("Size", "${details.sizeString} (${details.sizeBytes} bytes)")
                        InfoRow("MIME", details.mimeType)
                        InfoRow("Permissions", details.permissions)
                        InfoRow("Modified", details.lastModified)
                        InfoRow("Path", details.path)
                        InfoRow("SHA-256", details.sha256)
                    }
                },
                containerColor = Color(0xFF161B22),
                shape = RoundedCornerShape(12.dp)
            )
        }
    }
}
