package com.meridian.shell.security

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.SharedPreferences
import android.os.SystemClock
import android.util.Log
import androidx.biometric.BiometricManager
import androidx.biometric.BiometricPrompt
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.core.content.ContextCompat
import androidx.fragment.app.FragmentActivity

/**
 * Manages TRUX App Lock state, lifecycle checks, and native Android biometric authentication.
 *
 * Supports:
 * - Fingerprint
 * - Face authentication
 * - Device PIN
 * - Device Pattern
 * - Device Password
 *
 * Never stores passwords, PINs, or biometric templates. Android Keystore and Keyguard manage credentials.
 */
class AppLockManager(context: Context) {

    companion object {
        private const val TAG = "AppLockManager"
        private const val PREFS_NAME = "trux_app_lock_prefs"
        private const val KEY_ENABLED = "app_lock_enabled"
        private const val KEY_TIMEOUT_MS = "lock_timeout_ms"
        private const val KEY_LOCK_ON_BG = "lock_on_background"
        private const val KEY_LOCK_ON_SCREEN_OFF = "lock_on_screen_off"

        const val TIMEOUT_IMMEDIATELY = 0L
        const val TIMEOUT_30_SECONDS = 30_000L
        const val TIMEOUT_1_MINUTE = 60_000L
        const val TIMEOUT_5_MINUTES = 300_000L
    }

    private val prefs: SharedPreferences =
        context.applicationContext.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    var isEnabled: Boolean by mutableStateOf(prefs.getBoolean(KEY_ENABLED, false))
        private set

    var lockTimeoutMs: Long by mutableStateOf(prefs.getLong(KEY_TIMEOUT_MS, TIMEOUT_IMMEDIATELY))
        private set

    var lockOnBackground: Boolean by mutableStateOf(prefs.getBoolean(KEY_LOCK_ON_BG, true))
        private set

    var lockOnScreenOff: Boolean by mutableStateOf(prefs.getBoolean(KEY_LOCK_ON_SCREEN_OFF, true))
        private set

    // Runtime lock state — if enabled, app starts locked until authenticated
    var isLocked: Boolean by mutableStateOf(prefs.getBoolean(KEY_ENABLED, false))
        private set

    // Prevents lifecycle onPause() during BiometricPrompt presentation from re-locking
    var isAuthenticating: Boolean by mutableStateOf(false)
        private set

    private var lastBackgroundTimestamp: Long = 0L
    private var screenOffReceiverRegistered = false

