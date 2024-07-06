/*
 * convert.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-18

Description: Convert Utils for Windows

**************************************************/

#ifndef ATOM_UTILS_CONVERT_HPP
#define ATOM_UTILS_CONVERT_HPP

#ifdef _WIN32

#include <windows.h>
#include <string>
#include <string_view>

namespace atom::utils {
/**
 * @brief Converts a string_view containing char characters to LPWSTR (wide
 * character string).
 * @param charString The string_view containing char characters to be converted.
 * @return LPWSTR representing the wide character version of the input string.
 */
[[nodiscard]] auto CharToLPWSTR(std::string_view charString) -> LPWSTR;

/**
 * @brief Converts a WCHAR array to a std::string.
 *
 * This function converts a wide character array (WCHAR) to a std::string
 * using the UTF-8 encoding. It uses the WideCharToMultiByte function
 * provided by the Windows API for the conversion.
 *
 * @param wCharArray A pointer to the null-terminated wide character array
 * (WCHAR) to be converted.
 * @return [[nodiscard]] std::string The converted std::string in UTF-8
 * encoding.
 */
[[nodiscard]] auto WCharArrayToString(const WCHAR* wCharArray) -> std::string;

/**
 * @brief Converts a string containing char characters to LPSTR (character
 * string).
 * @param str The string containing char characters to be converted.
 * @return LPSTR representing the character version of the input string.
 */
[[nodiscard]] auto StringToLPSTR(const std::string& str) -> LPSTR;

/**
 * @brief Converts a wstring to LPSTR (character string).
 * @param wstr The wstring to be converted to LPSTR.
 * @return LPSTR representing the character version of the input wstring.
 */
[[nodiscard]] auto WStringToLPSTR(const std::wstring& wstr) -> LPSTR;

/**
 * @brief Converts a string containing char characters to LPWSTR (wide character
 * string).
 * @param str The string containing char characters to be converted.
 * @return LPWSTR representing the wide character version of the input string.
 */
[[nodiscard]] auto StringToLPWSTR(const std::string& str) -> LPWSTR;

/**
 * @brief Converts LPWSTR (wide character string) to a string containing char
 * characters.
 * @param lpwstr The LPWSTR to be converted to a string.
 * @return std::string containing char characters.
 */
[[nodiscard]] auto LPWSTRToString(LPWSTR lpwstr) -> std::string;

/**
 * @brief Converts LPCWSTR (const wide character string) to a string containing
 * char characters.
 * @param lpcwstr The LPCWSTR to be converted to a string.
 * @return std::string containing char characters.
 */
[[nodiscard]] auto LPCWSTRToString(LPCWSTR lpcwstr) -> std::string;

/**
 * @brief Converts a wstring to LPWSTR (wide character string).
 * @param wstr The wstring to be converted.
 * @return LPWSTR representing the wide character version of the input wstring.
 */
[[nodiscard]] auto WStringToLPWSTR(const std::wstring& wstr) -> LPWSTR;

/**
 * @brief Converts LPWSTR (wide character string) to a wstring.
 * @param lpwstr The LPWSTR to be converted to a wstring.
 * @return std::wstring.
 */
[[nodiscard]] auto LPWSTRToWString(LPWSTR lpwstr) -> std::wstring;

/**
 * @brief Converts LPCWSTR (const wide character string) to a wstring.
 * @param lpcwstr The LPCWSTR to be converted to a wstring.
 * @return std::wstring.
 */
[[nodiscard]] auto LPCWSTRToWString(LPCWSTR lpcwstr) -> std::wstring;

}  // namespace atom::utils

#endif

#endif
