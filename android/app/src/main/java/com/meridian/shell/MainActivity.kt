package com.meridian.shell

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.Settings
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.compose.foundation.Image
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.ui.draw.clip
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import androidx.lifecycle.lifecycleScope
import android.view.WindowManager
import androidx.fragment.app.FragmentActivity
import com.meridian.shell.security.AppLockManager
import com.meridian.shell.ui.AppLockScreen
import com.meridian.shell.ui.TerminalScreen
import com.meridian.shell.viewer.FileViewerManager
import com.meridian.shell.viewer.UniversalFileViewerDialog

class MainActivity : FragmentActivity() {

    // ViewModel survives rotation; PTYs are never recreated due to orientation changes.
    private val viewModel: MeridianViewModel by viewModels()

    lateinit var appLockManager: AppLockManager
        private set

    val fileViewerManager = FileViewerManager()

    private val finishReceiver = object : android.content.BroadcastReceiver() {
        override fun onReceive(context: android.content.Context?, intent: android.content.Intent?) {
            finishAffinity()
        }
    }

    private val openFileReceiver = object : android.content.BroadcastReceiver() {
        override fun onReceive(context: android.content.Context?, intent: android.content.Intent?) {
            handleOpenFileIntent(intent)
        }
    }

    private val exportPdfReceiver = object : android.content.BroadcastReceiver() {
        override fun onReceive(context: android.content.Context?, intent: android.content.Intent?) {
            val session = viewModel.activeSession ?: return
            val format = intent?.getStringExtra("format") ?: "pdf"
            this@MainActivity.lifecycleScope.launch {
                if (format.equals("txt", ignoreCase = true) || format.equals("text", ignoreCase = true)) {
                    val safeName = session.name.replace(Regex("[^a-zA-Z0-9_-]"), "_")
                    val fileName = "TRUX_Session_${safeName}_${System.currentTimeMillis()}.txt"
                    val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                    downloadsDir.mkdirs()
                    val outFile = java.io.File(downloadsDir, fileName)
                    outFile.writeText(session.visibleText())
                    val internalCopy = java.io.File(filesDir, "sessions/$fileName")
                    internalCopy.parentFile?.mkdirs()
                    runCatching { outFile.copyTo(internalCopy, overwrite = true) }
                    android.widget.Toast.makeText(this@MainActivity, "Session text saved to Downloads/$fileName", android.widget.Toast.LENGTH_LONG).show()
                    fileViewerManager.openFile(outFile.absolutePath)
                } else {
                    val file = com.meridian.shell.export.SessionPdfExporter.exportSessionToPdf(this@MainActivity, session)
                    file?.let {
                        fileViewerManager.openFile(it.absolutePath)
                    }
                }
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        appLockManager = AppLockManager(this)

        val finishFilter = android.content.IntentFilter("com.meridian.shell.action.FINISH_ACTIVITY")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(finishReceiver, finishFilter, android.content.Context.RECEIVER_NOT_EXPORTED)
        } else {
            registerReceiver(finishReceiver, finishFilter)
        }

        val openFilter = android.content.IntentFilter("com.meridian.shell.action.OPEN_FILE")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(openFileReceiver, openFilter, android.content.Context.RECEIVER_EXPORTED)
        } else {
            registerReceiver(openFileReceiver, openFilter)
        }

        val exportFilter = android.content.IntentFilter("com.meridian.shell.action.EXPORT_PDF")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(exportPdfReceiver, exportFilter, android.content.Context.RECEIVER_EXPORTED)
        } else {
            registerReceiver(exportPdfReceiver, exportFilter)
        }

        // Start the foreground service so sessions survive backgrounding
        MeridianService.start(this)

        // Ensure storage access for phone memory and ~/storage mounts
        requestStoragePermissionIfNeeded()
        requestNotificationPermissionIfNeeded()

        handleOpenFileIntent(intent)

        setContent {
            MaterialTheme(colorScheme = darkColorScheme(background = Color.Black)) {
                val fatal by rememberMeridianFatalError()
                var showSplash by remember { mutableStateOf(true) }
                val isLocked = appLockManager.isLocked

                if (isLocked) {
                    window.setFlags(WindowManager.LayoutParams.FLAG_SECURE, WindowManager.LayoutParams.FLAG_SECURE)
                    AppLockScreen(
                        onUnlockRequest = {
                            appLockManager.authenticate(
                                activity = this@MainActivity,
                                title = "TRUX App Lock",
                                subtitle = "Authenticate to continue",
                                onSuccess = {
                                    window.clearFlags(WindowManager.LayoutParams.FLAG_SECURE)
                                }
                            )
                        }
                    )
                } else {
                    Box(Modifier.fillMaxSize().background(Color.Black)) {
                        when {
                            showSplash -> SplashScreen(onTimeout = { showSplash = false })
                            fatal != null -> FatalMessage(fatal!!)
                            else -> TerminalScreen(
                                viewModel = viewModel,
                                appLockManager = appLockManager
                            )
                        }

                        // Universal File Viewer Dialog
                        val activeFile = fileViewerManager.activeFile
                        val activeType = fileViewerManager.activeFileType
                        if (activeFile != null && activeType != null) {
                            UniversalFileViewerDialog(
                                file = activeFile,
                                fileType = activeType,
                                fileViewerManager = fileViewerManager,
                                onDismiss = { fileViewerManager.close() }
                            )
                        }
                    }
                }
            }
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        setIntent(intent)
        handleOpenFileIntent(intent)
    }

