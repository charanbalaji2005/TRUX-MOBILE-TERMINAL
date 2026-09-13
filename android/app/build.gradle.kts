plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("kotlin-kapt")
    id("org.jetbrains.kotlin.plugin.serialization")
    id("org.jetbrains.kotlin.plugin.compose")
}

android {
    namespace = "com.meridian.shell"
    compileSdk = 35
    ndkVersion = "27.1.12297006"

    defaultConfig {
        applicationId = "com.meridian.shell"
        minSdk = 24

        // ----------------------------------------------------------------
        // targetSdk = 28 is DELIBERATE, not an oversight.
        //
        // Android 10 (API 29) enforces W^X: an app targeting 29+ may not
        // exec() a file inside its own writable data directory. A Termux
        // style package manager REQUIRES exactly that. Termux itself pins
        // targetSdkVersion 28 for this reason and ships via F-Droid/GitHub
        // rather than Google Play.
        // ----------------------------------------------------------------
        targetSdk = 28

        versionCode = 2
        versionName = "2.0.1"

        ndk { abiFilters += listOf("arm64-v8a", "armeabi-v7a") }

        externalNativeBuild {
            cmake {
                arguments += listOf("-DANDROID_STL=c++_shared", "-DCMAKE_BUILD_TYPE=Release")
                cppFlags += listOf("-std=c++20", "-fexceptions", "-frtti")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    packaging {
        jniLibs { useLegacyPackaging = true }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
        debug { isJniDebuggable = true }
    }

    buildFeatures { compose = true }
    testOptions {
        unitTests.isReturnDefaultValues = true
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
        // Suppress Java 26 module warnings
        freeCompilerArgs += listOf("-Xjvm-default=all")
    }

    kapt {
        arguments {
            arg("room.schemaLocation", "$projectDir/schemas")
        }
    }
}

dependencies {
    // Core Android / Compose
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.activity:activity-compose:1.9.3")
    implementation(platform("androidx.compose:compose-bom:2024.10.01"))
    implementation("androidx.compose.ui:ui")
    implementation("androidx.compose.foundation:foundation")
    implementation("androidx.compose.material3:material3")
    implementation("androidx.compose.material:material-icons-extended")
    implementation("androidx.biometric:biometric:1.2.0-alpha05")
    implementation("androidx.fragment:fragment-ktx:1.8.5")

    // Media3 (video & audio playback)
    implementation("androidx.media3:media3-exoplayer:1.4.1")
    implementation("androidx.media3:media3-ui:1.4.1")

    // Lifecycle / ViewModel
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.8.7")
    implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:2.8.7")
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.8.7")
    implementation("androidx.lifecycle:lifecycle-runtime-compose:2.8.7")

    // Room (persistence)
    implementation("androidx.room:room-runtime:2.6.1")
    implementation("androidx.room:room-ktx:2.6.1")
    kapt("androidx.room:room-compiler:2.6.1")

    // Coroutines
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.8.1")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.8.1")

    // DataStore (settings)
    implementation("androidx.datastore:datastore-preferences:1.1.1")

    // JSON (package manager)
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.7.3")

    // OkHttp (package downloads)
    implementation("com.squareup.okhttp3:okhttp:4.12.0")

    // Testing
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.jetbrains.kotlinx:kotlinx-coroutines-test:1.8.1")
    androidTestImplementation("androidx.test.ext:junit:1.2.1")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.6.1")
    androidTestImplementation("androidx.room:room-testing:2.6.1")
    androidTestImplementation(platform("androidx.compose:compose-bom:2024.10.01"))
    androidTestImplementation("androidx.compose.ui:ui-test-junit4")
}
