package com.meridian.shell.ui

import androidx.compose.animation.*
import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.meridian.shell.StorageManager
import kotlinx.coroutines.launch

/**
 * Storage management screen.
 *
 * Shows real filesystem sizes (no fake numbers):
 *
 *   STORAGE
 *
 *   Sessions             18 MB
 *   Linux environment   1.2 GB
 *   Packages            310 MB
 *   Package cache        74 MB
 *   User files           90 MB
 *   ────────────────────────
 *   Total               1.69 GB
 *
 *   [ Clear package cache ]      (safe)
 *   [ Clear Meridian data ]      (dangerous — 2-step confirm)
 */
@Composable
fun StorageScreen(
    storageManager: StorageManager,
    onNavigateBack: () -> Unit
) {
    val scope = rememberCoroutineScope()
    var breakdown by remember { mutableStateOf<StorageManager.StorageBreakdown?>(null) }
    var loading by remember { mutableStateOf(true) }
    var clearCacheConfirm by remember { mutableStateOf(false) }
    var clearAllStep by remember { mutableStateOf(0) }  // 0=none, 1=first confirm, 2=second
    var statusMessage by remember { mutableStateOf("") }

    // Load sizes on entry
    LaunchedEffect(Unit) {
        loading = true
        breakdown = runCatching { storageManager.calculate() }.getOrNull()
        loading = false
    }

    Column(
        Modifier
            .fillMaxSize()
            .background(Color.Black)
            .verticalScroll(rememberScrollState())
            .padding(24.dp)
            .systemBarsPadding()
    ) {
        // Header
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(
                "←",
                color = Color(0xFF7FD1FF),
                fontSize = 20.sp,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier.clickable { onNavigateBack() }.padding(end = 16.dp)
            )
            Text(
                "STORAGE",
                color = Color(0xFF5A5A5A),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.Bold
            )
        }

        Spacer(Modifier.height(32.dp))

        if (loading) {
            Text(
                "Calculating...",
                color = Color(0xFF5A5A5A),
                fontFamily = FontFamily.Monospace,
                fontSize = 13.sp
            )
        } else if (breakdown != null) {
            val b = breakdown!!
            StorageTable(b)
        } else {
            Text(
                "Could not read storage information.",
                color = Color(0xFFE06C6C),
                fontFamily = FontFamily.Monospace,
                fontSize = 13.sp
            )
        }

        Spacer(Modifier.height(40.dp))

        // Status message
        if (statusMessage.isNotBlank()) {
            Text(
                statusMessage,
                color = Color(0xFF0DBC79),
                fontFamily = FontFamily.Monospace,
                fontSize = 12.sp,
                modifier = Modifier.padding(bottom = 16.dp)
            )
        }

        // Clear package cache
        OutlinedButton(
            onClick = { clearCacheConfirm = true },
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(contentColor = Color(0xFFE8E8E8)),
            border = BorderStroke(1.dp, Color(0xFF2A2A2A))
        ) {
            Text("Clear package cache", fontFamily = FontFamily.Monospace)
        }

        if (clearCacheConfirm) {
            Spacer(Modifier.height(8.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                TextButton(
                    onClick = {
                        clearCacheConfirm = false
                        scope.launch {
                            val freed = storageManager.clearPackageCache()
                            val mb = freed / (1024.0 * 1024)
                            statusMessage = "Freed %.1f MB of package cache.".format(mb)
                            breakdown = runCatching { storageManager.calculate() }.getOrNull()
                        }
                    }
                ) {
                    Text("Confirm clear cache", color = Color(0xFFE5E510), fontFamily = FontFamily.Monospace)
                }
                TextButton(onClick = { clearCacheConfirm = false }) {
                    Text("Cancel", color = Color(0xFF5A5A5A), fontFamily = FontFamily.Monospace)
                }
            }
        }

        Spacer(Modifier.height(12.dp))

        // Clear all data (dangerous)
        OutlinedButton(
            onClick = { clearAllStep = 1 },
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(contentColor = Color(0xFFE06C6C)),
            border = BorderStroke(1.dp, Color(0xFF5A2020))
        ) {
            Text("Clear Meridian data", fontFamily = FontFamily.Monospace)
        }

        when (clearAllStep) {
            1 -> {
                Spacer(Modifier.height(8.dp))
                Text(
                    "⚠ This will delete ALL sessions, the Linux environment, packages, and user files. This cannot be undone.",
                    color = Color(0xFFE06C6C),
                    fontFamily = FontFamily.Monospace,
                    fontSize = 12.sp
                )
                Spacer(Modifier.height(8.dp))
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    TextButton(onClick = { clearAllStep = 2 }) {
                        Text("I understand, continue", color = Color(0xFFE06C6C), fontFamily = FontFamily.Monospace)
                    }
                    TextButton(onClick = { clearAllStep = 0 }) {
                        Text("Cancel", color = Color(0xFF5A5A5A), fontFamily = FontFamily.Monospace)
                    }
                }
            }
            2 -> {
                Spacer(Modifier.height(8.dp))
                Text(
                    "Final confirmation: delete everything?",
                    color = Color(0xFFFF4040),
                    fontFamily = FontFamily.Monospace,
                    fontSize = 13.sp,
                    fontWeight = FontWeight.Bold
                )
                Spacer(Modifier.height(8.dp))
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    TextButton(onClick = {
                        clearAllStep = 0
                        scope.launch {
                            runCatching { storageManager.clearAllData() }
                            statusMessage = "All Meridian data cleared. Restart the app."
                        }
                    }) {
                        Text("DELETE EVERYTHING", color = Color(0xFFFF4040),
                            fontFamily = FontFamily.Monospace, fontWeight = FontWeight.Bold)
                    }
                    TextButton(onClick = { clearAllStep = 0 }) {
                        Text("Cancel", color = Color(0xFF5A5A5A), fontFamily = FontFamily.Monospace)
                    }
                }
            }
        }

        Spacer(Modifier.height(32.dp))

        // Reload button
        TextButton(
            onClick = {
                scope.launch {
                    loading = true
                    breakdown = runCatching { storageManager.calculate() }.getOrNull()
                    loading = false
                }
            },
            modifier = Modifier.align(Alignment.CenterHorizontally)
        ) {
            Text("↻ Refresh", color = Color(0xFF5A5A5A), fontFamily = FontFamily.Monospace, fontSize = 12.sp)
        }
    }
}

