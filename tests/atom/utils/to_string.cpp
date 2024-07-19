#include "atom/utils/to_string.hpp"
#include <gtest/gtest.h>
using namespace atom::utils;

TEST(StringUtilsTest, ToStringTest) {
    // Basic type
    int num = 123;
    EXPECT_EQ(toString(num), "123");

    // String type
    std::string str = "hello";
    EXPECT_EQ(toString(str), "hello");

    // Container type
    std::vector<int> vec = {1, 2, 3};
    EXPECT_EQ(toString(vec), "[1, 2, 3]");
}

TEST(StringUtilsTest, JoinKeyValuePairTest) {
    // String type key-value pair
    std::string key = "name";
    std::string value = "Max";
    EXPECT_EQ(joinKeyValuePair(key, value), "nameMax");
}

TEST(StringUtilsTest, JoinCommandLineTest) {
    std::string arg1 = "arg1";
    std::string arg2 = "arg2";
    std::string arg3 = "arg3";
    EXPECT_EQ(joinCommandLine(arg1, arg2, arg3), "arg1 arg2 arg3");
}

TEST(StringUtilsTest, ToStringArrayTest) {
    std::vector<int> array = {1, 2, 3, 4, 5};
    EXPECT_EQ(toStringArray(array), "1 2 3 4 5");
}