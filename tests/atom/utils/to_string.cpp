#include "atom/utils/to_string.hpp"
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <vector>

using namespace atom::utils;

// 基本类型测试
TEST(ToStringTest, BasicTypes) {
    EXPECT_EQ(toString(42), "42");
    EXPECT_EQ(toString(3.14), "3.14");
    EXPECT_EQ(toString('A'), "A");
    EXPECT_EQ(toString(true), "1");
}

// 字符串类型测试
TEST(ToStringTest, StringTypes) {
    EXPECT_EQ(toString(std::string("hello")), "hello");
    EXPECT_EQ(toString("world"), "world");
}

// 枚举类型测试
enum class MyEnum { Value1 = 1, Value2 = 2 };

TEST(ToStringTest, EnumType) {
    EXPECT_EQ(toString(MyEnum::Value1), "1");
    EXPECT_EQ(toString(MyEnum::Value2), "2");
}

// 容器类型测试
TEST(ToStringTest, ContainerTypes) {
    std::vector<int> vec = {1, 2, 3};
    EXPECT_EQ(toString(vec), "[1, 2, 3]");

    std::vector<std::string> strVec = {"one", "two", "three"};
    EXPECT_EQ(toString(strVec), "[one, two, three]");
}

// 指针类型测试
TEST(ToStringTest, PointerTypes) {
    int val = 42;
    int* ptr = &val;
    EXPECT_EQ(toString(ptr), "Pointer(42)");

    int* nullPtr = nullptr;
    EXPECT_EQ(toString(nullPtr), "nullptr");
}

// 智能指针类型测试
TEST(ToStringTest, SmartPointerTypes) {
    auto smartPtr = std::make_unique<int>(42);
    EXPECT_EQ(toString(smartPtr), "SmartPointer(42)");

    std::unique_ptr<int> nullSmartPtr = nullptr;
    EXPECT_EQ(toString(nullSmartPtr), "nullptr");
}

// 映射类型测试
TEST(ToStringTest, MapTypes) {
    std::map<int, std::string> map = {{1, "one"}, {2, "two"}};
    EXPECT_EQ(toString(map), "{1: one, 2: two}");

    std::unordered_map<std::string, int> unorderedMap = {{"one", 1},
                                                         {"two", 2}};
    EXPECT_EQ(toString(unorderedMap), "{one: 1, two: 2}");
}

// 键值对测试
TEST(ToStringTest, PairType) {
    std::pair<int, std::string> pair = {1, "one"};
    EXPECT_EQ(toString(pair), "(1, one)");
}

// 数组测试
TEST(ToStringTest, ArrayType) {
    int arr[] = {1, 2, 3};
    EXPECT_EQ(toString(arr), "[1, 2, 3]");
}

// 命令行参数测试
TEST(ToStringTest, JoinCommandLine) {
    EXPECT_EQ(joinCommandLine(1, "two", 3.14), "1 two 3.14");
}

// toStringArray 测试
TEST(ToStringTest, ToStringArray) {
    std::vector<int> vec = {1, 2, 3};
    EXPECT_EQ(toStringArray(vec), "1 2 3");
}

// 范围测试
TEST(ToStringTest, ToStringRange) {
    std::vector<int> vec = {1, 2, 3};
    EXPECT_EQ(toStringRange(vec.begin(), vec.end()), "[1, 2, 3]");
}
