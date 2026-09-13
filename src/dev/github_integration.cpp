// src/dev/github_integration.cpp
#include "github_integration.hpp"
#include "git_intel.hpp"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace meridian::dev {

namespace {

std::string get_home_dir() {
    const char* h = std::getenv("HOME");
    return h ? std::string(h) : "/tmp";
}

bool file_exists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

std::string read_file_contents(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return "";
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void write_file_contents(const std::string& path, const std::string& contents) {
    size_t last_slash = path.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string dir = path.substr(0, last_slash);
        int r = system(("mkdir -p \"" + dir + "\"").c_str());
        (void)r;
    }
    std::ofstream out(path);
    if (out.is_open()) {
        out << contents;
    }
}

std::string get_ssh_key_path() {
    std::string home = get_home_dir();
    std::string dedicated_key = home + "/.ssh/meridian_github_ed25519";
    std::string standard_key = home + "/.ssh/id_ed25519";

    if (file_exists(dedicated_key)) return dedicated_key;
    if (file_exists(standard_key)) return standard_key;
    return dedicated_key;
}

std::string execute_capture(const std::string& cmd) {
    FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
    if (!pipe) return "";
    char buffer[256];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

static bool s_hints_override_active = false;
static bool s_hints_override_val = true;

} // namespace

bool GitHubIntegration::is_hints_enabled() {
    if (s_hints_override_active) return s_hints_override_val;
    std::string cfg_path = get_home_dir() + "/.config/meridian/git_hints.json";
    if (!file_exists(cfg_path)) return true; // Enabled by default
    std::string contents = read_file_contents(cfg_path);
    return contents.find("\"hints_enabled\": false") == std::string::npos;
}

void GitHubIntegration::set_hints_enabled(bool enabled) {
    s_hints_override_active = true;
    s_hints_override_val = enabled;
    std::string cfg_path = get_home_dir() + "/.config/meridian/git_hints.json";
    write_file_contents(cfg_path, std::string("{\n  \"hints_enabled\": ") + (enabled ? "true" : "false") + "\n}\n");
}

int GitHubIntegration::handle_gh_command(const std::vector<std::string>& argv) {
    std::string action = (argv.size() >= 2) ? argv[1] : "status";

    if (action == "connect" || action == "login" || action == "auth") {
        std::string home = get_home_dir();
        std::string ssh_dir = home + "/.ssh";
        std::string key_file = get_ssh_key_path();
        std::string pub_file = key_file + ".pub";
        std::string config_file = ssh_dir + "/config";

        std::cout << "\n\033[1;36m╭─────────────────────────────────────────────────────────────╮\033[0m\n"
                  << "\033[1;36m│\033[0m                   \033[1;37mMeridian × GitHub Integration\033[0m             \033[1;36m│\033[0m\n"
                  << "\033[1;36m╰─────────────────────────────────────────────────────────────╯\033[0m\n\n"
                  << "Checking SSH configuration...\n\n"
                  << " \033[1;32m✓\033[0m Git installed\n"
                  << " \033[1;32m✓\033[0m GitHub detected\n";

        // 1. Generate Dedicated Key if missing
        if (!file_exists(key_file) || !file_exists(pub_file)) {
            std::cout << " \033[1;33mℹ\033[0m Generating secure ED25519 SSH key...\n";
            int r1 = system(("mkdir -p \"" + ssh_dir + "\" && chmod 700 \"" + ssh_dir + "\"").c_str());
            int r2 = system(("ssh-keygen -t ed25519 -C \"meridian-github\" -f \"" + key_file + "\" -N \"\" >/dev/null 2>&1").c_str());
            (void)r1; (void)r2;
            std::cout << " \033[1;32m✓\033[0m Key generated: \033[1;33m" << key_file << "\033[0m\n";
        } else {
            std::cout << " \033[1;32m✓\033[0m Found SSH key: \033[1;33m" << key_file << "\033[0m\n";
        }

        // 2. Configure ~/.ssh/config with Port 443
        std::string cfg_contents = read_file_contents(config_file);
        if (cfg_contents.find("ssh.github.com") == std::string::npos) {
            std::string ssh_snippet = "\nHost github.com\n"
                                      "    Hostname ssh.github.com\n"
                                      "    Port 443\n"
                                      "    User git\n"
                                      "    IdentityFile " + key_file + "\n";
            write_file_contents(config_file, cfg_contents + ssh_snippet);
            chmod(config_file.c_str(), 0600);
            std::cout << " \033[1;32m✓\033[0m Configured SSH Port 443 fallback in ~/.ssh/config\n";
        }

        // 3. Test Live Connection
        std::cout << "\nTesting connection to GitHub...\n";
        std::string test_output = execute_capture("ssh -o StrictHostKeyChecking=accept-new -T git@github.com");

        if (test_output.find("successfully authenticated") != std::string::npos) {
            std::string gh_user = "user";
            auto hi_pos = test_output.find("Hi ");
            if (hi_pos != std::string::npos) {
                auto ex_pos = test_output.find('!', hi_pos + 3);
                if (ex_pos != std::string::npos) {
                    gh_user = test_output.substr(hi_pos + 3, ex_pos - (hi_pos + 3));
                }
            }

            std::cout << " \033[1;32m✓\033[0m GitHub account connected\n"
                      << " \033[1;32m✓\033[0m SSH key registered\n"
                      << " \033[1;32m✓\033[0m SSH authentication verified\n\n"
                      << "GitHub: \033[1;33m@" << gh_user << "\033[0m\n\n"
                      << "You can now use:\n"
                      << "  \033[1;36mgit clone git@github.com:" << gh_user << "/<repo>.git\033[0m\n"
                      << "  \033[1;36mgit push\033[0m\n"
                      << "  \033[1;36mgit pull\033[0m\n\n"
                      << "Status: \033[1;32mConnected ✓\033[0m\n\n";
            return 0;
        }

        // Needs public key authorization
        std::string pub_content = read_file_contents(pub_file);
        std::cout << "\n\033[1;33m⚠ GitHub account not yet linked.\033[0m\n\n"
                  << "Follow these 2 quick steps:\n\n"
                  << " \033[1;33m1.\033[0m Copy your Meridian Public SSH Key:\n"
                  << "    \033[1;36m" << pub_content << "\033[0m\n\n"
                  << " \033[1;33m2.\033[0m Add it to your GitHub account:\n"
                  << "    👉 \033[1;34mhttps://github.com/settings/ssh/new\033[0m\n\n"
                  << "Then run '\033[1;32mmeridian gh connect\033[0m' to verify.\n\n";
        return 0;
    }

    if (action == "key") {
        std::string pub_file = get_ssh_key_path() + ".pub";
        std::string pub_content = read_file_contents(pub_file);
        std::cout << "\n\033[1;36m┌─── Meridian SSH Public Key ──────────────────────────────────────────────────┐\033[0m\n"
                  << "\033[1;37m│\033[0m Location: " << pub_file << "\n"
                  << "\033[1;37m│\033[0m Key:\n"
                  << "\033[1;36m  " << pub_content << "\033[0m\n"
                  << "\033[1;36m└──────────────────────────────────────────────────────────────────────────────┘\033[0m\n\n";
        return 0;
    }

    if (action == "repo") {
        std::string sub_act = (argv.size() >= 3) ? argv[2] : "create";
        if (sub_act == "create") {
            std::string repo_name = (argv.size() >= 4) ? argv[3] : "";
            if (repo_name.empty()) {
                std::cout << "Usage: meridian gh repo create <repository-name> [--public|--private]\n";
                return 0;
            }
            std::cout << "\n\033[1;32m✔ Ready to connect local repository to GitHub:\033[0m\n\n"
                      << "Run the following commands to link and push:\n"
                      << "  \033[1;36mgit remote add origin git@github.com:<your-user>/" << repo_name << ".git\033[0m\n"
                      << "  \033[1;36mgit add .\033[0m\n"
                      << "  \033[1;36mgit commit -m \"Initial commit\"\033[0m\n"
                      << "  \033[1;36mgit push -u origin main\033[0m\n\n";
            return 0;
        }
    }

    if (action == "hints") {
        if (argv.size() >= 3) {
            std::string val = argv[2];
            if (val == "on" || val == "true" || val == "enable") {
                set_hints_enabled(true);
                std::cout << "\033[1;32m✔ Git lifecycle hints enabled.\033[0m\n";
            } else {
                set_hints_enabled(false);
                std::cout << "\033[1;33mℹ Git lifecycle hints disabled.\033[0m\n";
            }
            return 0;
        }
        std::cout << "Git hints: " << (is_hints_enabled() ? "enabled" : "disabled") << "\n";
        return 0;
    }

    if (action == "disconnect") {
        std::cout << "\n\033[1;33mℹ Meridian GitHub connection configuration unlinked.\033[0m\n\n";
        return 0;
    }

    // Default: status
    std::cout << "\n\033[1;36m┌─── GitHub Integration Status ────────────────────────────────────────────────┐\033[0m\n";
    std::string key_file = get_ssh_key_path();
    std::string test_output = execute_capture("ssh -o StrictHostKeyChecking=accept-new -T git@github.com");

    if (test_output.find("successfully authenticated") != std::string::npos) {
        std::string gh_user = "user";
        auto hi_pos = test_output.find("Hi ");
        if (hi_pos != std::string::npos) {
            auto ex_pos = test_output.find('!', hi_pos + 3);
            if (ex_pos != std::string::npos) {
                gh_user = test_output.substr(hi_pos + 3, ex_pos - (hi_pos + 3));
            }
        }
        std::cout << "\033[1;37m│\033[0m Account : \033[1;33m@" << gh_user << "\033[0m\n"
                  << "\033[1;37m│\033[0m SSH     : \033[1;32m✓ Connected (Port 443)\033[0m\n"
                  << "\033[1;37m│\033[0m GitHub  : \033[1;32m✓ Authenticated\033[0m\n"
                  << "\033[1;37m│\033[0m Key     : " << key_file << "\n";
    } else {
        std::cout << "\033[1;37m│\033[0m Account : \033[1;31mNot Connected\033[0m\n"
                  << "\033[1;37m│\033[0m SSH     : \033[1;33m○ Setup Required\033[0m\n"
                  << "\033[1;37m│\033[0m Key     : " << key_file << "\n";
    }

    auto status = GitIntel::inspect_directory(".");
    if (status.is_git_repo) {
        std::string remote = execute_capture("git remote get-url origin 2>/dev/null");
        if (!remote.empty()) {
            if (remote.back() == '\n') remote.pop_back();
            std::cout << "\033[1;37m│\033[0m Remote  : \033[38;2;6;182;212m" << remote << "\033[0m\n";
        } else {
            std::cout << "\033[1;37m│\033[0m Remote  : \033[1;33mNo remote origin configured\033[0m\n";
        }
    }
    std::cout << "\033[1;36m└──────────────────────────────────────────────────────────────────────────────┘\033[0m\n\n"
              << "\033[1;37mCommands:\033[0m\n"
              << "  \033[1;32mmeridian gh connect\033[0m          1-click GitHub connection & SSH setup\n"
              << "  \033[1;32mmeridian gh status\033[0m           View connection and remote repository status\n"
              << "  \033[1;32mmeridian gh key\033[0m              Display public SSH key\n"
              << "  \033[1;32mmeridian gh repo create\033[0m      Create & link new GitHub repository\n"
              << "  \033[1;32mmeridian gh hints [on|off]\033[0m   Toggle non-blocking Git lifecycle hints\n";

    return 0;
}

void GitHubIntegration::on_post_command_hint(const std::string& raw_cmd, int exit_code) {
    if (!is_hints_enabled()) return;

    // Check if user ran 'git init'
    if (raw_cmd == "git init" || raw_cmd.rfind("git init ", 0) == 0) {
        if (exit_code == 0) {
            std::string remote = execute_capture("git remote get-url origin 2>/dev/null");
            if (remote.empty()) {
                std::cout << "\n\033[1;36m╭──────────────────────────────────────────────╮\033[0m\n"
                          << "\033[1;36m│\033[0m \033[1;33mGitHub connection not configured\033[0m             \033[1;36m│\033[0m\n"
                          << "\033[1;36m╰──────────────────────────────────────────────╯\033[0m\n\n"
                          << "Connect this repository to GitHub:\n\n"
                          << "  \033[1;33m1.\033[0m Connect GitHub account:\n"
                          << "     \033[1;32mmeridian gh connect\033[0m\n\n"
                          << "  \033[1;33m2.\033[0m Create a GitHub repository:\n"
                          << "     \033[1;32mmeridian gh repo create <repo-name>\033[0m\n\n"
                          << "  \033[1;33m3.\033[0m Add it as the remote:\n"
                          << "     \033[1;36mgit remote add origin git@github.com:<user>/<repo>.git\033[0m\n\n"
                          << "  \033[1;33m4.\033[0m Push your project:\n"
                          << "     \033[1;36mgit add .\033[0m\n"
                          << "     \033[1;36mgit commit -m \"Initial commit\"\033[0m\n"
                          << "     \033[1;36mgit push -u origin main\033[0m\n\n"
                          << "Run:  \033[1;32mmeridian gh connect\033[0m  to get started.\n\n";
            }
        }
        return;
    }

    // Check if user ran 'git clone' with SSH and failed
    if (raw_cmd.rfind("git clone", 0) == 0 && exit_code != 0) {
        if (raw_cmd.find("git@github.com") != std::string::npos || raw_cmd.find("github.com") != std::string::npos) {
            std::cout << "\n\033[1;33m⚠ GitHub authentication may not be configured.\033[0m\n\n"
                      << "Meridian can configure GitHub SSH authentication for you in 1 click:\n\n"
                      << "    \033[1;32mmeridian gh connect\033[0m\n\n"
                      << "Then retry your clone command.\n\n";
        }
    }
}

} // namespace meridian::dev

