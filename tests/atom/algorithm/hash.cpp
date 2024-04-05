#include "atom/algorithm/hash.hpp"
#include <gtest/gtest.h>

using namespace Atom::Algorithm;

TEST(ComputeHashTest, TestInt) {
    int value = 123;
    std::size_t expected = std::hash<int>{}(value);
    std::size_t result = computeHash(value);
    EXPECT_EQ(result, expected);
}

TEST(ComputeHashTest, TestString) {
    std::string value = "hello";
    std::size_t expected = std::hash<std::string>{}(value);
    std::size_t result = computeHash(value);
    EXPECT_EQ(result, expected);
}

TEST(ComputeHashTest, TestVector) {
    std::vector<int> values = {1, 2, 3};
    std::size_t expected = 0;
    for (const auto& value : values) {
        expected ^= std::hash<int>{}(value) + 0x9e3779b9 + (expected << 6) +
                    (expected >> 2);
    }
    std::size_t result = computeHash(values);
    EXPECT_EQ(result, expected);
}

TEST(ComputeHashTest, TestTuple) {
    std::tuple<int, std::string> tuple = std::make_tuple(123, "hello");
    std::size_t expected = 0;
    apply(
        [&expected](const int& value1, const std::string& value2) {
            expected ^= computeHash(value1) + 0x9e3779b9 +
                        (expected << 6) + (expected >> 2);
            expected ^= computeHash(value2) + 0x9e3779b9 +
                        (expected << 6) + (expected >> 2);
        },
        tuple);
    std::size_t result = computeHash(tuple);
    EXPECT_EQ(result, expected);
}

TEST(ComputeHashTest, TestArray) {
    std::array<int, 3> array = {{1, 2, 3}};
    std::size_t expected = 0;
    for (const auto& value : array) {
        expected ^= std::hash<int>{}(value) + 0x9e3779b9 + (expected << 6) +
                    (expected >> 2);
    }
    std::size_t result = computeHash(array);
    EXPECT_EQ(result, expected);
}

#ifndef __MAIN__
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
