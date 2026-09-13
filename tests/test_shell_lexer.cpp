// tests / test_shell_lexer.cpp
#include "mini_test.hpp"
#include "../src/shell/lexer.hpp"

using namespace meridian::shell;

static std::string word_text(const Word& w) {
    std::string s;
    for (auto& part : w) s += part.text;
    return s;
}

MTEST(lexer_splits_simple_words) {
    Lexer lex("ls -la /tmp");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)4); // 3 words + End
    ASSERT_TRUE(toks[0].type == TokenType::Word);
    ASSERT_EQ(word_text(toks[0].word), std::string("ls"));
    ASSERT_EQ(word_text(toks[1].word), std::string("-la"));
    ASSERT_EQ(word_text(toks[2].word), std::string("/tmp"));
    ASSERT_TRUE(toks[3].type == TokenType::End);
}

MTEST(lexer_single_quotes_are_literal) {
    Lexer lex("echo 'a b | c'");
    auto toks = lex.tokenize();
    ASSERT_EQ(word_text(toks[1].word), std::string("a b | c"));
    ASSERT_TRUE(toks[1].word[0].literal);
}

MTEST(lexer_double_quotes_stay_one_word_but_expandable) {
    Lexer lex("echo \"hi there\"");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, word, End
    ASSERT_EQ(word_text(toks[1].word), std::string("hi there"));
    ASSERT_FALSE(toks[1].word[0].literal);
}

MTEST(lexer_recognizes_pipe_and_operators) {
    Lexer lex("a | b && c || d ; e &");
    auto toks = lex.tokenize();
    std::vector<TokenType> types;
    for (auto& t : toks) types.push_back(t.type);
    std::vector<TokenType> expected = {
        TokenType::Word, TokenType::Pipe, TokenType::Word, TokenType::And, TokenType::Word,
        TokenType::Or, TokenType::Word, TokenType::Semicolon, TokenType::Word, TokenType::Background, TokenType::End
    };
    ASSERT_EQ(types.size(), expected.size());
    for (std::size_t i = 0; i < types.size(); ++i) ASSERT_TRUE(types[i] == expected[i]);
}

MTEST(lexer_recognizes_redirections) {
    Lexer lex("cmd > out.txt 2>> err.txt < in.txt &> both.txt");
    auto toks = lex.tokenize();
    std::vector<TokenType> types;
    for (auto& t : toks) types.push_back(t.type);
    ASSERT_TRUE(types[1] == TokenType::RedirOut);
    ASSERT_TRUE(types[3] == TokenType::RedirErrAppend);
    ASSERT_TRUE(types[5] == TokenType::RedirIn);
    ASSERT_TRUE(types[7] == TokenType::RedirAll);
}

MTEST(lexer_backslash_escapes_next_char) {
    Lexer lex("echo a\\ b");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, "a b", End
    ASSERT_EQ(word_text(toks[1].word), std::string("a b"));
}

MTEST(lexer_comment_ignored_to_end_of_line) {
    Lexer lex("echo hi # this is a comment | not a pipe");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, hi, End
}

MTEST(lexer_reports_unterminated_quote) {
    Lexer lex("echo 'unterminated");
    std::string err;
    auto toks = lex.tokenize(&err);
    ASSERT_TRUE(toks.empty());
    ASSERT_FALSE(err.empty());
}

MTEST(lexer_command_substitution_spans_spaces_as_one_word) {
    // Regression test: spaces *inside* an unquoted $(...) must not split
    // the word, even though bare spaces normally do.
    Lexer lex("echo [$(echo inner)]");
    auto toks = lex.tokenize();
    ASSERT_EQ(toks.size(), (std::size_t)3); // echo, "[$(echo inner)]", End
    ASSERT_EQ(word_text(toks[1].word), std::string("[$(echo inner)]"));
}

MTEST(lexer_reports_unterminated_command_substitution) {
    std::string err;
    auto toks = Lexer("echo $(echo unterminated").tokenize(&err);
    ASSERT_TRUE(toks.empty());
    ASSERT_FALSE(err.empty());
}

MTEST(lexer_stderr_redirect_only_at_token_boundary) {
    // "file2" is a plain word (ends in a digit but is not just "2"), so
    // this must lex as WORD("file2") RedirOut WORD("out"), not as a
    // stderr redirection.
    Lexer lex("cmd file2>out");
    auto toks = lex.tokenize();
    ASSERT_EQ(word_text(toks[1].word), std::string("file2"));
    ASSERT_TRUE(toks[2].type == TokenType::RedirOut);
}
