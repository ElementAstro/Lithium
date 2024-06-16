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
#include <codecvt>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

#include <charconv>
#include "atom/error/exception.hpp"

namespace atom::utils {
bool hasUppercase(std::string_view str) {
    return std::any_of(str.begin(), str.end(),
                       [](unsigned char ch) { return std::isupper(ch); });
}

std::string toUnderscore(std::string_view str) {
    std::string result;
    result.reserve(str.size() +
                   std::count_if(str.begin(), str.end(), [](unsigned char ch) {
                       return std::isupper(ch);
                   }));

    for (char ch : str) {
        if (std::isupper(ch)) {
            result.push_back('_');
            result.push_back(std::tolower(ch));
        } else {
            result.push_back(ch);
        }
    }

    return result;
}

std::string toCamelCase(std::string_view str) {
    std::string result;
    result.reserve(str.size());

    bool capitalize = false;
    for (char ch : str) {
        if (ch == '_') {
            capitalize = true;
        } else if (capitalize) {
            result.push_back(std::toupper(ch));
            capitalize = false;
        } else {
            result.push_back(ch);
        }
    }

    return result;
}

std::string urlEncode(std::string_view str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << '+';
        } else {
            escaped << '%' << std::setw(2)
                    << static_cast<int>(static_cast<unsigned char>(c));
        }
    }

    return escaped.str();
}

std::string urlDecode(std::string_view str) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 >= str.size()) {
                throw std::invalid_argument("urlDecode failed");
            }

            int value;
            if (auto [p, ec] =
                    std::from_chars(&str[i + 1], &str[i + 3], value, 16);
                ec != std::errc()) {
                throw std::invalid_argument("urlDecode failed");
            }

            result.push_back(static_cast<char>(value));
            i += 2;
        } else if (str[i] == '+') {
            result.push_back(' ');
        } else {
            result.push_back(str[i]);
        }
    }

    return result;
}

bool startsWith(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() &&
           str.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() &&
           str.substr(str.size() - suffix.size()) == suffix;
}

std::vector<std::string_view> splitString(const std::string &str,
                                          char delimiter) {
    std::vector<std::string_view> tokens;
    auto start = str.begin();
    auto end = str.end();

    while (start != end) {
        auto next = std::find(start, end, delimiter);
        tokens.emplace_back(str.substr(start - str.begin(), next - start));
        if (next == end)
            break;
        start = next + 1;
    }

    return tokens;
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

std::string replaceString(std::string_view text, std::string_view oldStr,
                          std::string_view newStr) {
    std::string result = text.data();
    size_t pos = 0;
    while ((pos = result.find(std::string(oldStr), pos)) != std::string::npos) {
        result.replace(pos, oldStr.length(), std::string(newStr));
        pos += newStr.length();
    }
    return result;
}

std::string replaceStrings(
    std::string_view text,
    const std::vector<std::pair<std::string_view, std::string_view>>
        &replacements) {
    std::string result(text);
    for (const auto &[oldStr, newStr] : replacements) {
        result = replaceString(result, oldStr, newStr);
    }
    return result;
}

std::vector<std::string> SVVtoSV(const std::vector<std::string_view> &svv) {
    return std::vector<std::string>(svv.begin(), svv.end());
}

std::vector<std::string> explode(std::string_view text, char symbol) {
    std::vector<std::string> lines;
    auto start = text.begin();
    auto end = text.end();

    while (start != end) {
        auto pos = std::find(start, end, symbol);
        lines.emplace_back(start, pos);
        if (pos == end)
            break;
        start = std::next(pos);
    }

    return lines;
}

std::string trim(std::string_view line, std::string_view symbols) {
    auto first = std::find_if(line.begin(), line.end(), [&symbols](char c) {
        return symbols.find(c) == std::string_view::npos;
    });

    if (first == line.end())
        return "";

    auto last = std::find_if(line.rbegin(), line.rend(), [&symbols](char c) {
                    return symbols.find(c) == std::string_view::npos;
                }).base();

    return std::string(first, last);
}

std::wstring stringToWString(const std::string &str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wstringToString(const std::wstring &wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(wstr);
}

}  // namespace atom::utils
