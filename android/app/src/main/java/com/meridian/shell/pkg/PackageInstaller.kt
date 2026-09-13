package com.meridian.shell.pkg

import android.content.Context
import android.util.Log
import com.meridian.shell.Bootstrap
import com.meridian.shell.data.InstalledPackageEntity
import com.meridian.shell.data.MeridianDatabase
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.OkHttpClient
import okhttp3.Request
import java.io.*
import java.security.MessageDigest
import java.util.concurrent.TimeUnit
import java.util.zip.GZIPInputStream

/**
 * Downloads, verifies, extracts, and installs packages into the Meridian
 * Linux userspace.
 *
 * Installation flow:
 *   1. Resolve → find PackageEntry in repository
 *   2. Check dependencies → install transitively
 *   3. Download → stream .tar.xz to packages/cache/
 *   4. Verify → SHA-256 must match PackageEntry.sha256
 *   5. Extract → untar to $PREFIX (filesDir/usr/)
 *   6. Record → write to Room installed_packages table
 *
 * Removal flow:
 *   1. Read file list from Room
 *   2. Delete each listed file from $PREFIX
 *   3. Remove from Room
 *
 * All I/O runs on Dispatchers.IO. Progress is reported via a callback
 * (written to PTY output so the user sees real terminal output).
 */
