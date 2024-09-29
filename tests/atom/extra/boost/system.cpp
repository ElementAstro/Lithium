#include "atom/extra/boost/system.hpp"

#include <gtest/gtest.h>

using namespace atom::extra::boost;

TEST(ErrorTest, DefaultConstructor) {
    Error error;
    EXPECT_EQ(error.value(), 0);
    EXPECT_EQ(error.message(), "");
    EXPECT_FALSE(error);
}

TEST(ErrorTest, ErrorCodeConstructor) {
    boost::system::error_code ec(1, boost::system::generic_category());
    Error error(ec);
    EXPECT_EQ(error.value(), 1);
    EXPECT_EQ(error.category(), boost::system::generic_category());
    EXPECT_EQ(error.message(), ec.message());
    EXPECT_TRUE(error);
}

TEST(ErrorTest, ValueAndCategoryConstructor) {
    Error error(1, boost::system::generic_category());
    EXPECT_EQ(error.value(), 1);
    EXPECT_EQ(error.category(), boost::system::generic_category());
    EXPECT_EQ(error.message(), boost::system::generic_category().message(1));
    EXPECT_TRUE(error);
}

TEST(ErrorTest, ToBoostErrorCode) {
    boost::system::error_code ec(1, boost::system::generic_category());
    Error error(ec);
    EXPECT_EQ(error.toBoostErrorCode(), ec);
}

TEST(ErrorTest, EqualityOperators) {
    Error error1(1, boost::system::generic_category());
    Error error2(1, boost::system::generic_category());
    Error error3(2, boost::system::generic_category());
    EXPECT_EQ(error1, error2);
    EXPECT_NE(error1, error3);
}

TEST(ExceptionTest, Constructor) {
    Error error(1, boost::system::generic_category());
    Exception ex(error);
    EXPECT_EQ(ex.code().value(), 1);
    EXPECT_EQ(ex.code().category(), boost::system::generic_category());
    EXPECT_EQ(ex.what(), error.message());
}

TEST(ExceptionTest, ErrorMethod) {
    Error error(1, boost::system::generic_category());
    Exception ex(error);
    EXPECT_EQ(ex.error(), error);
}

TEST(ResultTest, ValueConstructor) {
    Result<int> result(42);
    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(result.value(), 42);
}

TEST(ResultTest, ErrorConstructor) {
    Error error(1, boost::system::generic_category());
    Result<int> result(error);
    EXPECT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), error);
}

TEST(ResultTest, ValueOr) {
    Result<int> result(42);
    EXPECT_EQ(result.valueOr(0), 42);
    Result<int> errorResult(Error(1, boost::system::generic_category()));
    EXPECT_EQ(errorResult.valueOr(0), 0);
}

TEST(ResultTest, Map) {
    Result<int> result(42);
    auto mappedResult = result.map([](int value) { return value * 2; });
    EXPECT_TRUE(mappedResult.hasValue());
    EXPECT_EQ(mappedResult.value(), 84);
}

TEST(ResultTest, AndThen) {
    Result<int> result(42);
    auto andThenResult =
        result.andThen([](int value) { return Result<int>(value * 2); });
    EXPECT_TRUE(andThenResult.hasValue());
    EXPECT_EQ(andThenResult.value(), 84);
}

TEST(ResultVoidTest, ErrorConstructor) {
    Error error(1, boost::system::generic_category());
    Result<void> result(error);
    EXPECT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), error);
}

TEST(MakeResultTest, FunctionReturnsValue) {
    auto func = []() { return 42; };
    auto result = makeResult(func);
    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(result.value(), 42);
}

TEST(MakeResultTest, FunctionThrowsException) {
    auto func = []() -> int {
        throw Exception(Error(1, boost::system::generic_category()));
    };
    auto result = makeResult(func);
    EXPECT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().value(), 1);
}

TEST(MakeResultTest, FunctionThrowsStdException) {
    auto func = []() -> int { throw std::runtime_error("error"); };
    auto result = makeResult(func);
    EXPECT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().value(), boost::system::errc::invalid_argument);
}