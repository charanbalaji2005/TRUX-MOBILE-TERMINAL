#pragma once
// meridian-security / credential_store.hpp
//
// Spec §57: API keys must never be hardcoded, committed, or logged,
// and should prefer the Linux Secret Service / desktop keyring when
// available. This defines that interface plus the fallback actually
// usable in this build environment — see docs/security.md for exactly
// why the fallback is what it is (short version: this machine has the
// libsecret *runtime* library but not its development headers, and
// there's no network access to install them, so nothing here has been
// compiled against real Secret Service).
//
// Nothing in this repo currently calls store()/retrieve() for a real
// secret — no provider is implemented yet to have credentials for. This
// exists so that work can plug in directly once a provider does.

#include <optional>
#include <string>

namespace meridian::security {

class CredentialStore {
public:
    virtual ~CredentialStore() = default;
    virtual bool store(const std::string& key_name, const std::string& secret) = 0;
    virtual std::optional<std::string> retrieve(const std::string& key_name) = 0;
    virtual bool remove(const std::string& key_name) = 0;
    virtual std::string backend_name() const = 0;
};

// Fallback backend: stores each secret in its own file under
// `<dir>/<key_name>`, created with mode 0600 (owner read/write only).
// This is NOT OS-keyring-grade protection — no encryption at rest, no
// integration with the session keyring/login unlock, just filesystem
// permissions. It exists so the interface has a real, working
// implementation to test against; docs/security.md is explicit that a
// real Secret Service backend should replace this before any provider
// integration ships credentials through it for real users.
class FileCredentialStore : public CredentialStore {
public:
    explicit FileCredentialStore(std::string dir);

    bool store(const std::string& key_name, const std::string& secret) override;
    std::optional<std::string> retrieve(const std::string& key_name) override;
    bool remove(const std::string& key_name) override;
    std::string backend_name() const override { return "file (0600) — NOT OS-keyring-backed"; }

private:
    std::string path_for(const std::string& key_name) const;
    std::string dir_;
};

} // namespace meridian::security
