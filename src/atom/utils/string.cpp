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
#include <charconv>
#include <codecvt>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

namespace atom::utils {
auto hasUppercase(std::string_view str) -> bool {
    return std::any_of(str.begin(), str.end(),
                       [](unsigned char ch) { return std::isupper(ch); });
}

auto toUnderscore(std::string_view str) -> std::string {
    std::string result;
    result.reserve(str.size() +
                   std::count_if(str.begin(), str.end(), [](unsigned char ch) {
                       return std::isupper(ch);
                   }));

    bool firstChar = true;

    for (char ch : str) {
        if (std::isupper(static_cast<unsigned char>(ch))) {
            if (!firstChar) {
                result.push_back('_');
            }
            result.push_back(std::tolower(static_cast<unsigned char>(ch)));
            firstChar = false;
        } else {
            result.push_back(ch);
            firstChar = false;
        }
    }

    return result;
}

auto toCamelCase(std::string_view str) -> std::string {
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

auto urlEncode(std::string_view str) -> std::string {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto c : str) {
        if ((std::isalnum(static_cast<unsigned char>(c)) != 0) || c == '-' ||
            c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << '+';
        } else {
            escaped << '%' << std::setw(2)
                    << static_cast<int>(
                           static_cast<unsigned char>(c));  // 编码其他字符
        }
    }

    return escaped.str();
}

auto urlDecode(std::string_view str) -> std::string {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 >= str.size()) {
                throw std::invalid_argument(
                    "urlDecode failed: incomplete escape sequence");
            }

            int value;
            if (auto [p, ec] =
                    std::from_chars(&str[i + 1], &str[i + 3], value, 16);
                ec != std::errc()) {
                throw std::invalid_argument(
                    "urlDecode failed: invalid escape sequence");
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

auto startsWith(std::string_view str, std::string_view prefix) -> bool {
    return str.size() >= prefix.size() &&
           str.substr(0, prefix.size()) == prefix;
}

auto endsWith(std::string_view str, std::string_view suffix) -> bool {
    return str.size() >= suffix.size() &&
           str.substr(str.size() - suffix.size()) == suffix;
}

auto splitString(const std::string &str,
                 char delimiter) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    auto start = str.begin();
    auto end = str.end();

    while (start != end) {
        auto next = std::find(start, end, delimiter);
        tokens.emplace_back(str.substr(start - str.begin(), next - start));
        if (next == end) {
            break;
        }
        start = next + 1;
    }

    return tokens;
}

auto joinStrings(const std::vector<std::string_view> &strings,
                 const std::string_view &delimiter) -> std::string {
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

auto replaceString(std::string_view text, std::string_view oldStr,
                   std::string_view newStr) -> std::string {
    std::string result = text.data();
    size_t pos = 0;
    while ((pos = result.find(std::string(oldStr), pos)) != std::string::npos) {
        result.replace(pos, oldStr.length(), std::string(newStr));
        pos += newStr.length();
    }
    return result;
}

auto replaceStrings(
    std::string_view text,
    const std::vector<std::pair<std::string_view, std::string_view>>
        &replacements) -> std::string {
    std::string result(text);
    for (const auto &[oldStr, newStr] : replacements) {
        result = replaceString(result, oldStr, newStr);
    }
    return result;
}

auto SVVtoSV(const std::vector<std::string_view> &svv)
    -> std::vector<std::string> {
    return std::vector<std::string>(svv.begin(), svv.end());
}

auto explode(std::string_view text, char symbol) -> std::vector<std::string> {
    std::vector<std::string> lines;
    const auto *start = text.begin();
    const auto *end = text.end();

    while (start != end) {
        const auto *pos = std::find(start, end, symbol);
        lines.emplace_back(start, pos);
        if (pos == end) {
            break;
        }
        start = std::next(pos);
    }

    return lines;
}

std::string trim(std::string_view line, std::string_view symbols) {
    const auto *first =
        std::find_if(line.begin(), line.end(), [&symbols](char c) {
            return symbols.find(c) == std::string_view::npos;
        });

    if (first == line.end()) {
        return "";
    }

    const auto *last =
        std::find_if(line.rbegin(), line.rend(), [&symbols](char c) {
            return symbols.find(c) == std::string_view::npos;
        }).base();

    return std::string(first, last);
}

auto stringToWString(const std::string &str) -> std::wstring {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

auto wstringToString(const std::wstring &wstr) -> std::string {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(wstr);
}

auto stod(std::string_view str, std::size_t *idx) -> double {
    return std::stod(std::string(str), idx);
}

auto stof(std::string_view str, std::size_t *idx) -> float {
    return std::stof(std::string(str), idx);
}

auto stoi(std::string_view str, std::size_t *idx, int base) -> int {
    return std::stoi(std::string(str), idx, base);
}

auto stol(std::string_view str, std::size_t *idx, int base) -> long {
    return std::stol(std::string(str), idx, base);
}

auto nstrtok(std::string_view &str, const std::string_view &delims)
    -> std::optional<std::string_view> {
    if (str.empty()) {
        return std::nullopt;
    }

    size_t start = str.find_first_not_of(delims);
    if (start == std::string_view::npos) {
        str = {};
        return std::nullopt;
    }

    size_t end = str.find_first_of(delims, start);
    std::string_view token;
    if (end == std::string_view::npos) {
        token = str.substr(start);
        str = {};
    } else {
        token = str.substr(start, end - start);
        str.remove_prefix(end + 1);
    }

    return token;
}
}  // namespace atom::utils
