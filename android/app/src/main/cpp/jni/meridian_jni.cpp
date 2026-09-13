// JNI surface for com.meridian.shell.PtyBridge.
//
// Kotlin only ever sees an opaque `Long` handle. No C++ type, pointer layout or
// header ever crosses the boundary.
//
// Thread safety: g_mutex protects g_sessions. Individual TerminalCore operations
// are internally thread-safe (they hold their own mutex). The global map lock is
// released before any blocking operation so one slow session cannot stall others.
//
// Lifetime: TerminalCore is reference-counted via shared_ptr. Destroying a session
// moves it out of the map (under g_mutex) then resets the shared_ptr outside the
// lock, so the reader thread joins without holding g_mutex.

#include "bridge/TerminalCore.hpp"

#include <jni.h>
#include <android/log.h>

#include <fcntl.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#define LOG_TAG "MeridianNative"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,   LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,    LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,    LOG_TAG, __VA_ARGS__)

using meridian::android::TerminalCore;
using meridian::android::PtySpawnSpec;

namespace {

// Global session registry. Protected by g_mutex.
// Max sessions: 256 (sane device limit).
constexpr size_t kMaxSessions = 256;

std::mutex g_mutex;
std::unordered_map<jlong, std::shared_ptr<TerminalCore>> g_sessions;
jlong g_next_handle = 1;

// ---- JNI helper: safe string conversion ------------------------------------

std::string jstr(JNIEnv* env, jstring s) {
    if (!s) return {};
    const char* c = env->GetStringUTFChars(s, nullptr);
    if (!c) return {};
    std::string out(c);
    env->ReleaseStringUTFChars(s, c);
    return out;
}

std::vector<std::string> jstr_array(JNIEnv* env, jobjectArray arr) {
    std::vector<std::string> out;
    if (!arr) return out;
    jsize n = env->GetArrayLength(arr);
    out.reserve(static_cast<size_t>(n));
    for (jsize i = 0; i < n; ++i) {
        auto s = reinterpret_cast<jstring>(env->GetObjectArrayElement(arr, i));
        if (s) {
            out.push_back(jstr(env, s));
            env->DeleteLocalRef(s);
        }
    }
    return out;
}

// ---- Session lookup (safe, returns nullptr if not found) -------------------

std::shared_ptr<TerminalCore> lookup(jlong handle) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_sessions.find(handle);
    return it == g_sessions.end() ? nullptr : it->second;
}

} // namespace

// ---- JNI_OnLoad: validate version, log library load -----------------------

JNIEXPORT jint JNI_OnLoad(JavaVM* /*vm*/, void* /*reserved*/) {
    LOGI("Meridian native library loaded (JNI_OnLoad)");
    return JNI_VERSION_1_6;
}

extern "C" {

// ---------------------------------------------------------------------------
// nativeCreateSession
// ---------------------------------------------------------------------------
JNIEXPORT jlong JNICALL
Java_com_meridian_shell_PtyBridge_nativeCreateSession(
        JNIEnv* env, jobject,
        jstring shell, jobjectArray argv, jobjectArray envPairs,
        jstring cwd, jint cols, jint rows, jint scrollback) {

    // Guard against runaway session creation
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_sessions.size() >= kMaxSessions) {
            LOGE("Session limit (%zu) reached", kMaxSessions);
            return 0;
        }
    }

    PtySpawnSpec spec;
    spec.shell = jstr(env, shell);
    spec.argv  = jstr_array(env, argv);
    spec.cwd   = jstr(env, cwd);

    if (spec.shell.empty()) {
        LOGE("nativeCreateSession: shell path is empty");
        return 0;
    }

    // envPairs is a flat [k0, v0, k1, v1, ...] array.
    auto flat = jstr_array(env, envPairs);
    for (size_t i = 0; i + 1 < flat.size(); i += 2) {
        spec.env.emplace_back(flat[i], flat[i + 1]);
    }

    auto core = std::make_shared<TerminalCore>(
            cols  > 0 ? cols  : 80,
            rows  > 0 ? rows  : 24,
            static_cast<std::size_t>(scrollback > 0 ? scrollback : 10000));

    if (!core->start(spec)) {
        LOGE("forkpty failed for shell=%s", spec.shell.c_str());
        return 0;
    }

    jlong handle;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        handle = g_next_handle++;
        g_sessions.emplace(handle, std::move(core));
    }

    LOGI("Session %lld created (shell=%s)", (long long)handle, spec.shell.c_str());
    return handle;
}

// ---------------------------------------------------------------------------
// nativeDestroySession
// ---------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_meridian_shell_PtyBridge_nativeDestroySession(JNIEnv*, jobject, jlong handle) {
    std::shared_ptr<TerminalCore> core;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_sessions.find(handle);
        if (it == g_sessions.end()) return;
        core = std::move(it->second);
        g_sessions.erase(it);
    }
    // core's destructor joins the reader thread OUTSIDE the map lock.
    // A slow shutdown (e.g., zombie process wait) cannot stall other sessions.
    LOGI("Session %lld destroyed", (long long)handle);
    core.reset();
}

