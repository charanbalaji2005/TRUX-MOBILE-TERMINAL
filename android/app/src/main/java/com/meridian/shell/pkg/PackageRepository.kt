package com.meridian.shell.pkg

import android.content.Context
import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import okhttp3.OkHttpClient
import okhttp3.Request
import java.io.File
import java.util.concurrent.TimeUnit

/**
 * Fetches and caches the Meridian package repository index.
 *
 * Repository: packages.json hosted at REPO_BASE_URL
 *
 * Cache: <filesDir>/packages/cache/index.json (TTL: 24h)
 *
 * All network I/O runs on Dispatchers.IO. No data is ever uploaded.
 * The only outbound request is GET packages.json.
 */
class PackageRepository(private val context: Context) {

    companion object {
        /**
         * Change this URL to your own package repository.
         * GitHub Releases is a good free option:
         *   https://github.com/<YOU>/meridian-packages/releases/latest/download/packages.json
         */
        const val REPO_BASE_URL = "https://raw.githubusercontent.com/meridianshell/packages/main"
        private const val INDEX_URL = "$REPO_BASE_URL/packages.json"
        private const val CACHE_TTL_MS = 24L * 60 * 60 * 1000  // 24 hours
        private const val TAG = "PackageRepository"
    }

    private val cacheDir: File
        get() = File(context.filesDir, "packages/cache").also { it.mkdirs() }

    private val indexCache: File
        get() = File(cacheDir, "index.json")

    private val http = OkHttpClient.Builder()
        .connectTimeout(30, TimeUnit.SECONDS)
        .readTimeout(60, TimeUnit.SECONDS)
        .build()

    private val json = Json {
        ignoreUnknownKeys = true
        isLenient = true
    }

    private var cachedManifest: PackageManifest? = null

    /**
     * Returns the package manifest. Uses cache if fresh; otherwise fetches from network.
     *
     * @param forceRefresh Skip cache and always fetch from network.
     */
    suspend fun getManifest(forceRefresh: Boolean = false): Result<PackageManifest> =
        withContext(Dispatchers.IO) {
            // Return in-memory cache first
            if (!forceRefresh && cachedManifest != null) {
                return@withContext Result.success(cachedManifest!!)
            }

            // Check disk cache freshness
            if (!forceRefresh && indexCache.exists()) {
                val age = System.currentTimeMillis() - indexCache.lastModified()
                if (age < CACHE_TTL_MS) {
                    return@withContext runCatching {
                        val manifest = json.decodeFromString<PackageManifest>(indexCache.readText())
                        cachedManifest = manifest
                        manifest
                    }
                }
            }

            // Fetch from network
            fetchAndCache()
        }

    private suspend fun fetchAndCache(): Result<PackageManifest> =
        withContext(Dispatchers.IO) {
            runCatching {
                val request = Request.Builder().url(INDEX_URL).get().build()
                val response = http.newCall(request).execute()

                if (!response.isSuccessful) {
                    error("HTTP ${response.code}: ${response.message}")
                }

                val body = response.body?.string() ?: error("Empty response body")
                indexCache.writeText(body)

                val manifest = json.decodeFromString<PackageManifest>(body)
                cachedManifest = manifest
                Log.i(TAG, "Fetched ${manifest.packages.size} packages from repository")
                manifest
            }.onFailure { e ->
                Log.e(TAG, "Failed to fetch package index", e)
                // Fall back to stale cache if available
                if (indexCache.exists()) {
                    runCatching {
                        cachedManifest = json.decodeFromString<PackageManifest>(indexCache.readText())
                    }
                }
            }
        }

    /**
     * Search packages by name or description substring (case-insensitive).
     */
    suspend fun search(query: String): List<PackageEntry> {
        val manifest = getManifest().getOrNull() ?: return emptyList()
        val q = query.lowercase()
        return manifest.packages.filter {
            it.name.lowercase().contains(q) || it.description.lowercase().contains(q)
        }
    }

    /**
     * Find exact package by name.
     */
    suspend fun find(name: String): PackageEntry? {
        val manifest = getManifest().getOrNull() ?: return null
        return manifest.packages.firstOrNull { it.name == name }
    }

    /**
     * Returns all packages whose names match the installed map.
     */
    suspend fun findMany(names: Set<String>): List<PackageEntry> {
        val manifest = getManifest().getOrNull() ?: return emptyList()
        return manifest.packages.filter { it.name in names }
    }

    /** Invalidate the in-memory and disk cache. */
    fun clearCache() {
        cachedManifest = null
        indexCache.delete()
    }
}
