/*
 * cstring.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-18

Description: String methods in compilation time

**************************************************/

#ifndef ATOM_UTILS_CSTRING_HPP
#define ATOM_UTILS_CSTRING_HPP

#include <algorithm>
#include <array>
#include <string_view>

using namespace std::literals;

namespace atom::utils {
template <std::size_t N>
constexpr auto deduplicate(const char (&str)[N]) {
    std::array<char, N> result{};
    std::size_t index = 0;

    for (std::size_t i = 0; i < N - 1; ++i) {
        if (std::find(result.begin(), result.begin() + index, str[i]) ==
            result.begin() + index) {
            result[index++] = str[i];
        }
    }

    result[index] = '\0';
    return result;
}

template<std::size_t N, std::size_t... Is>
constexpr auto splitImpl(const char(&str)[N], char delimiter, std::index_sequence<Is...>) {
    std::array<std::string_view, N> result{};
    size_t index = 0;
    size_t start = 0;

    for (size_t i = 0; i < N; ++i) {
        if (str[i] == delimiter) {
            result[index++] = std::string_view(str + start, i - start);
            start = i + 1;
        }
    }

    if (start < N) {
        result[index++] = std::string_view(str + start, N - start);
    }

    return result;
}

template<std::size_t N>
constexpr auto split(const char(&str)[N], char delimiter) {
    return splitImpl(str, delimiter, std::make_index_sequence<N>());
}

template <std::size_t N>
constexpr auto replace(const char (&str)[N], char oldChar, char newChar) {
    std::array<char, N> result{};
    for (std::size_t i = 0; i < N - 1; ++i) {
        result[i] = (str[i] == oldChar) ? newChar : str[i];
    }
    result[N - 1] = '\0';
    return result;
}

template <std::size_t N>
constexpr auto toLower(const char (&str)[N]) {
    std::array<char, N> result{};
    for (std::size_t i = 0; i < N - 1; ++i) {
        result[i] =
            (str[i] >= 'A' && str[i] <= 'Z') ? str[i] + ('a' - 'A') : str[i];
    }
    result[N - 1] = '\0';
    return result;
}

template <std::size_t N>
constexpr auto toUpper(const char (&str)[N]) {
    std::array<char, N> result{};
    for (std::size_t i = 0; i < N - 1; ++i) {
        result[i] =
            (str[i] >= 'a' && str[i] <= 'z') ? str[i] - ('a' - 'A') : str[i];
    }
    result[N - 1] = '\0';
    return result;
}

template <std::size_t N1, std::size_t N2>
constexpr auto concat(const char (&str1)[N1], const char (&str2)[N2]) {
    std::array<char, N1 + N2 - 1> result{};
    std::size_t index = 0;
    for (std::size_t i = 0; i < N1 - 1; ++i) {
        result[index++] = str1[i];
    }
    for (std::size_t i = 0; i < N2 - 1; ++i) {
        result[index++] = str2[i];
    }
    return result;
}

template <std::size_t N>
constexpr auto trim(const char (&str)[N]) {
    std::array<char, N> result{};
    std::size_t index = 0;

    auto view = std::string_view(str);

    auto start = view.find_first_not_of(' ');
    if (start == std::string_view::npos) {
        result[0] = '\0'; // 如果全是空格，返回空字符串
        return result;
    }

    auto end = view.find_last_not_of(' ');

    std::ranges::copy(view.substr(start, end - start + 1), result.begin());

    result[end - start + 1] = '\0';

    return result;
}

template <std::size_t N>
constexpr auto substring(const char (&str)[N], std::size_t start,
                         std::size_t length) {
    std::array<char, N> result{};
    std::size_t index = 0;
    for (std::size_t i = start; i < start + length && i < N - 1; ++i) {
        result[index++] = str[i];
    }
    result[index] = '\0';
    return result;
}

template <std::size_t N1, std::size_t N2>
constexpr bool equal(const char (&str1)[N1], const char (&str2)[N2]) {
    if (N1 != N2) {
        return false;
    }
    for (std::size_t i = 0; i < N1 - 1; ++i) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

template <std::size_t N>
constexpr std::size_t find(const char (&str)[N], char ch) {
    for (std::size_t i = 0; i < N - 1; ++i) {
        if (str[i] == ch) {
            return i;
        }
    }
    return N - 1;
}

template <std::size_t N>
constexpr std::size_t length([[maybe_unused]] const char (&str)[N]) {
    return N - 1;
}

template <std::size_t N>
constexpr auto reverse(const char (&str)[N]) {
    std::array<char, N> result{};
    for (std::size_t i = 0; i < N - 1; ++i) {
        result[i] = str[N - 2 - i];
    }
    result[N - 1] = '\0';
    return result;
}

inline constexpr std::string_view trim(std::string_view str) noexcept {
    constexpr auto WHITESPACE = " \t\n\r\f\v"sv;
    const auto START = str.find_first_not_of(WHITESPACE);
    if (START == std::string_view::npos) {
        return {};
    }
    const auto END = str.find_last_not_of(WHITESPACE);
    return str.substr(START, END - START + 1);
}
}  // namespace atom::utils

#endif
