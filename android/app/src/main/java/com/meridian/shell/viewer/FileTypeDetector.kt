package com.meridian.shell.viewer

import java.io.File
import java.io.FileInputStream

enum class FileType(val displayName: String) {
    PDF("PDF Document"),
    IMAGE("Image"),
    VIDEO("Video"),
    AUDIO("Audio"),
    TEXT("Text Document"),
    CODE("Source Code"),
    MARKDOWN("Markdown"),
    JSON("JSON Data"),
    CSV("CSV Spreadsheet"),
    XML("XML Document"),
    ARCHIVE("Archive"),
    OFFICE("Office Document"),
    EXECUTABLE("Binary Executable"),
    BINARY("Binary File")
}

object FileTypeDetector {

    fun detect(file: File): FileType {
        if (!file.exists() || file.isDirectory) return FileType.TEXT

        // 1. Check Magic Bytes first
        val magicType = detectByMagicBytes(file)
        if (magicType != null) return magicType

        // 2. By Extension
        val ext = file.extension.lowercase()
        return when (ext) {
            "pdf" -> FileType.PDF
            "jpg", "jpeg", "png", "webp", "gif", "bmp", "heic", "heif", "avif" -> FileType.IMAGE
            "mp4", "m4v", "webm", "mkv", "3gp", "mov", "avi", "flv" -> FileType.VIDEO
            "mp3", "wav", "aac", "m4a", "flac", "ogg", "opus", "wma", "mka" -> FileType.AUDIO
            "md", "markdown" -> FileType.MARKDOWN
            "json" -> FileType.JSON
            "csv", "tsv" -> FileType.CSV
            "xml", "svg" -> FileType.XML
            "zip", "tar", "gz", "tgz", "bz2", "xz", "zst", "7z", "rar" -> FileType.ARCHIVE
            "doc", "docx", "xls", "xlsx", "ppt", "pptx", "odt", "ods", "odp" -> FileType.OFFICE
            "py", "js", "ts", "jsx", "tsx", "kt", "kts", "java", "cpp", "c", "cc", "h", "hpp",
            "rs", "go", "html", "htm", "css", "scss", "sql", "sh", "bash", "zsh", "lua", "rb",
            "php", "swift", "dart", "yaml", "yml", "toml", "gradle" -> FileType.CODE
            "txt", "log", "ini", "conf", "cfg", "properties", "env", "diff", "patch" -> FileType.TEXT
            else -> {
                if (isTextFile(file)) FileType.TEXT else FileType.BINARY
            }
        }
    }

    private fun detectByMagicBytes(file: File): FileType? {
        if (file.length() < 4) return null
        return try {
            FileInputStream(file).use { fis ->
                val header = ByteArray(12)
                val read = fis.read(header)
                if (read < 4) return null

                // ELF executable
                if (header[0] == 0x7F.toByte() && header[1] == 'E'.code.toByte() &&
                    header[2] == 'L'.code.toByte() && header[3] == 'F'.code.toByte()) {
                    return FileType.EXECUTABLE
                }

                // PDF (%PDF)
                if (header[0] == '%'.code.toByte() && header[1] == 'P'.code.toByte() &&
                    header[2] == 'D'.code.toByte() && header[3] == 'F'.code.toByte()) {
                    return FileType.PDF
                }

                // PNG (\x89PNG)
                if (header[0] == 0x89.toByte() && header[1] == 'P'.code.toByte() &&
                    header[2] == 'N'.code.toByte() && header[3] == 'G'.code.toByte()) {
                    return FileType.IMAGE
                }

                // JPEG (\xFF\xD8\xFF)
                if (header[0] == 0xFF.toByte() && header[1] == 0xD8.toByte() && header[2] == 0xFF.toByte()) {
                    return FileType.IMAGE
                }

                // GIF (GIF87a / GIF89a)
                if (header[0] == 'G'.code.toByte() && header[1] == 'I'.code.toByte() &&
                    header[2] == 'F'.code.toByte() && header[3] == '8'.code.toByte()) {
                    return FileType.IMAGE
                }

                // ZIP (PK\x03\x04)
                if (header[0] == 'P'.code.toByte() && header[1] == 'K'.code.toByte() &&
                    header[2] == 0x03.toByte() && header[3] == 0x04.toByte()) {
                    val ext = file.extension.lowercase()
                    if (ext in listOf("docx", "xlsx", "pptx", "odt", "ods", "odp")) FileType.OFFICE
                    else FileType.ARCHIVE
                } else null
            }
        } catch (e: Exception) {
            null
        }
    }

    fun isTextFile(file: File): Boolean {
        if (!file.exists() || file.length() == 0L) return true
        return try {
            FileInputStream(file).use { fis ->
                val buffer = ByteArray(minOf(file.length(), 4096L).toInt())
                val read = fis.read(buffer)
                if (read <= 0) return true
                var nonPrintable = 0
                for (i in 0 until read) {
                    val b = buffer[i].toInt() and 0xFF
                    if (b == 0) return false // Null byte indicates binary
                    if (b < 0x09 || (b in 0x0E..0x1F)) {
                        nonPrintable++
                    }
                }
                (nonPrintable.toFloat() / read.toFloat()) < 0.05f
            }
        } catch (e: Exception) {
            false
        }
    }

    fun getMimeType(file: File): String {
        val ext = file.extension.lowercase()
        return android.webkit.MimeTypeMap.getSingleton().getMimeTypeFromExtension(ext)
            ?: when (detect(file)) {
                FileType.PDF -> "application/pdf"
                FileType.IMAGE -> "image/*"
                FileType.VIDEO -> "video/*"
                FileType.AUDIO -> "audio/*"
                FileType.MARKDOWN -> "text/markdown"
                FileType.JSON -> "application/json"
                FileType.CSV -> "text/csv"
                FileType.XML -> "text/xml"
                FileType.ARCHIVE -> "application/zip"
                FileType.OFFICE -> "application/octet-stream"
                FileType.CODE, FileType.TEXT -> "text/plain"
                FileType.EXECUTABLE, FileType.BINARY -> "application/octet-stream"
            }
    }
}
