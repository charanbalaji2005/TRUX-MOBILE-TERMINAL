#pragma once
// meridian-shell / ast.hpp
//
// AST for Meridian Shell. Words are kept as segments (literal vs
// expandable) rather than fully-resolved strings, so expansion — $VAR,
// ${VAR}, $(command substitution) — happens lazily at execution time.
// This matters for correctness: with &&/||, an unevaluated branch must
// never run its command substitutions.

#include <string>
#include <vector>

namespace meridian::shell {

// A word segment. `literal == true` means "print this text verbatim,
// never expand it" — that's what single-quoted text and backslash-escaped
// characters produce. `literal == false` means "scan this text for
// $VAR / ${VAR} / $(...) and expand them".
struct WordPart {
    std::string text;
    bool literal = false;
};
using Word = std::vector<WordPart>;

enum class RedirType { In, Out, Append, ErrOut, ErrAppend, All, AllAppend };

struct Redirection {
    RedirType type;
    Word target;
};

// One command within a pipeline.
struct Command {
    std::vector<Word> argv;
    std::vector<Redirection> redirections;
};

// One or more commands connected by '|'.
struct Pipeline {
    std::vector<Command> commands;
    bool background = false; // trailing '&'
};

enum class Connector { None, Semicolon, And, Or };

struct SequenceItem {
    Pipeline pipeline;
    Connector connector = Connector::None; // how this item connects to the NEXT one
};

using Sequence = std::vector<SequenceItem>;

} // namespace meridian::shell
