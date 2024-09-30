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
#include <charconv>
#include <string>
#include <string_view>

using namespace std::literals;

namespace atom::utils {

/**
 * @brief Deduplicates characters in a C-style string.
 *
 * This function removes duplicate characters from a C-style string
 * and returns a new C-style string with the unique characters.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string with potential duplicate characters.
 * @return std::array<char, N> A new C-style string with duplicate characters
 * removed.
 */
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

/**
 * @brief Splits a C-style string into substrings based on a delimiter.
 *
 * This function splits a C-style string into substrings wherever the
 * specified delimiter character is found, returning an array of
 * `std::string_view` representing the substrings.
 *
 * @tparam N The size of the input C-style string.
 * @tparam Is A parameter pack of indices for template specialization.
 * @param str The input C-style string to be split.
 * @param delimiter The delimiter character used to split the string.
 * @param indices A sequence of indices used for the split operation.
 * @return std::array<std::string_view, N> An array of `std::string_view`
 * representing the substrings.
 */
template <std::size_t N, std::size_t... Is>
constexpr auto splitImpl(const char (&str)[N], char delimiter,
                         std::index_sequence<Is...>) {
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

/**
 * @brief Splits a C-style string into substrings based on a delimiter.
 *
 * This function splits a C-style string into substrings wherever the
 * specified delimiter character is found, returning an array of
 * `std::string_view` representing the substrings.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string to be split.
 * @param delimiter The delimiter character used to split the string.
 * @return std::array<std::string_view, N> An array of `std::string_view`
 * representing the substrings.
 */
template <std::size_t N>
constexpr auto split(const char (&str)[N], char delimiter) {
    return splitImpl(str, delimiter, std::make_index_sequence<N>());
}

/**
 * @brief Replaces all occurrences of a character in a C-style string.
 *
 * This function replaces all occurrences of a specified character in a C-style
 * string with a new character and returns the modified string.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string where replacements will be made.
 * @param oldChar The character to be replaced.
 * @param newChar The character to replace with.
 * @return std::array<char, N> A new C-style string with characters replaced.
 */
template <std::size_t N>
constexpr auto replace(const char (&str)[N], char oldChar, char newChar) {
    std::array<char, N> result{};
    for (std::size_t i = 0; i < N - 1; ++i) {
        result[i] = (str[i] == oldChar) ? newChar : str[i];
    }
    result[N - 1] = '\0';
    return result;
}

/**
 * @brief Converts all characters in a C-style string to lowercase.
 *
 * This function converts all uppercase characters in a C-style string
 * to their lowercase equivalents and returns the modified string.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string to be converted to lowercase.
 * @return std::array<char, N> A new C-style string with all characters
 * converted to lowercase.
 */
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

/**
 * @brief Converts all characters in a C-style string to uppercase.
 *
 * This function converts all lowercase characters in a C-style string
 * to their uppercase equivalents and returns the modified string.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string to be converted to uppercase.
 * @return std::array<char, N> A new C-style string with all characters
 * converted to uppercase.
 */
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

/**
 * @brief Concatenates two C-style strings.
 *
 * This function concatenates two C-style strings and returns a new C-style
 * string that combines the contents of both input strings.
 *
 * @tparam N1 The size of the first input C-style string.
 * @tparam N2 The size of the second input C-style string.
 * @param str1 The first C-style string to be concatenated.
 * @param str2 The second C-style string to be concatenated.
 * @return std::array<char, N1 + N2 - 1> A new C-style string resulting from the
 * concatenation of `str1` and `str2`.
 */
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

/**
 * @brief Trims leading and trailing whitespace from a C-style string.
 *
 * This function removes leading and trailing whitespace characters from a
 * C-style string and returns a new C-style string with the remaining content.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string to be trimmed.
 * @return std::array<char, N> A new C-style string with leading and trailing
 * whitespace removed.
 */
template <std::size_t N>
constexpr auto trim(const char (&str)[N]) {
    std::array<char, N> result{};

    auto view = std::string_view(str);

    auto start = view.find_first_not_of(' ');
    if (start == std::string_view::npos) {
        result[0] = '\0';  // If the string contains only spaces, return an
                           // empty string.
        return result;
    }

    auto end = view.find_last_not_of(' ');

    std::ranges::copy(view.substr(start, end - start + 1), result.begin());

    result[end - start + 1] = '\0';

    return result;
}

/*/ * @brief Extracts a substring from a C-style string.
 *
 * This function extracts a substring of a specified length from a C-style
 * string, starting at a given index, and returns the new substring.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string from which the substring will be
 * extracted.
 * @param start The starting index of the substring.
 * @param length The length of the substring.
 * @return std::array<char, N> A new C-style string containing the extracted
 * substring.
 */
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

/**
 * @brief Compares two C-style strings for equality.
 *
 * This function compares two C-style strings to determine if they are equal.
 * The comparison is performed lexicographically, and the function returns
 * `true` if the strings are equal and `false` otherwise.
 *
 * @tparam N1 The size of the first input C-style string.
 * @tparam N2 The size of the second input C-style string.
 * @param str1 The first C-style string to be compared.
 * @param str2 The second C-style string to be compared.
 * @return bool `true` if the strings are equal, `false` otherwise.
 */
template <std::size_t N1, std::size_t N2>
constexpr auto equal(const char (&str1)[N1], const char (&str2)[N2]) -> bool {
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

/**
 * @brief Finds the first occurrence of a character in a C-style string.
 *
 * This function searches for the first occurrence of a specified character
 * in a C-style string and returns its index. If the character is not found,
 * the function returns the size of the string minus one.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string in which the search is performed.
 * @param ch The character to be found.
 * @return std::size_t The index of the first occurrence of the character, or `N
 * - 1` if not found.
 */
template <std::size_t N>
constexpr auto find(const char (&str)[N], char ch) -> std::size_t {
    for (std::size_t i = 0; i < N - 1; ++i) {
        if (str[i] == ch) {
            return i;
        }
    }
    return N - 1;
}

/**
 * @brief Returns the length of a C-style string.
 *
 * This function returns the length of a C-style string, which is defined as
 * the number of characters before the null terminator.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string.
 * @return std::size_t The length of the C-style string.
 */
template <std::size_t N>
constexpr auto length([[maybe_unused]] const char (&str)[N]) -> std::size_t {
    return N - 1;
}

/**
 * @brief Reverses the characters in a C-style string.
 *
 * This function reverses the characters in a C-style string and returns a new
 * C-style string with the characters in reverse order.
 *
 * @tparam N The size of the input C-style string.
 * @param str The input C-style string to be reversed.
 * @return std::array<char, N> A new C-style string with the characters
 * reversed.
 */
template <std::size_t N>
constexpr auto reverse(const char (&str)[N]) {
    std::array<char, N> result{};
    for (std::size_t i = 0; i < N - 1; ++i) {
        result[i] = str[N - 2 - i];
    }
    result[N - 1] = '\0';
    return result;
}

/**
 * @brief Trims leading and trailing whitespace from a `std::string_view`.
 *
 * This function removes leading and trailing whitespace characters from a
 * `std::string_view` and returns a new `std::string_view` with the remaining
 * content.
 *
 * @param str The `std::string_view` to be trimmed.
 * @return std::string_view A new `std::string_view` with leading and trailing
 * whitespace removed.
 */
constexpr auto trim(std::string_view str) noexcept -> std::string_view {
    constexpr auto WHITESPACE = " \t\n\r\f\v"sv;
    const auto START = str.find_first_not_of(WHITESPACE);
    if (START == std::string_view::npos) {
        return {};
    }
    const auto END = str.find_last_not_of(WHITESPACE);
    return str.substr(START, END - START + 1);
}

constexpr int BASE_10 = 10;
constexpr int BASE_2 = 2;
constexpr int BASE_16 = 16;
constexpr int MIN_DIGIT = 10;

// Constexpr version for compile-time string literals
template <size_t N>
constexpr auto charArrayToArrayConstexpr(const std::array<char, N>& input)
    -> std::array<char, N> {
    std::array<char, N> result{};
#pragma unroll
    for (size_t i = 0; i < N; ++i) {
        result[i] = input[i];
    }
    return result;
}

// Non-constexpr version for runtime strings
template <size_t N>
auto charArrayToArray(const std::array<char, N>& input) -> std::array<char, N> {
    std::array<char, N> result{};
    std::copy_n(input.begin(), N, result.begin());
    return result;
}

// Rest of the functions remain the same
template <size_t N>
constexpr auto isNegative(const std::array<char, N>& arr) -> bool {
    if constexpr (N > 1) {
        return arr[0] == '-';
    }
    return false;
}

template <size_t N>
constexpr auto arrayToInt(const std::array<char, N>& arr,
                          int base = BASE_10) -> int {
    int result = 0;
    const char* begin = arr.data();
    const char* end = arr.data() + arr.size();
    std::from_chars(begin, end, result, base);
    return result;
}

template <size_t N>
constexpr auto absoluteValue(const std::array<char, N>& arr) -> int {
    int value = arrayToInt(arr);
    return std::abs(value);
}

template <size_t N>
auto convertBase(const std::array<char, N>& arr, int from_base,
                 int to_base) -> std::string {
    int value = arrayToInt(arr, from_base);
    std::string result;

    if (value == 0) {
        return "0";
    }

    bool isNegative = value < 0;
    value = std::abs(value);

    while (value > 0) {
        int digit = value % to_base;
        char character =
            digit < MIN_DIGIT ? '0' + digit : 'A' + digit - MIN_DIGIT;
        result.append(1, character);
        value /= to_base;
    }

    if (isNegative) {
        result = "-" + result;
    }

    return result;
}

}  // namespace atom::utils

#endif
