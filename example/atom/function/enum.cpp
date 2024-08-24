/*!
 * \file enum_examples.cpp
 * \brief Examples of using enum utilities.
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/enum.hpp"

#include <array>
#include <iostream>
#include <string_view>

// Define an enum for demonstration
enum class Color { Red, Green, Blue, Yellow };

// Specialize EnumTraits for Color
template <>
struct EnumTraits<Color> {
    static constexpr std::array<Color, 4> values = {Color::Red, Color::Green,
                                                    Color::Blue, Color::Yellow};
    static constexpr std::array<std::string_view, 4> names = {"Red", "Green",
                                                              "Blue", "Yellow"};
};

// Define another enum for demonstration
enum class Direction { North, East, South, West };

// Specialize EnumTraits for Direction
template <>
struct EnumTraits<Direction> {
    static constexpr std::array<Direction, 4> values = {
        Direction::North, Direction::East, Direction::South, Direction::West};
    static constexpr std::array<std::string_view, 4> names = {"North", "East",
                                                              "South", "West"};
};

// Specialize EnumAliasTraits for Direction
template <>
struct EnumAliasTraits<Direction> {
    static constexpr std::array<std::string_view, 4> ALIASES = {"N", "E", "S",
                                                                "W"};
};

// Example usage of the utility functions
int main() {
    // Example 1: Enum to String and String to Enum
    Color color = Color::Green;
    std::string_view colorName = enum_name(color);
    std::cout << "Color: " << colorName << std::endl;

    std::optional<Color> colorFromString = enum_cast<Color>("Blue");
    if (colorFromString) {
        std::cout << "Color from string: " << enum_name(*colorFromString)
                  << std::endl;
    } else {
        std::cout << "Color not found" << std::endl;
    }

    // Example 2: Integer to Enum and Enum to Integer
    auto colorInt = enum_to_integer(Color::Yellow);
    std::cout << "Color Yellow as integer: " << colorInt << std::endl;

    std::optional<Color> colorFromInt = integer_to_enum<Color>(2);
    if (colorFromInt) {
        std::cout << "Enum from integer 2: " << enum_name(*colorFromInt)
                  << std::endl;
    } else {
        std::cout << "Enum not found for integer 2" << std::endl;
    }

    // Example 3: Enum contains check
    if (enum_contains(Color::Red)) {
        std::cout << "Color Red is a valid enum value" << std::endl;
    } else {
        std::cout << "Color Red is not a valid enum value" << std::endl;
    }

    // Example 4: Get all enum entries
    auto entries = enum_entries<Color>();
    std::cout << "Color enum entries:" << std::endl;
    for (const auto& [value, name] : entries) {
        std::cout << "  " << name << " (" << enum_to_integer(value) << ")"
                  << std::endl;
    }

    // Example 5: Sorted by name and value
    auto sortedByName = enum_sorted_by_name<Color>();
    std::cout << "Color enum sorted by name:" << std::endl;
    for (const auto& [value, name] : sortedByName) {
        std::cout << "  " << name << " (" << enum_to_integer(value) << ")"
                  << std::endl;
    }

    auto sortedByValue = enum_sorted_by_value<Color>();
    std::cout << "Color enum sorted by value:" << std::endl;
    for (const auto& [value, name] : sortedByValue) {
        std::cout << "  " << name << " (" << enum_to_integer(value) << ")"
                  << std::endl;
    }

    // Example 6: Fuzzy match enum
    auto directionFromFuzzyName = enum_cast_fuzzy<Direction>("E");
    if (directionFromFuzzyName) {
        std::cout << "Direction from fuzzy name 'E': "
                  << enum_name(*directionFromFuzzyName) << std::endl;
    } else {
        std::cout << "Direction not found from fuzzy name 'E'" << std::endl;
    }

    // Example 7: Enum with aliases
    auto directionFromAlias = enum_cast_with_alias<Direction>("S");
    if (directionFromAlias) {
        std::cout << "Direction from alias 'S': "
                  << enum_name(*directionFromAlias) << std::endl;
    } else {
        std::cout << "Direction not found from alias 'S'" << std::endl;
    }

    return 0;
}
