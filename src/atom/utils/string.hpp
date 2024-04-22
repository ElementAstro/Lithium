/*
 * string.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Some useful string functions

**************************************************/

#ifndef ATOM_UTILS_STRING_HPP
#define ATOM_UTILS_STRING_HPP

#include <string>
#include <string_view>
#include <vector>

namespace Atom::Utils {
/**
 * @brief Checks if the given string contains any uppercase characters.
 *
 * @param str The string to check.
 * @return true if the string contains uppercase characters, otherwise false.
 */
[[nodiscard]] bool hasUppercase(std::string_view str);

/**
 * @brief Converts the given string to snake_case format.
 *
 * @param str The string to convert.
 * @return The string converted to snake_case.
 */
[[nodiscard]] std::string toUnderscore(std::string_view str);

/**
 * @brief Converts the given string to camelCase format.
 *
 * @param str The string to convert.
 * @return The string converted to camelCase.
 */
[[nodiscard]] std::string toCamelCase(std::string_view str);

/**
 * @brief Encodes the given string using URL encoding.
 *
 * @param str The string to encode.
 * @return The URL encoded string.
 */
[[nodiscard]] std::string urlEncode(std::string_view str);

/**
 * @brief Decodes the given URL encoded string.
 *
 * @param str The URL encoded string to decode.
 * @return The decoded string.
 */
[[nodiscard]] std::string urlDecode(std::string_view str);

/**
 * @brief Checks if the given string starts with the specified prefix.
 *
 * @param str The string to check.
 * @param prefix The prefix to search for.
 * @return true if the string starts with the prefix, otherwise false.
 */
[[nodiscard]] bool startsWith(std::string_view str, std::string_view prefix);

/**
 * @brief Checks if the given string ends with the specified suffix.
 *
 * @param str The string to check.
 * @param suffix The suffix to search for.
 * @return true if the string ends with the suffix, otherwise false.
 */
[[nodiscard]] bool endsWith(std::string_view str, std::string_view suffix);

/**
 * @brief 将字符串分割为多个字符串。
 * @param input 输入字符串。
 * @param delimiter 分隔符。
 * @return 分割后的字符串数组。
 */
[[nodiscard(
    "the result of splitString is not used")]] std::vector<std::string_view>
splitString(const std::string& str, char delimiter);

/**
 * @brief Concatenates an array of strings into a single string with a specified
 * delimiter.
 *
 * @param strings The array of strings to concatenate.
 * @param delimiter The delimiter to use for concatenation.
 * @return The concatenated string.
 */
[[nodiscard("the result of joinStrings is not used")]] std::string joinStrings(
    const std::vector<std::string_view>& strings,
    const std::string_view& delimiter);

/**
 * @brief Replaces all occurrences of a substring with another substring in a
 * given text.
 *
 * @param text The text in which replacements will be made.
 * @param oldStr The substring to replace.
 * @param newStr The substring to replace with.
 * @return The text with replacements made.
 */
[[nodiscard("the result of replaceString is not used")]] std::string
replaceString(std::string_view text, std::string_view oldStr,
              std::string_view newStr);

/**
 * @brief Replaces multiple substrings with their corresponding replacements in
 * a given text.
 *
 * @param text The text in which replacements will be made.
 * @param replacements A vector of pairs, where each pair represents the
 * substring to replace and its replacement.
 * @return The text with replacements made.
 */
[[nodiscard("the result of replaceStrings is not used")]] std::string
replaceStrings(std::string_view text,
               const std::vector<std::pair<std::string_view, std::string_view>>&
                   replacements);

}  // namespace Atom::Utils

#endif