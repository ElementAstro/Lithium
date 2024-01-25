/*
 * string.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Some useful string functions

**************************************************/

#pragma once

#include <string>
#include <vector>

namespace Atom::Utils
{
    /**
     * @brief 检查字符串中是否包含大写字母。
     * @param str 输入字符串。
     * @return 如果字符串中至少包含一个大写字母，则返回 true，否则返回 false。
     */
    [[nodiscard]] bool HasUppercase(const std::string &str);

    /**
     * @brief 将字符串转换为下划线命名法（underscore）。
     * @param str 输入字符串。
     * @return 转换后的下划线命名法字符串。
     */
    [[nodiscard]] std::string ToUnderscore(const std::string &str);

    /**
     * @brief 将字符串转换为驼峰命名法（camel case）。
     * @param str 输入字符串。
     * @return 转换后的驼峰命名法字符串。
     */
    [[nodiscard]] std::string ToCamelCase(const std::string &str);

    /**
     * @brief 将字符串转换为下划线命名法（underscore）。
     * @param str 输入字符串。
     * @return 转换后的下划线命名法字符串。
     * @deprecated 请使用 ToUnderscore() 函数替代。
     */
    [[nodiscard]] std::string ConvertToUnderscore(const std::string &str);

    /**
     * @brief 将字符串转换为驼峰命名法（camel case）。
     * @param str 输入字符串。
     * @return 转换后的驼峰命名法字符串。
     * @deprecated 请使用 ToCamelCase() 函数替代。
     */
    [[nodiscard]] std::string ConvertToCamelCase(const std::string &str);

    /**
     * @brief 对字符串进行 URL 编码。
     * @param str 输入字符串。
     * @return URL 编码后的字符串。
     */
    [[nodiscard]] std::string UrlEncode(const std::string &str);

    /**
     * @brief 对 URL 编码的字符串进行解码。
     * @param str 输入字符串。
     * @return 解码后的字符串。
     */
    [[nodiscard]] std::string UrlDecode(const std::string &str);

    [[nodiscard]] std::vector<std::string> SplitString(const std::string &input, char delimiter);
}
