/*
 * string.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A super enhanced string class.

**************************************************/

#ifndef ATOM_EXPERIMENT_STRING_HPP
#define ATOM_EXPERIMENT_STRING_HPP

#include <algorithm>
#include <cstdarg>
#include <string>
#include <string_view>
#include <vector>

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
     * @brief Constructor.
     * @param str - C-style string.
     */
    String(const char *str);

    /**
     * @brief Constructor.
     * @param str - std::string_view.
     */
    String(std::string_view str);

    /**
     * @brief Constructor.
     * @param str - std::string.
     */
    String(const std::string &str);

    /**
     * @brief Copy constructor.
     * @param other - other String.
     */
    String(const String &other) = default;

    /**
     * @brief Move constructor.
     * @param other - other String.
     */
    String(String &&other) noexcept = default;

    /**
     * @brief Copy assignment.
     * @param other - other String.
     */
    String &operator=(const String &other) = default;

    /**
     * @brief Move assignment.
     * @param other - other String.
     */
    String &operator=(String &&other) noexcept = default;

    /**
     * @brief Equality.
     * @param other - other String.
     */
    bool operator==(const String &other) const;

    /**
     * @brief Inequality.
     * @param other - other String.
     */
    bool operator!=(const String &other) const;

    /**
     * @brief Check if the String is empty.
     */
    [[nodiscard]] bool empty() const;

    /**
     * @brief Less than.
     * @param other - other String.
     */
    bool operator<(const String &other) const;

    /**
     * @brief Greater than.
     * @param other - other String.
     */
    bool operator>(const String &other) const;

    /**
     * @brief Less than or equal.
     * @param other - other String.
     */
    bool operator<=(const String &other) const;

    /**
     * @brief Greater than or equal.
     * @param other - other String.
     */
    bool operator>=(const String &other) const;

    /**
     * @brief Concatenation.
     * @param other - other String.
     */
    String &operator+=(const String &other);

    /**
     * @brief Concatenation.
     * @param str - C-style string.
     */
    String &operator+=(const char *str);

    /**
     * @brief Concatenation.
     * @param c - char.
     */
    String &operator+=(char c);

    /**
     * @brief Get C-style string.
     */
    [[nodiscard]] const char *c_str() const;

    /**
     * @brief Get length.
     */
    [[nodiscard]] size_t length() const;

    /**
     * @brief Get substring.
     * @param pos - start position.
     * @param count - length.
     */
    [[nodiscard]] String substr(size_t pos,
                                size_t count = std::string::npos) const;

    /**
     * @brief Find.
     * @param str - string to find.
     * @param pos - start position.
     */
    [[nodiscard]] size_t find(const String &str, size_t pos = 0) const;

    /**
     * @brief Replace.
     * @param oldStr - old string.
     * @param newStr - new string.
     */
    bool replace(const String &oldStr, const String &newStr);

    /**
     * @brief Replace all.
     * @param oldStr - old string.
     * @param newStr - new string.
     */
    size_t replace_all(const String &oldStr, const String &newStr);

    /**
     * @brief To uppercase.
     */
    [[nodiscard]] String to_upper() const;

    /**
     * @brief To lowercase.
     */
    [[nodiscard]] String to_lower() const;

    /**
     * @brief Split.
     * @param delimiter - delimiter.
     */
    [[nodiscard]] std::vector<String> split(const String &delimiter) const;

    /**
     * @brief Join.
     * @param strings - strings.
     * @param separator - separator.
     */
    static String join(const std::vector<String> &strings,
                       const String &separator);

    /**
     * @brief Insert char.
     * @param pos - position.
     * @param c - char.
     */
    void insert(size_t pos, char c);

    /**
     * @brief Erase char.
     * @param pos - position.
     * */
    void erase(size_t pos = 0, size_t count = std::string::npos);

    /**
     * @brief Reverse.
     */
    [[nodiscard]] String reverse() const;

    /**
     * @brief Equals ignore case.
     * @param other - other String.
     */
    [[nodiscard]] bool equals_ignore_case(const String &other) const;

    /**
     * @brief Starts with.
     * @param prefix - prefix.
     */
    [[nodiscard]] bool starts_with(const String &prefix) const;

    /**
     * @brief Ends with.
     * @param suffix - suffix.
     */
    [[nodiscard]] bool ends_with(const String &suffix) const;

    /**
     * @brief Trim.
     */
    void trim();

    /**
     * @brief Left trim.
     */
    void ltrim();

    /**
     * @brief Right trim.
     */
    void rtrim();

    /**
     * @brief Format.
     * @param format - format string.
     * @param ... - arguments.
     */
    template <typename... Args>
    static String format(const char *format, Args &&...args) {
        int size =
            std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
        String result;
        result.m_data.resize(size + 1);
        std::snprintf(result.m_data.data(), size + 1, format,
                      std::forward<Args>(args)...);
        result.m_data.pop_back();
        return result;
    }

    static constexpr size_t npos = std::string::npos;

    [[nodiscard]] std::string data() const;

private:
    std::string m_data;
};

/**
 * @brief Concatenation.
 * @param lhs - left operand.
 * @param rhs - right operand.
 * @return - result.
 */
String operator+(const String &lhs, const String &rhs);

/**
 * @brief Output stream operator.
 * @param os - output stream.
 * @param str - string.
 * @return - output stream.
 */
std::ostream &operator<<(std::ostream &os, const String &str);

/**
 * @brief Less than operator.
 * @param lhs - left operand.
 * @param rhs - right operand.
 * @return - result.
 */
bool operator<(const String &lhs, const String &rhs);

namespace std {
template <>
struct hash<String> {
    size_t operator()(const String &str) const {
        return std::hash<std::string>()(str.data());
    }
};
}  // namespace std

#endif