class PackageInstaller(
    private val context: Context,
    private val bootstrap: Bootstrap,
    private val repo: PackageRepository,
    private val db: MeridianDatabase
) {
    companion object {
        private const val TAG = "PackageInstaller"
        private const val BUFFER_SIZE = 64 * 1024  // 64 KB
    }

    private val cacheDir = File(context.filesDir, "packages/cache").also { it.mkdirs() }

    private val http = OkHttpClient.Builder()
        .connectTimeout(30, TimeUnit.SECONDS)
        .readTimeout(300, TimeUnit.SECONDS)  // large downloads
        .build()

    /**
     * Install a package by name.
     *
     * @param name Package name
     * @param output Callback receiving progress lines (written to PTY)
     * @return true if successful
     */
    suspend fun install(name: String, output: (String) -> Unit): Boolean =
        withContext(Dispatchers.IO) {
            output("[pkg] Looking up $name...")

            val entry = repo.find(name)
            if (entry == null) {
                output("[pkg] ERROR: Package '$name' not found in repository.")
                output("[pkg] Run 'pkg update' to refresh the package index.")
                return@withContext false
            }

            // Check if already installed
            val existing = db.packageDao().findByName(name)
            if (existing != null && !entry.isNewerThan(existing.version)) {
                output("[pkg] $name (${existing.version}) is already up to date.")
                return@withContext true
            }

            // Install dependencies first
            for (dep in entry.depends) {
                val depInstalled = db.packageDao().findByName(dep) != null
                if (!depInstalled) {
                    output("[pkg] Installing dependency: $dep")
                    val depOk = install(dep, output)
                    if (!depOk) {
                        output("[pkg] ERROR: Failed to install dependency '$dep'.")
                        return@withContext false
                    }
                }
            }

            // Download
            val archiveFile = File(cacheDir, "${name}_${entry.version}.tar.xz")
            output("[pkg] Downloading ${entry.name} ${entry.version}...")

            val downloaded = download(entry.url, archiveFile) { bytesRead, total ->
                if (total > 0) {
                    val pct = (bytesRead * 100 / total).toInt()
                    // Print progress every 10%
                    if (pct % 10 == 0) output("[pkg] Downloading... $pct%")
                }
            }

            if (!downloaded) {
                output("[pkg] ERROR: Download failed.")
                return@withContext false
            }

            // Verify SHA-256
            output("[pkg] Verifying integrity...")
            val actualHash = sha256(archiveFile)
            if (!actualHash.equals(entry.sha256, ignoreCase = true)) {
                output("[pkg] ERROR: Integrity check failed!")
                output("[pkg]   expected: ${entry.sha256}")
                output("[pkg]   got:      $actualHash")
                archiveFile.delete()
                return@withContext false
            }
            output("[pkg] Integrity OK.")

            // Extract
            output("[pkg] Extracting to ${bootstrap.prefix.absolutePath}...")
            val installedFiles = extract(archiveFile, bootstrap.prefix, output)

            if (installedFiles == null) {
                output("[pkg] ERROR: Extraction failed.")
                return@withContext false
            }

            // Make installed binaries executable
            val binDir = File(bootstrap.prefix, "bin")
            installedFiles
                .filter { it.startsWith("bin/") || it.startsWith("usr/bin/") }
                .forEach { File(bootstrap.prefix, it).setExecutable(true, false) }

            // Record in Room
            db.packageDao().insertOrReplace(
                InstalledPackageEntity(
                    name = entry.name,
                    version = entry.version,
                    description = entry.description,
                    installedAt = System.currentTimeMillis(),
                    fileListJson = installedFiles.joinToString("\n"),
                    installedSizeBytes = entry.installed_size
                )
            )

            output("[pkg] Successfully installed ${entry.name} ${entry.version}.")
            Log.i(TAG, "Installed ${entry.name} ${entry.version}, ${installedFiles.size} files")
            true
        }

    /**
     * Remove a package.
     */
    suspend fun remove(name: String, output: (String) -> Unit): Boolean =
        withContext(Dispatchers.IO) {
            val pkg = db.packageDao().findByName(name)
            if (pkg == null) {
                output("[pkg] ERROR: '$name' is not installed.")
                return@withContext false
            }

            output("[pkg] Removing $name ${pkg.version}...")
            val files = pkg.fileListJson.lines().filter { it.isNotBlank() }
            var removed = 0
            for (relPath in files) {
                val f = File(bootstrap.prefix, relPath)
                if (f.exists() && f.delete()) removed++
            }

            db.packageDao().remove(name)
            output("[pkg] Removed $name ($removed files).")
            true
        }

    /**
     * Upgrade all installed packages that have newer versions in the index.
     */
    suspend fun upgrade(output: (String) -> Unit): Boolean =
        withContext(Dispatchers.IO) {
            output("[pkg] Checking for upgrades...")
            val manifest = repo.getManifest(forceRefresh = true).getOrNull()
            if (manifest == null) {
                output("[pkg] ERROR: Could not reach repository.")
                return@withContext false
            }

            val installed = db.packageDao().getAll()
            val toUpgrade = installed.filter { inst ->
                val entry = manifest.packages.firstOrNull { it.name == inst.name }
                entry != null && entry.isNewerThan(inst.version)
            }

            if (toUpgrade.isEmpty()) {
                output("[pkg] All packages are up to date.")
                return@withContext true
            }

            output("[pkg] ${toUpgrade.size} package(s) to upgrade.")
            var allOk = true
            for (inst in toUpgrade) {
                allOk = allOk && install(inst.name, output)
            }
            allOk
        }

    /**
     * List installed packages.
     */
    suspend fun listInstalled(): List<InstalledPackageEntity> =
        withContext(Dispatchers.IO) { db.packageDao().getAll() }

    // ---- internal helpers --------------------------------------------------

    private fun download(
        url: String,
        dest: File,
        onProgress: (Long, Long) -> Unit
    ): Boolean = runCatching {
        val request = Request.Builder().url(url).get().build()
        val response = http.newCall(request).execute()
        if (!response.isSuccessful) return@runCatching false

        val body = response.body ?: return@runCatching false
        val total = body.contentLength()
        var bytesRead = 0L

        FileOutputStream(dest).use { out ->
            body.byteStream().use { input ->
                val buf = ByteArray(BUFFER_SIZE)
                var n: Int
                while (input.read(buf).also { n = it } >= 0) {
                    out.write(buf, 0, n)
                    bytesRead += n
                    onProgress(bytesRead, total)
                }
            }
        }
        true
    }.getOrElse { e ->
        Log.e(TAG, "Download failed: $url", e)
        dest.delete()
        false
    }

    private fun sha256(file: File): String {
        val md = MessageDigest.getInstance("SHA-256")
        FileInputStream(file).use { fis ->
            val buf = ByteArray(BUFFER_SIZE)
            var n: Int
            while (fis.read(buf).also { n = it } >= 0) md.update(buf, 0, n)
        }
        return md.digest().joinToString("") { "%02x".format(it) }
    }

    /**
     * Extract a .tar.xz (or .tar.gz) archive into [destDir].
     * Returns list of relative paths that were extracted, or null on failure.
     *
     * We use Apache Commons Compress-style manual extraction since we can't
     * add a full Compress dep. Instead we shell out to the tar binary if
     * available, otherwise fall back to gzip+tar parsing.
     */
    private fun extract(archive: File, destDir: File, output: (String) -> Unit): List<String>? =
        runCatching {
            // Prefer the system tar binary (works on API 21+, available in /system/bin)
            val tarBin = listOf("/system/bin/tar", "/system/xbin/tar")
                .firstOrNull { File(it).canExecute() }

            if (tarBin != null) {
                return@runCatching extractWithSystemTar(tarBin, archive, destDir, output)
            }

            // Fallback: pure Java extraction for .tar.gz only
            output("[pkg] WARNING: system tar not found, using Java extractor (gz only)")
            extractTarGz(archive, destDir)
        }.getOrElse { e ->
            Log.e(TAG, "Extraction failed", e)
            null
        }

    private fun extractWithSystemTar(
        tarBin: String, archive: File, destDir: File, output: (String) -> Unit
    ): List<String> {
        val files = mutableListOf<String>()
        val process = ProcessBuilder(tarBin, "-xf", archive.absolutePath, "-C", destDir.absolutePath,
            "--no-same-owner", "-v")
            .redirectErrorStream(true)
            .start()

        BufferedReader(InputStreamReader(process.inputStream)).use { reader ->
            var line: String?
            while (reader.readLine().also { line = it } != null) {
                line?.let {
                    files.add(it.trimStart('/'))
                }
            }
        }

        val exit = process.waitFor()
        if (exit != 0) error("tar exited with code $exit")
        return files
    }

    private fun extractTarGz(archive: File, destDir: File): List<String> {
        val files = mutableListOf<String>()
        // Basic .tar.gz extraction using Java streams
        // For .tar.xz a native tar is required; this is the fallback only
        GZIPInputStream(FileInputStream(archive)).use { gzip ->
            val buf = ByteArray(512)  // TAR block size
            while (true) {
                if (gzip.read(buf) < 0) break
                val name = String(buf, 0, 100).trimEnd('\u0000')
                if (name.isEmpty()) break
                val sizeStr = String(buf, 124, 12).trimEnd('\u0000').trimStart()
                val size = if (sizeStr.isEmpty()) 0L else sizeStr.toLong(8)
                val type = buf[156]  // '0' = file, '5' = dir
                files.add(name)

                if (type == '5'.code.toByte()) {
                    File(destDir, name).mkdirs()
                } else if (type == '0'.code.toByte() || type == 0.toByte()) {
                    val dest = File(destDir, name)
                    dest.parentFile?.mkdirs()
                    FileOutputStream(dest).use { out ->
                        var remaining = size
                        val data = ByteArray(1024)
                        while (remaining > 0) {
                            val toRead = minOf(remaining, data.size.toLong()).toInt()
                            val read = gzip.read(data, 0, toRead)
                            if (read < 0) break
                            out.write(data, 0, read)
                            remaining -= read
                        }
                    }
                }

                // Skip to next 512-byte boundary
                val blocks = (size + 511) / 512
                repeat((blocks * 512 - size).toInt()) { gzip.read() }
            }
        }
        return files
    }
}
