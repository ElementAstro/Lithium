/*
 * cmdline.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Implementation of command line generator.

**************************************************/

#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <iterator>

template <typename T>
constexpr bool is_string_type = std::is_same_v<T, char *> || std::is_same_v<T, const char *> || std::is_same_v<T, std::string>;

template <typename T, typename _ = void>
struct is_container : std::false_type
{
};

template <typename... Ts>
struct is_container_helper
{
};

template <typename T>
struct is_container<
    T,
    std::conditional_t<
        false,
        is_container_helper<
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end()),
            decltype(std::declval<T>().size()),
            typename T::value_type>,
        void>> : public std::integral_constant<bool,
                                               !is_string_type<T>>
{
};

template <typename T>
struct is_map : std::false_type
{
};

template <typename... Args>
struct is_map<std::map<Args...>> : std::true_type
{
};

template <typename... Args>
struct is_map<std::unordered_map<Args...>> : std::true_type
{
};

#if __cplusplus >= 202002L

#include <concepts>

template <typename T>
concept BasicType = std::is_arithmetic_v<T>;

template <typename T>
concept StringType = requires(T a) {
    std::is_same_v<T, std::string> || std::is_same_v<T, const char *> || std::is_same_v<T, char *>;
};

template <typename T>
concept SequenceContainer = requires(T a) {
    typename T::value_type;
    {
        a.begin()
    } -> std::forward_iterator;
    {
        a.end()
    } -> std::forward_iterator;
};

template <typename T>
concept AssociativeContainer = requires(T a) {
    typename T::key_type;
    typename T::mapped_type;
    {
        a.begin()
    } -> std::forward_iterator;
    {
        a.end()
    } -> std::forward_iterator;
};

template <typename T>
concept SmartPointer = requires(T a) {
    {
        *a
    } -> std::convertible_to<typename T::element_type &>;
};

#endif

template <typename T>
auto toString(const T &value) -> std::enable_if_t<!is_map<T>::value && !is_container<T>::value, std::string>
{
    if constexpr (is_string_type<T>)
    {
        return std::string(value);
    }
    else
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
}

template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue)
{
    return "(" + toString(keyValue.first) + ", " + toString(keyValue.second) + ")";
}

template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue, const std::string &separator)
{
    return toString(keyValue.first) + separator + toString(keyValue.second);
}

template <typename Container>
std::enable_if_t<is_map<Container>::value, std::string>
toString(const Container &container)
{
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto &elem : container)
    {
        if (!first)
        {
            oss << ", ";
        }
        oss << toString(elem.first) << ": " << toString(elem.second);
        first = false;
    }
    oss << "}";
    return oss.str();
}

template <typename Container>
auto toString(const Container &container) -> std::enable_if_t<is_container<Container>::value && !is_map<Container>::value && !is_string_type<typename Container::value_type>, std::string>
{
    std::ostringstream oss;
    oss << "[";
    auto it = container.begin();
    while (it != container.end())
    {
        oss << toString(*it);
        ++it;
        if (it != container.end())
            oss << ", ";
    }
    oss << "]";
    return oss.str();
}

template <typename T>
std::string toString(const std::vector<T> &value)
{
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < value.size(); ++i)
    {
        oss << toString(value[i]);
        if (i != value.size() - 1)
        {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

template <typename T>
std::enable_if_t<is_string_type<T>, std::string>
joinKeyValuePair(const std::string &key, const T &value, const std::string &separator = "")
{
    return key + separator + std::string(value);
}

template <typename Key, typename Value>
std::string joinKeyValuePair(const std::pair<Key, Value> &keyValue, const std::string &separator = "")
{
    return joinKeyValuePair(keyValue.first, keyValue.second, separator);
}

template <typename... Args>
std::string joinCommandLine(const Args &...args)
{
    std::ostringstream oss;
    bool firstArg = true;
    ((oss << (!firstArg ? " " : " ") << toString(args)), ...);
    return oss.str();
}

template <typename T>
auto toStringArray(const std::vector<T> &array)
{
    std::ostringstream oss;
    for (const auto &elem : array)
    {
        oss << toString(elem) << " ";
    }
    return oss.str();
}
