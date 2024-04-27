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


#include <array>
#include <string_view>

template <std::size_t N>
constexpr auto deduplicate(const char (&str)[N]) {
    std::array<char, N> result{};
    std::size_t index = 0;

    for (std::size_t i = 0; i < N - 1; ++i) {
        bool duplicate = false;
        for (std::size_t j = 0; j < index; ++j) {
            if (str[i] == result[j]) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            result[index++] = str[i];
        }
    }

    result[index] = '\0';
    return result;
}

constexpr auto split(std::string_view str, char delimiter) {
    std::array<std::string_view, 10> result;
    std::size_t pos = 0;
    std::size_t index = 0;
    while ((pos = str.find(delimiter)) != std::string_view::npos &&
           index < result.size()) {
        result[index++] = str.substr(0, pos);
        str.remove_prefix(pos + 1);
    }
    if (index < result.size()) {
        result[index++] = str;
    }
    return result;
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
    for (std::size_t i = 0; i < N - 1; ++i) {
        if (str[i] != ' ') {
            result[index++] = str[i];
        }
    }
    result[index] = '\0';
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
constexpr std::size_t length(const char (&str)[N]) {
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

#endif
