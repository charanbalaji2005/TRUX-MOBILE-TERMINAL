package com.meridian.shell

import android.app.*
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.os.PowerManager
import androidx.core.app.NotificationCompat

/**
 * Foreground service that keeps the Trux process alive while terminal
 * sessions are running, manages the wake lock, and displays the persistent
 * notification matching Termux style with Exit and WakeLock actions.
 */
class MeridianService : Service() {

    companion object {
        private const val CHANNEL_ID = "trux_sessions"
        private const val NOTIFICATION_ID = 1001

        const val ACTION_PKG = "com.meridian.shell.action.PKG"
        const val EXTRA_PKG_ARGS = "pkg_args"

        const val ACTION_EXIT = "com.meridian.shell.action.EXIT"
        const val ACTION_ACQUIRE_WAKELOCK = "com.meridian.shell.action.ACQUIRE_WAKELOCK"
        const val ACTION_RELEASE_WAKELOCK = "com.meridian.shell.action.RELEASE_WAKELOCK"
        const val ACTION_TOGGLE_WAKELOCK = "com.meridian.shell.action.TOGGLE_WAKELOCK"
        const val ACTION_UPDATE = "com.meridian.shell.action.UPDATE"

        @Volatile
        private var wakeLock: PowerManager.WakeLock? = null

        fun start(context: Context) {
            val intent = Intent(context, MeridianService::class.java)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                context.startForegroundService(intent)
            } else {
                context.startService(intent)
            }
        }

        fun stop(context: Context) {
            context.stopService(Intent(context, MeridianService::class.java))
        }

        fun isWakeLockHeld(): Boolean = wakeLock?.isHeld == true

        @Synchronized
        fun acquireWakeLock(context: Context): Boolean {
            if (wakeLock?.isHeld == true) return true
            val pm = context.applicationContext.getSystemService(Context.POWER_SERVICE) as? PowerManager ?: return false
            wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Trux:WakeLock").apply {
                setReferenceCounted(false)
                acquire()
            }
            return true
        }

        @Synchronized
        fun releaseWakeLock(): Boolean {
            wakeLock?.let {
                if (it.isHeld) {
                    it.release()
                }
            }
            wakeLock = null
            return true
        }

        fun update(context: Context) {
            val intent = Intent(context, MeridianService::class.java).apply {
                action = ACTION_UPDATE
            }
            runCatching {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    context.startForegroundService(intent)
                } else {
                    context.startService(intent)
                }
            }
        }
    }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        startForeground(NOTIFICATION_ID, buildNotification(0))
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        when (intent?.action) {
            ACTION_PKG -> handlePkgCommand(intent)
            ACTION_ACQUIRE_WAKELOCK -> {
                acquireWakeLock(this)
                updateNotification()
            }
            ACTION_RELEASE_WAKELOCK -> {
                releaseWakeLock()
                updateNotification()
            }
            ACTION_TOGGLE_WAKELOCK -> {
                if (isWakeLockHeld()) releaseWakeLock() else acquireWakeLock(this)
                updateNotification()
            }
            ACTION_EXIT -> {
                handleExit()
                return START_NOT_STICKY
            }
            else -> updateNotification()
        }
        return START_STICKY
    }

    private fun handleExit() {
        releaseWakeLock()
        runCatching {
            SessionManager.get(applicationContext).closeAll()
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            stopForeground(STOP_FOREGROUND_REMOVE)
        } else {
            @Suppress("DEPRECATION")
            stopForeground(true)
        }
        stopSelf()

        sendBroadcast(Intent("com.meridian.shell.action.FINISH_ACTIVITY").setPackage(packageName))

        android.os.Handler(android.os.Looper.getMainLooper()).postDelayed({
            android.os.Process.killProcess(android.os.Process.myPid())
        }, 200)
    }

    private fun handlePkgCommand(intent: Intent) {
        val args = intent.getStringArrayListExtra(EXTRA_PKG_ARGS) ?: return
    }

    fun updateNotification() {
        val count = runCatching {
            SessionManager.get(applicationContext).sessions.size
        }.getOrDefault(0)

        val nm = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        nm.notify(NOTIFICATION_ID, buildNotification(count))
    }

    private fun buildNotification(sessionCount: Int): Notification {
        val tapIntent = PendingIntent.getActivity(
            this, 0,
            Intent(this, MainActivity::class.java).apply {
                flags = Intent.FLAG_ACTIVITY_SINGLE_TOP or Intent.FLAG_ACTIVITY_CLEAR_TOP
            },
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val exitIntent = PendingIntent.getService(
            this, 1,
            Intent(this, MeridianService::class.java).apply {
                action = ACTION_EXIT
            },
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val wakeLockHeld = isWakeLockHeld()
        val wakeLockIntent = PendingIntent.getService(
            this, 2,
            Intent(this, MeridianService::class.java).apply {
                action = if (wakeLockHeld) ACTION_RELEASE_WAKELOCK else ACTION_ACQUIRE_WAKELOCK
            },
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val sessionText = when (sessionCount) {
            0 -> "Starting…"
            1 -> "1 session"
            else -> "$sessionCount sessions"
        }
        val body = if (wakeLockHeld && sessionCount > 0) "$sessionText (wake lock held)" else if (wakeLockHeld) "$sessionText (wake lock held)" else sessionText
        val wakeLockActionTitle = if (wakeLockHeld) "Release wakelock" else "Acquire wakelock"

        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Trux")
            .setContentText(body)
            .setSmallIcon(R.drawable.ic_notification)
            .setContentIntent(tapIntent)
            .setOngoing(true)
            .setSilent(true)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .addAction(0, "Exit", exitIntent)
            .addAction(0, wakeLockActionTitle, wakeLockIntent)
            .build()
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                "Trux Sessions",
                NotificationManager.IMPORTANCE_LOW
            ).apply {
                description = "Keeps Trux terminal sessions active in the background"
                setShowBadge(false)
            }
            (getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager)
                .createNotificationChannel(channel)
        }
    }
}
