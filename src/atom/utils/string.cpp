/*
 * string.cpp
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

#include "string.hpp"
#include <algorithm>

bool HasUppercase(const std::string& str)
{
    // 判断字符串中是否存在大写字母
    // 参数：
    // - str: 要检查的字符串
    // 返回值：
    // - 如果字符串中存在大写字母，返回true；否则，返回false
    return std::any_of(str.begin(), str.end(), [](char ch) { return std::isupper(ch); });
}

std::string ToUnderscore(const std::string& str)
{
    std::string result;  // 生成一个空字符串用于存储转换结果
    for (char ch : str)  // 遍历输入字符串中的每个字符
    {
        if (std::isupper(ch))  // 如果字符是大写字母
        {
            result += '_';  // 在结果字符串前加上下划线
            result += std::tolower(ch);  // 将大写字母转换为小写字母并添加到结果字符串
        }
        else  // 如果字符不是大写字母
        {
            result += ch;  // 直接将字符添加到结果字符串
        }
    }
    return result;  // 返回转换后的字符串结果
}

std::string ToCamelCase(const std::string& str)
{
    std::string result; // 用于保存转换后的字符串结果
    bool capitalize = false; // 用于标记是否需要大写转换

    for (char ch : str) // 遍历输入字符串
    {
        if (ch == '_') // 如果遇到下划线
        {
            capitalize = true; // 设置大写标记为true
        }
        else // 如果不是下划线
        {
            if (capitalize) // 如果需要大写转换
            {
                result += std::toupper(ch); // 将字符转换为大写并添加到结果字符串
                capitalize = false; // 清除大写标记
            }
            else // 如果不需要大写转换
            {
                result += ch; // 直接将字符添加到结果字符串
            }
        }
    }
    return result; // 返回转换后的字符串结果
}

std::string ConvertToUnderscore(const std::string& str)
{
    return HasUppercase(str) ? ToUnderscore(str) : str;
}

std::string ConvertToCamelCase(const std::string& str)
{
    if (str.find('_') != std::string::npos)
    {
        return ToCamelCase(str);
    }
    else
    {
        std::string result = str;
        result[0] = std::tolower(result[0]);
        return result;
    }
}
