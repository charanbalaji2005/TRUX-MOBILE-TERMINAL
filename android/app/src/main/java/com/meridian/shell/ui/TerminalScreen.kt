package com.meridian.shell.ui

import android.app.Activity
import androidx.activity.compose.BackHandler
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import kotlinx.coroutines.launch
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import com.meridian.shell.MeridianViewModel
import com.meridian.shell.TerminalSession
import com.meridian.shell.TerminalView

@Composable
fun TerminalScreen(
    viewModel: MeridianViewModel,
    appLockManager: com.meridian.shell.security.AppLockManager? = null
) {
    val context = LocalContext.current
    val activity = context as? Activity
    val scope = rememberCoroutineScope()
    val sessions = viewModel.sessions
    val active = viewModel.activeSession
    var terminalView by remember { mutableStateOf<TerminalView?>(null) }
    var ctrl by remember { mutableStateOf(false) }
    var alt by remember { mutableStateOf(false) }
    var showNewSessionSheet by remember { mutableStateOf(false) }
    var showAppLockSheet by remember { mutableStateOf(false) }
    var actionTarget by remember { mutableStateOf<TerminalSession?>(null) }

    GestureSessionDrawer(
        sessions = sessions,
        activeSession = active,
        onSelect = { viewModel.switchTo(it) },
        onNewSession = { showNewSessionSheet = true },
        onLongPress = { actionTarget = it },
        onCloseSession = { viewModel.close(it) },
        isAppLockEnabled = appLockManager?.isEnabled ?: false,
        onOpenAppLock = { showAppLockSheet = true }
    ) { openDrawer ->
        Column(
            Modifier
                .fillMaxSize()
                .background(Color.Black)
        ) {
            CommandCenterTopBar(
                session = active,
                sessionIndex = sessions.indexOf(active) + 1,
                totalSessions = sessions.size,
                onOpenDrawer = openDrawer,
                onNewSession = { showNewSessionSheet = true },
                onToggleKeyboard = { terminalView?.toggleKeyboard() },
                onClear = { active?.write("\u001B[2J\u001B[H".toByteArray()) },
                onZoomIn = { terminalView?.zoomIn() },
                onZoomOut = { terminalView?.zoomOut() },
                onResetZoom = { terminalView?.resetZoom() },
                onOpenStorage = { active?.write("cd ~/storage && ls -la\n".toByteArray()) },
                onManageSession = { actionTarget = active },
                onExportPdf = {
                    active?.let { target ->
                        scope.launch {
                            val file = com.meridian.shell.export.SessionPdfExporter.exportSessionToPdf(context, target)
                            file?.let { com.meridian.shell.export.SessionPdfExporter.openOrSharePdf(context, it) }
                        }
                    }
                },
                onExitApp = { activity?.finishAffinity() }
            )

            Box(
                Modifier
                    .weight(1f)
                    .fillMaxWidth()
                    .padding(bottom = 6.dp)
            ) {
                AndroidView(
                    factory = { ctx ->
                        TerminalView(ctx).also { view ->
                            terminalView = view
                            view.onCtrlConsumed = { ctrl = false }
                            view.onAltConsumed = { alt = false }
                            view.onGeometryChanged = { cols, rows ->
                                viewModel.manager.activeSession?.let {
                                    viewModel.manager.updateSessionSize(it, cols, rows)
                                }
                            }
                            view.onSessionExited = { exited ->
                                viewModel.onSessionExited(exited)
                            }
                        }
                    },
                    update = { view ->
                        if (view.session !== active) view.session = active
                        view.ctrlLatched = ctrl
                        view.altLatched = alt
                        view.onCtrlConsumed = { ctrl = false }
                        view.onAltConsumed = { alt = false }
                        view.onSessionExited = { exited ->
                            viewModel.onSessionExited(exited)
                        }
                    },
                    modifier = Modifier.fillMaxSize()
                )
            }

            ExtraKeysRow(
                ctrlLatched = ctrl,
                altLatched = alt,
                onKey = { key ->
                    when (key) {
                        ExtraKey.CTRL -> ctrl = !ctrl
                        ExtraKey.ALT -> alt = !alt
                        ExtraKey.PGUP -> {
                            if (ctrl || alt) {
                                terminalView?.pageUp()
                            } else {
                                active?.write(escapeFor(key, ctrl, alt))
                            }
                            ctrl = false
                            alt = false
                        }
                        ExtraKey.PGDN -> {
                            if (ctrl || alt) {
                                terminalView?.pageDown()
                            } else {
                                active?.write(escapeFor(key, ctrl, alt))
                            }
                            ctrl = false
                            alt = false
                        }
                        else -> {
                            active?.write(escapeFor(key, ctrl, alt))
                            ctrl = false
                            alt = false
                        }
                    }
                },
                modifier = Modifier.navigationBarsPadding().imePadding()
            )
        }
    }

    // Back handler — closes action sheet, app lock sheet, or new session sheet, never sessions
    BackHandler(enabled = actionTarget != null || showNewSessionSheet || showAppLockSheet) {
        actionTarget = null
        showNewSessionSheet = false
        showAppLockSheet = false
    }

    // New session creation sheet
    if (showNewSessionSheet) {
        NewSessionSheet(
            sessionIndex = sessions.size + 1,
            onCreateSession = { name, shell, env ->
                viewModel.createSession(name = name, shell = shell, environmentName = env)
            },
            onDismiss = { showNewSessionSheet = false }
        )
    }

    // App Lock configuration bottom sheet
    if (showAppLockSheet && appLockManager != null) {
        AppLockSettingsSheet(
            appLockManager = appLockManager,
            onDismiss = { showAppLockSheet = false }
        )
    }

    // Session actions bottom sheet (rename, restart, close, export PDF)
    actionTarget?.let { target ->
        SessionActionsSheet(
            session = target,
            onRename = { viewModel.rename(target, it); actionTarget = null },
            onRestart = { viewModel.restart(target); actionTarget = null },
            onClose = { viewModel.close(target); actionTarget = null },
            onExportPdf = {
                scope.launch {
                    val file = com.meridian.shell.export.SessionPdfExporter.exportSessionToPdf(context, target)
                    file?.let { com.meridian.shell.export.SessionPdfExporter.openOrSharePdf(context, it) }
                }
                actionTarget = null
            },
            onDismiss = { actionTarget = null }
        )
    }
}

