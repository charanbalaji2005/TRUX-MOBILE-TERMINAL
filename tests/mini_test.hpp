#pragma once
// tests / mini_test.hpp
//
// A tiny, dependency-free test framework. googletest would normally be
// used here (see docs/status.md for why it isn't available in this
// build), but the assertion/registration/reporting behavior is the same
// shape: TEST-per-function, auto-registration via static initializers,
// ASSERT_* macros, one pass/fail summary at the end.

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace mtest {

inline std::vector<std::pair<std::string, std::function<void()>>>& registry() {
    static std::vector<std::pair<std::string, std::function<void()>>> r;
    return r;
}
inline int& failure_count() { static int f = 0; return f; }
inline int& assertion_count() { static int a = 0; return a; }

struct Registrar {
    Registrar(const std::string& name, std::function<void()> fn) { registry().emplace_back(name, std::move(fn)); }
};

inline void check(bool cond, const char* expr, const char* file, int line) {
    assertion_count()++;
    if (!cond) {
        failure_count()++;
        std::cerr << "    ASSERT FAILED: " << expr << "  (" << file << ":" << line << ")\n";
    }
}

inline int run_all() {
    int total = 0, failed_tests = 0;
    for (auto& [name, fn] : registry()) {
        int before = failure_count();
        std::cerr << "[ RUN  ] " << name << "\n";
        fn();
        total++;
        if (failure_count() > before) { failed_tests++; std::cerr << "[ FAIL ] " << name << "\n"; }
        else { std::cerr << "[  OK  ] " << name << "\n"; }
    }
    std::cerr << "\n" << total << " tests, " << assertion_count() << " assertions, "
              << failed_tests << " failed test(s), " << failure_count() << " failed assertion(s)\n";
    return failed_tests == 0 ? 0 : 1;
}

} // namespace mtest

#define MTEST(name)                                                        \
    void mtest_##name();                                                   \
    static mtest::Registrar mtest_reg_##name(#name, mtest_##name);         \
    void mtest_##name()

#define ASSERT_TRUE(cond) mtest::check((cond), #cond, __FILE__, __LINE__)
#define ASSERT_FALSE(cond) mtest::check(!(cond), "!(" #cond ")", __FILE__, __LINE__)
#define ASSERT_EQ(a, b) mtest::check((a) == (b), #a " == " #b, __FILE__, __LINE__)
#define ASSERT_NE(a, b) mtest::check((a) != (b), #a " != " #b, __FILE__, __LINE__)
#define ASSERT_GT(a, b) mtest::check((a) > (b), #a " > " #b, __FILE__, __LINE__)
#define ASSERT_GE(a, b) mtest::check((a) >= (b), #a " >= " #b, __FILE__, __LINE__)
#define ASSERT_LT(a, b) mtest::check((a) < (b), #a " < " #b, __FILE__, __LINE__)
#define ASSERT_LE(a, b) mtest::check((a) <= (b), #a " <= " #b, __FILE__, __LINE__)
