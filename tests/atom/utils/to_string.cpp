#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

#include "atom/utils/to_string.hpp"

using namespace atom::utils;

TEST(ToStringTest, StringType) {
    EXPECT_EQ(toString("hello"), "hello");
    EXPECT_EQ(toString(std::string("world")), "world");
}

TEST(ToStringTest, EnumType) {
    enum class Color { RED, GREEN, BLUE };
    EXPECT_EQ(toString(Color::RED), "0");
    EXPECT_EQ(toString(Color::GREEN), "1");
    EXPECT_EQ(toString(Color::BLUE), "2");
}

TEST(ToStringTest, PointerType) {
    int value = 42;
    int* ptr = &value;
    int* nullPtr = nullptr;
    EXPECT_EQ(toString(ptr), "Pointer(42)");
    EXPECT_EQ(toString(nullPtr), "nullptr");
}

TEST(ToStringTest, SmartPointerType) {
    auto ptr = std::make_shared<int>(42);
    std::shared_ptr<int> nullPtr = nullptr;
    EXPECT_EQ(toString(ptr), "SmartPointer(42)");
    EXPECT_EQ(toString(nullPtr), "nullptr");
}

TEST(ToStringTest, ContainerType) {
    std::vector<int> vec = {1, 2, 3};
    std::map<std::string, int> map = {{"one", 1}, {"two", 2}};
    EXPECT_EQ(toString(vec), "[1, 2, 3]");
    EXPECT_EQ(toString(map), "{one: 1, two: 2}");
}

TEST(ToStringTest, GeneralType) {
    EXPECT_EQ(toString(42), "42");
    EXPECT_EQ(toString(3.14), "3.140000");
}

TEST(ToStringTest, JoinCommandLine) {
    EXPECT_EQ(joinCommandLine("echo", "Hello", "World"), "echo Hello World");
}

TEST(ToStringTest, ToStringArray) {
    std::array<int, 3> arr = {1, 2, 3};
    EXPECT_EQ(toString(arr), "[1, 2, 3]");
}

TEST(ToStringTest, ToStringRange) {
    std::vector<int> vec = {1, 2, 3};
    EXPECT_EQ(toStringRange(vec.begin(), vec.end()), "[1, 2, 3]");
}

TEST(ToStringTest, TupleToString) {
    auto tpl = std::make_tuple(1, "hello", 3.14);
    EXPECT_EQ(toString(tpl), "(1, hello, 3.140000)");
}

TEST(ToStringTest, OptionalToString) {
    std::optional<int> opt = 42;
    std::optional<int> nullOpt = std::nullopt;
    EXPECT_EQ(toString(opt), "Optional(42)");
    EXPECT_EQ(toString(nullOpt), "nullopt");
}

TEST(ToStringTest, VariantToString) {
    std::variant<int, std::string> var = 42;
    EXPECT_EQ(toString(var), "42");
    var = "hello";
    EXPECT_EQ(toString(var), "hello");
}
