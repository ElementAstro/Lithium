#ifndef ATOM_EXPERIMENT_STRING_HPP
#define ATOM_EXPERIMENT_STRING_HPP

#include <string>
#include <vector>
#include <sstream>
#include <cstdarg>
#include <algorithm>

class String
{
public:
    // 默认构造函数
    String() {}

    // 根据C风格字符串构造
    String(const char *str)
        : m_data(str) {}

    // 根据std::string构造
    String(const std::string &str)
        : m_data(str) {}

    // 拷贝构造函数
    String(const String &other)
        : m_data(other.m_data) {}

    // 赋值运算符重载
    String &operator=(const String &other)
    {
        if (this != &other)
        {
            m_data = other.m_data;
        }
        return *this;
    }

    // 比较运算符重载
    bool operator==(const String &other) const
    {
        return m_data == other.m_data;
    }

    bool operator!=(const String &other) const
    {
        return m_data != other.m_data;
    }

    bool operator<(const String &other) const
    {
        return m_data < other.m_data;
    }

    bool operator>(const String &other) const
    {
        return m_data > other.m_data;
    }

    bool operator<=(const String &other) const
    {
        return m_data <= other.m_data;
    }

    bool operator>=(const String &other) const
    {
        return m_data >= other.m_data;
    }

    // 追加字符串
    String &operator+=(const String &other)
    {
        m_data += other.m_data;
        return *this;
    }

    String &operator+=(const char *str)
    {
        m_data += str;
        return *this;
    }

    String &operator+=(char c)
    {
        m_data += c;
        return *this;
    }

    // 获取字符数组
    const char *toCharArray() const
    {
        return m_data.c_str();
    }

    // 获取字符串长度
    size_t length() const
    {
        return m_data.length();
    }

    // 子字符串提取
    String substring(size_t pos, size_t len = std::string::npos) const
    {
        return m_data.substr(pos, len);
    }

    // 查找和替换
    size_t find(const String &str, size_t pos = 0) const
    {
        return m_data.find(str.m_data, pos);
    }

    size_t replace(const String &oldStr, const String &newStr)
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
    String toUpperCase() const
    {
        String result(*this);

        for (size_t i = 0; i < result.length(); ++i)
        {
            result.m_data[i] = toupper(result.m_data[i]);
        }

        return result;
    }

    String toLowerCase() const
    {
        String result(*this);

        for (size_t i = 0; i < result.length(); ++i)
        {
            result.m_data[i] = tolower(result.m_data[i]);
        }

        return result;
    }

    // 字符串拆分和连接
    std::vector<String> split(const String &delimiter) const
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

    static String join(const std::vector<String> &strings, const String &separator)
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

    size_t replaceAll(const String &oldStr, const String &newStr)
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

    // 插入字符
    void insertChar(size_t pos, char c)
    {
        if (pos <= m_data.length())
        {
            m_data.insert(pos, 1, c);
        }
    }

    // 删除字符
    void deleteChar(size_t pos)
    {
        if (pos < m_data.length())
        {
            m_data.erase(pos, 1);
        }
    }

    // 字符串反转
    String reverse() const
    {
        String result(*this);
        std::reverse(result.m_data.begin(), result.m_data.end());
        return result;
    }

    // 字符串比较（忽略大小写）
    bool equalsIgnoreCase(const String &other) const
    {
        return std::equal(m_data.begin(), m_data.end(), other.m_data.begin(), other.m_data.end(),
                          [](char a, char b)
                          {
                              return std::tolower(a) == std::tolower(b);
                          });
    }

    // 获取子串出现的位置
    size_t indexOf(const String &subStr, size_t startPos = 0) const
    {
        return m_data.find(subStr.m_data, startPos);
    }

    // 去除首尾空格
    void trim()
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

    // 判断字符串是否以指定的前缀开头
    bool startsWith(const String &prefix) const
    {
        if (prefix.length() > m_data.length())
        {
            return false;
        }

        return std::equal(prefix.m_data.begin(), prefix.m_data.end(), m_data.begin());
    }

    // 判断字符串是否以指定的后缀结尾
    bool endsWith(const String &suffix) const
    {
        if (suffix.length() > m_data.length())
        {
            return false;
        }

        return std::equal(suffix.m_data.rbegin(), suffix.m_data.rend(), m_data.rbegin());
    }

    // 字符串转义
    String escape() const
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

    // 字符串反转义
    String unescape() const
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

    // 数字转换
    int toInt() const
    {
        std::istringstream iss(m_data);
        int value = 0;
        iss >> value;
        return value;
    }

    float toFloat() const
    {
        std::istringstream iss(m_data);
        float value = 0;
        iss >> value;
        return value;
    }

    // 字符串格式化
    static String format(const char *format, ...)
    {
        char buffer[1024];

        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        return String(buffer);
    }

private:
    std::string m_data;
};

// 重载+运算符
String operator+(const String &lhs, const String &rhs)
{
    String result(lhs);
    result += rhs;
    return result;
}

#endif
