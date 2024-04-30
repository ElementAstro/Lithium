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
[[nodiscard]] LPWSTR CharToLPWSTR(std::string_view charString);

/**
 * @brief Converts a string containing char characters to LPWSTR (wide character
 * string).
 * @param str The string containing char characters to be converted.
 * @return LPWSTR representing the wide character version of the input string.
 */
[[nodiscard]] LPWSTR StringToLPWSTR(const std::string& str);

/**
 * @brief Converts LPWSTR (wide character string) to a string containing char
 * characters.
 * @param lpwstr The LPWSTR to be converted to a string.
 * @return std::string containing char characters.
 */
[[nodiscard]] std::string LPWSTRToString(LPWSTR lpwstr);

/**
 * @brief Converts LPCWSTR (const wide character string) to a string containing
 * char characters.
 * @param lpcwstr The LPCWSTR to be converted to a string.
 * @return std::string containing char characters.
 */
[[nodiscard]] std::string LPCWSTRToString(LPCWSTR lpcwstr);

/**
 * @brief Converts a wstring to LPWSTR (wide character string).
 * @param wstr The wstring to be converted.
 * @return LPWSTR representing the wide character version of the input wstring.
 */
[[nodiscard]] LPWSTR WStringToLPWSTR(const std::wstring& wstr);

/**
 * @brief Converts LPWSTR (wide character string) to a wstring.
 * @param lpwstr The LPWSTR to be converted to a wstring.
 * @return std::wstring.
 */
[[nodiscard]] std::wstring LPWSTRToWString(LPWSTR lpwstr);

/**
 * @brief Converts LPCWSTR (const wide character string) to a wstring.
 * @param lpcwstr The LPCWSTR to be converted to a wstring.
 * @return std::wstring.
 */
[[nodiscard]] std::wstring LPCWSTRToWString(LPCWSTR lpcwstr);

}  // namespace atom::utils

#endif

#endif
