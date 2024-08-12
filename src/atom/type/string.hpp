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
    auto operator=(const String &other) -> String & = default;

    /**
     * @brief Move assignment.
     * @param other - other String.
     */
    auto operator=(String &&other) noexcept -> String & = default;

    /**
     * @brief Equality.
     * @param other - other String.
     */
    auto operator==(const String &other) const -> bool;

    /**
     * @brief Inequality.
     * @param other - other String.
     */
    auto operator!=(const String &other) const -> bool;

    /**
     * @brief Check if the String is empty.
     */
    [[nodiscard]] auto empty() const -> bool;

    /**
     * @brief Less than.
     * @param other - other String.
     */
    auto operator<(const String &other) const -> bool;

    /**
     * @brief Greater than.
     * @param other - other String.
     */
    auto operator>(const String &other) const -> bool;

    /**
     * @brief Less than or equal.
     * @param other - other String.
     */
    auto operator<=(const String &other) const -> bool;

    /**
     * @brief Greater than or equal.
     * @param other - other String.
     */
    auto operator>=(const String &other) const -> bool;

    /**
     * @brief Concatenation.
     * @param other - other String.
     */
    auto operator+=(const String &other) -> String &;

    /**
     * @brief Concatenation.
     * @param str - C-style string.
     */
    auto operator+=(const char *str) -> String &;

    /**
     * @brief Concatenation.
     * @param c - char.
     */
    auto operator+=(char c) -> String &;

    /**
     * @brief Get C-style string.
     */
    [[nodiscard]] auto cStr() const -> const char *;

    /**
     * @brief Get length.
     */
    [[nodiscard]] auto length() const -> size_t;

    /**
     * @brief Get substring.
     * @param pos - start position.
     * @param count - length.
     */
    [[nodiscard]] auto substr(size_t pos,
                              size_t count = std::string::npos) const -> String;

    /**
     * @brief Find.
     * @param str - string to find.
     * @param pos - start position.
     */
    [[nodiscard]] auto find(const String &str, size_t pos = 0) const -> size_t;

    /**
     * @brief Replace.
     * @param oldStr - old string.
     * @param newStr - new string.
     */
    auto replace(const String &oldStr, const String &newStr) -> bool;

    /**
     * @brief Replace all.
     * @param oldStr - old string.
     * @param newStr - new string.
     */
    auto replaceAll(const String &oldStr, const String &newStr) -> size_t;

    /**
     * @brief To uppercase.
     */
    [[nodiscard]] auto toUpper() const -> String;

    /**
     * @brief To lowercase.
     */
    [[nodiscard]] auto toLower() const -> String;

    /**
     * @brief Split.
     * @param delimiter - delimiter.
     */
    [[nodiscard]] auto split(const String &delimiter) const
        -> std::vector<String>;

    /**
     * @brief Join.
     * @param strings - strings.
     * @param separator - separator.
     */
    static auto join(const std::vector<String> &strings,
                     const String &separator) -> String;

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
    [[nodiscard]] auto reverse() const -> String;

    /**
     * @brief Equals ignore case.
     * @param other - other String.
     */
    [[nodiscard]] auto equalsIgnoreCase(const String &other) const -> bool;

    /**
     * @brief Starts with.
     * @param prefix - prefix.
     */
    [[nodiscard]] auto startsWith(const String &prefix) const -> bool;

    /**
     * @brief Ends with.
     * @param suffix - suffix.
     */
    [[nodiscard]] auto endsWith(const String &suffix) const -> bool;

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
        result.m_data_.resize(size + 1);
        std::snprintf(result.m_data_.data(), size + 1, format,
                      std::forward<Args>(args)...);
        result.m_data_.pop_back();
        return result;
    }

    static constexpr size_t NPOS = std::string::npos;

    [[nodiscard]] auto data() const -> std::string;

private:
    std::string m_data_;
};

/**
 * @brief Concatenation.
 * @param lhs - left operand.
 * @param rhs - right operand.
 * @return - result.
 */
auto operator+(const String &lhs, const String &rhs) -> String;

/**
 * @brief Output stream operator.
 * @param os - output stream.
 * @param str - string.
 * @return - output stream.
 */
auto operator<<(std::ostream &os, const String &str) -> std::ostream &;

namespace std {
template <>
struct hash<String> {
    auto operator()(const String &str) const -> size_t {
        return std::hash<std::string>()(str.data());
    }
};
}  // namespace std

#endif
