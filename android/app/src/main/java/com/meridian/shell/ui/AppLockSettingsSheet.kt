package com.meridian.shell.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Fingerprint
import androidx.compose.material.icons.filled.Lock
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
import androidx.fragment.app.FragmentActivity
import com.meridian.shell.security.AppLockManager

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AppLockSettingsSheet(
    appLockManager: AppLockManager,
    onDismiss: () -> Unit
) {
    val context = LocalContext.current
    val activity = context as? FragmentActivity

    var statusMessage by remember { mutableStateOf<String?>(null) }
    var isError by remember { mutableStateOf(false) }

    val isEnabled = appLockManager.isEnabled
    val timeoutMs = appLockManager.lockTimeoutMs
    val lockOnBg = appLockManager.lockOnBackground
    val lockOnScreenOff = appLockManager.lockOnScreenOff

    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = Color(0xFF0F1216),
        scrimColor = Color.Black.copy(alpha = 0.75f)
    ) {
        Column(
            Modifier
                .fillMaxWidth()
                .padding(horizontal = 24.dp)
                .padding(bottom = 36.dp)
        ) {
            // Header
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        imageVector = Icons.Default.Lock,
                        contentDescription = "App Lock",
                        tint = Color(0xFFF1F5F9),
                        modifier = Modifier.size(20.dp)
                    )
                    Spacer(Modifier.width(10.dp))
                    Text(
                        "APP LOCK",
                        color = Color(0xFFF1F5F9),
                        fontSize = 15.sp,
                        fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace,
                        letterSpacing = 1.sp
                    )
                }

                IconButton(
                    onClick = onDismiss,
                    modifier = Modifier.size(32.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.Close,
                        contentDescription = "Close",
                        tint = Color(0xFF94A3B8),
                        modifier = Modifier.size(18.dp)
                    )
                }
            }

            Spacer(Modifier.height(18.dp))

            // Main Toggle Row
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(10.dp))
                    .background(Color(0xFF191F28))
                    .padding(horizontal = 16.dp, vertical = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Column(Modifier.weight(1f).padding(end = 16.dp)) {
                    Text(
                        "Enable App Lock",
                        color = Color(0xFFF8FAFC),
                        fontSize = 14.sp,
                        fontFamily = FontFamily.Monospace,
                        fontWeight = FontWeight.SemiBold
                    )
                    Text(
                        if (isEnabled) "TRUX requires authentication to access" else "Terminal access is unprotected",
                        color = Color(0xFF64748B),
                        fontSize = 11.sp,
                        fontFamily = FontFamily.Monospace
                    )
                }

                Switch(
                    checked = isEnabled,
                    onCheckedChange = { checked ->
                        if (activity == null) return@Switch
                        statusMessage = null
                        if (checked) {
                            appLockManager.enable(
                                activity = activity,
                                onSuccess = {
                                    statusMessage = "App Lock enabled successfully."
                                    isError = false
                                },
                                onError = { error ->
                                    statusMessage = error
                                    isError = true
                                }
                            )
                        } else {
                            appLockManager.disable(
                                activity = activity,
                                onSuccess = {
                                    statusMessage = "App Lock disabled."
                                    isError = false
                                },
                                onError = { error ->
                                    statusMessage = error
                                    isError = true
                                }
                            )
                        }
                    },
                    colors = SwitchDefaults.colors(
                        checkedThumbColor = Color(0xFFF8FAFC),
                        checkedTrackColor = Color(0xFF334155),
                        uncheckedThumbColor = Color(0xFF64748B),
                        uncheckedTrackColor = Color(0xFF141820)
                    )
                )
            }

            // Status message
            if (statusMessage != null) {
                Spacer(Modifier.height(12.dp))
                Text(
                    text = statusMessage!!,
                    color = if (isError) Color(0xFFF87171) else Color(0xFF4ADE80),
                    fontSize = 11.sp,
                    fontFamily = FontFamily.Monospace,
                    modifier = Modifier.padding(horizontal = 4.dp)
                )
            }

            if (isEnabled) {
                Spacer(Modifier.height(20.dp))

                // Authentication Info
                Text(
                    "AUTHENTICATION",
                    color = Color(0xFF64748B),
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace,
                    letterSpacing = 1.sp
                )
                Spacer(Modifier.height(8.dp))

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(8.dp))
                        .background(Color(0xFF141820))
                        .padding(horizontal = 14.dp, vertical = 10.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(
                        imageVector = Icons.Default.Fingerprint,
                        contentDescription = "Biometric",
                        tint = Color(0xFF94A3B8),
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(Modifier.width(10.dp))
                    Column {
                        Text(
                            "Biometric + Device Credential",
                            color = Color(0xFFE2E8F0),
                            fontSize = 12.sp,
                            fontFamily = FontFamily.Monospace,
                            fontWeight = FontWeight.Medium
                        )
                        Text(
                            "Fingerprint, Face, Device PIN / Pattern / Password",
                            color = Color(0xFF64748B),
                            fontSize = 10.sp,
                            fontFamily = FontFamily.Monospace
                        )
                    }
                }

                Spacer(Modifier.height(20.dp))

                // Lock Behavior (Timeout)
                Text(
                    "LOCK BEHAVIOR",
                    color = Color(0xFF64748B),
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace,
                    letterSpacing = 1.sp
                )
                Spacer(Modifier.height(8.dp))

                val timeouts = listOf(
                    AppLockManager.TIMEOUT_IMMEDIATELY to "Immediately",
                    AppLockManager.TIMEOUT_30_SECONDS to "After 30 seconds",
                    AppLockManager.TIMEOUT_1_MINUTE to "After 1 minute",
                    AppLockManager.TIMEOUT_5_MINUTES to "After 5 minutes"
                )

                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(8.dp))
                        .background(Color(0xFF141820))
                ) {
                    timeouts.forEachIndexed { index, (time, label) ->
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .clickable {
                                    if (activity != null && time != timeoutMs) {
                                        appLockManager.updateSettings(
                                            activity = activity,
                                            timeoutMs = time,
                                            onBackground = lockOnBg,
                                            onScreenOff = lockOnScreenOff,
                                            onSuccess = {
                                                statusMessage = "Lock timeout set to $label."
                                                isError = false
                                            },
                                            onError = { err ->
                                                statusMessage = err
                                                isError = true
                                            }
                                        )
                                    }
                                }
                                .padding(horizontal = 14.dp, vertical = 10.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            RadioButton(
                                selected = (timeoutMs == time),
                                onClick = null,
                                colors = RadioButtonDefaults.colors(
                                    selectedColor = Color(0xFFF1F5F9),
                                    unselectedColor = Color(0xFF475569)
                                ),
                                modifier = Modifier.size(18.dp)
                            )
                            Spacer(Modifier.width(12.dp))
                            Text(
                                text = label,
                                color = if (timeoutMs == time) Color(0xFFF8FAFC) else Color(0xFF94A3B8),
                                fontSize = 12.sp,
                                fontFamily = FontFamily.Monospace
                            )
                        }
                        if (index < timeouts.size - 1) {
                            HorizontalDivider(color = Color(0xFF1E293B), thickness = 0.5.dp)
                        }
                    }
                }

                Spacer(Modifier.height(20.dp))

                // Lock Triggers
                Text(
                    "LOCK TRIGGERS",
                    color = Color(0xFF64748B),
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace,
                    letterSpacing = 1.sp
                )
                Spacer(Modifier.height(8.dp))

                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(8.dp))
                        .background(Color(0xFF141820))
                ) {
                    // Trigger 1: Background
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable {
                                if (activity != null) {
                                    val newBg = !lockOnBg
                                    appLockManager.updateSettings(
                                        activity = activity,
                                        timeoutMs = timeoutMs,
                                        onBackground = newBg,
                                        onScreenOff = lockOnScreenOff,
                                        onSuccess = {},
                                        onError = { err -> statusMessage = err; isError = true }
                                    )
                                }
                            }
                            .padding(horizontal = 14.dp, vertical = 10.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Text(
                            "Lock when app leaves foreground",
                            color = Color(0xFFCBD5E1),
                            fontSize = 12.sp,
                            fontFamily = FontFamily.Monospace
                        )
                        Checkbox(
                            checked = lockOnBg,
                            onCheckedChange = null,
                            colors = CheckboxDefaults.colors(
                                checkedColor = Color(0xFFF1F5F9),
                                checkmarkColor = Color.Black,
                                uncheckedColor = Color(0xFF475569)
                            ),
                            modifier = Modifier.size(18.dp)
                        )
                    }

                    HorizontalDivider(color = Color(0xFF1E293B), thickness = 0.5.dp)

                    // Trigger 2: Screen off
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable {
                                if (activity != null) {
                                    val newOff = !lockOnScreenOff
                                    appLockManager.updateSettings(
                                        activity = activity,
                                        timeoutMs = timeoutMs,
                                        onBackground = lockOnBg,
                                        onScreenOff = newOff,
                                        onSuccess = {},
                                        onError = { err -> statusMessage = err; isError = true }
                                    )
                                }
                            }
                            .padding(horizontal = 14.dp, vertical = 10.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Text(
                            "Lock when screen turns off",
                            color = Color(0xFFCBD5E1),
                            fontSize = 12.sp,
                            fontFamily = FontFamily.Monospace
                        )
                        Checkbox(
                            checked = lockOnScreenOff,
                            onCheckedChange = null,
                            colors = CheckboxDefaults.colors(
                                checkedColor = Color(0xFFF1F5F9),
                                checkmarkColor = Color.Black,
                                uncheckedColor = Color(0xFF475569)
                            ),
                            modifier = Modifier.size(18.dp)
                        )
                    }
                }
            }
        }
    }
}
