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

#include <codecvt>
#include <locale>
#include <string>
#include <string_view>

namespace atom::utils {

/**
 * @brief 将宽字符字符串转换为 UTF-8 字符串
 *
 * @param wstr 宽字符字符串视图
 * @return std::string 转换后的 UTF-8 字符串
 */
std::string toUTF8(std::wstring_view wstr);

/**
 * @brief 将 UTF-8 字符串转换为宽字符字符串
 *
 * @param str UTF-8 字符串视图
 * @return std::wstring 转换后的宽字符字符串
 */
std::wstring fromUTF8(std::string_view str);

/**
 * @brief 将 UTF-8 字符串转换为 UTF-16 字符串
 *
 * @param str UTF-8 字符串视图
 * @return std::u16string 转换后的 UTF-16 字符串
 */
std::u16string UTF8toUTF16(std::string_view str);

/**
 * @brief 将 UTF-8 字符串转换为 UTF-32 字符串
 *
 * @param str UTF-8 字符串视图
 * @return std::u32string 转换后的 UTF-32 字符串
 */
std::u32string UTF8toUTF32(std::string_view str);

/**
 * @brief 将 UTF-16 字符串转换为 UTF-8 字符串
 *
 * @param str UTF-16 字符串视图
 * @return std::string 转换后的 UTF-8 字符串
 */
std::string UTF16toUTF8(std::u16string_view str);

/**
 * @brief 将 UTF-16 字符串转换为 UTF-32 字符串
 *
 * @param str UTF-16 字符串视图
 * @return std::u32string 转换后的 UTF-32 字符串
 */
std::u32string UTF16toUTF32(std::u16string_view str);

/**
 * @brief 将 UTF-32 字符串转换为 UTF-8 字符串
 *
 * @param str UTF-32 字符串视图
 * @return std::string 转换后的 UTF-8 字符串
 */
std::string UTF32toUTF8(std::u32string_view str);

/**
 * @brief 将 UTF-32 字符串转换为 UTF-16 字符串
 *
 * @param str UTF-32 字符串视图
 * @return std::u16string 转换后的 UTF-16 字符串
 */
std::u16string UTF32toUTF16(std::u32string_view str);

/**
 * @brief 验证 UTF-8 字符串是否有效
 *
 * @param str UTF-8 字符串视图
 * @return true 如果字符串是有效的 UTF-8 字符串
 * @return false 如果字符串不是有效的 UTF-8 字符串
 */
bool isValidUTF8(std::string_view str);

}  // namespace atom::utils

#endif  // ATOM_UTILS_UTF_HPP
