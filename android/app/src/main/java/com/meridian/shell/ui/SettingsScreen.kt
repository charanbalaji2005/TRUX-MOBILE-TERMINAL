package com.meridian.shell.ui

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

@Composable
fun SettingsScreen(
    storageManager: StorageManager,
    onNavigateBack: () -> Unit
) {
    var showStorage by remember { mutableStateOf(false) }
    var fontSize by remember { mutableStateOf(13f) }

    if (showStorage) {
        StorageScreen(storageManager = storageManager, onNavigateBack = { showStorage = false })
        return
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
                "SETTINGS",
                color = Color(0xFF5A5A5A),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.Bold
            )
        }

        Spacer(Modifier.height(32.dp))

        // Font size
        Text("TERMINAL", color = Color(0xFF5A5A5A), fontSize = 10.sp, fontFamily = FontFamily.Monospace)
        Spacer(Modifier.height(12.dp))

        Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
            Text(
                "Font size",
                color = Color(0xFFB8B8B8),
                fontFamily = FontFamily.Monospace,
                fontSize = 13.sp,
                modifier = Modifier.weight(1f)
            )
            Text(
                "${fontSize.toInt()}sp",
                color = Color(0xFF7FD1FF),
                fontFamily = FontFamily.Monospace,
                fontSize = 13.sp
            )
        }
        Slider(
            value = fontSize,
            onValueChange = { fontSize = it },
            valueRange = 9f..20f,
            steps = 10,
            colors = SliderDefaults.colors(
                thumbColor = Color(0xFF7FD1FF),
                activeTrackColor = Color(0xFF7FD1FF),
                inactiveTrackColor = Color(0xFF2A2A2A)
            ),
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(Modifier.height(32.dp))
        Divider(color = Color(0xFF1A1A1A))
        Spacer(Modifier.height(24.dp))

        // Storage section
        Text("DATA", color = Color(0xFF5A5A5A), fontSize = 10.sp, fontFamily = FontFamily.Monospace)
        Spacer(Modifier.height(12.dp))

        SettingsRow(
            label = "Storage",
            sublabel = "Manage disk usage",
            onClick = { showStorage = true }
        )

        Spacer(Modifier.height(32.dp))
        Divider(color = Color(0xFF1A1A1A))
        Spacer(Modifier.height(24.dp))

        // About section
        Text("ABOUT", color = Color(0xFF5A5A5A), fontSize = 10.sp, fontFamily = FontFamily.Monospace)
        Spacer(Modifier.height(12.dp))

        SettingsRowStatic(label = "Version", value = "2.0.0")
        SettingsRowStatic(label = "Target SDK", value = "28 (Termux-compatible)")
        SettingsRowStatic(label = "License", value = "GPL-3.0")
    }
}

@Composable
private fun SettingsRow(label: String, sublabel: String, onClick: () -> Unit) {
    Row(
        Modifier
            .fillMaxWidth()
            .clickable { onClick() }
            .padding(vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(Modifier.weight(1f)) {
            Text(label, color = Color(0xFFE8E8E8), fontFamily = FontFamily.Monospace, fontSize = 13.sp)
            Text(sublabel, color = Color(0xFF5A5A5A), fontFamily = FontFamily.Monospace, fontSize = 11.sp)
        }
        Text("→", color = Color(0xFF5A5A5A), fontFamily = FontFamily.Monospace)
    }
}

@Composable
private fun SettingsRowStatic(label: String, value: String) {
    Row(
        Modifier.fillMaxWidth().padding(vertical = 10.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(label, color = Color(0xFFB8B8B8), fontFamily = FontFamily.Monospace, fontSize = 13.sp)
        Text(value, color = Color(0xFF5A5A5A), fontFamily = FontFamily.Monospace, fontSize = 13.sp)
    }
}