private fun escapeFor(key: ExtraKey, ctrl: Boolean, alt: Boolean): ByteArray {
    val esc = byteArrayOf(0x1B)
    val base = when (key) {
        ExtraKey.ESC   -> esc
        ExtraKey.SLASH -> byteArrayOf('/'.code.toByte())
        ExtraKey.DASH  -> byteArrayOf('-'.code.toByte())
        ExtraKey.TAB   -> byteArrayOf(0x09)
        ExtraKey.HOME  -> if (ctrl) byteArrayOf(0x01) else "\u001B[H".toByteArray()
        ExtraKey.END   -> if (ctrl) byteArrayOf(0x05) else "\u001B[F".toByteArray()
        ExtraKey.PGUP  -> "\u001B[5~".toByteArray()
        ExtraKey.PGDN  -> "\u001B[6~".toByteArray()
        ExtraKey.UP    -> if (ctrl) "\u001B[1;5A".toByteArray() else "\u001B[A".toByteArray()
        ExtraKey.DOWN  -> if (ctrl) "\u001B[1;5B".toByteArray() else "\u001B[B".toByteArray()
        ExtraKey.LEFT  -> if (ctrl) "\u001B[1;5D".toByteArray() else "\u001B[D".toByteArray()
        ExtraKey.RIGHT -> if (ctrl) "\u001B[1;5C".toByteArray() else "\u001B[C".toByteArray()
        else           -> ByteArray(0)
    }
    return if (alt && base.isNotEmpty() && base[0] != 0x1B.toByte()) esc + base else base
}