@Composable
private fun StorageTable(b: StorageManager.StorageBreakdown) {
    Column {
        StorageRow("Sessions", b.sessionsBytes)
        StorageRow("Linux environment", b.linuxEnvBytes)
        StorageRow("Packages", b.packagesBytes)
        StorageRow("Package cache", b.packageCacheBytes)
        StorageRow("User files", b.userFilesBytes)
        Divider(color = Color(0xFF2A2A2A), modifier = Modifier.padding(vertical = 8.dp))
        StorageRow("Total", b.totalBytes, highlight = true)
    }
}

@Composable
private fun StorageRow(label: String, bytes: Long, highlight: Boolean = false) {
    Row(
        Modifier.fillMaxWidth().padding(vertical = 6.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            label,
            color = if (highlight) Color(0xFFE8E8E8) else Color(0xFFB8B8B8),
            fontFamily = FontFamily.Monospace,
            fontSize = 13.sp,
            fontWeight = if (highlight) FontWeight.Bold else FontWeight.Normal
        )
        Text(
            formatBytes(bytes),
            color = if (highlight) Color(0xFF7FD1FF) else Color(0xFFE8E8E8),
            fontFamily = FontFamily.Monospace,
            fontSize = 13.sp,
            fontWeight = if (highlight) FontWeight.Bold else FontWeight.Normal
        )
    }
}

private fun formatBytes(bytes: Long): String = when {
    bytes < 1024 -> "$bytes B"
    bytes < 1024 * 1024 -> "%.1f KB".format(bytes / 1024.0)
    bytes < 1024L * 1024 * 1024 -> "%.1f MB".format(bytes / (1024.0 * 1024))
    else -> "%.2f GB".format(bytes / (1024.0 * 1024 * 1024))
}
