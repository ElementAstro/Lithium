/*
 * string.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Some useful string functions

**************************************************/

#include "string.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "atom/error/exception.hpp"

namespace Atom::Utils {
bool hasUppercase(const std::string &str) {
    // 判断字符串中是否存在大写字母
    // 参数：
    // - str: 要检查的字符串
    // 返回值：
    // - 如果字符串中存在大写字母，返回true；否则，返回false
    return std::any_of(str.begin(), str.end(),
                       [](char ch) { return std::isupper(ch); });
}

std::string toUnderscore(const std::string &str) {
    std::string result;  // 生成一个空字符串用于存储转换结果
    for (char ch : str)  // 遍历输入字符串中的每个字符
    {
        if (std::isupper(ch))  // 如果字符是大写字母
        {
            result += '_';  // 在结果字符串前加上下划线
            result +=
                std::tolower(ch);  // 将大写字母转换为小写字母并添加到结果字符串
        } else  // 如果字符不是大写字母
        {
            result += ch;  // 直接将字符添加到结果字符串
        }
    }
    return result;  // 返回转换后的字符串结果
}

std::string toCamelCase(const std::string &str) {
    std::string result;       // 用于保存转换后的字符串结果
    bool capitalize = false;  // 用于标记是否需要大写转换

    for (char ch : str)  // 遍历输入字符串
    {
        if (ch == '_')  // 如果遇到下划线
        {
            capitalize = true;  // 设置大写标记为true
        } else                  // 如果不是下划线
        {
            if (capitalize)  // 如果需要大写转换
            {
                result +=
                    std::toupper(ch);  // 将字符转换为大写并添加到结果字符串
                capitalize = false;  // 清除大写标记
            } else                   // 如果不需要大写转换
            {
                result += ch;  // 直接将字符添加到结果字符串
            }
        }
    }
    return result;  // 返回转换后的字符串结果
}

std::string urlEncode(const std::string &str) {
    // 创建一个输出字符串流对象escaped，设置填充字符为'0'，输出基数为十六进制
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    // 遍历输入字符串str的每个字符
    for (auto c : str) {
        // 如果字符是alnum字符，或者字符是'-'、'_'、'.'、'~'之一，则直接将字符输出到escaped
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        // 如果字符是空格，则将字符替换为'+'
        else if (c == ' ') {
            escaped << '+';
        }
        // 其他情况下，将字符替换为字符对应的百分比表示形式，例如 '%' +
        // 十六进制字符的字符串形式
        else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }

    // 返回经过编码处理的字符串
    return escaped.str();
}

std::string urlDecode(const std::string &str) {
    // 初始化一个空字符串用于存储解码后的结果
    std::string result;

    // 遍历输入的字符串的每个字符
    for (size_t i = 0; i < str.size(); ++i) {
        // 如果当前字符是 '%'，则进行编码解析
        if (str[i] == '%') {
            int value;
            std::istringstream is(str.substr(i + 1, 2));

            // 将当前字符的后两位作为十六进制字符串，转换为整数
            if (is >> std::hex >> value) {
                // 将整数转换为字符，并添加到结果中
                result += static_cast<char>(value);

                // 跳过解析后的两位字符，继续下一个字符的处理
                i += 2;
            } else {
                // 如果解析失败，则抛出异常
                throw Error::WrongArgument("urlDecode failed");
            }
        }
        // 如果当前字符是 '+'，则将其替换为空格并添加到结果中
        else if (str[i] == '+') {
            result += ' ';
        }
        // 对于其他字符，直接将其添加到结果中
        else {
            result += str[i];
        }
    }

    // 返回解码后的结果
    return result;
}

std::vector<std::string_view> splitString(const std::string &str,
                                          char delimiter) {
    std::vector<std::string_view> result;
    std::string_view view(str);
    size_t pos = 0;

    while (true) {
        size_t next_pos = view.find(delimiter, pos);
        if (next_pos == std::string_view::npos) {
            result.emplace_back(view.substr(pos));
            break;
        }
        result.emplace_back(view.substr(pos, next_pos - pos));
        pos = next_pos + 1;
    }

    return result;
}

std::string joinStrings(const std::vector<std::string_view> &strings,
                        const std::string_view &delimiter) {
    std::ostringstream oss;
    bool first = true;

    for (const auto &str : strings) {
        if (!first) {
            oss << delimiter;
        }
        oss << str;
        first = false;
    }

    return oss.str();
}

std::string replaceString(std::string_view text, std::string_view oldStr, std::string_view newStr) {
    std::string result = text.data();
    size_t pos = 0;
    while ((pos = result.find(std::string(oldStr), pos)) != std::string::npos) {
        result.replace(pos, oldStr.length(), std::string(newStr));
        pos += newStr.length();
    }
    return result;
}

std::string replaceStrings(std::string_view text, const std::vector<std::pair<std::string_view, std::string_view>>& replacements) {
    std::string result(text);
    for (const auto& [oldStr, newStr] : replacements) {
        result = replaceString(result, oldStr, newStr);
    }
    return result;
}
}  // namespace Atom::Utils