// ---------------------------------------------------------------------------
// nativeWrite
// ---------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_meridian_shell_PtyBridge_nativeWrite(
        JNIEnv* env, jobject, jlong handle, jbyteArray data, jint length) {
    auto core = lookup(handle);
    if (!core || !data || length <= 0) return;

    jbyte* buf = env->GetByteArrayElements(data, nullptr);
    if (!buf) return;
    // write_input does not block indefinitely; EAGAIN breaks after one retry.
    core->write_input(reinterpret_cast<const char*>(buf), static_cast<size_t>(length));
    env->ReleaseByteArrayElements(data, buf, JNI_ABORT);
}

// ---------------------------------------------------------------------------
// nativeResize
// ---------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_meridian_shell_PtyBridge_nativeResize(
        JNIEnv*, jobject, jlong handle, jint cols, jint rows) {
    if (auto core = lookup(handle)) core->resize(cols, rows);
}

// ---------------------------------------------------------------------------
// nativeSignal
// ---------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_meridian_shell_PtyBridge_nativeSignal(JNIEnv*, jobject, jlong handle, jint sig) {
    if (auto core = lookup(handle)) core->signal(sig);
}

// ---------------------------------------------------------------------------
// nativeIsAlive
// ---------------------------------------------------------------------------
JNIEXPORT jboolean JNICALL
Java_com_meridian_shell_PtyBridge_nativeIsAlive(JNIEnv*, jobject, jlong handle) {
    auto core = lookup(handle);
    return (core && core->alive()) ? JNI_TRUE : JNI_FALSE;
}

// ---------------------------------------------------------------------------
// nativeExitStatus
// ---------------------------------------------------------------------------
JNIEXPORT jint JNICALL
Java_com_meridian_shell_PtyBridge_nativeExitStatus(JNIEnv*, jobject, jlong handle) {
    auto core = lookup(handle);
    return core ? core->exit_status() : -1;
}

// ---------------------------------------------------------------------------
// nativeGeneration
// ---------------------------------------------------------------------------
JNIEXPORT jlong JNICALL
Java_com_meridian_shell_PtyBridge_nativeGeneration(JNIEnv*, jobject, jlong handle) {
    auto core = lookup(handle);
    return core ? static_cast<jlong>(core->generation()) : 0;
}

// ---------------------------------------------------------------------------
// nativeAwaitChange
// ---------------------------------------------------------------------------
JNIEXPORT jboolean JNICALL
Java_com_meridian_shell_PtyBridge_nativeAwaitChange(
        JNIEnv*, jobject, jlong handle, jlong since, jint timeoutMs) {
    auto core = lookup(handle);
    if (!core) return JNI_FALSE;
    return core->await_change(static_cast<uint64_t>(since), timeoutMs) ? JNI_TRUE : JNI_FALSE;
}

// ---------------------------------------------------------------------------
// nativeSnapshot
// Fills a caller-owned IntArray. The renderer reuses one array for the life of
// the view, so a steady-state frame allocates nothing on the Java heap.
// ---------------------------------------------------------------------------
JNIEXPORT jint JNICALL
Java_com_meridian_shell_PtyBridge_nativeSnapshot(
        JNIEnv* env, jobject, jlong handle, jintArray cells, jintArray meta, jint scrollOffset) {
    auto core = lookup(handle);
    if (!core || !cells || !meta) return -1;
    if (env->GetArrayLength(meta) < 5) return -1;

    jint* cbuf = env->GetIntArrayElements(cells, nullptr);
    jint* mbuf = env->GetIntArrayElements(meta, nullptr);
    if (!cbuf || !mbuf) {
        if (cbuf) env->ReleaseIntArrayElements(cells, cbuf, JNI_ABORT);
        if (mbuf) env->ReleaseIntArrayElements(meta, mbuf, JNI_ABORT);
        return -1;
    }

    int written = core->snapshot(reinterpret_cast<int32_t*>(cbuf),
                                 env->GetArrayLength(cells),
                                 reinterpret_cast<int32_t*>(mbuf),
                                 scrollOffset);

    env->ReleaseIntArrayElements(cells, cbuf, written < 0 ? JNI_ABORT : 0);
    env->ReleaseIntArrayElements(meta,  mbuf, written < 0 ? JNI_ABORT : 0);
    return written;
}

// ---------------------------------------------------------------------------
// nativeDumpText
// ---------------------------------------------------------------------------
JNIEXPORT jstring JNICALL
Java_com_meridian_shell_PtyBridge_nativeDumpText(JNIEnv* env, jobject, jlong handle) {
    auto core = lookup(handle);
    if (!core) return env->NewStringUTF("");
    return env->NewStringUTF(core->dump_text().c_str());
}

} // extern "C"
