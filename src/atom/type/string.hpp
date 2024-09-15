/*
 * string.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A super enhanced string class.

**************************************************/

#ifndef ATOM_TYPE_STRING_HPP
#define ATOM_TYPE_STRING_HPP

#include <algorithm>
#include <cstdarg>
#include <format>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "macro.hpp"

/**
 * @brief A super enhanced string class.
 */
class String {
public:
    /**
     * @brief Constructor.
     */
    String() = default;

    /**
     * @brief Constructor from C-style string.
     */
    String(const char *str) : m_data_(str) {}

    /**
     * @brief Constructor from std::string_view.
     */
    String(std::string_view str) : m_data_(str) {}

    /**
     * @brief Constructor from std::string.
     */
    String(std::string str) : m_data_(std::move(str)) {}

    /**
     * @brief Copy constructor.
     */
    String(const String &other) = default;

    /**
     * @brief Move constructor.
     */
    String(String &&other) ATOM_NOEXCEPT = default;

    /**
     * @brief Copy assignment.
     */
    auto operator=(const String &other) -> String & = default;

    /**
     * @brief Move assignment.
     */
    auto operator=(String &&other) ATOM_NOEXCEPT->String & = default;

    /**
     * @brief Equality comparison.
     */
    auto operator==(const String &other) const -> bool = default;

    /**
     * @brief Three-way comparison (C++20).
     */
    auto operator<=>(const String &other) const = default;

    /**
     * @brief Concatenation with another String.
     */
    auto operator+=(const String &other) -> String & {
        m_data_ += other.m_data_;
        return *this;
    }

    /**
     * @brief Concatenation with C-style string.
     */
    auto operator+=(const char *str) -> String & {
        m_data_ += str;
        return *this;
    }

    /**
     * @brief Concatenation with a single character.
     */
    auto operator+=(char c) -> String & {
        m_data_ += c;
        return *this;
    }

    /**
     * @brief Get C-style string.
     */
    ATOM_NODISCARD auto cStr() const -> const char * { return m_data_.c_str(); }

    /**
     * @brief Get length of the string.
     */
    ATOM_NODISCARD auto length() const -> size_t { return m_data_.length(); }

    /**
     * @brief Get substring.
     */
    ATOM_NODISCARD auto substr(
        size_t pos, size_t count = std::string::npos) const -> String {
        return m_data_.substr(pos, count);
    }

    /**
     * @brief Find a substring.
     */
    ATOM_NODISCARD auto find(const String &str,
                             size_t pos = 0) const -> size_t {
        return m_data_.find(str.m_data_, pos);
    }

    /**
     * @brief Replace first occurrence of oldStr with newStr.
     */
    auto replace(const String &oldStr, const String &newStr) -> bool {
        if (size_t pos = m_data_.find(oldStr.m_data_);
            pos != std::string::npos) {
            m_data_.replace(pos, oldStr.length(), newStr.m_data_);
            return true;
        }
        return false;
    }

    /**
     * @brief Replace all occurrences of oldStr with newStr.
     */
    auto replaceAll(const String &oldStr, const String &newStr) -> size_t {
        size_t count = 0;
        size_t pos = 0;

        while ((pos = m_data_.find(oldStr.m_data_, pos)) != std::string::npos) {
            m_data_.replace(pos, oldStr.length(), newStr.m_data_);
            pos += newStr.length();
            ++count;
        }

        return count;
    }

    /**
     * @brief Convert string to uppercase.
     */
    ATOM_NODISCARD auto toUpper() const -> String {
        String result;
        std::ranges::transform(m_data_, std::back_inserter(result.m_data_),
                               [](unsigned char c) { return std::toupper(c); });
        return result;
    }

