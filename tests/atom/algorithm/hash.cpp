#include "atom/algorithm/hash.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

// Helper function to test FNV-1a hash
void test_fnv1a_hash(const std::string& input, uint32_t expected) {
    EXPECT_EQ(fnv1a_hash(input), expected);
    EXPECT_EQ(fnv1a_hash(std::string_view(input)), expected);
}

// Helper function to test Jenkins One-at-a-Time hash
void test_jenkins_hash(const std::string& input, uint32_t expected) {
    EXPECT_EQ(jenkins_one_at_a_time_hash(input), expected);
    EXPECT_EQ(jenkins_one_at_a_time_hash(std::string_view(input)), expected);
}

TEST(HashTest, ComputeHashSingleValue) {
    EXPECT_EQ(computeHash(42), std::hash<int>{}(42));
    EXPECT_EQ(computeHash(std::string("hello")),
              std::hash<std::string>{}("hello"));
    EXPECT_EQ(computeHash(std::string_view("world")),
              std::hash<std::string_view>{}("world"));
}

TEST(HashTest, ComputeHashVector) {
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::size_t expected = 0;
    for (const auto& value : vec) {
        expected ^=
            computeHash(value) + 0x9e3779b9 + (expected << 6) + (expected >> 2);
    }
    EXPECT_EQ(computeHash(vec), expected);
}

TEST(HashTest, ComputeHashTuple) {
    auto tuple = std::make_tuple(42, std::string("hello"), 3.14);
    std::size_t expected = 0;
    std::apply(
        [&expected](const auto&... values) {
            ((expected ^= computeHash(values) + 0x9e3779b9 + (expected << 6) +
                          (expected >> 2)),
             ...);
        },
        tuple);
    EXPECT_EQ(computeHash(tuple), expected);
}

TEST(HashTest, ComputeHashArray) {
    std::array<int, 5> arr{1, 2, 3, 4, 5};
    std::size_t expected = 0;
    for (const auto& value : arr) {
        expected ^=
            computeHash(value) + 0x9e3779b9 + (expected << 6) + (expected >> 2);
    }
    EXPECT_EQ(computeHash(arr), expected);
}

TEST(HashTest, FNV1aHashString) {
    test_fnv1a_hash("hello", 0x4f9f2cab);
    test_fnv1a_hash("world", 0x4f9f2c9b);
}

TEST(HashTest, JenkinsOneAtATimeHashString) {
    test_jenkins_hash("hello", 0x4f9f2cab);
    test_jenkins_hash("world", 0x4f9f2c9b);
}

TEST(HashTest, QuickHashString) {
    EXPECT_EQ(quickHash("hello"), 0x5d41402a);
    EXPECT_EQ(quickHash("world"), 0x7c211433);
}

TEST(HashTest, QuickHashData) {
    const char data[] = {'a', 'b', 'c', 'd'};
    EXPECT_EQ(quickHash(data, sizeof(data)), 0x5e809ac0);
}

TEST(HashTest, LiteralHash) {
    EXPECT_EQ("hello"_hash, 0xa430d846);
    EXPECT_EQ("world"_hash, 0xb7a2f18e);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
