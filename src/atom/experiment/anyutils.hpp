/*
 * anyutils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: A collection of useful functions with std::any Or Any

**************************************************/

#ifndef ATOM_EXPERIMENT_ANYUTILS_HPP
#define ATOM_EXPERIMENT_ANYUTILS_HPP

#include <any>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#if __cplusplus >= 202002L
#include <concepts>
#endif
#ifdef __cpp_concepts
template <typename T>
concept CanBeStringified = requires(T t) {
    { toString(t) } -> std::convertible_to<std::string>;
};
template <typename T>
concept CanBeStringifiedToJson = requires(T t) {
    { toJson(t) } -> std::convertible_to<std::string>;
};
#endif

template <typename T>
[[nodiscard]] std::string toString(const T &value, bool prettyPrint = false);

template <typename T>
[[nodiscard]] std::string toString(const std::vector<T> &vec,
                                   bool prettyPrint = false) {
    std::string result = "[";
    for (const auto &item : vec) {
        result += toString(item, prettyPrint) + ", ";
    }
    if (!vec.empty()) {
        result.erase(result.length() - 2, 2);
    }
    result += "]";
    return result;
}

template <typename K, typename V>
[[nodiscard]] std::string toString(const std::unordered_map<K, V> &map,
                                   bool prettyPrint = false) {
    std::string result = "{";
    for (const auto &pair : map) {
        result += toString(pair.first, prettyPrint) + ": " +
                  toString(pair.second, prettyPrint) + ", ";
    }
    if (!map.empty()) {
        result.erase(result.length() - 2, 2);
    }
    result += "}";
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] std::string toString(const std::pair<T1, T2> &pair,
                                   bool prettyPrint = false) {
    return "(" + toString(pair.first, prettyPrint) + ", " +
           toString(pair.second, prettyPrint) + ")";
}

template <typename T>
[[nodiscard]] std::string toString(const T &value, bool prettyPrint) {
    if constexpr (std::is_same_v<T, std::string> ||
                  std::is_same_v<T, const char *> ||
                  std::is_same_v<T, char *>) {
        return value;
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_pointer_v<T>) {
        if (value == nullptr) {
            return "nullptr";
        } else {
            return toString(*value, prettyPrint);
        }
    } else {
        return "unknown type";
    }
}

template <typename T>
[[nodiscard]] std::string toJson(const T &value, bool prettyPrint = false);

template <typename T>
[[nodiscard]] std::string toJson(const std::vector<T> &vec,
                                 bool prettyPrint = false) {
    std::string result = "[";
    for (const auto &item : vec) {
        result += toJson(item, prettyPrint) + ", ";
    }
    if (!vec.empty()) {
        result.erase(result.length() - 2, 2);
    }
    result += "]";
    return result;
}

template <typename K, typename V>
[[nodiscard]] std::string toJson(const std::unordered_map<K, V> &map,
                                 bool prettyPrint = false) {
    std::string result = "{";
    for (const auto &pair : map) {
        result += toJson(pair.first, prettyPrint) + ": " +
                  toJson(pair.second, prettyPrint) + ", ";
    }
    if (!map.empty()) {
        result.erase(result.length() - 2, 2);
    }
    result += "}";
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] std::string toJson(const std::pair<T1, T2> &pair,
                                 bool prettyPrint = false) {
    return "{" + toJson(pair.first, prettyPrint) + ", " +
           toJson(pair.second, prettyPrint) + "}";
}

template <typename... Args>
[[nodiscard]] std::string toJson(bool prettyPrint, const Args &...args) {
    std::string result = "{";
    ((result += toJson(args, prettyPrint) + ", "), ...);
    if (sizeof...(args) > 0) {
        result.erase(result.length() - 2, 2);
    }
    result += "}";
    return result;
}

