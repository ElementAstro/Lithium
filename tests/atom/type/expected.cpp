#include <gtest/gtest.h>

#include "atom/type/expected.hpp"

using namespace atom::type;

// 测试expected<T, E> 的基础功能
TEST(ExpectedTest, BasicFunctionality) {
    // 测试成功情况
    expected<int, std::string> success(42);
    EXPECT_TRUE(success.has_value());
    EXPECT_EQ(success.value(), 42);

    // 测试错误情况
    expected<int, std::string> failure(make_unexpected("error"));
    EXPECT_FALSE(failure.has_value());
    EXPECT_EQ(failure.error().error(), "error");
}

// 测试expected<void, E> 的基础功能
TEST(ExpectedTest, VoidTypeFunctionality) {
    // 测试成功情况
    expected<void, std::string> success;
    EXPECT_TRUE(success.has_value());

    // 测试错误情况
    expected<void, std::string> failure(make_unexpected("void error"));
    EXPECT_FALSE(failure.has_value());
    EXPECT_EQ(failure.error().error(), "void error");

    // 测试value_or功能
    bool lambda_called = false;
    failure.value_or([&](std::string err) {
        lambda_called = true;
        EXPECT_EQ(err, "void error");
    });
    EXPECT_TRUE(lambda_called);
}

// 测试错误比较和处理
TEST(ExpectedTest, ErrorComparison) {
    Error<std::string> error1("Error1");
    Error<std::string> error2("Error2");

    EXPECT_EQ(error1, Error<std::string>("Error1"));
    EXPECT_NE(error1, error2);
}

// 测试map功能
TEST(ExpectedTest, MapFunctionality) {
    expected<int, std::string> success(10);
    auto mapped = success.map([](int value) { return value * 2; });

    EXPECT_TRUE(mapped.has_value());
    EXPECT_EQ(mapped.value(), 20);

    expected<int, std::string> failure(make_unexpected("map error"));
    auto mapped_failure = failure.map([](int value) { return value * 2; });

    EXPECT_FALSE(mapped_failure.has_value());
    EXPECT_EQ(mapped_failure.error().error(), "map error");
}

// 测试and_then功能
TEST(ExpectedTest, AndThenFunctionality) {
    expected<int, std::string> success(10);
    auto chained =
        success.and_then([](int value) { return make_expected(value + 5); });

    EXPECT_TRUE(chained.has_value());
    EXPECT_EQ(chained.value(), 15);

    expected<int, std::string> failure(make_unexpected("and_then error"));
    auto chained_failure =
        failure.and_then([](int value) { return make_expected(value + 5); });

    EXPECT_FALSE(chained_failure.has_value());
    EXPECT_EQ(chained_failure.error().error(), "and_then error");
}

// 测试边缘情况：空字符串错误
TEST(ExpectedTest, EmptyStringError) {
    expected<int, std::string> failure(make_unexpected(""));
    EXPECT_FALSE(failure.has_value());
    EXPECT_EQ(failure.error().error(), "");

    bool lambda_called = false;
    int result = failure.value_or([&](std::string err) {
        lambda_called = true;
        EXPECT_EQ(err, "");
        return 0;
    });
    EXPECT_TRUE(lambda_called);
    EXPECT_EQ(result, 0);
}

// 测试边缘情况：传递const char*的错误
TEST(ExpectedTest, ConstCharError) {
    expected<int, std::string> failure(make_unexpected("const char* error"));
    EXPECT_FALSE(failure.has_value());
    EXPECT_EQ(failure.error().error(), "const char* error");
}

// 测试异常情况：访问错误的value
TEST(ExpectedTest, AccessErrorInsteadOfValue) {
    expected<int, std::string> failure(make_unexpected("access error"));

    EXPECT_THROW(
        {
            try {
                [[maybe_unused]] int value = failure.value();
            } catch (const std::logic_error& e) {
                EXPECT_STREQ(
                    "Attempted to access value, but it contains an error.",
                    e.what());
                throw;
            }
        },
        std::logic_error);
}

// 测试异常情况：访问value时的错误
TEST(ExpectedTest, AccessValueInsteadOfError) {
    expected<int, std::string> success(42);

    EXPECT_THROW(
        {
            try {
                auto error = success.error();
            } catch (const std::logic_error& e) {
                EXPECT_STREQ(
                    "Attempted to access error, but it contains a value.",
                    e.what());
                throw;
            }
        },
        std::logic_error);
}

// 测试不同类型的错误
TEST(ExpectedTest, DifferentErrorTypes) {
    expected<int, int> int_error(make_unexpected(404));
    EXPECT_FALSE(int_error.has_value());
    EXPECT_EQ(int_error.error().error(), 404);

    expected<int, std::string> string_error(make_unexpected("error message"));
    EXPECT_FALSE(string_error.has_value());
    EXPECT_EQ(string_error.error().error(), "error message");
}