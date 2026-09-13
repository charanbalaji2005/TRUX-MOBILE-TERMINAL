# Proguard rules for Meridian Shell

# Keep all Kotlin data classes used for Room
-keep class com.meridian.shell.data.** { *; }

# Keep package manager data classes (kotlinx.serialization)
-keep class com.meridian.shell.pkg.** { *; }
-keepattributes *Annotation*, InnerClasses
-dontnote kotlinx.serialization.AnnotationsKt

# kotlinx.serialization
-keepclassmembers class kotlinx.serialization.json.** { *** Companion; }
-keepclasseswithmembers class kotlinx.serialization.** { kotlinx.serialization.KSerializer serializer(...); }

# Room
-keep class * extends androidx.room.RoomDatabase
-dontwarn androidx.room.**

# OkHttp
-dontwarn okhttp3.**
-dontwarn okio.**

# Native library: never obfuscate JNI entry points
-keepclasseswithmembernames class * {
    native <methods>;
}

# Compose
-dontwarn androidx.compose.**
