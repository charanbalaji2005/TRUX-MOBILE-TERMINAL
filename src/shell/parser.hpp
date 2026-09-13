#pragma once
// meridian-shell / parser.hpp
//
// Recursive-descent parser: Token stream -> Sequence AST. Never uses
// string-splitting; every operator is structural.
//
// Grammar:
//   sequence  := seq_item ((';' | '&&' | '||' | '&') seq_item)*
//   seq_item  := pipeline
//   pipeline  := command ('|' command)*
//   command   := (WORD | redir WORD)+
//   redir     := '>' | '>>' | '<' | '2>' | '2>>' | '&>' | '&>>'

#include "ast.hpp"
#include "lexer.hpp"
#include <vector>

namespace meridian::shell {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Returns the parsed sequence. On syntax error, sets *error and
    // returns an empty sequence.
    Sequence parse(std::string* error = nullptr);

private:
    const Token& peek() const;
    const Token& advance();

    bool parse_command(Command* out, std::string* error);
    bool parse_pipeline(Pipeline* out, std::string* error);

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
};

} // namespace meridian::shell
