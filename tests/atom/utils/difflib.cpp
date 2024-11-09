#ifndef ATOM_UTILS_TEST_DIFFLIB_HPP
#define ATOM_UTILS_TEST_DIFFLIB_HPP

#include <gtest/gtest.h>

#include "atom/utils/difflib.hpp"

using namespace atom::utils;

TEST(SequenceMatcherTest, Ratio) {
    SequenceMatcher matcher("hello", "hallo");
    EXPECT_NEAR(matcher.ratio(), 0.8, 0.01);
}

TEST(SequenceMatcherTest, SetSeqs) {
    SequenceMatcher matcher("hello", "world");
    matcher.setSeqs("hello", "hallo");
    EXPECT_NEAR(matcher.ratio(), 0.8, 0.01);
}

TEST(SequenceMatcherTest, GetMatchingBlocks) {
    SequenceMatcher matcher("hello", "hallo");
    auto blocks = matcher.getMatchingBlocks();
    ASSERT_EQ(blocks.size(), 3);
    EXPECT_EQ(blocks[0], std::make_tuple(0, 0, 1));
    EXPECT_EQ(blocks[1], std::make_tuple(2, 2, 3));
    EXPECT_EQ(blocks[2], std::make_tuple(5, 5, 0));
}

TEST(SequenceMatcherTest, GetOpcodes) {
    SequenceMatcher matcher("hello", "hallo");
    auto opcodes = matcher.getOpcodes();
    ASSERT_EQ(opcodes.size(), 3);
    EXPECT_EQ(opcodes[0], std::make_tuple("equal", 0, 1, 0, 1));
    EXPECT_EQ(opcodes[1], std::make_tuple("replace", 1, 2, 1, 2));
    EXPECT_EQ(opcodes[2], std::make_tuple("equal", 2, 5, 2, 5));
}

TEST(DifferTest, Compare) {
    std::vector<std::string> vec1 = {"line1", "line2", "line3"};
    std::vector<std::string> vec2 = {"line1", "lineX", "line3"};
    auto result = Differ::compare(vec1, vec2);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "  line1");
    EXPECT_EQ(result[1], "- line2");
    EXPECT_EQ(result[2], "+ lineX");
}

TEST(DifferTest, UnifiedDiff) {
    std::vector<std::string> vec1 = {"line1", "line2", "line3"};
    std::vector<std::string> vec2 = {"line1", "lineX", "line3"};
    auto result = Differ::unifiedDiff(vec1, vec2);
    ASSERT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], "--- a");
    EXPECT_EQ(result[1], "+++ b");
    EXPECT_EQ(result[2], "@@ -1,3 +1,3 @@");
    EXPECT_EQ(result[3], " line1");
    EXPECT_EQ(result[4], "-line2");
    EXPECT_EQ(result[5], "+lineX");
}

TEST(HtmlDiffTest, MakeFile) {
    std::vector<std::string> fromlines = {"line1", "line2", "line3"};
    std::vector<std::string> tolines = {"line1", "lineX", "line3"};
    auto result = HtmlDiff::makeFile(fromlines, tolines);
    EXPECT_NE(result.find("<html>"), std::string::npos);
    EXPECT_NE(result.find("<h2>Differences</h2>"), std::string::npos);
    EXPECT_NE(result.find("<td>  line1</td>"), std::string::npos);
    EXPECT_NE(result.find("<td>- line2</td>"), std::string::npos);
    EXPECT_NE(result.find("<td>+ lineX</td>"), std::string::npos);
}

TEST(HtmlDiffTest, MakeTable) {
    std::vector<std::string> fromlines = {"line1", "line2", "line3"};
    std::vector<std::string> tolines = {"line1", "lineX", "line3"};
    auto result = HtmlDiff::makeTable(fromlines, tolines);
    EXPECT_NE(result.find("<table"), std::string::npos);
    EXPECT_NE(result.find("<td>  line1</td>"), std::string::npos);
    EXPECT_NE(result.find("<td>- line2</td>"), std::string::npos);
    EXPECT_NE(result.find("<td>+ lineX</td>"), std::string::npos);
}

TEST(GetCloseMatchesTest, Basic) {
    std::vector<std::string> possibilities = {"hello", "hallo", "hullo"};
    auto matches = getCloseMatches("hello", possibilities);
    ASSERT_EQ(matches.size(), 3);
    EXPECT_EQ(matches[0], "hello");
    EXPECT_EQ(matches[1], "hallo");
    EXPECT_EQ(matches[2], "hullo");
}

#endif  // ATOM_UTILS_TEST_DIFFLIB_HPP
