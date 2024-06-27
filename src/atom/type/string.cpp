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

#include <algorithm>

String::String(const char *str) : m_data_(str) {}

String::String(std::string_view str) : m_data_(str) {}

String::String(const std::string &str) : m_data_(str) {}

auto String::operator==(const String &other) const -> bool {
    return m_data_ == other.m_data_;
}

auto String::operator!=(const String &other) const -> bool {
    return m_data_ != other.m_data_;
}

auto String::empty() const -> bool { return m_data_.empty(); }

auto String::operator<(const String &other) const -> bool {
    return m_data_ < other.m_data_;
}

auto String::operator>(const String &other) const -> bool {
    return m_data_ > other.m_data_;
}

auto String::operator<=(const String &other) const -> bool {
    return m_data_ <= other.m_data_;
}

auto String::operator>=(const String &other) const -> bool {
    return m_data_ >= other.m_data_;
}

auto String::operator+=(const String &other) -> String & {
    m_data_ += other.m_data_;
    return *this;
}

auto String::operator+=(const char *str) -> String & {
    m_data_ += str;
    return *this;
}

auto String::operator+=(char c) -> String & {
    m_data_ += c;
    return *this;
}

auto String::cStr() const -> const char * { return m_data_.c_str(); }

auto String::length() const -> size_t { return m_data_.length(); }

auto String::substr(size_t pos, size_t count) const -> String {
    return m_data_.substr(pos, count);
}

auto String::find(const String &str, size_t pos) const -> size_t {
    return m_data_.find(str.m_data_, pos);
}

auto String::replace(const String &oldStr, const String &newStr) -> bool {
    size_t pos = m_data_.find(oldStr.m_data_);
    if (pos != std::string::npos) {
        m_data_.replace(pos, oldStr.length(), newStr.m_data_);
        return true;
    }
    return false;
}

auto String::replaceAll(const String &oldStr, const String &newStr) -> size_t {
    size_t count = 0;
    size_t pos = 0;

    while ((pos = m_data_.find(oldStr.m_data_, pos)) != std::string::npos) {
        m_data_.replace(pos, oldStr.length(), newStr.m_data_);
        pos += newStr.length();
        ++count;
    }

    return count;
}

auto String::toUpper() const -> String {
    String result;
    std::transform(m_data_.begin(), m_data_.end(),
                   std::back_inserter(result.m_data_),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

auto String::toLower() const -> String {
    String result;
    std::transform(m_data_.begin(), m_data_.end(),
                   std::back_inserter(result.m_data_),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

auto String::split(const String &delimiter) const -> std::vector<String> {
    std::vector<String> tokens;
    size_t start = 0;
    size_t end = m_data_.find(delimiter.m_data_);

    while (end != std::string::npos) {
        tokens.emplace_back(substr(start, end - start));
        start = end + delimiter.length();
        end = m_data_.find(delimiter.m_data_, start);
    }

    tokens.emplace_back(substr(start));

    return tokens;
}

auto String::join(const std::vector<String> &strings,
                  const String &separator) -> String {
    String result;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) {
            result += separator;
        }
        result += strings[i];
    }
    return result;
}

void String::insert(size_t pos, char c) { m_data_.insert(pos, 1, c); }

void String::erase(size_t pos, size_t count) { m_data_.erase(pos, count); }

auto String::reverse() const -> String {
    String result(m_data_);
    std::reverse(result.m_data_.begin(), result.m_data_.end());
    return result;
}

auto String::equalsIgnoreCase(const String &other) const -> bool {
    return std::equal(m_data_.begin(), m_data_.end(), other.m_data_.begin(),
                      other.m_data_.end(), [](char a, char b) {
                          return std::tolower(a) == std::tolower(b);
                      });
}

auto String::startsWith(const String &prefix) const -> bool {
    return m_data_.find(prefix.m_data_) == 0;
}

auto String::endsWith(const String &suffix) const -> bool {
    if (suffix.length() > m_data_.length()) {
        return false;
    }
    return std::equal(suffix.m_data_.rbegin(), suffix.m_data_.rend(),
                      m_data_.rbegin());
}

void String::trim() {
    auto start =
        std::find_if_not(m_data_.begin(), m_data_.end(),
                         [](unsigned char c) { return std::isspace(c); });
    auto end =
        std::find_if_not(m_data_.rbegin(), m_data_.rend(), [](unsigned char c) {
            return std::isspace(c);
        }).base();
    m_data_ = std::string(start, end);
}

void String::ltrim() {
    auto start =
        std::find_if_not(m_data_.begin(), m_data_.end(),
                         [](unsigned char c) { return std::isspace(c); });
    m_data_.erase(m_data_.begin(), start);
}

void String::rtrim() {
    auto end =
        std::find_if_not(m_data_.rbegin(), m_data_.rend(), [](unsigned char c) {
            return std::isspace(c);
        }).base();
    m_data_.erase(end, m_data_.end());
}

auto String::data() const -> std::string { return m_data_; }

auto operator+(const String &lhs, const String &rhs) -> String {
    String result(lhs);
    result += rhs;
    return result;
}

auto operator<<(std::ostream &os, const String &str) -> std::ostream & {
    os << str.data();
    return os;
}
