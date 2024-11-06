#include <gtest/gtest.h>

#include "atom/type/expected.hpp"

using namespace atom::type;

// Test fixture for expected class
template <typename T, typename E = std::string>
class ExpectedTest : public ::testing::Test {
protected:
    expected<T, E> value_expected;
    expected<T, E> error_expected;

    ExpectedTest() : value_expected(T{}), error_expected(Error<E>("error")) {}
};

// Test fixture for expected<void, E> specialization
template <typename E = std::string>
class ExpectedVoidTest : public ::testing::Test {
protected:
    expected<void, E> value_expected;
    expected<void, E> error_expected;

    ExpectedVoidTest() : value_expected(), error_expected(Error<E>("error")) {}
};

// Test cases for expected<int, std::string>
using ExpectedIntTest = ExpectedTest<int>;

TEST_F(ExpectedIntTest, DefaultConstructor) {
    expected<int> e;
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(e.value(), 0);
}

TEST_F(ExpectedIntTest, ValueConstructor) {
    expected<int> e(42);
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(e.value(), 42);
}

TEST_F(ExpectedIntTest, ErrorConstructor) {
    expected<int> e(Error<std::string>("error"));
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error().error(), "error");
}

TEST_F(ExpectedIntTest, UnexpectedConstructor) {
    expected<int> e(unexpected<std::string>("error"));
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error().error(), "error");
}

TEST_F(ExpectedIntTest, CopyConstructor) {
    expected<int> e1(42);
    expected<int> e2(e1);
    EXPECT_TRUE(e2.has_value());
    EXPECT_EQ(e2.value(), 42);
}

TEST_F(ExpectedIntTest, MoveConstructor) {
    expected<int> e1(42);
    expected<int> e2(std::move(e1));
    EXPECT_TRUE(e2.has_value());
    EXPECT_EQ(e2.value(), 42);
}

TEST_F(ExpectedIntTest, CopyAssignment) {
    expected<int> e1(42);
    expected<int> e2;
    e2 = e1;
    EXPECT_TRUE(e2.has_value());
    EXPECT_EQ(e2.value(), 42);
}

TEST_F(ExpectedIntTest, MoveAssignment) {
    expected<int> e1(42);
    expected<int> e2;
    e2 = std::move(e1);
    EXPECT_TRUE(e2.has_value());
    EXPECT_EQ(e2.value(), 42);
}

TEST_F(ExpectedIntTest, AndThen) {
    auto result =
        value_expected.and_then([](int& v) { return expected<int>(v + 1); });
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);

    result =
        error_expected.and_then([](int& v) { return expected<int>(v + 1); });
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error(), "error");
}

TEST_F(ExpectedIntTest, Map) {
    auto result = value_expected.map([](int& v) { return v + 1; });
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);

    result = error_expected.map([](int& v) { return v + 1; });
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error(), "error");
}

/*
TEST_F(ExpectedIntTest, TransformError) {
    auto result = error_expected.transform_error([](const std::string& e) {
return Error<std::string>(e + " transformed"); });
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error(), "error transformed");
}
*/

// Test cases for expected<void, std::string>
using ExpectedVoidStringTest = ExpectedVoidTest<>;

TEST_F(ExpectedVoidStringTest, DefaultConstructor) {
    expected<void> e;
    EXPECT_TRUE(e.has_value());
}

TEST_F(ExpectedVoidStringTest, ErrorConstructor) {
    expected<void> e(Error<std::string>("error"));
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error().error(), "error");
}

TEST_F(ExpectedVoidStringTest, UnexpectedConstructor) {
    expected<void> e(unexpected<std::string>("error"));
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error().error(), "error");
}

TEST_F(ExpectedVoidStringTest, CopyConstructor) {
    expected<void> e1;
    expected<void> e2(e1);
    EXPECT_TRUE(e2.has_value());
}

TEST_F(ExpectedVoidStringTest, MoveConstructor) {
    expected<void> e1;
    expected<void> e2(std::move(e1));
    EXPECT_TRUE(e2.has_value());
}

TEST_F(ExpectedVoidStringTest, CopyAssignment) {
    expected<void> e1;
    expected<void> e2;
    e2 = e1;
    EXPECT_TRUE(e2.has_value());
}

TEST_F(ExpectedVoidStringTest, MoveAssignment) {
    expected<void> e1;
    expected<void> e2;
    e2 = std::move(e1);
    EXPECT_TRUE(e2.has_value());
}

TEST_F(ExpectedVoidStringTest, AndThen) {
    auto result = value_expected.and_then([]() { return expected<void>(); });
    EXPECT_TRUE(result.has_value());

    result = error_expected.and_then([]() { return expected<void>(); });
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error(), "error");
}

/*
TEST_F(ExpectedVoidStringTest, TransformError) {
    auto result = error_expected.transform_error([](const std::string& e) {
return e + " transformed"; }); EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error(), "error transformed");
}
*/
