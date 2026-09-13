@file:OptIn(androidx.compose.foundation.ExperimentalFoundationApi::class)

package com.meridian.shell.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.gestures.detectHorizontalDragGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Lock
import androidx.compose.material.icons.filled.LockOpen
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.meridian.shell.TerminalSession
import kotlinx.coroutines.launch


/**
 * TRUX gesture-only session drawer:
 * - Slides in ONLY via a left-edge swipe (no hamburger button per spec).
 * - Slides out when swiped left.
 * - Tapping scrim or pressing Back closes the drawer.
 * - Drawer occupies ~80% of screen width.
 */
@Composable
fun GestureSessionDrawer(
    sessions: List<TerminalSession>,
    activeSession: TerminalSession?,
    onSelect: (TerminalSession) -> Unit,
    onNewSession: () -> Unit,
    onLongPress: (TerminalSession) -> Unit,
    onCloseSession: (TerminalSession) -> Unit,
    isAppLockEnabled: Boolean = false,
    onOpenAppLock: () -> Unit = {},
    content: @Composable (openDrawer: () -> Unit) -> Unit
) {
    val drawerState = androidx.compose.material3.rememberDrawerState(initialValue = androidx.compose.material3.DrawerValue.Closed)
    val scope = rememberCoroutineScope()

    // Back handler: closes drawer when open (Section 30)
    androidx.activity.compose.BackHandler(enabled = drawerState.isOpen) {
        scope.launch { drawerState.close() }
    }

    BoxWithConstraints(Modifier.fillMaxSize()) {
        val drawerWidthDp = minOf(maxWidth * 0.80f, 360.dp)

        androidx.compose.material3.ModalNavigationDrawer(
            drawerState = drawerState,
            gesturesEnabled = true, // Enables native left-edge swipe to open, drag-left to close, tap scrim to close
            drawerContent = {
                androidx.compose.material3.ModalDrawerSheet(
                    drawerContainerColor = Color(0xFF101318),
                    drawerContentColor = Color(0xFFF1F5F9),
                    modifier = Modifier
                        .width(drawerWidthDp)
                        .fillMaxHeight()
                ) {
                    DrawerContent(
                        sessions = sessions,
                        active = activeSession,
                        onSelect = {
                            onSelect(it)
                            scope.launch { drawerState.close() }
                        },
                        onNewSession = {
                            onNewSession()
                            scope.launch { drawerState.close() }
                        },
                        onLongPress = onLongPress,
                        onCloseSession = onCloseSession,
                        isAppLockEnabled = isAppLockEnabled,
                        onOpenAppLock = {
                            onOpenAppLock()
                            scope.launch { drawerState.close() }
                        }
                    )
                }
            }
        ) {
            content {
                scope.launch {
                    if (drawerState.isClosed) drawerState.open() else drawerState.close()
                }
            }
        }
    }
}

