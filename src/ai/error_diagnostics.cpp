#include "error_diagnostics.hpp"

#include <regex>
#include <sstream>

namespace meridian::ai {

std::string DiagnosticCard::format() const {
    if (empty()) return "No diagnostic errors detected in output.\n";
    std::ostringstream ss;
    ss << "┌─── AI Terminal Diagnostics ─────────────────────────────\n";
    ss << "│ Issue: " << title << "\n";
    ss << "│ ────────────────────────────────────────────────────────\n";
    ss << "│ What Happened:\n│   " << what_happened << "\n";
    ss << "│\n";
    ss << "│ Evidence:\n│   " << evidence << "\n";
    ss << "│\n";
    ss << "│ Suggested Fixes:\n";
    for (std::size_t i = 0; i < suggested_fixes.size(); ++i) {
        const auto& fix = suggested_fixes[i];
        ss << "│   [" << (i + 1) << "] " << fix.label << "\n";
        if (!fix.command.empty()) {
            ss << "│       $ " << fix.command << "\n";
        }
    }
    ss << "└─────────────────────────────────────────────────────────\n";
    return ss.str();
}

DiagnosticCard ErrorDiagnostics::analyze(const std::string& output) const {
    DiagnosticCard card;

    // 1. Node.js Cannot find module
    {
        std::regex r(R"(Cannot find module ['"]([^'"]+)['"])");
        std::smatch m;
        if (std::regex_search(output, m, r)) {
            std::string mod = m[1].str();
            card.category = DiagnosticCategory::NodeJs;
            card.title = "Node.js Missing Dependency: " + mod;
            card.what_happened = "Node.js failed to resolve package '" + mod + "' required by your application.";
            card.evidence = m[0].str();
            card.suggested_fixes.push_back({"Install package via npm", "npm install " + mod, true});
            card.suggested_fixes.push_back({"Install package via yarn", "yarn add " + mod, true});
            return card;
        }
    }

    // 2. Python ModuleNotFoundError
    {
        std::regex r(R"(ModuleNotFoundError: No module named ['"]([^'"]+)['"])");
        std::smatch m;
        if (std::regex_search(output, m, r)) {
            std::string mod = m[1].str();
            card.category = DiagnosticCategory::Python;
            card.title = "Python Missing Module: " + mod;
            card.what_happened = "Python runtime could not locate the imported module '" + mod + "'.";
            card.evidence = m[0].str();
            card.suggested_fixes.push_back({"Install module via pip", "pip install " + mod, true});
            card.suggested_fixes.push_back({"Install module into active venv", "python3 -m pip install " + mod, true});
            return card;
        }
    }

    // 3. Port conflict EADDRINUSE
    {
        std::regex r(R"((?:EADDRINUSE|Address already in use|bind: address already in use).*?(\d{2,5}))");
        std::smatch m;
        if (std::regex_search(output, m, r)) {
            std::string port = m[1].str();
            card.category = DiagnosticCategory::PortConflict;
            card.title = "TCP Port Conflict on Port " + port;
            card.what_happened = "Failed to bind server socket because port " + port + " is already occupied by another process.";
            card.evidence = m[0].str();
            card.suggested_fixes.push_back({"Inspect process occupying port", "lsof -i :" + port, true});
            card.suggested_fixes.push_back({"Kill process occupying port", "lsof -ti:" + port + " | xargs kill -9", false});
            return card;
        }
    }

    // 4. C++ Header not found
    {
        std::regex r(R"(fatal error:\s*([^:\n]+):\s*No such file or directory)");
        std::smatch m;
        if (std::regex_search(output, m, r)) {
            std::string header = m[1].str();
            card.category = DiagnosticCategory::CppCompiler;
            card.title = "C/C++ Missing Header: " + header;
            card.what_happened = "The C++ preprocessor could not find header file '" + header + "' in system or project include paths.";
            card.evidence = m[0].str();
            card.suggested_fixes.push_back({"Add include directory flag to compiler", "-I<include_dir>", true});
            card.suggested_fixes.push_back({"Search for matching development package", "dnf provides '*/" + header + "' || apt-file search " + header, true});
            return card;
        }
    }

    // 5. C++ Undefined reference (Linker error)
    {
        std::regex r(R"(undefined reference to [`']([^']+?)['])");
        std::smatch m;
        if (std::regex_search(output, m, r)) {
            std::string symbol = m[1].str();
            card.category = DiagnosticCategory::CppCompiler;
            card.title = "C/C++ Linker Undefined Reference: " + symbol;
            card.what_happened = "Compilation succeeded but the linker failed to find the definition for '" + symbol + "'.";
            card.evidence = m[0].str();
            card.suggested_fixes.push_back({"Link required library (e.g. -lpthread, -lutil, -lm)", "-l<library>", true});
            return card;
        }
    }

    // 6. Docker daemon offline
    {
        if (output.find("Cannot connect to the Docker daemon") != std::string::npos ||
            output.find("Is the docker daemon running") != std::string::npos) {
            card.category = DiagnosticCategory::Docker;
            card.title = "Docker Daemon Unreachable";
            card.what_happened = "The Docker CLI client cannot connect to unix:///var/run/docker.sock.";
            card.evidence = "Docker socket unreachable";
            card.suggested_fixes.push_back({"Start Docker service via systemd", "sudo systemctl start docker", true});
            card.suggested_fixes.push_back({"Check Docker service status", "sudo systemctl status docker", true});
            return card;
        }
    }

    // 7. Git merge conflict
    {
        if (output.find("CONFLICT (content): Merge conflict in") != std::string::npos) {
            card.category = DiagnosticCategory::Git;
            card.title = "Git Merge Conflict";
            card.what_happened = "Concurrent conflicting modifications detected during git merge or rebase.";
            card.evidence = "CONFLICT (content): Merge conflict";
            card.suggested_fixes.push_back({"Show conflicting files", "git status", true});
            card.suggested_fixes.push_back({"View diff markers in conflict files", "git diff", true});
            card.suggested_fixes.push_back({"Abort merge to restore previous state", "git merge --abort", true});
            return card;
        }
    }

    return card;
}

} // namespace meridian::ai

