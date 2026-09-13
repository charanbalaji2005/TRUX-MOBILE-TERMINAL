package com.meridian.shell.ui

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.meridian.shell.TerminalSession

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SessionActionsSheet(
    session: TerminalSession,
    onRename: (String) -> Unit,
    onRestart: () -> Unit,
    onClose: () -> Unit,
    onExportPdf: () -> Unit,
    onDismiss: () -> Unit
) {
    var name by remember(session.id) { mutableStateOf(session.name) }
    var confirmClose by remember { mutableStateOf(false) }

    ModalBottomSheet(onDismissRequest = onDismiss, containerColor = Color(0xFF0C0C0C)) {
        Column(Modifier.padding(24.dp)) {
            Text("SESSION ACTIONS", color = Color(0xFF94A3B8), fontSize = 11.sp,
                 fontFamily = FontFamily.Monospace, fontWeight = androidx.compose.ui.text.font.FontWeight.Bold)
            Spacer(Modifier.height(16.dp))
            OutlinedTextField(
                value = name,
                onValueChange = { name = it },
                label = { Text("Session Name") },
                singleLine = true,
                modifier = Modifier.fillMaxWidth()
            )
            Spacer(Modifier.height(20.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Button(
                    onClick = onExportPdf,
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF1E293B)),
                    modifier = Modifier.weight(1f)
                ) {
                    Text("📄 PDF", fontSize = 12.sp, fontFamily = FontFamily.Monospace)
                }
                Button(
                    onClick = { onRename(name) },
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF1E293B)),
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Rename", fontSize = 12.sp, fontFamily = FontFamily.Monospace)
                }
                Button(
                    onClick = onRestart,
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF1E293B)),
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Restart", fontSize = 12.sp, fontFamily = FontFamily.Monospace)
                }
            }
            Spacer(Modifier.height(12.dp))
            Button(
                onClick = {
                    if (session.isAlive && !confirmClose) confirmClose = true else onClose()
                },
                colors = ButtonDefaults.buttonColors(containerColor = if (confirmClose) Color(0xFFEF4444) else Color(0xFF3B1E1E)),
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(
                    if (confirmClose) "Confirm: Close Session & Process" else "Close Session",
                    color = Color.White,
                    fontSize = 12.sp,
                    fontFamily = FontFamily.Monospace
                )
            }
            Spacer(Modifier.height(12.dp))
        }
    }
}