@Composable
private fun DrawerContent(
    sessions: List<TerminalSession>,
    active: TerminalSession?,
    onSelect: (TerminalSession) -> Unit,
    onNewSession: () -> Unit,
    onLongPress: (TerminalSession) -> Unit,
    onCloseSession: (TerminalSession) -> Unit,
    isAppLockEnabled: Boolean,
    onOpenAppLock: () -> Unit
) {
    Column(
        Modifier
            .fillMaxSize()
            .statusBarsPadding()
            .navigationBarsPadding()
            .padding(vertical = 12.dp)
    ) {
        // App Header
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                androidx.compose.foundation.Image(
                    painter = androidx.compose.ui.res.painterResource(id = com.meridian.shell.R.drawable.trux_logo),
                    contentDescription = "TRUX",
                    modifier = Modifier
                        .size(28.dp)
                        .clip(RoundedCornerShape(6.dp))
                )
                Spacer(Modifier.width(10.dp))
                Column {
                    Text(
                        "TRUX",
                        color = Color(0xFFF1F5F9),
                        fontSize = 15.sp,
                        fontFamily = FontFamily.Monospace,
                        fontWeight = FontWeight.Bold,
                        letterSpacing = 2.sp
                    )
                    Text(
                        "BUILD > EXPLORE > BEYOND",
                        color = Color(0xFF64748B),
                        fontSize = 8.sp,
                        fontFamily = FontFamily.Monospace,
                        letterSpacing = 1.sp
                    )
                }
            }
            Text(
                "v2.0.1",
                color = Color(0xFF64748B),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace
            )
        }

        // New Session Button
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 8.dp)
                .clip(RoundedCornerShape(10.dp))
                .background(Color(0xFF1E293B))
                .clickable { onNewSession() }
                .padding(horizontal = 14.dp, vertical = 10.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Center
        ) {
            Icon(
                imageVector = Icons.Default.Add,
                contentDescription = "New Session",
                tint = Color(0xFF38BDF8),
                modifier = Modifier.size(18.dp)
            )
            Spacer(Modifier.width(8.dp))
            Text(
                "New Session",
                color = Color(0xFFF1F5F9),
                fontSize = 13.sp,
                fontWeight = FontWeight.SemiBold,
                fontFamily = FontFamily.Monospace
            )
        }

        Spacer(Modifier.height(8.dp))

        // Session Count Label
        Text(
            "ACTIVE SESSIONS (${sessions.size})",
            color = Color(0xFF64748B),
            fontSize = 11.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.SemiBold,
            modifier = Modifier.padding(horizontal = 18.dp, vertical = 6.dp)
        )

        // Session List
        LazyColumn(
            Modifier
                .weight(1f)
                .fillMaxWidth()
                .padding(horizontal = 8.dp)
        ) {
            items(sessions, key = { it.id }) { session ->
                SessionRow(
                    session = session,
                    isActive = session === active,
                    onClick = { onSelect(session) },
                    onLongClick = { onLongPress(session) },
                    onClose = { onCloseSession(session) }
                )
            }
        }

        // App Lock entry (near bottom above settings/footer per specification)
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 14.dp, vertical = 4.dp)
                .clip(RoundedCornerShape(8.dp))
                .background(Color(0xFF141820))
                .clickable { onOpenAppLock() }
                .padding(horizontal = 14.dp, vertical = 10.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                imageVector = if (isAppLockEnabled) Icons.Default.Lock else Icons.Default.LockOpen,
                contentDescription = "App Lock",
                tint = if (isAppLockEnabled) Color(0xFFF1F5F9) else Color(0xFF94A3B8),
                modifier = Modifier.size(20.dp)
            )
            Spacer(Modifier.width(12.dp))
            Column(Modifier.weight(1f)) {
                Text(
                    text = "App Lock",
                    color = Color(0xFFF1F5F9),
                    fontSize = 13.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    text = if (isAppLockEnabled) "On" else "Off",
                    color = if (isAppLockEnabled) Color(0xFF22C55E) else Color(0xFF64748B),
                    fontSize = 10.sp,
                    fontFamily = FontFamily.Monospace
                )
            }
            Text(
                text = "⚙",
                color = Color(0xFF64748B),
                fontSize = 14.sp
            )
        }

        // Welcome screen on new session setting
        val context = androidx.compose.ui.platform.LocalContext.current
        var welcomeEnabled by remember { mutableStateOf(com.meridian.shell.welcome.WelcomeBanner.isWelcomeEnabled(context)) }

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 14.dp, vertical = 4.dp)
                .clip(RoundedCornerShape(8.dp))
                .background(Color(0xFF141820))
                .padding(horizontal = 14.dp, vertical = 6.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(Modifier.weight(1f)) {
                Text(
                    text = "Welcome Screen",
                    color = Color(0xFFF1F5F9),
                    fontSize = 13.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    text = "Show on new session",
                    color = Color(0xFF64748B),
                    fontSize = 10.sp,
                    fontFamily = FontFamily.Monospace
                )
            }
            androidx.compose.material3.Switch(
                checked = welcomeEnabled,
                onCheckedChange = {
                    welcomeEnabled = it
                    com.meridian.shell.welcome.WelcomeBanner.setWelcomeEnabled(context, it)
                },
                colors = androidx.compose.material3.SwitchDefaults.colors(
                    checkedThumbColor = Color.White,
                    checkedTrackColor = Color(0xFF475569),
                    uncheckedThumbColor = Color(0xFF94A3B8),
                    uncheckedTrackColor = Color(0xFF1E293B)
                )
            )
        }

        // Quick tip & Footer at bottom of sidebar
        Column(
            Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 8.dp)
        ) {
            Text(
                "Long press a session for options.\nType 'exit' to stop & remove.",
                color = Color(0xFF475569),
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
                lineHeight = 14.sp
            )

            Spacer(Modifier.height(10.dp))
            androidx.compose.material3.Divider(
                color = Color(0xFF1E293B),
                thickness = 1.dp
            )
            Spacer(Modifier.height(10.dp))

            Text(
                text = "Developed by Charan Balaji",
                color = Color(0xFF94A3B8),
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.SemiBold
            )
            val currentYear = java.util.Calendar.getInstance().get(java.util.Calendar.YEAR)
            Text(
                text = "Trux © $currentYear • All rights reserved",
                color = Color(0xFF475569),
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier.padding(top = 2.dp)
            )
        }
    }
}

@Composable
private fun SessionRow(
    session: TerminalSession,
    isActive: Boolean,
    onClick: () -> Unit,
    onLongClick: () -> Unit,
    onClose: () -> Unit
) {
    Row(
        Modifier
            .fillMaxWidth()
            .padding(vertical = 3.dp)
            .clip(RoundedCornerShape(8.dp))
            .background(if (isActive) Color(0xFF1E293B) else Color(0xFF141820))
            .combinedClickableCompat(onClick, onLongClick)
            .padding(start = 12.dp, top = 8.dp, bottom = 8.dp, end = 4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Status indicator
        Box(
            modifier = Modifier
                .size(8.dp)
                .clip(CircleShape)
                .background(
                    when {
                        session.exited -> Color(0xFFEF4444)
                        isActive -> Color(0xFF22C55E)
                        else -> Color(0xFF64748B)
                    }
                )
        )

        Spacer(Modifier.width(10.dp))

        Column(Modifier.weight(1f)) {
            Text(
                text = session.name,
                color = if (isActive) Color(0xFFF8FAFC) else Color(0xFFCBD5E1),
                fontSize = 13.sp,
                fontWeight = if (isActive) FontWeight.Bold else FontWeight.Normal,
                fontFamily = FontFamily.Monospace
            )
            val subtitle = if (session.exited) "[stopped]"
                           else session.shell.substringAfterLast('/')
            Text(
                text = subtitle,
                color = Color(0xFF64748B),
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace
            )
        }

        // Direct Close Button
        IconButton(
            onClick = onClose,
            modifier = Modifier.size(32.dp)
        ) {
            Icon(
                imageVector = Icons.Default.Close,
                contentDescription = "Close Session",
                tint = Color(0xFF94A3B8),
                modifier = Modifier.size(16.dp)
            )
        }
    }
}

private fun Modifier.combinedClickableCompat(
    onClick: () -> Unit,
    onLongClick: () -> Unit
): Modifier = this.combinedClickable(
    onClick = onClick,
    onLongClick = onLongClick
)