@Composable
private fun CommandCenterTopBar(
    session: TerminalSession?,
    sessionIndex: Int,
    totalSessions: Int,
    onOpenDrawer: () -> Unit,
    onNewSession: () -> Unit,
    onToggleKeyboard: () -> Unit,
    onClear: () -> Unit,
    onZoomIn: () -> Unit,
    onZoomOut: () -> Unit,
    onResetZoom: () -> Unit,
    onOpenStorage: () -> Unit,
    onManageSession: () -> Unit,
    onExportPdf: () -> Unit,
    onExitApp: () -> Unit
) {
    var menuExpanded by remember { mutableStateOf(false) }

    Box(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF161A22))
            .statusBarsPadding()
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .height(48.dp)
                .padding(horizontal = 4.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            // Left: Clean Session Indicator Pill (No hamburger button per spec Section 22/23)
            Row(
                modifier = Modifier
                    .padding(start = 12.dp)
                    .clip(RoundedCornerShape(8.dp))
                    .background(Color(0xFF262C38))
                    .padding(horizontal = 10.dp, vertical = 6.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                val isExited = session?.exited == true
                Box(
                    modifier = Modifier
                        .size(8.dp)
                        .clip(CircleShape)
                        .background(if (isExited) Color(0xFFEF4444) else Color(0xFF22C55E))
                )
                Spacer(Modifier.width(8.dp))
                Text(
                    text = session?.name ?: "SESSION $sessionIndex",
                    color = Color(0xFFF1F5F9),
                    fontSize = 12.sp,
                    fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace
                )
                Spacer(Modifier.width(6.dp))
                Text(
                    text = "[$totalSessions]",
                    color = Color(0xFF94A3B8),
                    fontSize = 10.sp,
                    fontFamily = FontFamily.Monospace
                )
            }

            // Right: Quick Action Buttons (Clear, Keyboard, New Session, 3-dots Overflow Menu)
            Row(verticalAlignment = Alignment.CenterVertically) {
                IconButton(
                    onClick = onClear,
                    modifier = Modifier.size(36.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.Backspace,
                        contentDescription = "Clear Screen",
                        tint = Color(0xFFCBD5E1),
                        modifier = Modifier.size(18.dp)
                    )
                }

                IconButton(
                    onClick = onToggleKeyboard,
                    modifier = Modifier.size(36.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.Keyboard,
                        contentDescription = "Toggle Keyboard",
                        tint = Color(0xFF38BDF8),
                        modifier = Modifier.size(19.dp)
                    )
                }

                IconButton(
                    onClick = onNewSession,
                    modifier = Modifier.size(36.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.Add,
                        contentDescription = "New Session",
                        tint = Color(0xFF4ADE80),
                        modifier = Modifier.size(22.dp)
                    )
                }

                // 3-dots Overflow Menu Button (Top Right)
                Box {
                    IconButton(
                        onClick = { menuExpanded = true },
                        modifier = Modifier.size(36.dp)
                    ) {
                        Icon(
                            imageVector = Icons.Default.MoreVert,
                            contentDescription = "More Options",
                            tint = Color(0xFFCBD5E1),
                            modifier = Modifier.size(20.dp)
                        )
                    }

                    DropdownMenu(
                        expanded = menuExpanded,
                        onDismissRequest = { menuExpanded = false },
                        modifier = Modifier.background(Color(0xFF1B202B))
                    ) {
                        DropdownMenuItem(
                            text = { Text("New Session", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.Add, contentDescription = null, tint = Color(0xFF4ADE80)) },
                            onClick = {
                                menuExpanded = false
                                onNewSession()
                            }
                        )
                        DropdownMenuItem(
                            text = { Text("Open Storage (~/storage)", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.Folder, contentDescription = null, tint = Color(0xFF38BDF8)) },
                            onClick = {
                                menuExpanded = false
                                onOpenStorage()
                            }
                        )
                        DropdownMenuItem(
                            text = { Text("Zoom In (+)", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.ZoomIn, contentDescription = null, tint = Color(0xFFFACC15)) },
                            onClick = {
                                menuExpanded = false
                                onZoomIn()
                            }
                        )
                        DropdownMenuItem(
                            text = { Text("Zoom Out (-)", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.ZoomOut, contentDescription = null, tint = Color(0xFFFACC15)) },
                            onClick = {
                                menuExpanded = false
                                onZoomOut()
                            }
                        )
                        DropdownMenuItem(
                            text = { Text("Reset Zoom", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.RestartAlt, contentDescription = null, tint = Color(0xFF94A3B8)) },
                            onClick = {
                                menuExpanded = false
                                onResetZoom()
                            }
                        )
                        DropdownMenuItem(
                            text = { Text("Clear Screen", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.Backspace, contentDescription = null, tint = Color(0xFFFB7185)) },
                            onClick = {
                                menuExpanded = false
                                onClear()
                            }
                        )
                        DropdownMenuItem(
                            text = { Text("Session Settings", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.Settings, contentDescription = null, tint = Color(0xFFA78BFA)) },
                            onClick = {
                                menuExpanded = false
                                onManageSession()
                            }
                        )
                        DropdownMenuItem(
                            text = { Text("Export Session (PDF)", color = Color(0xFFF1F5F9), fontSize = 14.sp) },
                            leadingIcon = { Icon(Icons.Default.Share, contentDescription = null, tint = Color(0xFF38BDF8)) },
                            onClick = {
                                menuExpanded = false
                                onExportPdf()
                            }
                        )
                        HorizontalDivider(color = Color(0xFF333D52))
                        DropdownMenuItem(
                            text = { Text("Exit App", color = Color(0xFFEF4444), fontSize = 14.sp, fontWeight = FontWeight.Bold) },
                            leadingIcon = { Icon(Icons.Default.ExitToApp, contentDescription = null, tint = Color(0xFFEF4444)) },
                            onClick = {
                                menuExpanded = false
                                onExitApp()
                            }
                        )
                    }
                }
            }
        }
    }
}