    private fun handleOpenFileIntent(intent: Intent?) {
        val path = intent?.getStringExtra("path") ?: intent?.data?.path
        if (!path.isNullOrBlank()) {
            fileViewerManager.openFile(path)
        }
    }

    private fun requestStoragePermissionIfNeeded() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                runCatching {
                    val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION).apply {
                        data = Uri.parse("package:$packageName")
                    }
                    startActivity(intent)
                }.onFailure {
                    runCatching {
                        val intent = Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION)
                        startActivity(intent)
                    }
                }
            }
        } else {
            val perms = arrayOf(
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            )
            val needed = perms.filter {
                ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
            }
            if (needed.isNotEmpty()) {
                ActivityCompat.requestPermissions(this, needed.toTypedArray(), 1001)
            }
        }
    }

    private fun requestNotificationPermissionIfNeeded() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS)
                != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.POST_NOTIFICATIONS), 1002)
            }
        }
    }

    override fun onPause() {
        super.onPause()
        appLockManager.onActivityPaused(isChangingConfigurations)
        if (appLockManager.isEnabled) {
            window.setFlags(WindowManager.LayoutParams.FLAG_SECURE, WindowManager.LayoutParams.FLAG_SECURE)
        }
    }

    override fun onResume() {
        super.onResume()
        // Ensure service is running (may have been stopped by system)
        MeridianService.start(this)
        appLockManager.onActivityResumed(this)
        if (!appLockManager.shouldLock() && !appLockManager.isEnabled) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_SECURE)
        }
    }

    override fun onStop() {
        super.onStop()
        viewModel.manager.saveAllActiveSessions()
    }

    override fun onDestroy() {
        super.onDestroy()
        viewModel.manager.saveAllActiveSessions()
        runCatching { unregisterReceiver(finishReceiver) }
        runCatching { unregisterReceiver(openFileReceiver) }
        runCatching { unregisterReceiver(exportPdfReceiver) }
        appLockManager.unregisterScreenOffReceiver(this)
        if (isFinishing) {
            viewModel.cleanupStoppedSessions()
        }
    }
}

@Composable
private fun rememberMeridianFatalError(): State<String?> {
    // In a real implementation this would observe a SharedFlow<String> from the ViewModel.
    // For now it returns null (no fatal error) since session creation is async in ViewModel.init.
    return remember { mutableStateOf(null) }
}

@Composable
private fun FatalMessage(message: String) {
    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        Text(
            message,
            color = Color(0xFFE06C6C),
            fontSize = 13.sp,
            fontFamily = FontFamily.Monospace,
            modifier = Modifier.padding(32.dp)
        )
    }
}

@Composable
private fun SplashScreen(onTimeout: () -> Unit) {
    LaunchedEffect(Unit) {
        delay(2000L)
        onTimeout()
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color(0xFF000000)),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            // Metallic TRUX Emblem
            Image(
                painter = painterResource(id = R.drawable.trux_logo),
                contentDescription = "TRUX Logo",
                modifier = Modifier
                    .size(150.dp)
                    .clip(RoundedCornerShape(32.dp))
            )

            Spacer(Modifier.height(16.dp))

            Text(
                text = "TRUX",
                color = Color.White,
                fontSize = 28.sp,
                fontWeight = FontWeight.Black,
                fontFamily = FontFamily.Monospace,
                letterSpacing = 6.sp
            )

            Spacer(Modifier.height(6.dp))

            Text(
                text = "BUILD  >  EXPLORE  >  BEYOND",
                color = Color(0xFF94A3B8),
                fontSize = 11.sp,
                fontWeight = FontWeight.SemiBold,
                fontFamily = FontFamily.Monospace,
                letterSpacing = 2.sp
            )

            Spacer(Modifier.height(32.dp))

            CircularProgressIndicator(
                modifier = Modifier.size(24.dp),
                color = Color(0xFFE2E8F0),
                strokeWidth = 2.dp
            )
        }

        Text(
            text = "TRUX TERMINAL • V2.0.1",
            color = Color(0xFF475569),
            fontSize = 10.sp,
            fontFamily = FontFamily.Monospace,
            letterSpacing = 1.sp,
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .navigationBarsPadding()
                .padding(bottom = 24.dp)
        )
    }
}
