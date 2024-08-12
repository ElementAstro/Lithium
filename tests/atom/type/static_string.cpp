#include <gtest/gtest.h>

#include "atom/type/static_string.hpp"

TEST(StaticStringTest, SizeTest) {
    StaticString s("hello");
    ASSERT_EQ(s.size(), 5);
}

TEST(StaticStringTest, CStrTest) {
    StaticString s("world");
    ASSERT_STREQ(s.cStr(), "world");
}

TEST(StaticStringTest, BeginTest) {
    StaticString s("test");
    ASSERT_EQ(*s.begin(), 't');
}

TEST(StaticStringTest, EndTest) {
    StaticString s("end");
    ASSERT_EQ(*(s.end() - 1), 'd');
}

TEST(StaticStringTest, EqualityTest) {
    StaticString s1("equal");
    std::string s2 = "equal";
    ASSERT_TRUE(s1 == s2);
}

TEST(StaticStringTest, InequalityTest) {
    StaticString s1("inequal");
    std::string s2 = "equal";
    ASSERT_TRUE(s1 != s2);
}

TEST(StaticStringTest, LessThanTest) {
    StaticString s1("less");
    std::string s2 = "more";
    ASSERT_TRUE(s1 < s2);
}

TEST(StaticStringTest, LessThanOrEqualToTest) {
    StaticString s1("less=");
    std::string s2 = "less=";
    ASSERT_TRUE(s1 <= s2);
}

TEST(StaticStringTest, GreaterThanTest) {
    StaticString s1("greater");
    std::string s2 = "less";
    ASSERT_TRUE(s1 > s2);
}

TEST(StaticStringTest, GreaterThanOrEqualToTest) {
    StaticString s1("greater=");
    std::string s2 = "greater=";
    ASSERT_TRUE(s1 >= s2);
}
