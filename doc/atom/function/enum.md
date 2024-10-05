# Enum Utilities Documentation

## Overview

This document covers a set of utility functions and templates for working with enums in C++. These utilities provide functionality for enum name extraction, conversion between enums and strings/integers, enum validation, and support for flag enums.

## Key Components

### EnumTraits

```cpp
template <typename T>
struct EnumTraits;
```

This is a template struct that needs to be specialized for each enum type. It should provide two static constexpr members:

- `values`: An array of all enum values
- `names`: An array of string_views representing the names of the enum values

### Core Functions

#### enum_name

```cpp
template <typename T, T Value>
constexpr std::string_view enum_name() noexcept;

template <typename T>
constexpr auto enum_name(T value) noexcept -> std::string_view;
```

These functions convert an enum value to its string representation.

Usage:

```cpp
auto name = enum_name<MyEnum, MyEnum::Value>();
auto name2 = enum_name(MyEnum::Value);
```

#### enum_cast

```cpp
template <typename T>
constexpr auto enum_cast(std::string_view name) noexcept -> std::optional<T>;
```

Converts a string to an enum value.

Usage:

```cpp
auto value = enum_cast<MyEnum>("ValueName");
```

#### enum_to_integer and integer_to_enum

```cpp
template <typename T>
constexpr auto enum_to_integer(T value) noexcept;

template <typename T>
constexpr auto integer_to_enum(std::underlying_type_t<T> value) noexcept -> std::optional<T>;
```

These functions convert between enum values and their underlying integer representations.

Usage:

```cpp
auto intValue = enum_to_integer(MyEnum::Value);
auto enumValue = integer_to_enum<MyEnum>(5);
```

#### enum_contains

```cpp
template <typename T>
constexpr auto enum_contains(T value) noexcept -> bool;
```

Checks if an enum value is valid (i.e., defined in the enum).

Usage:

```cpp
bool isValid = enum_contains(MyEnum::Value);
```

#### enum_entries

```cpp
template <typename T>
constexpr auto enum_entries() noexcept;
```

Returns an array of pairs containing all enum values and their string representations.

Usage:

```cpp
auto entries = enum_entries<MyEnum>();
```

### Flag Enum Support

The header provides overloaded bitwise operators for enums, allowing them to be used as flag enums:

```cpp
template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator|(T lhs, T rhs) noexcept -> T;

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator&(T lhs, T rhs) noexcept -> T;

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator^(T lhs, T rhs) noexcept -> T;

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator~(T rhs) noexcept -> T;
```

Usage:

```cpp
MyFlagEnum flags = MyFlagEnum::Flag1 | MyFlagEnum::Flag2;
```

### Additional Utilities

#### enum_default

```cpp
template <typename T>
constexpr auto enum_default() noexcept -> T;
```

Returns the default value of an enum (first value in the `EnumTraits::values` array).

Usage:

```cpp
auto defaultValue = enum_default<MyEnum>();
```

#### enum_sorted_by_name and enum_sorted_by_value

```cpp
template <typename T>
constexpr auto enum_sorted_by_name() noexcept;

template <typename T>
constexpr auto enum_sorted_by_value() noexcept;
```

Return sorted arrays of enum entries, either by name or by underlying value.

Usage:

```cpp
auto sortedByName = enum_sorted_by_name<MyEnum>();
auto sortedByValue = enum_sorted_by_value<MyEnum>();
```

#### enum_cast_fuzzy

```cpp
template <typename T>
auto enum_cast_fuzzy(std::string_view name) -> std::optional<T>;
```

Performs a fuzzy match to convert a string to an enum value.

Usage:

```cpp
auto value = enum_cast_fuzzy<MyEnum>("PartialName");
```

#### integer_in_enum_range

```cpp
template <typename T>
constexpr auto integer_in_enum_range(std::underlying_type_t<T> value) noexcept -> bool;
```

Checks if an integer value is within the range of valid enum values.

Usage:

```cpp
bool isInRange = integer_in_enum_range<MyEnum>(5);
```

#### enum_cast_with_alias

```cpp
template <typename T>
constexpr auto enum_cast_with_alias(std::string_view name) noexcept -> std::optional<T>;
```

Converts a string to an enum value, supporting aliases defined in `EnumAliasTraits`.

Usage:

```cpp
auto value = enum_cast_with_alias<MyEnum>("AliasName");
```

## Notes

- These utilities are designed to be used with C++17 or later.
- Many functions are marked `constexpr`, allowing for compile-time evaluation when possible.
- The `EnumTraits` struct must be specialized for each enum type you want to use with these utilities.
- The utilities support both scoped and unscoped enums.
- Some functions (`enum_name`, `extract_enum_name`) use compiler-specific features for name extraction.

## Example Usage

Here's an example of how to use these utilities with a custom enum:

```cpp
enum class Color { Red, Green, Blue };

template <>
struct EnumTraits<Color> {
    static constexpr std::array values = { Color::Red, Color::Green, Color::Blue };
    static constexpr std::array names = { "Red", "Green", "Blue" };
};

int main() {
    Color c = Color::Green;
    std::cout << enum_name(c) << std::endl;  // Outputs: Green

    auto blueOpt = enum_cast<Color>("Blue");
    if (blueOpt) {
        std::cout << "Enum cast successful" << std::endl;
    }

    auto entries = enum_entries<Color>();
    for (const auto& [value, name] : entries) {
        std::cout << name << ": " << enum_to_integer(value) << std::endl;
    }

    return 0;
}
```

This example demonstrates how to specialize `EnumTraits` for a custom enum and use various utility functions provided in the header.