    private val screenOffReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            if (intent?.action == Intent.ACTION_SCREEN_OFF && isEnabled && lockOnScreenOff) {
                Log.d(TAG, "Screen off event received -> locking TRUX")
                lock()
            }
        }
    }

    init {
        registerScreenOffReceiver(context.applicationContext)
    }

    fun registerScreenOffReceiver(appContext: Context) {
        if (!screenOffReceiverRegistered) {
            try {
                val filter = IntentFilter(Intent.ACTION_SCREEN_OFF)
                appContext.registerReceiver(screenOffReceiver, filter)
                screenOffReceiverRegistered = true
            } catch (e: Exception) {
                Log.w(TAG, "Could not register screenOffReceiver", e)
            }
        }
    }

    fun unregisterScreenOffReceiver(appContext: Context) {
        if (screenOffReceiverRegistered) {
            try {
                appContext.unregisterReceiver(screenOffReceiver)
            } catch (e: Exception) {
                Log.w(TAG, "Could not unregister screenOffReceiver", e)
            }
            screenOffReceiverRegistered = false
        }
    }

    fun lock() {
        if (isEnabled) {
            isLocked = true
        }
    }

    fun unlock() {
        isLocked = false
        lastBackgroundTimestamp = 0L
    }

    fun shouldLock(): Boolean = isEnabled && isLocked

    fun onActivityPaused(isChangingConfigurations: Boolean) {
        if (!isEnabled || isChangingConfigurations || isAuthenticating) return
        lastBackgroundTimestamp = SystemClock.elapsedRealtime()
        if (lockOnBackground && lockTimeoutMs == TIMEOUT_IMMEDIATELY) {
            lock()
        }
    }

    fun onActivityResumed(activity: FragmentActivity) {
        if (!isEnabled || isAuthenticating) return
        if (lastBackgroundTimestamp > 0L) {
            val elapsed = SystemClock.elapsedRealtime() - lastBackgroundTimestamp
            if (elapsed >= lockTimeoutMs) {
                lock()
            }
            lastBackgroundTimestamp = 0L
        }
    }

    /**
     * Authenticates using Android native BiometricPrompt supporting strong biometrics
     * (fingerprint, face) and device credentials (PIN, pattern, password).
     */
    fun authenticate(
        activity: FragmentActivity,
        title: String,
        subtitle: String,
        onSuccess: () -> Unit,
        onError: (String) -> Unit = {}
    ) {
        val biometricManager = BiometricManager.from(activity)
        val authenticators = BiometricManager.Authenticators.BIOMETRIC_STRONG or
                BiometricManager.Authenticators.DEVICE_CREDENTIAL

        val canAuth = biometricManager.canAuthenticate(authenticators)
        if (canAuth != BiometricManager.BIOMETRIC_SUCCESS) {
            val reason = when (canAuth) {
                BiometricManager.BIOMETRIC_ERROR_NONE_ENROLLED ->
                    "No biometric or device PIN/pattern/password enrolled on this device. Please configure a screen lock in Android Settings."
                BiometricManager.BIOMETRIC_ERROR_NO_HARDWARE ->
                    "No biometric hardware available on this device."
                BiometricManager.BIOMETRIC_ERROR_HW_UNAVAILABLE ->
                    "Biometric hardware currently unavailable."
                else -> "Authentication unavailable (code $canAuth)."
            }
            onError(reason)
            return
        }

        if (isAuthenticating) return
        isAuthenticating = true

        val executor = ContextCompat.getMainExecutor(activity)
        val promptInfo = BiometricPrompt.PromptInfo.Builder()
            .setTitle(title)
            .setSubtitle(subtitle)
            .setAllowedAuthenticators(authenticators)
            .build()

        val biometricPrompt = BiometricPrompt(activity, executor, object : BiometricPrompt.AuthenticationCallback() {
            override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
                super.onAuthenticationSucceeded(result)
                isAuthenticating = false
                unlock()
                onSuccess()
            }

            override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
                super.onAuthenticationError(errorCode, errString)
                isAuthenticating = false
                onError(errString.toString())
            }

            override fun onAuthenticationFailed() {
                super.onAuthenticationFailed()
                // User can retry on same dialog
            }
        })

        try {
            biometricPrompt.authenticate(promptInfo)
        } catch (e: Exception) {
            isAuthenticating = false
            Log.e(TAG, "BiometricPrompt failed to authenticate", e)
            onError(e.message ?: "Authentication failed")
        }
    }

    /**
     * Toggles App Lock to enabled. Requires biometric verification first.
     */
    fun enable(
        activity: FragmentActivity,
        onSuccess: () -> Unit,
        onError: (String) -> Unit
    ) {
        authenticate(
            activity = activity,
            title = "Enable App Lock",
            subtitle = "Verify identity to enable App Lock for TRUX",
            onSuccess = {
                prefs.edit().putBoolean(KEY_ENABLED, true).apply()
                isEnabled = true
                isLocked = false
                onSuccess()
            },
            onError = onError
        )
    }

    /**
     * Disables App Lock. Requires biometric verification first.
     */
    fun disable(
        activity: FragmentActivity,
        onSuccess: () -> Unit,
        onError: (String) -> Unit
    ) {
        authenticate(
            activity = activity,
            title = "Disable App Lock",
            subtitle = "Verify identity to disable App Lock",
            onSuccess = {
                prefs.edit().putBoolean(KEY_ENABLED, false).apply()
                isEnabled = false
                isLocked = false
                onSuccess()
            },
            onError = onError
        )
    }

    /**
     * Updates App Lock settings. If App Lock is enabled, requires authentication.
     */
    fun updateSettings(
        activity: FragmentActivity,
        timeoutMs: Long,
        onBackground: Boolean,
        onScreenOff: Boolean,
        onSuccess: () -> Unit,
        onError: (String) -> Unit
    ) {
        if (!isEnabled) {
            saveSettings(timeoutMs, onBackground, onScreenOff)
            onSuccess()
            return
        }

        authenticate(
            activity = activity,
            title = "App Lock Settings",
            subtitle = "Verify identity to change App Lock configuration",
            onSuccess = {
                saveSettings(timeoutMs, onBackground, onScreenOff)
                onSuccess()
            },
            onError = onError
        )
    }

    private fun saveSettings(timeoutMs: Long, onBackground: Boolean, onScreenOff: Boolean) {
        prefs.edit()
            .putLong(KEY_TIMEOUT_MS, timeoutMs)
            .putBoolean(KEY_LOCK_ON_BG, onBackground)
            .putBoolean(KEY_LOCK_ON_SCREEN_OFF, onScreenOff)
            .apply()

        lockTimeoutMs = timeoutMs
        lockOnBackground = onBackground
        lockOnScreenOff = onScreenOff
    }
}
