#include "atom/utils/switch.hpp"
#include <gtest/gtest.h>

using namespace atom::utils;

TEST(StringSwitchTest, RegisterCase) {
    StringSwitch<int> switcher;
    switcher.registerCase("case1", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    auto cases = switcher.getCases();
    ASSERT_EQ(cases.size(), 1);
    ASSERT_EQ(cases[0], "case1");
}

TEST(StringSwitchTest, RegisterDuplicateCase) {
    StringSwitch<int> switcher;
    switcher.registerCase("case1", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    ASSERT_THROW(
        switcher.registerCase(
            "case1",
            [](int x) {
                return std::variant<std::monostate, int, std::string>(x);
            }),
        std::runtime_error);
}

TEST(StringSwitchTest, UnregisterCase) {
    StringSwitch<int> switcher;
    switcher.registerCase("case1", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    switcher.unregisterCase("case1");
    auto cases = switcher.getCases();
    ASSERT_TRUE(cases.empty());
}

TEST(StringSwitchTest, MatchCase) {
    StringSwitch<int> switcher;
    switcher.registerCase("case1", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    auto result = switcher.match("case1", 42);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::get<int>(result.value()), 42);
}

TEST(StringSwitchTest, MatchUnregisteredCase) {
    StringSwitch<int> switcher;
    auto result = switcher.match("case1", 42);
    ASSERT_FALSE(result.has_value());
}

TEST(StringSwitchTest, DefaultFunction) {
    StringSwitch<int> switcher;
    switcher.setDefault([](int x) {
        return std::variant<std::monostate, int, std::string>(x * 2);
    });
    auto result = switcher.match("case1", 21);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::get<int>(result.value()), 42);
}

TEST(StringSwitchTest, ClearCases) {
    StringSwitch<int> switcher;
    switcher.registerCase("case1", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    switcher.clearCases();
    auto cases = switcher.getCases();
    ASSERT_TRUE(cases.empty());
}

TEST(StringSwitchTest, GetCases) {
    StringSwitch<int> switcher;
    switcher.registerCase("case1", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    switcher.registerCase("case2", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    auto cases = switcher.getCases();
    ASSERT_EQ(cases.size(), 2);
    ASSERT_EQ(cases[0], "case1");
    ASSERT_EQ(cases[1], "case2");
}

TEST(StringSwitchTest, MatchWithSpan) {
    StringSwitch<int> switcher;
    switcher.registerCase("case1", [](int x) {
        return std::variant<std::monostate, int, std::string>(x);
    });
    std::vector<int> args = {42};
    auto result = switcher.matchWithSpan("case1", std::span<int>(args));
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::get<int>(result.value()), 42);
}

TEST(StringSwitchTest, InitializerList) {
    StringSwitch<int> switcher(
        {{"case1",
          [](int x) {
              return std::variant<std::monostate, int, std::string>(x);
          }},
         {"case2", [](int x) {
              return std::variant<std::monostate, int, std::string>(x * 2);
          }}});
    auto cases = switcher.getCases();
    ASSERT_EQ(cases.size(), 2);
    ASSERT_EQ(cases[0], "case1");
    ASSERT_EQ(cases[1], "case2");
    auto result1 = switcher.match("case1", 21);
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(std::get<int>(result1.value()), 21);
    auto result2 = switcher.match("case2", 21);
    ASSERT_TRUE(result2.has_value());
    ASSERT_EQ(std::get<int>(result2.value()), 42);
}