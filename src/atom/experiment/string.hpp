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

#include <string>
#include <vector>
#include <sstream>
#include <cstdarg>
#include <algorithm>

/**
 * @brief A super enhanced string class.
 */
class String
{
public:
    /**
     * @brief Constructor.
     */
    String();

    /**
     * @brief Constructor.
     * @param str - C-style string.
     */
    String(const char *str);

    /**
     * @brief Constructor.
     * @param str - std::string.
     */
    String(const std::string &str);

    /**
     * @brief Copy constructor.
     * @param other - other String.
     */
    String(const String &other);

    /**
     * @brief Copy assignment.
     * @param other - other String.
     */
    String &operator=(const String &other)
    {
        if (this != &other)
        {
            m_data = other.m_data;
        }
        return *this;
    }

    /**
     * @brief Equality.
     * @param other - other String.
     */
    bool operator==(const String &other) const
    {
        return m_data == other.m_data;
    }

    /**
     * @brief Inequality.
     * @param other - other String.
     */
    bool operator!=(const String &other) const
    {
        return m_data != other.m_data;
    }

    /**
     * @brief Less than.
     * @param other - other String.
     */
    bool operator<(const String &other) const
    {
        return m_data < other.m_data;
    }

    /**
     * @brief Greater than.
     * @param other - other String.
     */
    bool operator>(const String &other) const
    {
        return m_data > other.m_data;
    }

    /**
     * @brief Less than or equal.
     * @param other - other String.
     */
    bool operator<=(const String &other) const
    {
        return m_data <= other.m_data;
    }

    /**
     * @brief Greater than or equal.
     * @param other - other String.
     */
    bool operator>=(const String &other) const
    {
        return m_data >= other.m_data;
    }

    /**
     * @brief Concatenation.
     * @param other - other String.
     */
    String &operator+=(const String &other)
    {
        m_data += other.m_data;
        return *this;
    }

    /**
     * @brief Concatenation.
     * @param str - C-style string.
     */
    String &operator+=(const char *str)
    {
        m_data += str;
        return *this;
    }

    /**
     * @brief Concatenation.
     * @param c - char.
     */
    String &operator+=(char c)
    {
        m_data += c;
        return *this;
    }

    /**
     * @brief Get C-style string.
     */
    const char *toCharArray() const;

    /**
     * @brief Get length.
     */
    size_t length() const;

    /**
     * @brief Get substring.
     * @param pos - start position.
     * @param len - length.
     */
    String substring(size_t pos, size_t len = std::string::npos) const;

    /**
     * @brief Find.
     * @param str - string to find.
     * @param pos - start position.
     */
    size_t find(const String &str, size_t pos = 0) const;

    /**
     * @brief Replace.
     * @param oldStr - old string.
     * @param newStr - new string.
     */
    size_t replace(const String &oldStr, const String &newStr);

    /**
     * @brief To uppercase.
     */
    String toUpperCase() const;

    /**
     * @brief To lowercase.
     */
    String toLowerCase() const;

    /**
     * @brief Split.
     * @param delimiter - delimiter.
     */
    std::vector<String> split(const String &delimiter) const;

    /**
     * @brief Join.
     * @param strings - strings.
     * @param separator - separator.
     */
    static String join(const std::vector<String> &strings, const String &separator);

    /**
     * @brief Replace all.
     * @param oldStr - old string.
     * @param newStr - new string.
     */
    size_t replaceAll(const String &oldStr, const String &newStr);

    /**
     * @brief Insert char.
     * @param pos - position.
     * @param c - char.
     */
    void insertChar(size_t pos, char c);

    /**
     * @brief Delete char.
     * @param pos - position.
     */
    void deleteChar(size_t pos);

    /**
     * @brief Reverse.
     */
    String reverse() const;

    /**
     * @brief Equals.
     * @param other - other String.
     */
    bool equalsIgnoreCase(const String &other) const;

    /**
     * @brief Index of.
     * @param subStr - sub string.
     * @param startPos - start position.
     */
    size_t indexOf(const String &subStr, size_t startPos = 0) const;

    /**
     * @brief Trim.
     */
    void trim();

    /**
     * @brief Start with.
     * @param prefix - prefix.
     */
    bool startsWith(const String &prefix) const;

    /**
     * @brief End with.
     * @param suffix - suffix.
     */
    bool endsWith(const String &suffix) const;

    /**
     * @brief Escape.
     */
    String escape() const;

    /**
     * @brief Unescape.
     */
    String unescape() const;

    /**
     * @brief To int.
     */
    int toInt() const;

    /**
     * @brief To float.
     */
    float toFloat() const;

    /**
     * @brief Format.
     * @param format - format.
     * @param ... - arguments.
     */
    static String format(const char *format, ...)
    {
        char buffer[1024];

        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        return String(buffer);
    }

    static const size_t npos;

private:
    std::string m_data;
};

/**
 * @brief Concatenation.
 * @param lhs - left operand.
 * @param rhs - right operand.
 * @return - result.
 */
String operator+(const String &lhs, const String &rhs)
{
    String result(lhs);
    result += rhs;
    return result;
}

const size_t String::npos = -1;

#endif
