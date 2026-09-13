package com.meridian.shell.data

import androidx.room.*
import kotlinx.coroutines.flow.Flow

/**
 * Tracks which packages are installed in the Meridian userspace.
 *
 * This is separate from the filesystem installation itself — it acts as an
 * index so `pkg list-installed` and `pkg remove` are fast without scanning
 * the entire $PREFIX/bin directory.
 */
@Entity(tableName = "installed_packages")
data class InstalledPackageEntity(
    @PrimaryKey val name: String,
    val version: String,
    val description: String,
    val installedAt: Long,
    /** Newline-separated list of paths installed by this package under $PREFIX */
    val fileListJson: String = "",
    /** Disk space used in bytes */
    val installedSizeBytes: Long = 0L
)

@Dao
interface InstalledPackageDao {

    @Query("SELECT * FROM installed_packages ORDER BY name ASC")
    fun observeAll(): Flow<List<InstalledPackageEntity>>

    @Query("SELECT * FROM installed_packages ORDER BY name ASC")
    suspend fun getAll(): List<InstalledPackageEntity>

    @Query("SELECT * FROM installed_packages WHERE name = :name LIMIT 1")
    suspend fun findByName(name: String): InstalledPackageEntity?

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertOrReplace(pkg: InstalledPackageEntity)

    @Query("DELETE FROM installed_packages WHERE name = :name")
    suspend fun remove(name: String)

    @Query("DELETE FROM installed_packages")
    suspend fun deleteAll()

    @Query("SELECT COUNT(*) FROM installed_packages")
    suspend fun count(): Int
}
