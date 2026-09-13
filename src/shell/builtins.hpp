#pragma once
// meridian-shell / builtins.hpp
//
// Builtins that must affect the shell's own process state (cd, export,
// exit, ...) and therefore cannot be implemented as external programs.

#include <string>
#include <vector>

namespace meridian::shell {

class Executor; // forward decl to avoid a header cycle

bool is_builtin(const std::string& name);
std::vector<std::string> get_builtin_names();

// Runs a builtin. `argv` is already word-expanded. Returns the exit
// status. Safe to call either in-process (lone builtin, no redirection)
// or inside a forked child (builtin used in a pipeline or with
// redirection) — callers decide which via Executor.
int run_builtin(const std::string& name, const std::vector<std::string>& argv, Executor& ctx);

} // namespace meridian::shell
