#include <gtest/gtest.h>

#include "atom/type/expected.hpp"

using namespace atom::type;

// 测试构造函数和value
TEST(ExpectedTest, ConstructsWithValue) {
    expected<int> e1 = make_expected(42);
    EXPECT_TRUE(e1.has_value());
    EXPECT_EQ(e1.value(), 42);
}

// 测试构造函数和错误
TEST(ExpectedTest, ConstructsWithError) {
    expected<int, std::string> e2 = make_unexpected<int>(std::string("Error"));
    EXPECT_FALSE(e2.has_value());
    EXPECT_EQ(e2.error().error(), "Error");
}

// 测试value_or方法
TEST(ExpectedTest, ValueOr) {
    expected<int> e1 = make_expected(42);
    EXPECT_EQ(e1.value_or(0), 42);

    expected<int, std::string> e2 = make_unexpected<int>(std::string("Error"));
    EXPECT_EQ(e2.value_or(0), 0);
}

// 测试map方法
TEST(ExpectedTest, Map) {
    expected<int> e1 = make_expected(42);
    auto e3 = e1.map([](int val) { return val * 2; });
    EXPECT_TRUE(e3.has_value());
    EXPECT_EQ(e3.value(), 84);

    expected<int, std::string> e2 = make_unexpected<int>(std::string("Error"));
    auto e4 = e2.map([](int val) { return val * 2; });
    EXPECT_FALSE(e4.has_value());
    EXPECT_EQ(e4.error().error(), "Error");
}

// 测试and_then方法
TEST(ExpectedTest, AndThen) {
    expected<int> e1 = make_expected(42);
    auto e3 = e1.and_then([](int val) { return make_expected(val * 2); });
    EXPECT_TRUE(e3.has_value());
    EXPECT_EQ(e3.value(), 84);

    expected<int, std::string> e2 = make_unexpected<int>(std::string("Error"));
    auto e4 = e2.and_then([](int val) { return make_expected(val * 2); });
    EXPECT_FALSE(e4.has_value());
    EXPECT_EQ(e4.error().error(), "Error");
}

// 测试自定义错误类型
struct MyError {
    int code;
    std::string message;

    bool operator==(const MyError& other) const {
        return code == other.code && message == other.message;
    }
};

TEST(ExpectedTest, CustomError) {
    expected<int, MyError> e5 = make_unexpected<int>(MyError{404, "Not Found"});
    EXPECT_FALSE(e5.has_value());
    auto err = e5.error().error();
    EXPECT_EQ(err.code, 404);
    EXPECT_EQ(err.message, "Not Found");
}

// 测试相等运算符
TEST(ExpectedTest, EqualityOperator) {
    expected<int> e1 = make_expected(42);
    expected<int> e2 = make_expected(42);
    expected<int, std::string> e3 = make_unexpected<int>(std::string("Error"));
    expected<int, std::string> e4 = make_unexpected<int>(std::string("Error"));

    EXPECT_EQ(e1, e2);
    EXPECT_EQ(e3, e4);
    EXPECT_NE(e1, e3);
}

// 测试异常
TEST(ExpectedTest, ExceptionOnValueAccess) {
    expected<int, std::string> e2 = make_unexpected<int>(std::string("Error"));
    EXPECT_THROW(e2.value(), std::logic_error);
}

TEST(ExpectedTest, ExceptionOnErrorAccess) {
    expected<int> e1 = make_expected(42);
    EXPECT_THROW(e1.error(), std::logic_error);
}
