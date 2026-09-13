// meridian-security / credential_store.cpp
#include "credential_store.hpp"

#include <cctype>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace meridian::security {

namespace {

void mkdir_p(const std::string& dir) {
    if (dir.empty()) return;
    std::string partial;
    std::size_t start = (dir[0] == '/') ? 1 : 0;
    if (dir[0] == '/') partial = "/";
    std::size_t pos = start;
    while (pos <= dir.size()) {
        auto slash = dir.find('/', pos);
        std::string component = dir.substr(pos, slash == std::string::npos ? std::string::npos : slash - pos);
        if (!component.empty()) {
            partial += component;
            mkdir(partial.c_str(), 0700);
            partial += "/";
        }
        if (slash == std::string::npos) break;
        pos = slash + 1;
    }
}

// Only [A-Za-z0-9_.-] survive; everything else becomes '_'. Prevents a
// key_name containing '/' or ".." from escaping the credentials
// directory (e.g. a provider name that somehow contained a path
// separator could otherwise write outside `dir_`).
std::string sanitize(const std::string& key_name) {
    std::string out;
    out.reserve(key_name.size());
    for (char c : key_name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.') out.push_back(c);
        else out.push_back('_');
    }
    if (out.empty()) out = "_";
    return out;
}

} // namespace

FileCredentialStore::FileCredentialStore(std::string dir) : dir_(std::move(dir)) {}

std::string FileCredentialStore::path_for(const std::string& key_name) const {
    return dir_ + "/" + sanitize(key_name);
}

bool FileCredentialStore::store(const std::string& key_name, const std::string& secret) {
    mkdir_p(dir_);
    std::string path = path_for(key_name);
    // Create with mode 0600 directly via open(), rather than creating
    // the file with default permissions and chmod'ing afterward — the
    // latter leaves a real (if brief) window where the file exists
    // world/group-readable before the chmod lands. open()'s mode
    // argument can only have bits *cleared* by umask, never added, so
    // requesting 0600 here guarantees no group/other bits are ever set.
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) return false;
    std::size_t written = 0;
    while (written < secret.size()) {
        ssize_t n = ::write(fd, secret.data() + written, secret.size() - written);
        if (n <= 0) { close(fd); return false; }
        written += static_cast<std::size_t>(n);
    }
    return close(fd) == 0;
}

std::optional<std::string> FileCredentialStore::retrieve(const std::string& key_name) {
    std::ifstream f(path_for(key_name));
    if (!f.is_open()) return std::nullopt;
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool FileCredentialStore::remove(const std::string& key_name) {
    std::string path = path_for(key_name);
    if (::access(path.c_str(), F_OK) != 0) return true; // already gone
    return ::unlink(path.c_str()) == 0;
}

} // namespace meridian::security
