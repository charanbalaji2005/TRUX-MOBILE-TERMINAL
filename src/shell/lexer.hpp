#pragma once
// meridian-shell / lexer.hpp
//
// Tokenizes one command line. Handles single/double quoting, backslash
// escapes, and the operator set from the spec: | ; & && || > >> < 2> 2>>
// &> &>>. This is a real tokenizer, not a whitespace split() — quoted
// operators and spaces inside words are handled correctly (see
// tests/test_shell_lexer.cpp).

#include "ast.hpp"
#include <string>
#include <vector>

namespace meridian::shell {

enum class TokenType {
    Word,
    Pipe, Semicolon, Background, And, Or,
    RedirOut, RedirAppend, RedirIn, RedirErr, RedirErrAppend, RedirAll, RedirAllAppend,
    End
};

struct Token {
    TokenType type;
    Word word; // populated only when type == Word
};

class Lexer {
public:
    explicit Lexer(std::string input);

    // Tokenizes the whole line. On a quoting error (unterminated ' or "),
    // returns an empty vector and sets *error.
    std::vector<Token> tokenize(std::string* error = nullptr);

private:
    std::string input_;
};

} // namespace meridian::shell
