#include "atom/type/optional.hpp"
#include "gtest/gtest.h"

using namespace atom::type;

// Test Fixture Class
class OptionalTest : public ::testing::Test {
protected:
    Optional<int> optInt;
    Optional<std::string> optStr;
};

// Test: 默认构造
TEST_F(OptionalTest, DefaultConstruction) {
    EXPECT_FALSE(optInt);
    EXPECT_FALSE(optStr);
}

// Test: 使用值构造
TEST_F(OptionalTest, ValueConstruction) {
    Optional<int> opt(42);
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 42);

    Optional<std::string> optStr("test");
    EXPECT_TRUE(optStr);
    EXPECT_EQ(*optStr, "test");
}

// Test: 空构造
TEST_F(OptionalTest, NulloptConstruction) {
    Optional<int> opt(std::nullopt);
    EXPECT_FALSE(opt);
}

// Test: Emplace 方法
TEST_F(OptionalTest, Emplace) {
    optInt.emplace(100);
    EXPECT_TRUE(optInt);
    EXPECT_EQ(*optInt, 100);

    optStr.emplace("hello");
    EXPECT_TRUE(optStr);
    EXPECT_EQ(*optStr, "hello");
}

// Test: Value_or 方法
TEST_F(OptionalTest, ValueOr) {
    EXPECT_EQ(optInt.value_or(99), 99);  // 默认值
    optInt.emplace(50);
    EXPECT_EQ(optInt.value_or(99), 50);  // 现有值
}

// Test: Map 方法
TEST_F(OptionalTest, Map) {
    optStr.emplace("test");
    auto optLength =
        optStr.map([](const std::string& str) { return str.size(); });
    EXPECT_TRUE(optLength);
    EXPECT_EQ(*optLength, 4);
}

// Test: Flat_map 方法
TEST_F(OptionalTest, FlatMap) {
    optStr.emplace("test");
    auto optFirstChar =
        optStr.flat_map([](const std::string& str) -> Optional<char> {
            if (!str.empty()) {
                return Optional<char>(str[0]);
            }
            return std::nullopt;
        });
    EXPECT_TRUE(optFirstChar);
    EXPECT_EQ(*optFirstChar, 't');

    Optional<std::string> optEmptyStr(std::nullopt);
    auto optEmptyResult =
        optEmptyStr.flat_map([](const std::string& str) -> Optional<int> {
            return Optional<int>(std::nullopt);
        });
    EXPECT_FALSE(optEmptyResult);
}

// Test: Or_else 方法
TEST_F(OptionalTest, OrElse) {
    auto result = optStr.or_else([]() { return std::string("default"); });
    EXPECT_EQ(result, "default");

    optStr.emplace("value");
    result = optStr.or_else([]() { return std::string("default"); });
    EXPECT_EQ(result, "value");
}

// Test: Transform_or 方法
TEST_F(OptionalTest, TransformOr) {
    auto transformed = optStr.transform_or(
        [](const std::string& str) { return "Transformed: " + str; },
        "Default value");
    EXPECT_EQ(transformed.value_or(""), "Default value");

    optStr.emplace("data");
    transformed = optStr.transform_or(
        [](const std::string& str) { return "Transformed: " + str; },
        "Default value");
    EXPECT_EQ(transformed.value_or(""), "Transformed: data");
}

// Test: 移动语义
TEST_F(OptionalTest, MoveSemantics) {
    optStr.emplace("move");
    Optional<std::string> movedStr(std::move(optStr));
    EXPECT_FALSE(optStr);
    EXPECT_TRUE(movedStr);
    EXPECT_EQ(*movedStr, "move");
}

// Test: And_then 方法
TEST_F(OptionalTest, AndThen) {
    optStr.emplace("hello");
    auto finalResult =
        optStr.and_then([](const std::string& str) -> Optional<std::string> {
            if (str == "hello") {
                return Optional<std::string>("world");
            }
            return std::nullopt;
        });
    EXPECT_TRUE(finalResult);
    EXPECT_EQ(*finalResult, "world");

    auto emptyResult = optStr.and_then(
        [](const std::string& str) -> Optional<int> { return std::nullopt; });
    EXPECT_FALSE(emptyResult);
}

// Test: Reset 方法
TEST_F(OptionalTest, Reset) {
    optInt.emplace(42);
    EXPECT_TRUE(optInt);
    optInt.reset();
    EXPECT_FALSE(optInt);
}
