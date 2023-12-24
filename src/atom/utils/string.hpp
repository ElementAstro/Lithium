/*
 * string.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

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
