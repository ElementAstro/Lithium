/*!
 * \file enum.hpp
 * \brief Enum Utilities
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef ATOM_META_ENUM_HPP
#define ATOM_META_ENUM_HPP

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace atom::meta {
/*!
 * \brief Template struct for EnumTraits, needs to be specialized for each enum
 * type. \tparam T Enum type.
 */
template <typename T>
struct EnumTraits;

/*!
 * \brief Helper function to extract enum name from compiler-generated function
 * signature. \tparam T Enum type. \param func_sig Function signature. \return
 * Extracted enum name.
 */
template <typename T>
constexpr std::string_view extract_enum_name(const char* func_sig) {
    std::string_view name(func_sig);

    auto prefixPos = name.find("= ") + 2;
    auto suffixPos = name.rfind(']');

    if (prefixPos == std::string_view::npos ||
        suffixPos == std::string_view::npos) {
        prefixPos = name.find_last_of(' ') + 1;
        suffixPos = name.rfind("::");
    }

    return name.substr(prefixPos, suffixPos - prefixPos);
}

/*!
 * \brief Generate string name for enum value.
 * \tparam T Enum type.
 * \tparam Value Enum value.
 * \return String name of the enum value.
 */
template <typename T, T Value>
constexpr std::string_view enum_name() noexcept {
#if defined(__clang__) || defined(__GNUC__)
    return extract_enum_name<T>(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
    return extract_enum_name<T>(__FUNCSIG__);
#else
    return {};
#endif
}

/*!
 * \brief Convert enum value to string.
 * \tparam T Enum type.
 * \param value Enum value.
 * \return String name of the enum value.
 */
template <typename T>
constexpr auto enum_name(T value) noexcept -> std::string_view {
    constexpr auto VALUES = EnumTraits<T>::values;
    constexpr auto NAMES = EnumTraits<T>::names;

    for (size_t i = 0; i < VALUES.size(); ++i) {
        if (VALUES[i] == value) {
            return NAMES[i];
        }
    }
    return {};
}

/*!
 * \brief Convert string to enum value.
 * \tparam T Enum type.
 * \param name String name of the enum value.
 * \return Optional enum value.
 */
template <typename T>
constexpr auto enum_cast(std::string_view name) noexcept -> std::optional<T> {
    constexpr auto VALUES = EnumTraits<T>::values;
    constexpr auto NAMES = EnumTraits<T>::names;

    for (size_t i = 0; i < NAMES.size(); ++i) {
        if (NAMES[i] == name) {
            return VALUES[i];
        }
    }
    return std::nullopt;
}

/*!
 * \brief Convert enum value to integer.
 * \tparam T Enum type.
 * \param value Enum value.
 * \return Integer representation of the enum value.
 */
template <typename T>
constexpr auto enum_to_integer(T value) noexcept {
    return static_cast<std::underlying_type_t<T>>(value);
}

/*!
 * \brief Convert integer to enum value.
 * \tparam T Enum type.
 * \param value Integer value.
 * \return Optional enum value.
 */
template <typename T>
constexpr auto integer_to_enum(std::underlying_type_t<T> value) noexcept
    -> std::optional<T> {
    constexpr auto VALUES = EnumTraits<T>::values;

    for (const auto& val : VALUES) {
        if (enum_to_integer(val) == value) {
            return val;
        }
    }
    return std::nullopt;
}

/*!
 * \brief Check if enum value is valid.
 * \tparam T Enum type.
 * \param value Enum value.
 * \return True if valid, false otherwise.
 */
template <typename T>
constexpr auto enum_contains(T value) noexcept -> bool {
    constexpr auto VALUES = EnumTraits<T>::values;
    for (const auto& val : VALUES) {
        if (val == value) {
            return true;
        }
    }
    return false;
}

/*!
 * \brief Get all enum values and names.
 * \tparam T Enum type.
 * \return Array of pairs of enum values and their names.
 */
template <typename T>
constexpr auto enum_entries() noexcept {
    constexpr auto VALUES = EnumTraits<T>::values;
    constexpr auto NAMES = EnumTraits<T>::names;
    std::array<std::pair<T, std::string_view>, VALUES.size()> entries{};

    for (size_t i = 0; i < VALUES.size(); ++i) {
        entries[i] = {VALUES[i], NAMES[i]};
    }

    return entries;
}

/*!
 * \brief Support for flag enums (bitwise operations).
 * \tparam T Enum type.
 * \param lhs Left-hand side enum value.
 * \param rhs Right-hand side enum value.
 * \return Result of bitwise OR operation.
 */
template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator|(T lhs, T rhs) noexcept -> T {
    using UT = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<UT>(lhs) | static_cast<UT>(rhs));
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator|=(T& lhs, T rhs) noexcept -> T& {
    return lhs = lhs | rhs;
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator&(T lhs, T rhs) noexcept -> T {
    using UT = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<UT>(lhs) & static_cast<UT>(rhs));
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator&=(T& lhs, T rhs) noexcept -> T& {
    return lhs = lhs & rhs;
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator^(T lhs, T rhs) noexcept -> T {
    using UT = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<UT>(lhs) ^ static_cast<UT>(rhs));
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator^=(T& lhs, T rhs) noexcept -> T& {
    return lhs = lhs ^ rhs;
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr auto operator~(T rhs) noexcept -> T {
    using UT = std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<UT>(rhs));
}

/*!
 * \brief Get the default value of an enum.
 * \tparam T Enum type.
 * \return Default enum value.
 */
template <typename T>
constexpr auto enum_default() noexcept -> T {
    return EnumTraits<T>::values[0];
}

/*!
 * \brief Sort enum values by their names.
 * \tparam T Enum type.
 * \return Sorted array of pairs of enum values and their names.
 */
template <typename T>
constexpr auto enum_sorted_by_name() noexcept {
    auto entries = enum_entries<T>();
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    return entries;
}

/*!
 * \brief Sort enum values by their integer values.
 * \tparam T Enum type.
 * \return Sorted array of pairs of enum values and their names.
 */
template <typename T>
constexpr auto enum_sorted_by_value() noexcept {
    auto entries = enum_entries<T>();
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return enum_to_integer(a.first) < enum_to_integer(b.first);
    });
    return entries;
}

/*!
 * \brief Fuzzy match string and convert to enum value.
 * \tparam T Enum type.
 * \param name String name of the enum value.
 * \return Optional enum value.
 */
template <typename T>
auto enum_cast_fuzzy(std::string_view name) -> std::optional<T> {
    constexpr auto names = EnumTraits<T>::names;

    for (size_t i = 0; i < names.size(); ++i) {
        if (names[i].find(name) != std::string_view::npos) {
            return EnumTraits<T>::values[i];
        }
    }
    return std::nullopt;
}

/*!
 * \brief Check if integer value is within enum range.
 * \tparam T Enum type.
 * \param value Integer value.
 * \return True if within range, false otherwise.
 */
template <typename T>
constexpr auto integer_in_enum_range(std::underlying_type_t<T> value) noexcept
    -> bool {
    constexpr auto VALUES = EnumTraits<T>::values;
    return std::any_of(VALUES.begin(), VALUES.end(),
                       [value](T e) { return enum_to_integer(e) == value; });
}

/*!
 * \brief Support for enum aliases.
 * \tparam T Enum type.
 */
template <typename T>
struct EnumAliasTraits {
    static constexpr std::array<std::string_view, 0> aliases = {};
};

/*!
 * \brief Convert string to enum value with alias support.
 * \tparam T Enum type.
 * \param name String name of the enum value.
 * \return Optional enum value.
 */
template <typename T>
constexpr auto enum_cast_with_alias(std::string_view name) noexcept
    -> std::optional<T> {
    constexpr auto VALUES = EnumTraits<T>::values;
    constexpr auto NAMES = EnumTraits<T>::names;
    constexpr auto ALIASES = EnumAliasTraits<T>::aliases;

    for (size_t i = 0; i < NAMES.size(); ++i) {
        if (NAMES[i] == name || (i < ALIASES.size() && ALIASES[i] == name)) {
            return VALUES[i];
        }
    }
    return std::nullopt;
}

/*!
 * \brief Get the description of an enum value.
 * \tparam T Enum type.
 * \param value Enum value.
 * \return Description of the enum value.
 */
template <typename T>
constexpr auto enum_description(T value) noexcept -> std::string_view {
    constexpr auto VALUES = EnumTraits<T>::values;
    constexpr auto DESCRIPTIONS = EnumTraits<T>::descriptions;

    for (size_t i = 0; i < VALUES.size(); ++i) {
        if (VALUES[i] == value) {
            return DESCRIPTIONS[i];
        }
    }
    return {};
}

/*!
 * \brief Serialize enum value to string.
 * \tparam T Enum type.
 * \param value Enum value.
 * \return Serialized string.
 */
template <typename T>
auto serialize_enum(T value) -> std::string {
    return std::string(enum_name(value));
}

/*!
 * \brief Deserialize string to enum value.
 * \tparam T Enum type.
 * \param str Serialized string.
 * \return Optional enum value.
 */
template <typename T>
auto deserialize_enum(const std::string& str) -> std::optional<T> {
    return enum_cast<T>(str);
}

/*!
 * \brief Check if enum value is within a specified range.
 * \tparam T Enum type.
 * \param value Enum value.
 * \param min Minimum value.
 * \param max Maximum value.
 * \return True if within range, false otherwise.
 */
template <typename T>
constexpr auto enum_in_range(T value, T min, T max) noexcept -> bool {
    return value >= min && value <= max;
}

/*!
 * \brief Get the bitmask of an enum value.
 * \tparam T Enum type.
 * \param value Enum value.
 * \return Bitmask of the enum value.
 */
template <typename T>
constexpr auto enum_bitmask(T value) noexcept -> std::underlying_type_t<T> {
    return static_cast<std::underlying_type_t<T>>(value);
}

/*!
 * \brief Convert bitmask to enum value.
 * \tparam T Enum type.
 * \param bitmask Bitmask value.
 * \return Optional enum value.
 */
template <typename T>
constexpr auto bitmask_to_enum(std::underlying_type_t<T> bitmask) noexcept
    -> std::optional<T> {
    constexpr auto VALUES = EnumTraits<T>::values;

    for (const auto& val : VALUES) {
        if (enum_bitmask(val) == bitmask) {
            return val;
        }
    }
    return std::nullopt;
}
}  // namespace atom::meta

#endif  // ATOM_META_ENUM_HPP
