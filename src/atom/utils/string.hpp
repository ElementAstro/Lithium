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

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace atom::utils {
/**
 * @brief Checks if the given string contains any uppercase characters.
 *
 * @param str The string to check.
 * @return true if the string contains uppercase characters, otherwise false.
 */
[[nodiscard]] auto hasUppercase(std::string_view str) -> bool;

/**
 * @brief Converts the given string to snake_case format.
 *
 * @param str The string to convert.
 * @return The string converted to snake_case.
 */
[[nodiscard]] auto toUnderscore(std::string_view str) -> std::string;

/**
 * @brief Converts the given string to camelCase format.
 *
 * @param str The string to convert.
 * @return The string converted to camelCase.
 */
[[nodiscard]] auto toCamelCase(std::string_view str) -> std::string;

/**
 * @brief Encodes the given string using URL encoding.
 *
 * @param str The string to encode.
 * @return The URL encoded string.
 */
[[nodiscard]] auto urlEncode(std::string_view str) -> std::string;

/**
 * @brief Decodes the given URL encoded string.
 *
 * @param str The URL encoded string to decode.
 * @return The decoded string.
 */
[[nodiscard]] auto urlDecode(std::string_view str) -> std::string;

/**
 * @brief Checks if the given string starts with the specified prefix.
 *
 * @param str The string to check.
 * @param prefix The prefix to search for.
 * @return true if the string starts with the prefix, otherwise false.
 */
[[nodiscard]] auto startsWith(std::string_view str,
                              std::string_view prefix) -> bool;

/**
 * @brief Checks if the given string ends with the specified suffix.
 *
 * @param str The string to check.
 * @param suffix The suffix to search for.
 * @return true if the string ends with the suffix, otherwise false.
 */
[[nodiscard]] auto endsWith(std::string_view str,
                            std::string_view suffix) -> bool;

/**
 * @brief 将字符串分割为多个字符串。
 * @param input 输入字符串。
 * @param delimiter 分隔符。
 * @return 分割后的字符串数组。
 */
[[nodiscard("the result of splitString is not used")]] auto splitString(
    const std::string& str, char delimiter) -> std::vector<std::string>;

/**
 * @brief Concatenates an array of strings into a single string with a specified
 * delimiter.
 *
 * @param strings The array of strings to concatenate.
 * @param delimiter The delimiter to use for concatenation.
 * @return The concatenated string.
 */
[[nodiscard("the result of joinStrings is not used")]] auto joinStrings(
    const std::vector<std::string_view>& strings,
    const std::string_view& delimiter) -> std::string;

/**
 * @brief Replaces all occurrences of a substring with another substring in a
 * given text.
 *
 * @param text The text in which replacements will be made.
 * @param oldStr The substring to replace.
 * @param newStr The substring to replace with.
 * @return The text with replacements made.
 */
[[nodiscard("the result of replaceString is not used")]] auto replaceString(
    std::string_view text, std::string_view oldStr,
    std::string_view newStr) -> std::string;

/**
 * @brief Replaces multiple substrings with their corresponding replacements in
 * a given text.
 *
 * @param text The text in which replacements will be made.
 * @param replacements A vector of pairs, where each pair represents the
 * substring to replace and its replacement.
 * @return The text with replacements made.
 */
[[nodiscard("the result of replaceStrings is not used")]] auto replaceStrings(
    std::string_view text,
    const std::vector<std::pair<std::string_view, std::string_view>>&
        replacements) -> std::string;

/**
 * @brief Converts a vector of string_view to a vector of string.
 *
 * @param svv The vector of string_view to convert.
 * @return The converted vector of string.
 */
[[nodiscard("the result of SVVtoSV is not used")]]
auto SVVtoSV(const std::vector<std::string_view>& svv)
    -> std::vector<std::string>;

/**
 * @brief Explodes a string_view into a vector of string_view.
 *
 * @param text The string_view to explode.
 * @param symbol The symbol to use for exploding.
 * @return The exploded vector of string_view.
 */
[[nodiscard("the result of explode is not used")]]
auto explode(std::string_view text, char symbol) -> std::vector<std::string>;

/**
 * @brief Trims a string_view.
 *
 * @param line The string_view to trim.
 * @param symbols The symbols to trim.
 * @return The trimmed string_view.
 */
[[nodiscard("the result of trim is not used")]]
auto trim(std::string_view line,
          std::string_view symbols = " \n\r\t") -> std::string;

/**
 * @brief Converts a u8string to a wstring.
 *
 * @param u8str The u8string to convert.
 * @return The converted wstring.
 */
[[nodiscard("the result of stringToWString is not used")]]
auto stringToWString(const std::string& str) -> std::wstring;

/**
 * @brief Converts a wstring to a u8string.
 *
 * @param wstr The wstring to convert.
 * @return The converted u8string.
 */
[[nodiscard("the result of wstringToString is not used")]]
auto wstringToString(const std::wstring& wstr) -> std::string;

/**
 * @brief Converts a string to a long integer.
 *
 * @param str The string to convert.
 * @param idx A pointer to the index of the first character after the number.
 * @param base The base of the number (default is 10).
 * @return The converted long integer.
 */
[[nodiscard("the result of stol is not used")]]
auto stod(std::string_view str, std::size_t* idx = nullptr) -> double;

/**
 * @brief Converts a string to a float.
 *
 * @param str The string to convert.
 * @param idx A pointer to the index of the first character after the number.
 * @return The converted float.
 */
[[nodiscard("the result of stof is not used")]]
auto stof(std::string_view str, std::size_t* idx = nullptr) -> float;

/**
 * @brief Converts a string to an integer.
 *
 * @param str The string to convert.
 * @param idx A pointer to the index of the first character after the number.
 * @param base The base of the number (default is 10).
 * @return The converted integer.
 */
[[nodiscard("the result of stoi is not used")]]
auto stoi(std::string_view str, std::size_t* idx = nullptr,
          int base = 10) -> int;

/**
 * @brief Converts a string to a long integer.
 *
 * @param str The string to convert.
 * @param idx A pointer to the index of the first character after the number.
 * @param base The base of the number (default is 10).
 * @return The converted long integer.
 */
[[nodiscard("the result of stol is not used")]]
auto stol(std::string_view str, std::size_t* idx = nullptr,
          int base = 10) -> long;

/**
 * @brief Splits a string into multiple strings.
 *
 * @param str The input string.
 * @param delimiter The delimiter.
 * @return The array of split strings.
 */
[[nodiscard("the result of nstrtok is not used")]]
auto nstrtok(std::string_view& str,
             const std::string_view& delims) -> std::optional<std::string_view>;
}  // namespace atom::utils

#endif
