#include "atom/utils/random.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include <string>

using namespace atom::utils;

TEST(RandomTest, ConstructorWithMinMax) {
    Random<std::mt19937, std::uniform_int_distribution<>> rand(1, 10);
    int value = rand();
    EXPECT_GE(value, 1);
    EXPECT_LE(value, 10);
}

TEST(RandomTest, ConstructorWithSeedAndArgs) {
    Random<std::mt19937, std::uniform_int_distribution<>> rand(42, 1, 10);
    int value = rand();
    EXPECT_GE(value, 1);
    EXPECT_LE(value, 10);
}

TEST(RandomTest, Seed) {
    Random<std::mt19937, std::uniform_int_distribution<>> rand(1, 10);
    rand.seed(42);
    int value1 = rand();
    rand.seed(42);
    int value2 = rand();
    EXPECT_EQ(value1, value2);
}

TEST(RandomTest, Generate) {
    Random<std::mt19937, std::uniform_int_distribution<>> rand(1, 10);
    std::vector<int> vec(10);
    rand.generate(vec.begin(), vec.end());
    for (int value : vec) {
        EXPECT_GE(value, 1);
        EXPECT_LE(value, 10);
    }
}

TEST(RandomTest, Vector) {
    Random<std::mt19937, std::uniform_int_distribution<>> rand(1, 10);
    auto vec = rand.vector(10);
    EXPECT_EQ(vec.size(), 10);
    for (int value : vec) {
        EXPECT_GE(value, 1);
        EXPECT_LE(value, 10);
    }
}

TEST(RandomTest, GenerateRandomString) {
    std::string str = generateRandomString(10);
    EXPECT_EQ(str.size(), 10);
    EXPECT_TRUE(std::all_of(str.begin(), str.end(),
                            [](unsigned char c) { return std::isalnum(c); }));
}
