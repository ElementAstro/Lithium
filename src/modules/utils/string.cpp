#include "string.hpp"
#include <algorithm>

bool HasUppercase(const std::string& str)
{
    return std::any_of(str.begin(), str.end(), [](char ch) { return std::isupper(ch); });
}

std::string ToUnderscore(const std::string& str)
{
    std::string result;
    for (char ch : str)
    {
        if (std::isupper(ch))
        {
            result += '_';
            result += std::tolower(ch);
        }
        else
        {
            result += ch;
        }
    }
    return result;
}

std::string ToCamelCase(const std::string& str)
{
    std::string result;
    bool capitalize = false;

    for (char ch : str)
    {
        if (ch == '_')
        {
            capitalize = true;
        }
        else
        {
            if (capitalize)
            {
                result += std::toupper(ch);
                capitalize = false;
            }
            else
            {
                result += ch;
            }
        }
    }
    return result;
}

std::string ConvertToUnderscore(const std::string& str)
{
    return HasUppercase(str) ? ToUnderscore(str) : str;
}

std::string ConvertToCamelCase(const std::string& str)
{
    if (str.find('_') != std::string::npos)
    {
        return ToCamelCase(str);
    }
    else
    {
        std::string result = str;
        result[0] = std::tolower(result[0]);
        return result;
    }
}