template <typename T>
[[nodiscard]] std::string toJson(const T &value, bool prettyPrint) {
    if constexpr (std::is_same_v<T, std::string>) {
        return "\"" + value + "\"";
    } else if constexpr (std::is_same_v<T, const char *> ||
                         std::is_same_v<T, char *>) {
        return "\"" + std::string(value) +
               "\"";  // 将 const char* 转换为 std::string
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_pointer_v<T>) {
        if (value == nullptr) {
            return "null";
        } else {
            return toJson(*value, prettyPrint);
        }
    } else {
        return "{}";  // Default to empty object for unknown type
    }
}

template <typename T>
[[nodiscard]] std::string toXml(const T &value, const std::string &tagName);

template <typename T>
[[nodiscard]] std::string toXml(const std::vector<T> &vec,
                                const std::string &tagName) {
    std::string result;
    for (const auto &item : vec) {
        result += toXml(item, tagName);
    }
    return result;
}

template <typename K, typename V>
[[nodiscard]] std::string toXml(const std::unordered_map<K, V> &map,
                                [[maybe_unused]] const std::string &tagName) {
    std::string result;
    for (const auto &pair : map) {
        result += toXml(pair.second, pair.first);
    }
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] std::string toXml(const std::pair<T1, T2> &pair,
                                const std::string &tagName) {
    std::string result = "<" + tagName + ">";
    result += toXml(pair.first, "key");
    result += toXml(pair.second, "value");
    result += "</" + tagName + ">";
    return result;
}

template <typename T>
[[nodiscard]] std::string toXml(const T &value, const std::string &tagName) {
    if constexpr (std::is_same_v<T, std::string> ||
                  std::is_same_v<T, const char *> || std::is_same_v<T, char *>)
        [[likely]] {
        return "<" + tagName + ">" + value + "</" + tagName + ">";
    } else if constexpr (std::is_arithmetic_v<T>) [[likely]] {
        return "<" + tagName + ">" + std::to_string(value) + "</" + tagName +
               ">";
    } else if constexpr (std::is_pointer_v<T>) [[unlikely]] {
        if (value == nullptr) [[unlikely]] {
            return "<" + tagName + "null" + "/>";
        } else [[likely]] {
            return toXml(*value, tagName);
        }
    } else [[unlikely]] {
        return "<" + tagName + "></" + tagName +
               ">";  // Default to empty element for unknown type
    }
}

template <typename T>
[[nodiscard]] std::string toYaml(const T &value, const std::string &key);

template <typename T>
[[nodiscard]] std::string toYaml(const std::vector<T> &vec,
                                 const std::string &key) {
    std::string result = key + ":\n";
    for (const auto &item : vec) {
        result += "  - " + toYaml(item, "");
    }
    return result;
}

template <typename K, typename V>
[[nodiscard]] std::string toYaml(const std::unordered_map<K, V> &map,
                                 const std::string &key) {
    std::string result = key + ":\n";
    for (const auto &pair : map) {
        result += "  " + toYaml(pair.second, pair.first);
    }
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] std::string toYaml(const std::pair<T1, T2> &pair,
                                 const std::string &key) {
    std::string result = key + ":\n";
    result += "  key: " + toYaml(pair.first, "");
    result += "  value: " + toYaml(pair.second, "");
    return result;
}

template <typename T>
[[nodiscard]] std::string toYaml(const T &value, const std::string &key) {
    if constexpr (std::is_same_v<T, std::string> ||
                  std::is_same_v<T, const char *> ||
                  std::is_same_v<T, char *>) {
        return key + ": \"" + value + "\"\n";
    } else if constexpr (std::is_arithmetic_v<T>) {
        return key + ": " + std::to_string(value) + "\n";
    } else if constexpr (std::is_pointer_v<T>) {
        if (value == nullptr) [[unlikely]] {
            return key + ": null\n";
        } else [[likely]] {
            return toYaml(*value, key);
        }
    } else {
        return key + ":\n";
    }
}

template <typename... Ts>
[[nodiscard]] std::string toYaml(const std::tuple<Ts...> &tuple,
                                 const std::string &key) {
    std::string result = key + ":\n";
    std::apply(
        [&result, &key](const Ts &...args) {
            ((result += "  - " + toYaml(args, "") + "\n"), ...);
        },
        tuple);
    return result;
}

#endif
