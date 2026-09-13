package com.meridian.shell.viewer

import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.widget.Toast
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.core.content.FileProvider
import java.io.File
import java.io.FileInputStream
import java.security.MessageDigest
import java.text.SimpleDateFormat
import java.util.*

data class FileDetails(
    val name: String,
    val path: String,
    val sizeString: String,
    val sizeBytes: Long,
    val mimeType: String,
    val lastModified: String,
    val permissions: String,
    val sha256: String
)

class FileViewerManager {

    var activeFile: File? by mutableStateOf(null)
        private set

    var activeFileType: FileType? by mutableStateOf(null)
        private set

    fun openFile(path: String) {
        val file = File(path)
        if (!file.exists() || file.isDirectory) {
            return
        }
        val type = FileTypeDetector.detect(file)
        activeFileType = type
        activeFile = file
    }

    fun close() {
        activeFile = null
        activeFileType = null
    }

    fun getFileDetails(file: File): FileDetails {
        val sizeBytes = file.length()
        val sizeStr = formatFileSize(sizeBytes)
        val mime = FileTypeDetector.getMimeType(file)
        val sdf = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault())
        val modified = sdf.format(Date(file.lastModified()))

        val r = if (file.canRead()) "r" else "-"
        val w = if (file.canWrite()) "w" else "-"
        val x = if (file.canExecute()) "x" else "-"
        val perms = "$r$w$x"

        val hash = if (sizeBytes <= 50L * 1024 * 1024) {
            calculateSha256(file)
        } else {
            "Skipped (>50MB)"
        }

        return FileDetails(
            name = file.name,
            path = file.absolutePath,
            sizeString = sizeStr,
            sizeBytes = sizeBytes,
            mimeType = mime,
            lastModified = modified,
            permissions = perms,
            sha256 = hash
        )
    }

    fun shareFile(context: Context, file: File) {
        try {
            val uri = FileProvider.getUriForFile(
                context,
                "${context.packageName}.fileprovider",
                file
            )
            val mime = FileTypeDetector.getMimeType(file)
            val intent = Intent(Intent.ACTION_SEND).apply {
                type = mime
                putExtra(Intent.EXTRA_STREAM, uri)
                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
            }
            val chooser = Intent.createChooser(intent, "Share ${file.name}").apply {
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(chooser)
        } catch (e: Exception) {
            Toast.makeText(context, "Cannot share file: ${e.message}", Toast.LENGTH_SHORT).show()
        }
    }

    fun openWithExternalApp(context: Context, file: File) {
        try {
            val uri = FileProvider.getUriForFile(
                context,
                "${context.packageName}.fileprovider",
                file
            )
            val mime = FileTypeDetector.getMimeType(file)
            val intent = Intent(Intent.ACTION_VIEW).apply {
                setDataAndType(uri, mime)
                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(intent)
        } catch (e: ActivityNotFoundException) {
            Toast.makeText(context, "No compatible application found for this file type.", Toast.LENGTH_LONG).show()
        } catch (e: Exception) {
            Toast.makeText(context, "Could not open file: ${e.message}", Toast.LENGTH_SHORT).show()
        }
    }

    private fun calculateSha256(file: File): String {
        return try {
            val md = MessageDigest.getInstance("SHA-256")
            FileInputStream(file).use { fis ->
                val buffer = ByteArray(8192)
                var bytesRead: Int
                while (fis.read(buffer).also { bytesRead = it } != -1) {
                    md.update(buffer, 0, bytesRead)
                }
            }
            val digest = md.digest()
            digest.joinToString("") { "%02x".format(it) }
        } catch (e: Exception) {
            "Error calculating hash"
        }
    }

    companion object {
        fun formatFileSize(bytes: Long): String {
            if (bytes <= 0) return "0 B"
            val units = arrayOf("B", "KB", "MB", "GB", "TB")
            val digitGroups = (Math.log10(bytes.toDouble()) / Math.log10(1024.0)).toInt()
            val group = digitGroups.coerceIn(0, units.size - 1)
            val size = bytes / Math.pow(1024.0, group.toDouble())
            return String.format(Locale.US, "%.1f %s", size, units[group])
        }
    }
}
