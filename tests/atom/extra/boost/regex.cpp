#include "atom/extra/boost/regex.hpp"

#include <gtest/gtest.h>

using namespace atom::extra::boost;

class RegexWrapperTest : public ::testing::Test {
protected:
    RegexWrapperTest() = default;
    ~RegexWrapperTest() override = default;
};

TEST_F(RegexWrapperTest, ConstructorAndPatternMatching) {
    RegexWrapper regex("abc");
    EXPECT_EQ(regex.getPattern(), "abc");
}

TEST_F(RegexWrapperTest, MatchFunction) {
    RegexWrapper regex("\\d+");
    EXPECT_TRUE(regex.match("123"));
    EXPECT_FALSE(regex.match("abc"));
}

TEST_F(RegexWrapperTest, SearchFunction) {
    RegexWrapper regex("\\d+");
    auto result = regex.search("abc123def");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "123");

    result = regex.search("abcdef");
    EXPECT_FALSE(result.has_value());
}

TEST_F(RegexWrapperTest, SearchAllFunction) {
    RegexWrapper regex("\\d+");
    auto results = regex.searchAll("abc123def456ghi");
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], "123");
    EXPECT_EQ(results[1], "456");
}

TEST_F(RegexWrapperTest, ReplaceFunction) {
    RegexWrapper regex("\\d+");
    auto result = regex.replace("abc123def456ghi", "X");
    EXPECT_EQ(result, "abcXdefXghi");
}

TEST_F(RegexWrapperTest, SplitFunction) {
    RegexWrapper regex("\\s+");
    auto results = regex.split("abc def ghi");
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], "abc");
    EXPECT_EQ(results[1], "def");
    EXPECT_EQ(results[2], "ghi");
}

TEST_F(RegexWrapperTest, MatchGroupsFunction) {
    RegexWrapper regex("(\\d+)-(\\d+)");
    auto results = regex.matchGroups("abc123-456def");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].first, "123-456");
    ASSERT_EQ(results[0].second.size(), 2);
    EXPECT_EQ(results[0].second[0], "123");
    EXPECT_EQ(results[0].second[1], "456");
}

TEST_F(RegexWrapperTest, ForEachMatchFunction) {
    RegexWrapper regex("\\d+");
    std::vector<std::string> matches;
    regex.forEachMatch("abc123def456ghi", [&](const ::boost::smatch& match) {
        matches.push_back(match.str());
    });
    ASSERT_EQ(matches.size(), 2);
    EXPECT_EQ(matches[0], "123");
    EXPECT_EQ(matches[1], "456");
}

TEST_F(RegexWrapperTest, NamedCapturesFunction) {
    RegexWrapper regex("(?<num>\\d+)");
    auto result = regex.namedCaptures("abc123def");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result["1"], "123");
}

TEST_F(RegexWrapperTest, IsValidFunction) {
    RegexWrapper regex("\\d+");
    EXPECT_TRUE(regex.isValid("123"));
    EXPECT_FALSE(regex.isValid("["));
}

TEST_F(RegexWrapperTest, ReplaceCallbackFunction) {
    RegexWrapper regex("\\d+");
    auto result = regex.replaceCallback(
        "abc123def456ghi",
        [](const ::boost::smatch& match) { return "[" + match.str() + "]"; });
    EXPECT_EQ(result, "abc[123]def[456]ghi");
}

TEST_F(RegexWrapperTest, EscapeStringFunction) {
    auto result = RegexWrapper::escapeString("a.b*c?");
    EXPECT_EQ(result, "a\\.b\\*c\\?");
}

TEST_F(RegexWrapperTest, BenchmarkMatchFunction) {
    RegexWrapper regex("\\d+");
    auto duration = regex.benchmarkMatch("123", 1000);
    EXPECT_GT(duration.count(), 0);
}

TEST_F(RegexWrapperTest, IsValidRegexFunction) {
    EXPECT_TRUE(RegexWrapper::isValidRegex("\\d+"));
    EXPECT_FALSE(RegexWrapper::isValidRegex("["));
}
