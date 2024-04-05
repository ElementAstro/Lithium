/*
 * sstring.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-2

Description: A simple static string class

**************************************************/

#ifndef ATOM_EXPERIMENT_SSTRING_HPP
#define ATOM_EXPERIMENT_SSTRING_HPP

#include <cstring>
#include <string_view>

class Static_String;
template <typename T>
concept Stringable = std::is_convertible_v<T, std::string> ||
                     std::is_convertible_v<T, std::string_view> ||
                     std::is_convertible_v<T, const char *> ||
                     std::is_convertible_v<T, Static_String>;

struct Static_String {
    template <size_t N>
    constexpr Static_String(const char (&str)[N]) noexcept
        : m_size(N - 1), data(str) {}

    constexpr size_t size() const noexcept { return m_size; }

    constexpr const char *c_str() const noexcept { return data; }

    constexpr const char *begin() const noexcept { return data; }

    constexpr const char *end() const noexcept { return data + m_size; }

    constexpr bool operator==(const std::string_view &other) const noexcept {
        return std::string_view(data, m_size) == other;
    }

    template <typename T>
        requires Stringable<T>
    constexpr bool operator==(T &&other) const noexcept {
        return std::string_view(data, m_size) == std::forward<T>(other);
    }

    template <typename T>
        requires Stringable<T>
    constexpr bool operator!=(T &&other) const noexcept {
        return !(*this == std::forward<T>(other));
    }

    template <typename T>
        requires Stringable<T>
    constexpr bool operator<(T &&other) const noexcept {
        return std::string_view(data, m_size) < std::forward<T>(other);
    }

    template <typename T>
        requires Stringable<T>
    constexpr bool operator<=(T &&other) const noexcept {
        return std::string_view(data, m_size) <= std::forward<T>(other);
    }

    template <typename T>
        requires Stringable<T>
    constexpr bool operator>(T &&other) const noexcept {
        return std::string_view(data, m_size) > std::forward<T>(other);
    }

    template <typename T>
        requires Stringable<T>
    constexpr bool operator>=(T &&other) const noexcept {
        return std::string_view(data, m_size) >= std::forward<T>(other);
    }

    const size_t m_size;
    const char *data;
};

#endif
