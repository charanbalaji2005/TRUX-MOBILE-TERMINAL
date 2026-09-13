package com.meridian.shell.data

import androidx.room.Database
import androidx.room.Room
import androidx.room.RoomDatabase
import android.content.Context

/**
 * Single SQLite database for the entire Meridian application.
 *
 * File: /data/data/com.meridian.shell/databases/meridian.db
 *
 * Versioning:
 *   v1 — initial schema: sessions, installed_packages
 *
 * Migration strategy: fallbackToDestructiveMigration() is acceptable for
 * sessions (metadata is non-critical; the PTY cannot be restored anyway).
 * Installed packages will be re-scanned from the filesystem on next launch.
 */
@Database(
    entities = [SessionEntity::class, InstalledPackageEntity::class],
    version = 1,
    exportSchema = true
)
abstract class MeridianDatabase : RoomDatabase() {

    abstract fun sessionDao(): SessionDao
    abstract fun packageDao(): InstalledPackageDao

    companion object {
        @Volatile
        private var INSTANCE: MeridianDatabase? = null

        fun get(context: Context): MeridianDatabase =
            INSTANCE ?: synchronized(this) {
                INSTANCE ?: Room.databaseBuilder(
                    context.applicationContext,
                    MeridianDatabase::class.java,
                    "trux.db"
                )
                    .fallbackToDestructiveMigration()
                    .build()
                    .also { INSTANCE = it }
            }
    }
}

typealias TruxDatabase = MeridianDatabase

