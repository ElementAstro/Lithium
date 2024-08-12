/*!
 * \file enum.hpp
 * \brief Enum Utilities
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ENUM_HPP
#define ATOM_META_ENUM_HPP

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>
#include <type_traits>

// EnumTraits 模板结构体需要为每个枚举类型特化。
template <typename T>
struct EnumTraits;

// 辅助函数：从编译器生成的函数签名中提取枚举名称
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

// 生成枚举值的字符串名称
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

// 枚举值转字符串
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

// 字符串转枚举值
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

// 枚举值转整数
template <typename T>
constexpr auto enum_to_integer(T value) noexcept {
    return static_cast<std::underlying_type_t<T>>(value);
}

// 整数转枚举值
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

// 检查枚举值是否有效
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

// 获取所有枚举值和名称
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

// 支持标志枚举（位运算）
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

// 获取枚举的默认值
template <typename T>
constexpr auto enum_default() noexcept -> T {
    return EnumTraits<T>::values[0];
}

// 根据名字排序枚举值
template <typename T>
constexpr auto enum_sorted_by_name() noexcept {
    auto entries = enum_entries<T>();
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    return entries;
}

// 根据整数值排序枚举值
template <typename T>
constexpr auto enum_sorted_by_value() noexcept {
    auto entries = enum_entries<T>();
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return enum_to_integer(a.first) < enum_to_integer(b.first);
    });
    return entries;
}

// 模糊匹配字符串并转换为枚举值
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

// 检查整数值是否在枚举范围内
template <typename T>
constexpr auto integer_in_enum_range(std::underlying_type_t<T> value) noexcept
    -> bool {
    constexpr auto VALUES = EnumTraits<T>::values;
    return std::any_of(VALUES.begin(), VALUES.end(),
                       [value](T e) { return enum_to_integer(e) == value; });
}

// 添加枚举别名支持
template <typename T>
struct EnumAliasTraits {
    static constexpr std::array<std::string_view, 0> ALIASES = {};
};

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

#endif  // ATOM_META_ENUM_HPP
