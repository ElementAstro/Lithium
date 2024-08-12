#include "atom/algorithm/hash.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

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

TEST(HashTest, LiteralHash) { EXPECT_EQ("hello"_hash, 0x4f9f2cabU); }
