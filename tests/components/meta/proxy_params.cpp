#include "atom/function/proxy_params.hpp"
#include <gtest/gtest.h>
#include <any>
#include <string>
#include <vector>
#include "exception.hpp"
#include "macro.hpp"

// 测试单个元素构造函数
TEST(FunctionParamsTest, SingleElementConstructor) {
    std::any value = 42;
    FunctionParams params(value);

    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(std::any_cast<int>(params[0]), 42);
}

// 测试范围构造函数
TEST(FunctionParamsTest, RangeConstructor) {
    std::vector<std::any> vec = {1, std::string("test"), 3.14};
    FunctionParams params(vec);

    ASSERT_EQ(params.size(), vec.size());
    EXPECT_EQ(std::any_cast<int>(params[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(params[1]), "test");
    EXPECT_EQ(std::any_cast<double>(params[2]), 3.14);
}

// 测试初始化列表构造函数
TEST(FunctionParamsTest, InitializerListConstructor) {
    FunctionParams params{1, std::string("test"), 3.14};

    ASSERT_EQ(params.size(), 3);
    EXPECT_EQ(std::any_cast<int>(params[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(params[1]), "test");
    EXPECT_EQ(std::any_cast<double>(params[2]), 3.14);
}

// 测试operator[]的边界情况
TEST(FunctionParamsTest, AccessOperator) {
    FunctionParams params{1, std::string("test"), 3.14};

    EXPECT_EQ(std::any_cast<int>(params[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(params[1]), "test");
    EXPECT_EQ(std::any_cast<double>(params[2]), 3.14);
    EXPECT_THROW(ATOM_UNUSED_RESULT(params[3]), atom::error::OutOfRange);  // 越界访问
}

// 测试begin和end方法
TEST(FunctionParamsTest, BeginEnd) {
    FunctionParams params{1, std::string("test"), 3.14};
    std::vector<std::any> expected = {1, std::string("test"), 3.14};

    EXPECT_TRUE(std::equal(params.begin(), params.end(), expected.begin(),
                           [](const std::any& a, const std::any& b) {
                               return a.type() == b.type() && a.has_value() &&
                                      b.has_value();
                           }));
}

// 测试front方法
TEST(FunctionParamsTest, FrontMethod) {
    FunctionParams params{1, std::string("test"), 3.14};

    EXPECT_EQ(std::any_cast<int>(params.front()), 1);
}

// 测试size和empty方法
TEST(FunctionParamsTest, SizeAndEmpty) {
    FunctionParams empty_params{};
    EXPECT_EQ(empty_params.size(), 0);
    EXPECT_TRUE(empty_params.empty());

    FunctionParams params{1, std::string("test")};
    EXPECT_EQ(params.size(), 2);
    EXPECT_FALSE(params.empty());
}

// 测试toVector方法
TEST(FunctionParamsTest, ToVectorMethod) {
    FunctionParams params{1, std::string("test"), 3.14};
    auto vec = params.toVector();

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(std::any_cast<int>(vec[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(vec[1]), "test");
    EXPECT_EQ(std::any_cast<double>(vec[2]), 3.14);
}

// 测试get方法
TEST(FunctionParamsTest, GetMethod) {
    FunctionParams params{1, std::string("test"), 3.14};

    auto value1 = params.get<int>(0);
    EXPECT_TRUE(value1.has_value());
    EXPECT_EQ(value1.value(), 1);

    auto value2 = params.get<std::string>(1);
    EXPECT_TRUE(value2.has_value());
    EXPECT_EQ(value2.value(), "test");

    auto value3 = params.get<double>(2);
    EXPECT_TRUE(value3.has_value());
    EXPECT_EQ(value3.value(), 3.14);

    auto invalidValue = params.get<float>(1);
    EXPECT_FALSE(invalidValue.has_value());

    auto outOfRangeValue = params.get<int>(10);
    EXPECT_FALSE(outOfRangeValue.has_value());
}

// 测试slice方法
TEST(FunctionParamsTest, SliceMethod) {
    FunctionParams params{1, std::string("test"), 3.14, 42};

    auto slice = params.slice(1, 3);
    EXPECT_EQ(slice.size(), 2);
    EXPECT_EQ(std::any_cast<std::string>(slice[0]), "test");
    EXPECT_EQ(std::any_cast<double>(slice[1]), 3.14);

    EXPECT_THROW(params.slice(3, 2), atom::error::OutOfRange);  // Invalid range
    EXPECT_THROW(params.slice(1, 5), atom::error::OutOfRange);  // Out of bounds
}

// 测试filter方法
TEST(FunctionParamsTest, FilterMethod) {
    FunctionParams params{1, std::string("test"), 3.14, 42};

    auto filtered = params.filter(
        [](const std::any& val) -> bool { return val.type() == typeid(int); });

    EXPECT_EQ(filtered.size(), 2);
    EXPECT_EQ(std::any_cast<int>(filtered[0]), 1);
    EXPECT_EQ(std::any_cast<int>(filtered[1]), 42);
}

// 测试set方法
TEST(FunctionParamsTest, SetMethod) {
    FunctionParams params{1, std::string("test"), 3.14};

    params.set(1, std::string("new_test"));
    EXPECT_EQ(std::any_cast<std::string>(params[1]), "new_test");

    EXPECT_THROW(params.set(3, 42), atom::error::OutOfRange);  // 越界设置
}
