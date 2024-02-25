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

String::String() {}

// 根据C风格字符串构造
String::String(const char *str)
    : m_data(str) {}

// 根据std::string构造
String::String(const std::string &str)
    : m_data(str) {}

// 拷贝构造函数
String::String(const String &other)
    : m_data(other.m_data) {}

// 获取字符数组
const char *String::toCharArray() const
{
    return m_data.c_str();
}

// 获取字符串长度
size_t String::length() const
{
    return m_data.length();
}

// 子字符串提取
String String::substring(size_t pos, size_t len = std::string::npos) const
{
    return m_data.substr(pos, len);
}

// 查找和替换
size_t String::find(const String &str, size_t pos = 0) const
{
    return m_data.find(str.m_data, pos);
}

size_t String::replace(const String &oldStr, const String &newStr)
{
    size_t count = 0;
    size_t pos = 0;

    while ((pos = m_data.find(oldStr.m_data, pos)) != std::string::npos)
    {
        m_data.replace(pos, oldStr.length(), newStr.m_data);
        pos += newStr.length();
        ++count;
    }

    return count;
}

// 大小写转换
String String::toUpperCase() const
{
    String result(*this);

    for (size_t i = 0; i < result.length(); ++i)
    {
        result.m_data[i] = toupper(result.m_data[i]);
    }

    return result;
}

String String::toLowerCase() const
{
    String result(*this);

    for (size_t i = 0; i < result.length(); ++i)
    {
        result.m_data[i] = tolower(result.m_data[i]);
    }

    return result;
}

// 字符串拆分和连接
std::vector<String> String::split(const String &delimiter) const
{
    std::vector<String> tokens;

    size_t start = 0;
    size_t end = m_data.find(delimiter.m_data);

    while (end != std::string::npos)
    {
        tokens.push_back(substring(start, end - start));
        start = end + delimiter.length();
        end = m_data.find(delimiter.m_data, start);
    }

    tokens.push_back(substring(start));

    return tokens;
}

static String String::join(const std::vector<String> &strings, const String &separator)
{
    String result;

    for (size_t i = 0; i < strings.size(); ++i)
    {
        if (i > 0)
        {
            result += separator;
        }

        result += strings[i];
    }

    return result;
}

size_t String::replaceAll(const String &oldStr, const String &newStr)
{
    size_t count = 0;
    size_t pos = 0;

    while ((pos = m_data.find(oldStr.m_data, pos)) != std::string::npos)
    {
        m_data.replace(pos, oldStr.length(), newStr.m_data);
        pos += newStr.length();
        ++count;
    }

    return count;
}

void String::insertChar(size_t pos, char c)
{
    if (pos <= m_data.length())
    {
        m_data.insert(pos, 1, c);
    }
}

void String::deleteChar(size_t pos)
{
    if (pos < m_data.length())
    {
        m_data.erase(pos, 1);
    }
}

String String::reverse() const
{
    String result(*this);
    std::reverse(result.m_data.begin(), result.m_data.end());
    return result;
}

bool String::equalsIgnoreCase(const String &other) const
{
    return std::equal(m_data.begin(), m_data.end(), other.m_data.begin(), other.m_data.end(),
                      [](char a, char b)
                      {
                          return std::tolower(a) == std::tolower(b);
                      });
}

size_t String::indexOf(const String &subStr, size_t startPos = 0) const
{
    return m_data.find(subStr.m_data, startPos);
}

// 去除首尾空格
void String::trim()
{
    size_t startPos = m_data.find_first_not_of(" \t\r\n");
    size_t endPos = m_data.find_last_not_of(" \t\r\n");

    if (startPos == std::string::npos || endPos == std::string::npos)
    {
        m_data.clear();
    }
    else
    {
        m_data = m_data.substr(startPos, endPos - startPos + 1);
    }
}

bool String::startsWith(const String &prefix) const
{
    if (prefix.length() > m_data.length())
    {
        return false;
    }

    return std::equal(prefix.m_data.begin(), prefix.m_data.end(), m_data.begin());
}

bool String::endsWith(const String &suffix) const
{
    if (suffix.length() > m_data.length())
    {
        return false;
    }

    return std::equal(suffix.m_data.rbegin(), suffix.m_data.rend(), m_data.rbegin());
}

String String::escape() const
{
    String result;
    for (char c : m_data)
    {
        if (c == '\\' || c == '\"' || c == '\'')
        {
            result += '\\';
        }
        result += c;
    }
    return result;
}

String String::unescape() const
{
    String result;
    bool escaped = false;

    for (char c : m_data)
    {
        if (escaped)
        {
            if (c == '\\' || c == '\"' || c == '\'')
            {
                result += c;
            }
            else
            {
                result += '\\';
                result += c;
            }
            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else
        {
            result += c;
        }
    }

    return result;
}

int String::toInt() const
{
    std::istringstream iss(m_data);
    int value = 0;
    iss >> value;
    return value;
}

float String::toFloat() const
{
    std::istringstream iss(m_data);
    float value = 0;
    iss >> value;
    return value;
}
