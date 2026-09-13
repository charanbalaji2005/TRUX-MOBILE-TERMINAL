// tests / test_shell_parser.cpp
#include "mini_test.hpp"
#include "../src/shell/lexer.hpp"
#include "../src/shell/parser.hpp"

using namespace meridian::shell;

static std::string wt(const Word& w) { std::string s; for (auto& p : w) s += p.text; return s; }

static Sequence parse_line(const std::string& line, std::string* err = nullptr) {
    Lexer lex(line);
    std::string lex_err;
    auto toks = lex.tokenize(&lex_err);
    if (!lex_err.empty()) { if (err) *err = lex_err; return {}; }
    Parser p(std::move(toks));
    return p.parse(err);
}

MTEST(parser_builds_single_command) {
    auto seq = parse_line("ls -la");
    ASSERT_EQ(seq.size(), (std::size_t)1);
    ASSERT_EQ(seq[0].pipeline.commands.size(), (std::size_t)1);
    ASSERT_EQ(seq[0].pipeline.commands[0].argv.size(), (std::size_t)2);
    ASSERT_EQ(wt(seq[0].pipeline.commands[0].argv[0]), std::string("ls"));
}

MTEST(parser_builds_pipeline) {
    auto seq = parse_line("cat file | grep foo | sort");
    ASSERT_EQ(seq.size(), (std::size_t)1);
    ASSERT_EQ(seq[0].pipeline.commands.size(), (std::size_t)3);
    ASSERT_EQ(wt(seq[0].pipeline.commands[1].argv[0]), std::string("grep"));
}

MTEST(parser_attaches_redirection_to_command) {
    auto seq = parse_line("sort < in.txt > out.txt");
    auto& cmd = seq[0].pipeline.commands[0];
    ASSERT_EQ(cmd.redirections.size(), (std::size_t)2);
    ASSERT_TRUE(cmd.redirections[0].type == RedirType::In);
    ASSERT_EQ(wt(cmd.redirections[0].target), std::string("in.txt"));
    ASSERT_TRUE(cmd.redirections[1].type == RedirType::Out);
}

MTEST(parser_sequence_connectors) {
    auto seq = parse_line("a && b || c ; d");
    ASSERT_EQ(seq.size(), (std::size_t)4);
    ASSERT_TRUE(seq[0].connector == Connector::And);
    ASSERT_TRUE(seq[1].connector == Connector::Or);
    ASSERT_TRUE(seq[2].connector == Connector::Semicolon);
    ASSERT_TRUE(seq[3].connector == Connector::None);
}

MTEST(parser_trailing_ampersand_marks_background) {
    auto seq = parse_line("sleep 10 &");
    ASSERT_EQ(seq.size(), (std::size_t)1);
    ASSERT_TRUE(seq[0].pipeline.background);
}

MTEST(parser_empty_line_is_empty_sequence_not_error) {
    std::string err;
    auto seq = parse_line("", &err);
    ASSERT_TRUE(seq.empty());
    ASSERT_TRUE(err.empty());
}

MTEST(parser_reports_error_on_leading_pipe) {
    std::string err;
    auto seq = parse_line("| ls", &err);
    ASSERT_TRUE(seq.empty());
    ASSERT_FALSE(err.empty());
}

MTEST(parser_reports_error_on_redirect_without_target) {
    std::string err;
    auto seq = parse_line("ls >", &err);
    ASSERT_TRUE(seq.empty());
    ASSERT_FALSE(err.empty());
}
