// meridian-shell / lexer.cpp
#include "lexer.hpp"

namespace meridian::shell {

Lexer::Lexer(std::string input) : input_(std::move(input)) {}

std::vector<Token> Lexer::tokenize(std::string* error) {
    std::vector<Token> tokens;
    const std::size_t len = input_.size();
    std::size_t pos = 0;

    Word current_word;
    std::string expand_buf;
    bool in_word = false;
    int subst_depth = 0; // nesting depth of unclosed, unquoted $( ... )

    auto flush_expand_buf = [&]() {
        if (!expand_buf.empty()) {
            current_word.push_back(WordPart{expand_buf, false});
            expand_buf.clear();
        }
    };
    auto push_literal_char = [&](char c) {
        flush_expand_buf();
        current_word.push_back(WordPart{std::string(1, c), true});
    };
    auto finish_word = [&]() {
        flush_expand_buf();
        if (!current_word.empty()) {
            tokens.push_back(Token{TokenType::Word, current_word});
            current_word.clear();
        }
        in_word = false;
    };

    while (pos < len) {
        char c = input_[pos];

        if (!in_word) {
            if (c == ' ' || c == '\t') { pos++; continue; }
            if (c == '#') { break; } // comment: rest of line ignored
            if (c == '\n') { finish_word(); tokens.push_back(Token{TokenType::Semicolon, {}}); pos++; continue; }
            if (c == '|') {
                if (pos + 1 < len && input_[pos + 1] == '|') { tokens.push_back(Token{TokenType::Or, {}}); pos += 2; }
                else { tokens.push_back(Token{TokenType::Pipe, {}}); pos += 1; }
                continue;
            }
            if (c == '&') {
                if (pos + 1 < len && input_[pos + 1] == '&') { tokens.push_back(Token{TokenType::And, {}}); pos += 2; }
                else if (pos + 1 < len && input_[pos + 1] == '>') {
                    if (pos + 2 < len && input_[pos + 2] == '>') { tokens.push_back(Token{TokenType::RedirAllAppend, {}}); pos += 3; }
                    else { tokens.push_back(Token{TokenType::RedirAll, {}}); pos += 2; }
                } else { tokens.push_back(Token{TokenType::Background, {}}); pos += 1; }
                continue;
            }
            if (c == ';') { tokens.push_back(Token{TokenType::Semicolon, {}}); pos += 1; continue; }
            if (c == '>') {
                if (pos + 1 < len && input_[pos + 1] == '>') { tokens.push_back(Token{TokenType::RedirAppend, {}}); pos += 2; }
                else { tokens.push_back(Token{TokenType::RedirOut, {}}); pos += 1; }
                continue;
            }
            if (c == '<') { tokens.push_back(Token{TokenType::RedirIn, {}}); pos += 1; continue; }
            if (c == '2' && pos + 1 < len && input_[pos + 1] == '>') {
                if (pos + 2 < len && input_[pos + 2] == '>') { tokens.push_back(Token{TokenType::RedirErrAppend, {}}); pos += 3; }
                else { tokens.push_back(Token{TokenType::RedirErr, {}}); pos += 2; }
                continue;
            }
            in_word = true; // fall through: this char starts a word
        }

        // --- word-building region ---

        // Track $( ... ) nesting so that spaces and operator characters
        // *inside* an unquoted command substitution don't end the word —
        // "echo $(echo a b)" must lex as two words (echo, $(echo a b)),
        // not four.
        if (c == '$' && pos + 1 < len && input_[pos + 1] == '(') {
            expand_buf.push_back('$');
            expand_buf.push_back('(');
            pos += 2;
            subst_depth++;
            continue;
        }
        if (subst_depth > 0) {
            if (c == '(') { expand_buf.push_back(c); pos++; subst_depth++; continue; }
            if (c == ')') { expand_buf.push_back(c); pos++; subst_depth--; continue; }
            if (c != '\'' && c != '"' && c != '\\') {
                // Any other character (including whitespace and the
                // operator characters) while inside an unclosed $(...):
                // accumulate literally rather than treating it as a
                // delimiter. Quotes/backslash still fall through to the
                // normal handling below so quoting still works inside a
                // substitution, e.g. $(echo "a b").
                expand_buf.push_back(c);
                pos++;
                continue;
            }
        }

        if (c == '\'') {
            pos++; // consume opening quote
            std::string lit;
            while (pos < len && input_[pos] != '\'') { lit.push_back(input_[pos]); pos++; }
            if (pos >= len) { if (error) *error = "unterminated single quote"; return {}; }
            pos++; // consume closing quote
            flush_expand_buf();
            current_word.push_back(WordPart{lit, true});
            continue;
        }
        if (c == '"') {
            pos++; // consume opening quote
            while (pos < len && input_[pos] != '"') {
                char dc = input_[pos];
                if (dc == '\\' && pos + 1 < len &&
                    (input_[pos + 1] == '"' || input_[pos + 1] == '\\' || input_[pos + 1] == '$')) {
                    push_literal_char(input_[pos + 1]);
                    pos += 2;
                    continue;
                }
                expand_buf.push_back(dc);
                pos++;
            }
            if (pos >= len) { if (error) *error = "unterminated double quote"; return {}; }
            pos++; // consume closing quote
            continue;
        }
        if (c == '\\') {
            if (pos + 1 < len) { push_literal_char(input_[pos + 1]); pos += 2; continue; }
            pos++; // trailing backslash at end of input: drop it
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\n' || c == '|' || c == '&' || c == ';' || c == '>' || c == '<') {
            finish_word();
            continue; // reprocess this char now that in_word == false
        }
        expand_buf.push_back(c);
        pos++;
    }

    if (subst_depth > 0) {
        if (error) *error = "unterminated command substitution: missing ')'";
        return {};
    }

    finish_word();
    tokens.push_back(Token{TokenType::End, {}});
    return tokens;
}

} // namespace meridian::shell
