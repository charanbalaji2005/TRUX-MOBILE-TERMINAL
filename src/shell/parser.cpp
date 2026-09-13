// meridian-shell / parser.cpp
#include "parser.hpp"

namespace meridian::shell {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {
    if (tokens_.empty()) tokens_.push_back(Token{TokenType::End, {}});
}

const Token& Parser::peek() const { return tokens_[pos_]; }

const Token& Parser::advance() {
    const Token& t = tokens_[pos_];
    if (pos_ + 1 < tokens_.size()) pos_++;
    return t;
}

static RedirType redir_type_for(TokenType t) {
    switch (t) {
        case TokenType::RedirOut: return RedirType::Out;
        case TokenType::RedirAppend: return RedirType::Append;
        case TokenType::RedirIn: return RedirType::In;
        case TokenType::RedirErr: return RedirType::ErrOut;
        case TokenType::RedirErrAppend: return RedirType::ErrAppend;
        case TokenType::RedirAll: return RedirType::All;
        case TokenType::RedirAllAppend: return RedirType::AllAppend;
        default: return RedirType::Out;
    }
}

static bool is_redir_token(TokenType t) {
    switch (t) {
        case TokenType::RedirOut:
        case TokenType::RedirAppend:
        case TokenType::RedirIn:
        case TokenType::RedirErr:
        case TokenType::RedirErrAppend:
        case TokenType::RedirAll:
        case TokenType::RedirAllAppend:
            return true;
        default:
            return false;
    }
}

bool Parser::parse_command(Command* out, std::string* error) {
    bool got_anything = false;
    while (true) {
        TokenType t = peek().type;
        if (t == TokenType::Word) {
            out->argv.push_back(peek().word);
            advance();
            got_anything = true;
        } else if (is_redir_token(t)) {
            RedirType rt = redir_type_for(t);
            advance();
            if (peek().type != TokenType::Word) {
                if (error) *error = "syntax error: expected a filename after redirection operator";
                return false;
            }
            out->redirections.push_back(Redirection{rt, peek().word});
            advance();
            got_anything = true;
        } else {
            break;
        }
    }
    if (!got_anything) {
        if (error) *error = "syntax error: expected a command";
        return false;
    }
    return true;
}

bool Parser::parse_pipeline(Pipeline* out, std::string* error) {
    Command first;
    if (!parse_command(&first, error)) return false;
    out->commands.push_back(std::move(first));

    while (peek().type == TokenType::Pipe) {
        advance();
        Command next;
        if (!parse_command(&next, error)) return false;
        out->commands.push_back(std::move(next));
    }
    return true;
}

Sequence Parser::parse(std::string* error) {
    Sequence seq;
    if (peek().type == TokenType::End) return seq; // empty line -> empty sequence, not an error

    while (peek().type != TokenType::End) {
        SequenceItem item;
        if (!parse_pipeline(&item.pipeline, error)) return {};

        switch (peek().type) {
            case TokenType::Semicolon: item.connector = Connector::Semicolon; advance(); break;
            case TokenType::And:       item.connector = Connector::And;       advance(); break;
            case TokenType::Or:        item.connector = Connector::Or;        advance(); break;
            case TokenType::Background:
                item.pipeline.background = true;
                item.connector = Connector::Semicolon;
                advance();
                break;
            case TokenType::End:
                item.connector = Connector::None;
                break;
            default:
                if (error) *error = "syntax error: unexpected token";
                return {};
        }
        seq.push_back(std::move(item));
    }
    return seq;
}

} // namespace meridian::shell
