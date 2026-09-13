#include "mini_test.hpp"
#include "../src/dev/github_integration.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace meridian::dev;

MTEST(github_integration_status_and_key) {
    std::streambuf* orig_buf = std::cout.rdbuf();
    std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    int r1 = GitHubIntegration::handle_gh_command({"gh", "status"});
    ASSERT_EQ(r1, 0);
    ASSERT_TRUE(capture.str().find("GitHub Integration Status") != std::string::npos);

    capture.str("");
    int r2 = GitHubIntegration::handle_gh_command({"gh", "key"});
    ASSERT_EQ(r2, 0);
    ASSERT_TRUE(capture.str().find("Meridian SSH Public Key") != std::string::npos);

    std::cout.rdbuf(orig_buf);
}

MTEST(github_integration_repo_and_hints) {
    std::streambuf* orig_buf = std::cout.rdbuf();
    std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    int r1 = GitHubIntegration::handle_gh_command({"gh", "repo", "create", "demo-app"});
    ASSERT_EQ(r1, 0);
    ASSERT_TRUE(capture.str().find("demo-app") != std::string::npos);

    GitHubIntegration::set_hints_enabled(true);
    ASSERT_TRUE(GitHubIntegration::is_hints_enabled());

    GitHubIntegration::set_hints_enabled(false);
    ASSERT_FALSE(GitHubIntegration::is_hints_enabled());

    GitHubIntegration::set_hints_enabled(true);

    capture.str("");
    GitHubIntegration::on_post_command_hint("git clone git@github.com:user/nonexistent.git", 1);
    ASSERT_TRUE(capture.str().find("meridian gh connect") != std::string::npos);

    std::cout.rdbuf(orig_buf);
}

