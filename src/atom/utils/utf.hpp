/*
 * utf.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-3

Description: Some useful functions about utf string

**************************************************/

#ifndef ATOM_UTILS_UTF_HPP
#define ATOM_UTILS_UTF_HPP

#include <string>

namespace atom::utils {
/**
 * @brief Converts a wide-character string to a UTF-8 encoded string.
 *
 * This function takes a wide-character string (e.g., `std::wstring` or
 * `std::wstring_view`) and converts it to a UTF-8 encoded string
 * (`std::string`).
 *
 * @param wstr A view of the wide-character string to be converted.
 * @return std::string The converted UTF-8 encoded string.
 */
auto toUTF8(std::wstring_view wstr) -> std::string;

/**
 * @brief Converts a UTF-8 encoded string to a wide-character string.
 *
 * This function takes a UTF-8 encoded string (e.g., `std::string` or
 * `std::string_view`) and converts it to a wide-character string
 * (`std::wstring`).
 *
 * @param str A view of the UTF-8 encoded string to be converted.
 * @return std::wstring The converted wide-character string.
 */
auto fromUTF8(std::string_view str) -> std::wstring;

/**
 * @brief Converts a UTF-8 encoded string to a UTF-16 encoded string.
 *
 * This function takes a UTF-8 encoded string (e.g., `std::string` or
 * `std::string_view`) and converts it to a UTF-16 encoded string
 * (`std::u16string`).
 *
 * @param str A view of the UTF-8 encoded string to be converted.
 * @return std::u16string The converted UTF-16 encoded string.
 */
auto utf8toUtF16(std::string_view str) -> std::u16string;

/**
 * @brief Converts a UTF-8 encoded string to a UTF-32 encoded string.
 *
 * This function takes a UTF-8 encoded string (e.g., `std::string` or
 * `std::string_view`) and converts it to a UTF-32 encoded string
 * (`std::u32string`).
 *
 * @param str A view of the UTF-8 encoded string to be converted.
 * @return std::u32string The converted UTF-32 encoded string.
 */
auto utf8toUtF32(std::string_view str) -> std::u32string;

/**
 * @brief Converts a UTF-16 encoded string to a UTF-8 encoded string.
 *
 * This function takes a UTF-16 encoded string (e.g., `std::u16string` or
 * `std::u16string_view`) and converts it to a UTF-8 encoded string
 * (`std::string`).
 *
 * @param str A view of the UTF-16 encoded string to be converted.
 * @return std::string The converted UTF-8 encoded string.
 */
auto utf16toUtF8(std::u16string_view str) -> std::string;

/**
 * @brief Converts a UTF-16 encoded string to a UTF-32 encoded string.
 *
 * This function takes a UTF-16 encoded string (e.g., `std::u16string` or
 * `std::u16string_view`) and converts it to a UTF-32 encoded string
 * (`std::u32string`).
 *
 * @param str A view of the UTF-16 encoded string to be converted.
 * @return std::u32string The converted UTF-32 encoded string.
 */
auto utf16toUtF32(std::u16string_view str) -> std::u32string;

/**
 * @brief Converts a UTF-32 encoded string to a UTF-8 encoded string.
 *
 * This function takes a UTF-32 encoded string (e.g., `std::u32string` or
 * `std::u32string_view`) and converts it to a UTF-8 encoded string
 * (`std::string`).
 *
 * @param str A view of the UTF-32 encoded string to be converted.
 * @return std::string The converted UTF-8 encoded string.
 */
auto utf32toUtF8(std::u32string_view str) -> std::string;

/**
 * @brief Converts a UTF-32 encoded string to a UTF-16 encoded string.
 *
 * This function takes a UTF-32 encoded string (e.g., `std::u32string` or
 * `std::u32string_view`) and converts it to a UTF-16 encoded string
 * (`std::u16string`).
 *
 * @param str A view of the UTF-32 encoded string to be converted.
 * @return std::u16string The converted UTF-16 encoded string.
 */
auto utf32toUtF16(std::u32string_view str) -> std::u16string;

/**
 * @brief Validates if a UTF-8 encoded string is well-formed.
 *
 * This function checks if the provided UTF-8 encoded string (e.g.,
 * `std::string` or `std::string_view`) is a valid UTF-8 sequence. It does not
 * verify the string against other criteria beyond UTF-8 encoding.
 *
 * @param str A view of the UTF-8 encoded string to be validated.
 * @return bool `true` if the string is a valid UTF-8 encoded string; `false`
 * otherwise.
 */
auto isValidUTF8(std::string_view str) -> bool;

}  // namespace atom::utils

#endif  // ATOM_UTILS_UTF_HPP
