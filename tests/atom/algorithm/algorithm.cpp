#include "atom/algorithm/algorithm.hpp"
#include <gtest/gtest.h>
#include <unordered_set>

// Test KMP class
TEST(KMPTest, SearchPattern) {
    atom::algorithm::KMP kmp("ABABC");
    auto result = kmp.Search("ABABABCABABABCABABC");
    std::vector<int> expected = {2, 8, 14};
    EXPECT_EQ(result, expected);
}

TEST(KMPTest, SetPattern) {
    atom::algorithm::KMP kmp("ABABC");
    kmp.SetPattern("AB");
    auto result = kmp.Search("ABABABCABABABCABABC");
    std::vector<int> expected = {0, 2, 4, 6, 8, 10, 12, 14};
    EXPECT_EQ(result, expected);
}

// Test MinHash class
TEST(MinHashTest, ComputeSignature) {
    atom::algorithm::MinHash minHash(100);
    std::unordered_set<std::string> set = {"apple", "banana", "cherry"};
    auto signature = minHash.compute_signature(set);
    EXPECT_EQ(signature.size(), 100);
}

TEST(MinHashTest, EstimateSimilarity) {
    atom::algorithm::MinHash minHash(100);
    std::unordered_set<std::string> set1 = {"apple", "banana", "cherry"};
    std::unordered_set<std::string> set2 = {"banana", "cherry", "date"};
    auto signature1 = minHash.compute_signature(set1);
    auto signature2 = minHash.compute_signature(set2);
    double similarity = minHash.estimate_similarity(signature1, signature2);
    EXPECT_GE(similarity, 0.0);
    EXPECT_LE(similarity, 1.0);
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

// Test BoyerMoore class
TEST(BoyerMooreTest, SearchPattern) {
    atom::algorithm::BoyerMoore bm("ABABC");
    auto result = bm.Search("ABABABCABABABCABABC");
    std::vector<int> expected = {2, 8, 14};
    EXPECT_EQ(result, expected);
}

TEST(BoyerMooreTest, SetPattern) {
    atom::algorithm::BoyerMoore bm("ABABC");
    bm.SetPattern("AB");
    auto result = bm.Search("ABABABCABABABCABABC");
    std::vector<int> expected = {0, 2, 4, 6, 8, 10, 12, 14};
    EXPECT_EQ(result, expected);
}
