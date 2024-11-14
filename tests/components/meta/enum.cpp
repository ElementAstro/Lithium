#ifndef TEST_ATOM_META_ENUM_HPP
#define TEST_ATOM_META_ENUM_HPP

#include "atom/function/enum.hpp"
#include <gtest/gtest.h>
#include <string_view>
using namespace std::literals::string_view_literals;


// Test enum definitions
enum class Color { Red, Green, Blue };

enum class Flags { None = 0, Flag1 = 1, Flag2 = 2, Flag3 = 4 };

// Specializations for test enums
template <>
struct atom::meta::EnumTraits<Color> {
    static constexpr std::array<Color, 3> values = {Color::Red, Color::Green,
                                                    Color::Blue};
    static constexpr std::array<std::string_view, 3> names = {
        "Red"sv, "Green"sv, "Blue"sv};
    static constexpr std::array<std::string_view, 3> descriptions = {
        "Red color"sv, "Green color"sv, "Blue color"sv};
};

template <>
struct atom::meta::EnumTraits<Flags> {
    static constexpr std::array<Flags, 4> values = {Flags::None, Flags::Flag1,
                                                    Flags::Flag2, Flags::Flag3};
    static constexpr std::array<std::string_view, 4> names = {
        "None"sv, "Flag1"sv, "Flag2"sv, "Flag3"sv};
    static constexpr std::array<std::string_view, 4> descriptions = {
        "No flags"sv, "First flag"sv, "Second flag"sv, "Third flag"sv};
};

template <>
struct atom::meta::EnumAliasTraits<Color> {
    static constexpr std::array aliases = {"R"sv, "G"sv, "B"sv};
};


namespace atom::meta::test {

class EnumTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Basic enum operations tests
TEST_F(EnumTest, EnumToString) {
    EXPECT_EQ(enum_name(Color::Red), "Red");
    EXPECT_EQ(enum_name(Color::Green), "Green");
    EXPECT_EQ(enum_name(Color::Blue), "Blue");
}

TEST_F(EnumTest, StringToEnum) {
    auto red = enum_cast<Color>("Red");
    EXPECT_TRUE(red.has_value());
    EXPECT_EQ(red.value(), Color::Red);

    auto invalid = enum_cast<Color>("Invalid");
    EXPECT_FALSE(invalid.has_value());
}

TEST_F(EnumTest, EnumToInteger) {
    EXPECT_EQ(enum_to_integer(Flags::None), 0);
    EXPECT_EQ(enum_to_integer(Flags::Flag1), 1);
    EXPECT_EQ(enum_to_integer(Flags::Flag2), 2);
}

TEST_F(EnumTest, IntegerToEnum) {
    auto flag1 = integer_to_enum<Flags>(1);
    EXPECT_TRUE(flag1.has_value());
    EXPECT_EQ(flag1.value(), Flags::Flag1);
}

// Enum validation tests
TEST_F(EnumTest, EnumContains) {
    EXPECT_TRUE(enum_contains(Color::Red));
    EXPECT_TRUE(enum_contains(Color::Green));
    EXPECT_TRUE(enum_contains(Color::Blue));
}

TEST_F(EnumTest, EnumEntries) {
    auto entries = enum_entries<Color>();
    EXPECT_EQ(entries.size(), 3);
    EXPECT_EQ(entries[0].first, Color::Red);
    EXPECT_EQ(entries[0].second, "Red");
}

// Bitwise operation tests
TEST_F(EnumTest, BitwiseOperations) {
    Flags f1 = Flags::Flag1;
    Flags f2 = Flags::Flag2;

    auto combined = f1 | f2;
    EXPECT_EQ(enum_to_integer(combined), 3);

    auto intersection = combined & f1;
    EXPECT_EQ(intersection, f1);

    auto exclusive = f1 ^ f2;
    EXPECT_EQ(enum_to_integer(exclusive), 3);

    auto complement = ~f1;
    EXPECT_NE(complement, f1);
}

// Sorting tests
TEST_F(EnumTest, SortByName) {
    auto sorted = enum_sorted_by_name<Color>();
    EXPECT_EQ(sorted[0].second, "Blue");
    EXPECT_EQ(sorted[1].second, "Green");
    EXPECT_EQ(sorted[2].second, "Red");
}

TEST_F(EnumTest, SortByValue) {
    auto sorted = enum_sorted_by_value<Flags>();
    EXPECT_EQ(sorted[0].first, Flags::None);
    EXPECT_EQ(sorted[1].first, Flags::Flag1);
    EXPECT_EQ(sorted[2].first, Flags::Flag2);
}

// Fuzzy matching tests
TEST_F(EnumTest, FuzzyMatch) {
    auto red = enum_cast_fuzzy<Color>("Re");
    EXPECT_TRUE(red.has_value());
    EXPECT_EQ(red.value(), Color::Red);
}

// Range checking tests
TEST_F(EnumTest, RangeCheck) {
    EXPECT_TRUE(integer_in_enum_range<Flags>(1));
    EXPECT_FALSE(integer_in_enum_range<Flags>(8));

    EXPECT_TRUE(enum_in_range(Color::Green, Color::Red, Color::Blue));
    EXPECT_TRUE(enum_in_range(Color::Red, Color::Red, Color::Blue));
    EXPECT_TRUE(enum_in_range(Color::Blue, Color::Red, Color::Blue));
}

// Alias tests
TEST_F(EnumTest, EnumAliases) {
    auto red = enum_cast_with_alias<Color>("R");
    EXPECT_TRUE(red.has_value());
    EXPECT_EQ(red.value(), Color::Red);
}

// Description tests
TEST_F(EnumTest, EnumDescriptions) {
    EXPECT_EQ(enum_description(Color::Red), "Red color");
    EXPECT_EQ(enum_description(Color::Green), "Green color");
    EXPECT_EQ(enum_description(Color::Blue), "Blue color");
}

// Serialization tests
TEST_F(EnumTest, Serialization) {
    auto serialized = serialize_enum(Color::Red);
    EXPECT_EQ(serialized, "Red");

    auto deserialized = deserialize_enum<Color>(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), Color::Red);
}

// Bitmask tests
TEST_F(EnumTest, Bitmasks) {
    auto mask = enum_bitmask(Flags::Flag1);
    EXPECT_EQ(mask, 1);

    auto flag = bitmask_to_enum<Flags>(1);
    EXPECT_TRUE(flag.has_value());
    EXPECT_EQ(flag.value(), Flags::Flag1);
}

// Default value tests
TEST_F(EnumTest, DefaultValue) {
    EXPECT_EQ(enum_default<Color>(), Color::Red);
    EXPECT_EQ(enum_default<Flags>(), Flags::None);
}

// Compound operations tests
TEST_F(EnumTest, CompoundOperations) {
    Flags f = Flags::Flag1;
    f |= Flags::Flag2;
    EXPECT_EQ(enum_to_integer(f), 3);

    f &= Flags::Flag1;
    EXPECT_EQ(f, Flags::Flag1);

    f ^= Flags::Flag2;
    EXPECT_EQ(enum_to_integer(f), 3);
}

}  // namespace atom::meta::test

#endif  // TEST_ATOM_META_ENUM_HPP