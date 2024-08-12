#include "atom/algorithm/fnmatch.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

TEST(FnmatchTest, SimplePatternMatch) {
    EXPECT_TRUE(fnmatch("foo*", "foobar"));
    EXPECT_FALSE(fnmatch("bar*", "foobar"));
}

TEST(FnmatchTest, QuestionMarkPattern) {
    EXPECT_TRUE(fnmatch("foo?", "fooz"));
    EXPECT_FALSE(fnmatch("foo?", "foobar"));
}

TEST(FnmatchTest, CharacterClassPattern) {
    EXPECT_TRUE(fnmatch("foo[ab]", "fooa"));
    EXPECT_FALSE(fnmatch("foo[ab]", "fooc"));
}

TEST(FnmatchTest, CharacterClassPatternWithRange) {
    EXPECT_TRUE(fnmatch("foo[a-c]", "foob"));
    EXPECT_FALSE(fnmatch("foo[a-c]", "fooe"));
}

TEST(FnmatchTest, FilterSinglePattern) {
    std::vector<std::string> names = {"foo", "bar", "baz"};
    EXPECT_TRUE(filter(names, "ba*"));
    EXPECT_FALSE(filter(names, "qux*"));
}

TEST(FnmatchTest, FilterMultiplePatterns) {
    std::vector<std::string> names = {"foo", "bar", "baz"};
    std::vector<std::string> patterns = {"fo*", "ba*"};
    auto result = filter(names, patterns);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "foo");
    EXPECT_EQ(result[1], "bar");
    EXPECT_EQ(result[2], "baz");
}

TEST(TranslateTest, SimpleTranslation) {
    std::string pattern = "foo*";
    std::string result;
    EXPECT_TRUE(translate(pattern, result));
    EXPECT_EQ(result, "foo.*");
}

TEST(TranslateTest, QuestionMarkTranslation) {
    std::string pattern = "foo?";
    std::string result;
    EXPECT_TRUE(translate(pattern, result));
    EXPECT_EQ(result, "foo.");
}

TEST(TranslateTest, CharacterClassTranslation) {
    std::string pattern = "foo[ab]";
    std::string result;
    EXPECT_TRUE(translate(pattern, result));
    EXPECT_EQ(result, "foo[ab]");
}

TEST(TranslateTest, CharacterClassWithRangeTranslation) {
    std::string pattern = "foo[a-c]";
    std::string result;
    EXPECT_TRUE(translate(pattern, result));
    EXPECT_EQ(result, "foo[a-c]");
}

TEST(TranslateTest, TranslationWithFlags) {
    std::string pattern = "foo*";
    std::string result;
    int flags = 0;
    EXPECT_TRUE(translate(pattern, result, flags));
    EXPECT_EQ(result, "foo.*");
}
