package com.meridian.shell.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/**
 * Bottom sheet for creating a new terminal session.
 *
 * New terminal
 *
 * Name
 * [ Main                ]
 *
 * Shell
 * [ bash            ▼  ]
 *
 * Environment
 * [ Meridian        ▼  ]
 *
 *              [ CREATE ]
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun NewSessionSheet(
    onCreateSession: (name: String, shell: String, environment: String) -> Unit,
    onDismiss: () -> Unit,
    sessionIndex: Int = 1
) {
    var name by remember { mutableStateOf(if (sessionIndex == 1) "Main" else "Session $sessionIndex") }
    var shellExpanded by remember { mutableStateOf(false) }
    var envExpanded by remember { mutableStateOf(false) }

    val shells = listOf("bash", "sh")
    val environments = listOf(
        "TRUX" to true,
        "Debian" to false,    // disabled until installed
        "Ubuntu" to false     // disabled until installed
    )

    var selectedShell by remember { mutableStateOf("bash") }
    var selectedEnv by remember { mutableStateOf("TRUX") }

    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = Color(0xFF0C0C0C),
        scrimColor = Color.Black.copy(alpha = 0.7f)
    ) {
        Column(
            Modifier
                .fillMaxWidth()
                .padding(horizontal = 24.dp)
                .padding(bottom = 32.dp)
        ) {
            Text(
                "NEW TERMINAL",
                color = Color(0xFF5A5A5A),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace
            )
            Spacer(Modifier.height(20.dp))

            // Name field
            OutlinedTextField(
                value = name,
                onValueChange = { name = it },
                label = { Text("Name", fontFamily = FontFamily.Monospace) },
                singleLine = true,
                modifier = Modifier.fillMaxWidth(),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedBorderColor = Color(0xFF7FD1FF),
                    unfocusedBorderColor = Color(0xFF2A2A2A),
                    focusedTextColor = Color(0xFFE8E8E8),
                    unfocusedTextColor = Color(0xFFE8E8E8),
                    cursorColor = Color(0xFF7FD1FF),
                    focusedLabelColor = Color(0xFF7FD1FF),
                    unfocusedLabelColor = Color(0xFF5A5A5A)
                )
            )

            Spacer(Modifier.height(16.dp))

            // Shell dropdown
            ExposedDropdownMenuBox(
                expanded = shellExpanded,
                onExpandedChange = { shellExpanded = it }
            ) {
                OutlinedTextField(
                    value = selectedShell,
                    onValueChange = {},
                    readOnly = true,
                    label = { Text("Shell", fontFamily = FontFamily.Monospace) },
                    trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(shellExpanded) },
                    modifier = Modifier.fillMaxWidth().menuAnchor(),
                    colors = OutlinedTextFieldDefaults.colors(
                        focusedBorderColor = Color(0xFF7FD1FF),
                        unfocusedBorderColor = Color(0xFF2A2A2A),
                        focusedTextColor = Color(0xFFE8E8E8),
                        unfocusedTextColor = Color(0xFFE8E8E8),
                        focusedLabelColor = Color(0xFF7FD1FF),
                        unfocusedLabelColor = Color(0xFF5A5A5A)
                    )
                )
                ExposedDropdownMenu(
                    expanded = shellExpanded,
                    onDismissRequest = { shellExpanded = false },
                    modifier = Modifier.background(Color(0xFF161616))
                ) {
                    shells.forEach { shell ->
                        DropdownMenuItem(
                            text = { Text(shell, fontFamily = FontFamily.Monospace,
                                color = Color(0xFFE8E8E8)) },
                            onClick = { selectedShell = shell; shellExpanded = false }
                        )
                    }
                }
            }

            Spacer(Modifier.height(16.dp))

            // Environment dropdown
            ExposedDropdownMenuBox(
                expanded = envExpanded,
                onExpandedChange = { envExpanded = it }
            ) {
                OutlinedTextField(
                    value = selectedEnv,
                    onValueChange = {},
                    readOnly = true,
                    label = { Text("Environment", fontFamily = FontFamily.Monospace) },
                    trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(envExpanded) },
                    modifier = Modifier.fillMaxWidth().menuAnchor(),
                    colors = OutlinedTextFieldDefaults.colors(
                        focusedBorderColor = Color(0xFF7FD1FF),
                        unfocusedBorderColor = Color(0xFF2A2A2A),
                        focusedTextColor = Color(0xFFE8E8E8),
                        unfocusedTextColor = Color(0xFFE8E8E8),
                        focusedLabelColor = Color(0xFF7FD1FF),
                        unfocusedLabelColor = Color(0xFF5A5A5A)
                    )
                )
                ExposedDropdownMenu(
                    expanded = envExpanded,
                    onDismissRequest = { envExpanded = false },
                    modifier = Modifier.background(Color(0xFF161616))
                ) {
                    environments.forEach { (env, enabled) ->
                        DropdownMenuItem(
                            text = {
                                Text(
                                    if (enabled) env else "$env (not installed)",
                                    fontFamily = FontFamily.Monospace,
                                    color = if (enabled) Color(0xFFE8E8E8) else Color(0xFF5A5A5A)
                                )
                            },
                            onClick = {
                                if (enabled) { selectedEnv = env; envExpanded = false }
                            },
                            enabled = enabled
                        )
                    }
                }
            }

            Spacer(Modifier.height(28.dp))

            // Create button
            Button(
                onClick = {
                    val sessionName = name.trim().ifEmpty { "Session $sessionIndex" }
                    onCreateSession(sessionName, selectedShell, selectedEnv.lowercase())
                    onDismiss()
                },
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color(0xFF1A3A4A),
                    contentColor = Color(0xFF7FD1FF)
                )
            ) {
                Text(
                    "CREATE",
                    fontFamily = FontFamily.Monospace,
                    fontSize = 14.sp,
                    modifier = Modifier.padding(vertical = 4.dp)
                )
            }
        }
    }
}
