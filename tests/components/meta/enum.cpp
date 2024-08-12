#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "atom/function/enum.hpp"

enum class Color { Red, Green, Blue };
enum class Permissions { Read = 1, Write = 2, Execute = 4 };

template <>
struct EnumTraits<Color> {
    static constexpr std::array<Color, 3> values = {Color::Red, Color::Green,
                                                    Color::Blue};
    static constexpr std::array<std::string_view, 3> names = {"Red", "Green",
                                                              "Blue"};
};

template <>
struct EnumAliasTraits<Color> {
    static constexpr std::array<std::string_view, 3> aliases = {"R", "G", "B"};
};

template <>
struct EnumTraits<Permissions> {
    static constexpr std::array<Permissions, 3> values = {
        Permissions::Read, Permissions::Write, Permissions::Execute};
    static constexpr std::array<std::string_view, 3> names = {"Read", "Write",
                                                              "Execute"};
};

// 枚举到字符串测试
TEST(EnumTest, EnumToString) {
    EXPECT_EQ(enum_name(Color::Red), "Red");
    EXPECT_EQ(enum_name(Color::Green), "Green");
    EXPECT_EQ(enum_name(Color::Blue), "Blue");

    // 边界情况：未定义的枚举值（在当前代码中不会发生，因为只有定义的枚举值会被转换）
    // EXPECT_EQ(enum_name(static_cast<Color>(-1)), ""); // Uncomment if needed
    // for additional boundary checks
}

// 字符串到枚举测试
TEST(EnumTest, StringToEnum) {
    EXPECT_EQ(enum_cast<Color>("Red"), Color::Red);
    EXPECT_EQ(enum_cast<Color>("Green"), Color::Green);
    EXPECT_EQ(enum_cast<Color>("Blue"), Color::Blue);

    // 边界情况：无效的字符串
    EXPECT_EQ(enum_cast<Color>("Purple"), std::nullopt);
}

// 枚举值到整数测试
TEST(EnumTest, EnumToInteger) {
    EXPECT_EQ(enum_to_integer(Color::Red), 0);
    EXPECT_EQ(enum_to_integer(Color::Green), 1);
    EXPECT_EQ(enum_to_integer(Color::Blue), 2);
}

// 整数到枚举值测试
TEST(EnumTest, IntegerToEnum) {
    EXPECT_EQ(integer_to_enum<Color>(0), Color::Red);
    EXPECT_EQ(integer_to_enum<Color>(1), Color::Green);
    EXPECT_EQ(integer_to_enum<Color>(2), Color::Blue);

    // 边界情况：无效的整数
    EXPECT_EQ(integer_to_enum<Color>(3), std::nullopt);
    EXPECT_EQ(integer_to_enum<Color>(-1), std::nullopt);
}

// 检查枚举值是否有效
TEST(EnumTest, EnumContains) {
    EXPECT_TRUE(enum_contains(Color::Red));
    EXPECT_TRUE(enum_contains(Color::Green));
    EXPECT_TRUE(enum_contains(Color::Blue));

    // 边界情况：无效的枚举值
    EXPECT_FALSE(enum_contains(static_cast<Color>(-1)));
}

// 枚举别名测试
TEST(EnumTest, EnumAlias) {
    EXPECT_EQ(enum_cast_with_alias<Color>("R"), Color::Red);
    EXPECT_EQ(enum_cast_with_alias<Color>("G"), Color::Green);
    EXPECT_EQ(enum_cast_with_alias<Color>("B"), Color::Blue);

    // 边界情况：无效的别名
    EXPECT_EQ(enum_cast_with_alias<Color>("X"), std::nullopt);
}

// 模糊匹配测试
TEST(EnumTest, EnumCastFuzzy) {
    EXPECT_EQ(enum_cast_fuzzy<Color>("Red"), Color::Red);
    EXPECT_EQ(enum_cast_fuzzy<Color>("Gre"), Color::Green);
    EXPECT_EQ(enum_cast_fuzzy<Color>("Blu"), Color::Blue);

    // 边界情况：无效的模糊匹配
    EXPECT_EQ(enum_cast_fuzzy<Color>("Purple"), std::nullopt);
}

// 位运算测试
TEST(EnumTest, EnumBitwiseOperations) {
    Permissions p = Permissions::Read | Permissions::Write;
    EXPECT_EQ(enum_to_integer(p), 3);  // 1 | 2 = 3

    p |= Permissions::Execute;
    EXPECT_EQ(enum_to_integer(p), 7);  // 3 | 4 = 7

    p &= Permissions::Write;
    EXPECT_EQ(enum_to_integer(p), 2);  // 7 & 2 = 2

    p ^= Permissions::Read;
    EXPECT_EQ(enum_to_integer(p), 3);  // 2 ^ 1 = 3

    p = ~Permissions::Execute;
    EXPECT_NE(
        enum_to_integer(p),
        enum_to_integer(Permissions::Execute));  // ~4 should not be equal to 4
}

// 枚举值根据名称排序测试
TEST(EnumTest, EnumSortedByName) {
    auto sorted = enum_sorted_by_name<Color>();
    EXPECT_EQ(sorted[0].second, "Blue");
    EXPECT_EQ(sorted[1].second, "Green");
    EXPECT_EQ(sorted[2].second, "Red");
}

// 枚举值根据整数值排序测试
TEST(EnumTest, EnumSortedByValue) {
    auto sorted = enum_sorted_by_value<Permissions>();
    EXPECT_EQ(sorted[0].second, "Read");
    EXPECT_EQ(sorted[1].second, "Write");
    EXPECT_EQ(sorted[2].second, "Execute");
}

// 检查整数值是否在枚举范围内
TEST(EnumTest, IntegerInEnumRange) {
    EXPECT_TRUE(integer_in_enum_range<Permissions>(1));  // Read
    EXPECT_TRUE(integer_in_enum_range<Permissions>(2));  // Write
    EXPECT_TRUE(integer_in_enum_range<Permissions>(4));  // Execute

    // 边界情况：无效的整数值
    EXPECT_FALSE(integer_in_enum_range<Permissions>(3));
    EXPECT_FALSE(integer_in_enum_range<Permissions>(0));
}

// 枚举的默认值测试
TEST(EnumTest, EnumDefault) {
    EXPECT_EQ(enum_default<Color>(), Color::Red);
    EXPECT_EQ(enum_default<Permissions>(), Permissions::Read);
}
