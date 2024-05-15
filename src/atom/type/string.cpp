/*
 * string.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A super enhanced string class.

**************************************************/

#include "string.hpp"

const size_t String::npos = -1;

String::String(const char *str) : m_data(str) {}

String::String(std::string_view str) : m_data(str) {}

String::String(const std::string &str) : m_data(str) {}

String::String(const String &other) = default;

String::String(String &&other) noexcept = default;

String &String::operator=(const String &other) = default;

String &String::operator=(String &&other) noexcept = default;

bool String::operator==(const String &other) const {
    return m_data == other.m_data;
}

bool String::operator!=(const String &other) const {
    return m_data != other.m_data;
}

bool String::empty() const { return m_data.empty(); }

bool String::operator<(const String &other) const {
    return m_data < other.m_data;
}

bool String::operator>(const String &other) const {
    return m_data > other.m_data;
}

bool String::operator<=(const String &other) const {
    return m_data <= other.m_data;
}

bool String::operator>=(const String &other) const {
    return m_data >= other.m_data;
}

String &String::operator+=(const String &other) {
    m_data += other.m_data;
    return *this;
}

String &String::operator+=(const char *str) {
    m_data += str;
    return *this;
}

String &String::operator+=(char c) {
    m_data += c;
    return *this;
}

const char *String::c_str() const { return m_data.c_str(); }

size_t String::length() const { return m_data.length(); }

String String::substr(size_t pos, size_t count = std::string::npos) const {
    return m_data.substr(pos, count);
}

size_t String::find(const String &str, size_t pos = 0) const {
    return m_data.find(str.m_data, pos);
}

bool String::replace(const String &oldStr, const String &newStr) {
    size_t pos = m_data.find(oldStr.m_data);
    if (pos != std::string::npos) {
        m_data.replace(pos, oldStr.length(), newStr.m_data);
        return true;
    }
    return false;
}

size_t String::replace_all(const String &oldStr, const String &newStr) {
    size_t count = 0;
    size_t pos = 0;

    while ((pos = m_data.find(oldStr.m_data, pos)) != std::string::npos) {
        m_data.replace(pos, oldStr.length(), newStr.m_data);
        pos += newStr.length();
        ++count;
    }

    return count;
}

String String::to_upper() const {
    String result;
    std::transform(m_data.begin(), m_data.end(),
                   std::back_inserter(result.m_data),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

String String::to_lower() const {
    String result;
    std::transform(m_data.begin(), m_data.end(),
                   std::back_inserter(result.m_data),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::vector<String> String::split(const String &delimiter) const {
    std::vector<String> tokens;
    size_t start = 0;
    size_t end = m_data.find(delimiter.m_data);

    while (end != std::string::npos) {
        tokens.emplace_back(substr(start, end - start));
        start = end + delimiter.length();
        end = m_data.find(delimiter.m_data, start);
    }

    tokens.emplace_back(substr(start));

    return tokens;
}

String String::join(const std::vector<String> &strings,
                    const String &separator) {
    String result;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) {
            result += separator;
        }
        result += strings[i];
    }
    return result;
}

void String::insert(size_t pos, char c) { m_data.insert(pos, 1, c); }

void String::erase(size_t pos = 0, size_t count = std::string::npos) {
    m_data.erase(pos, count);
}

String String::reverse() const {
    String result(m_data);
    std::reverse(result.m_data.begin(), result.m_data.end());
    return result;
}

bool String::equals_ignore_case(const String &other) const {
    return std::equal(
        m_data.begin(), m_data.end(), other.m_data.begin(), other.m_data.end(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

bool String::starts_with(const String &prefix) const {
    return m_data.find(prefix.m_data) == 0;
}

bool String::ends_with(const String &suffix) const {
    if (suffix.length() > m_data.length()) {
        return false;
    }
    return std::equal(suffix.m_data.rbegin(), suffix.m_data.rend(),
                      m_data.rbegin());
}

void String::trim() {
    auto start =
        std::find_if_not(m_data.begin(), m_data.end(),
                         [](unsigned char c) { return std::isspace(c); });
    auto end =
        std::find_if_not(m_data.rbegin(), m_data.rend(), [](unsigned char c) {
            return std::isspace(c);
        }).base();
    m_data = std::string(start, end);
}

void String::ltrim() {
    auto start =
        std::find_if_not(m_data.begin(), m_data.end(),
                         [](unsigned char c) { return std::isspace(c); });
    m_data.erase(m_data.begin(), start);
}

void String::rtrim() {
    auto end =
        std::find_if_not(m_data.rbegin(), m_data.rend(), [](unsigned char c) {
            return std::isspace(c);
        }).base();
    m_data.erase(end, m_data.end());
}

std::string String::data() const { return m_data; }

String operator+(const String &lhs, const String &rhs) {
    String result(lhs);
    result += rhs;
    return result;
}

std::ostream &operator<<(std::ostream &os, const String &str) {
    os << str.data();
    return os;
}
