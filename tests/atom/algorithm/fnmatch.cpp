#include "atom/algorithm/fnmatch.hpp"
#include <gtest/gtest.h>

// Test for fnmatch
TEST(FnmatchTest, BasicMatch) {
    EXPECT_TRUE(atom::algorithm::fnmatch("foo*", "foobar"));
    EXPECT_FALSE(atom::algorithm::fnmatch("bar*", "foobar"));
}

TEST(FnmatchTest, MatchWithFlags) {
    int flags = 0;  // Add appropriate flag values
    EXPECT_TRUE(atom::algorithm::fnmatch("foo*", "foobar", flags));
    EXPECT_FALSE(atom::algorithm::fnmatch("bar*", "foobar", flags));
}

// Test for filter with single pattern
TEST(FilterTest, SinglePatternMatch) {
    std::vector<std::string> names = {"foo", "bar", "foobar", "foobaz"};
    EXPECT_TRUE(atom::algorithm::filter(names, "foo*"));
    EXPECT_FALSE(atom::algorithm::filter(names, "baz*"));
}

TEST(FilterTest, SinglePatternMatchWithFlags) {
    std::vector<std::string> names = {"foo", "bar", "foobar", "foobaz"};
    int flags = 0;  // Add appropriate flag values
    EXPECT_TRUE(atom::algorithm::filter(names, "foo*", flags));
    EXPECT_FALSE(atom::algorithm::filter(names, "baz*", flags));
}

// Test for filter with multiple patterns
TEST(FilterTest, MultiplePatternMatch) {
    std::vector<std::string> names = {"foo", "bar", "foobar", "foobaz"};
    std::vector<std::string> patterns = {"foo*", "bar*"};
    auto result = atom::algorithm::filter(names, patterns);
    std::vector<std::string> expected = {"foo", "bar", "foobar", "foobaz"};
    EXPECT_EQ(result, expected);
}

TEST(FilterTest, MultiplePatternMatchWithFlags) {
    std::vector<std::string> names = {"foo", "bar", "foobar", "foobaz"};
    std::vector<std::string> patterns = {"foo*", "baz*"};
    int flags = 0;  // Add appropriate flag values
    auto result = atom::algorithm::filter(names, patterns, flags);
    std::vector<std::string> expected = {"foo", "foobar", "foobaz"};
    EXPECT_EQ(result, expected);
}

// Test for translate
TEST(TranslateTest, BasicTranslation) {
    std::string pattern = "foo*";
    std::string result;
    EXPECT_TRUE(atom::algorithm::translate(pattern, result));
    EXPECT_EQ(result, "foo.*");  // Expected translated pattern
}

TEST(TranslateTest, TranslationWithFlags) {
    std::string pattern = "foo*";
    std::string result;
    int flags = 0;  // Add appropriate flag values
    EXPECT_TRUE(atom::algorithm::translate(pattern, result, flags));
    EXPECT_EQ(result, "foo*");  // Expected translated pattern
}