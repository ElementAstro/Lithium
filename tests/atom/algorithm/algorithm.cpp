#include "atom/algorithm/algorithm.hpp"
#include <gtest/gtest.h>
#include <unordered_set>

// Test KMP class
class KMPTest : public ::testing::Test {
protected:
    atom::algorithm::KMP kmp{"pattern"};

    void SetUp() override { kmp.setPattern("pattern"); }
};

TEST_F(KMPTest, TestEmptyText) {
    auto result = kmp.search("");
    EXPECT_TRUE(result.empty());
}

TEST_F(KMPTest, TestEmptyPattern) {
    atom::algorithm::KMP emptyPatternKmp{""};
    auto result = emptyPatternKmp.search("some text");
    EXPECT_TRUE(result.empty());
}

TEST_F(KMPTest, TestNoOccurrences) {
    auto result = kmp.search("no match here");
    EXPECT_TRUE(result.empty());
}

TEST_F(KMPTest, TestSingleOccurrence) {
    auto result = kmp.search("this pattern is here");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 5);
}

TEST_F(KMPTest, TestOverlappingOccurrences) {
    kmp.setPattern("ana");
    auto result = kmp.search("banana");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 3);
}

TEST_F(KMPTest, TestPatternEqualsText) {
    auto result = kmp.search("pattern");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0);
}

TEST_F(KMPTest, TestPatternLongerThanText) {
    auto result = kmp.search("short");
    EXPECT_TRUE(result.empty());
}

TEST_F(KMPTest, TestCaseSensitivity) {
    auto result = kmp.search("Pattern with different case");
    EXPECT_TRUE(result.empty());
}

TEST_F(KMPTest, TestSetNewPattern) {
    kmp.setPattern("new");
    auto result = kmp.search("this is a new pattern");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 10);
}

// Test BloomFilter class
TEST(BloomFilterTest, InsertAndContains) {
    atom::algorithm::BloomFilter<1000> bloomFilter(3);
    bloomFilter.insert("apple");
    bloomFilter.insert("banana");
    EXPECT_TRUE(bloomFilter.contains("apple"));
    EXPECT_TRUE(bloomFilter.contains("banana"));
    EXPECT_FALSE(bloomFilter.contains("cherry"));
}

class BoyerMooreTest : public ::testing::Test {
protected:
    atom::algorithm::BoyerMoore bm{"pattern"};

    void SetUp() override { bm.setPattern("pattern"); }
};

TEST_F(BoyerMooreTest, TestEmptyText) {
    auto result = bm.search("");
    EXPECT_TRUE(result.empty());
}

TEST_F(BoyerMooreTest, TestEmptyPattern) {
    atom::algorithm::BoyerMoore emptyPatternBm{""};
    auto result = emptyPatternBm.search("some text");
    EXPECT_TRUE(result.empty());
}

TEST_F(BoyerMooreTest, TestNoOccurrences) {
    auto result = bm.search("no match here");
    EXPECT_TRUE(result.empty());
}

TEST_F(BoyerMooreTest, TestSingleOccurrence) {
    auto result = bm.search("this pattern is here");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 5);
}

TEST_F(BoyerMooreTest, TestOverlappingOccurrences) {
    bm.setPattern("ana");
    auto result = bm.search("banana");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 3);
}

TEST_F(BoyerMooreTest, TestPatternEqualsText) {
    auto result = bm.search("pattern");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0);
}

TEST_F(BoyerMooreTest, TestPatternLongerThanText) {
    auto result = bm.search("short");
    EXPECT_TRUE(result.empty());
}

TEST_F(BoyerMooreTest, TestCaseSensitivity) {
    auto result = bm.search("Pattern with different case");
    EXPECT_TRUE(result.empty());
}

TEST_F(BoyerMooreTest, TestSetNewPattern) {
    bm.setPattern("new");
    auto result = bm.search("this is a new pattern");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 10);
}