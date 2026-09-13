// tests / test_security.cpp
#include "mini_test.hpp"
#include "../src/security/credential_store.hpp"

#include <sys/stat.h>
#include <unistd.h>

using namespace meridian::security;

namespace {
std::string test_dir() {
    return "/tmp/meridian_test_credentials_" + std::to_string(getpid());
}
} // namespace

MTEST(credential_store_round_trips_a_secret) {
    FileCredentialStore store(test_dir());
    ASSERT_TRUE(store.store("groq", "sk-test-secret-value-123"));
    auto retrieved = store.retrieve("groq");
    ASSERT_TRUE(retrieved.has_value());
    if (retrieved) ASSERT_EQ(*retrieved, std::string("sk-test-secret-value-123"));
    store.remove("groq");
}

MTEST(credential_store_missing_key_returns_nullopt) {
    FileCredentialStore store(test_dir());
    auto retrieved = store.retrieve("does_not_exist_at_all");
    ASSERT_FALSE(retrieved.has_value());
}

MTEST(credential_store_file_has_owner_only_permissions) {
    // This is the actual point of the store: verify the REAL file mode
    // on disk is 0600, not just that round-tripping the value works.
    FileCredentialStore store(test_dir());
    store.store("perm_check", "secret");

    std::string path = test_dir() + "/perm_check";
    struct stat st{};
    ASSERT_EQ(stat(path.c_str(), &st), 0);
    mode_t perm_bits = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    ASSERT_EQ(perm_bits, (mode_t)(S_IRUSR | S_IWUSR));

    store.remove("perm_check");
}

MTEST(credential_store_remove_is_idempotent) {
    FileCredentialStore store(test_dir());
    store.store("temp", "value");
    ASSERT_TRUE(store.remove("temp"));
    ASSERT_TRUE(store.remove("temp")); // already gone -> still reports success
    ASSERT_FALSE(store.retrieve("temp").has_value());
}

MTEST(credential_store_sanitizes_path_traversal_attempts) {
    FileCredentialStore store(test_dir());
    // A key_name that looks like a path-traversal attempt must not
    // escape the credentials directory.
    ASSERT_TRUE(store.store("../../etc/passwd_like_name", "value"));
    auto retrieved = store.retrieve("../../etc/passwd_like_name");
    ASSERT_TRUE(retrieved.has_value());
    // The file actually written should be inside test_dir(), sanitized.
    std::string expected_path = test_dir() + "/.._.._etc_passwd_like_name";
    struct stat st{};
    ASSERT_EQ(stat(expected_path.c_str(), &st), 0);
    store.remove("../../etc/passwd_like_name");
}