    /**
     * @brief Convert string to lowercase.
     */
    ATOM_NODISCARD auto toLower() const -> String {
        String result;
        std::ranges::transform(m_data_, std::back_inserter(result.m_data_),
                               [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    /**
     * @brief Split the string by a delimiter.
     */
    ATOM_NODISCARD auto split(const String &delimiter) const
        -> std::vector<String> {
        if (delimiter.empty()) {
            return {*this};
        }
        if (m_data_.empty()) {
            return {};
        }
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

    /**
     * @brief Join a vector of strings with a separator.
     */
    static auto join(const std::vector<String> &strings,
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

    /**
     * @brief Insert a character at a position.
     */
    void insert(size_t pos, char c) { m_data_.insert(pos, 1, c); }

    /**
     * @brief Erase a portion of the string.
     */
    void erase(size_t pos = 0, size_t count = std::string::npos) {
        m_data_.erase(pos, count);
    }

    /**
     * @brief Reverse the string.
     */
    ATOM_NODISCARD auto reverse() const -> String {
        String result(m_data_);
        std::ranges::reverse(result.m_data_);
        return result;
    }

    /**
     * @brief Case-insensitive comparison.
     */
    ATOM_NODISCARD auto equalsIgnoreCase(const String &other) const -> bool {
        return std::ranges::equal(m_data_, other.m_data_, [](char a, char b) {
            return std::tolower(a) == std::tolower(b);
        });
    }

    /**
     * @brief Check if string starts with a prefix.
     */
    ATOM_NODISCARD auto startsWith(const String &prefix) const -> bool {
        return m_data_.starts_with(prefix.m_data_);
    }

    /**
     * @brief Check if string ends with a suffix.
     */
    ATOM_NODISCARD auto endsWith(const String &suffix) const -> bool {
        return m_data_.ends_with(suffix.m_data_);
    }

    /**
     * @brief Trim whitespace from both ends.
     */
    void trim() {
        ltrim();
        rtrim();
    }

    /**
     * @brief Left trim.
     */
    void ltrim() {
        m_data_.erase(m_data_.begin(),
                      std::ranges::find_if_not(m_data_, [](unsigned char c) {
                          return std::isspace(c);
                      }));
    }

    /**
     * @brief Right trim.
     */
    void rtrim() {
        m_data_.erase(std::ranges::find_if_not(
                          m_data_.rbegin(), m_data_.rend(),
                          [](unsigned char c) { return std::isspace(c); })
                          .base(),
                      m_data_.end());
    }

    /**
     * @brief Get the underlying data as a std::string.
     */
    ATOM_NODISCARD auto data() const -> std::string { return m_data_; }

    ATOM_NODISCARD auto empty() const -> bool { return m_data_.empty(); }

    auto replace(char oldChar, char newChar) -> size_t {
        size_t count = 0;
        for (auto &c : m_data_) {
            if (c == oldChar) {
                c = newChar;
                ++count;
            }
        }
        return count;
    }

    auto remove(char ch) -> size_t {
        size_t count = std::erase(m_data_, ch);
        return count;
    }

    /**
     * @brief Pad the string from the left with a specific character.
     */
    auto padLeft(size_t totalLength, char paddingChar = ' ') -> String & {
        if (m_data_.length() < totalLength) {
            m_data_.insert(m_data_.begin(), totalLength - m_data_.length(),
                           paddingChar);
        }
        return *this;
    }

    /**
     * @brief Pad the string from the right with a specific character.
     */
    auto padRight(size_t totalLength, char paddingChar = ' ') -> String & {
        if (m_data_.length() < totalLength) {
            m_data_.append(totalLength - m_data_.length(), paddingChar);
        }
        return *this;
    }

    /**
     * @brief Remove a specific prefix from the string.
     */
    auto removePrefix(const String &prefix) -> bool {
        if (startsWith(prefix)) {
            m_data_.erase(0, prefix.length());
            return true;
        }
        return false;
    }

    /**
     * @brief Remove a specific suffix from the string.
     */
    auto removeSuffix(const String &suffix) -> bool {
        if (endsWith(suffix)) {
            m_data_.erase(m_data_.length() - suffix.length());
            return true;
        }
        return false;
    }

    /**
     * @brief Check if the string contains a substring.
     */
    ATOM_NODISCARD auto contains(const String &str) const -> bool {
        return m_data_.find(str.m_data_) != std::string::npos;
    }

    /**
     * @brief Check if the string contains a specific character.
     */
    ATOM_NODISCARD auto contains(char c) const -> bool {
        return m_data_.find(c) != std::string::npos;
    }

    /**
     * @brief Compress multiple consecutive spaces into a single space.
     */
    void compressSpaces() {
        auto newEnd =
            std::unique(m_data_.begin(), m_data_.end(), [](char lhs, char rhs) {
                return std::isspace(lhs) && std::isspace(rhs);
            });
        m_data_.erase(newEnd, m_data_.end());
    }

    /**
     * @brief Reverse the order of words in the string.
     */
    ATOM_NODISCARD auto reverseWords() const -> String {
        auto words = split(" ");
        std::ranges::reverse(words);
        return join(words, " ");
    }

    auto replaceRegex(const std::string &pattern,
                      const std::string &replacement) -> String {
        std::regex re(pattern);
        return std::regex_replace(m_data_, re, replacement);
    }

    /**
     * @brief Format a string.
     */

    template <typename... Args>
    static auto format(const std::string &format_str,
                       Args &&...args) -> std::string {
        return std::format(format_str, std::forward<Args>(args)...);
    }

    static constexpr size_t NPOS = std::string::npos;

private:
    std::string m_data_;
};

/**
 * @brief Concatenation operator for String class.
 */
ATOM_INLINE auto operator+(const String &lhs, const String &rhs) -> String {
    String result(lhs);
    result += rhs;
    return result;
}

/**
 * @brief Output stream operator for String class.
 */
ATOM_INLINE auto operator<<(std::ostream &os,
                            const String &str) -> std::ostream & {
    os << str.data();
    return os;
}

namespace std {
/**
 * @brief Specialization of std::hash for String class.
 */
template <>
struct hash<String> {
    auto operator()(const String &str) const ATOM_NOEXCEPT->size_t {
        return std::hash<std::string>()(str.data());
    }
};
}  // namespace std

#endif  // ATOM_TYPE_STRING_HPP