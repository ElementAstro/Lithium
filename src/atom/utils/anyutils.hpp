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
#include <concepts>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename T>
concept CanBeStringified = requires(T t) {
    { toString(t) } -> std::convertible_to<std::string>;
};

template <typename T>
concept CanBeStringifiedToJson = requires(T t) {
    { toJson(t) } -> std::convertible_to<std::string>;
};

template <typename T>
concept IsBuiltIn =
    std::is_fundamental_v<T> || std::is_same_v<T, char> ||
    std::is_same_v<T, const char *> || std::is_same_v<T, std::string> ||
    std::is_same_v<T, std::string_view>;

template <typename Container>
concept ContainerLike = requires(const Container &c) {
    { c.begin() } -> std::input_iterator;
    { c.end() } -> std::input_iterator;
};

template <typename T>
[[nodiscard]] std::string toString(const T &value, bool prettyPrint = false);

template <ContainerLike Container>
[[nodiscard]] std::string toString(const Container &container,
                                   bool prettyPrint = false) {
    std::string result = "[";
    for (const auto &item : container) {
        if constexpr (IsBuiltIn<decltype(item)>) {
            result += toString(item, prettyPrint) + ", ";
        } else {
            result += "\"" + toString(item, prettyPrint) + "\", ";
        }
    }
    if (!container.empty()) {
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
                  std::is_same_v<T, std::string_view> ||
                  std::is_same_v<T, const char *> ||
                  std::is_same_v<T, char *>) {
        return value;
    } else if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
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

template <ContainerLike Container>
[[nodiscard]] std::string toJson(const Container &container,
                                 bool prettyPrint = false) {
    std::string result = "[";
    for (const auto &item : container) {
        result += toJson(item, prettyPrint) + ", ";
    }
    if (!container.empty()) {
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

template <typename T>
[[nodiscard]] std::string toJson(const T &value, bool prettyPrint) {
    if constexpr (std::is_same_v<T, std::string>) {
        return "\"" + value + "\"";
    } else if constexpr (std::is_same_v<T, const char *> ||
                         std::is_same_v<T, char *>) {
        return "\"" + std::string(value) + "\"";
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_pointer_v<T>) {
        if (value == nullptr) {
            return "null";
        } else {
            return toJson(*value, prettyPrint);
        }
    } else {
        return "{}";
    }
}

template <typename T>
[[nodiscard]] std::string toXml(const T &value, const std::string &tagName);

template <ContainerLike Container>
[[nodiscard]] std::string toXml(const Container &container,
                                const std::string &tagName) {
    std::string result;
    for (const auto &item : container) {
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
                  std::is_same_v<T, const char *> ||
                  std::is_same_v<T, char *>) {
        return "<" + tagName + ">" + value + "</" + tagName + ">";
    } else if constexpr (std::is_arithmetic_v<T>) {
        return "<" + tagName + ">" + std::to_string(value) + "</" + tagName +
               ">";
    } else if constexpr (std::is_pointer_v<T>) {
        if (value == nullptr) [[unlikely]] {
            return "<" + tagName + "null" + "/>";
        } else [[likely]] {
            return toXml(*value, tagName);
        }
    } else {
        return "<" + tagName + "></" + tagName + ">";
    }
}

template <typename T>
[[nodiscard]] std::string toYaml(const T &value, const std::string &key);

template <ContainerLike Container>
[[nodiscard]] std::string toYaml(const Container &container,
                                 const std::string &key) {
    std::string result = key.empty() ? "" : key + ":\n";
    for (const auto &item : container) {
        result += (key.empty() ? "- " : "  - ") + toYaml(item, "") + "\n";
    }
    return result;
}

template <typename K, typename V>
[[nodiscard]] std::string toYaml(const std::unordered_map<K, V> &map,
                                 const std::string &key) {
    std::string result = key.empty() ? "" : key + ":\n";
    for (const auto &pair : map) {
        result += (key.empty() ? "" : "  ") + toYaml(pair.second, pair.first);
    }
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] std::string toYaml(const std::pair<T1, T2> &pair,
                                 const std::string &key) {
    std::string result = key.empty() ? "" : key + ":\n";
    result += std::string((key.empty() ? "" : "  ")) +
              "key: " + toYaml(pair.first, "");
    result += std::string((key.empty() ? "" : "  ")) +
              "value: " + toYaml(pair.second, "");
    return result;
}

template <typename T>
[[nodiscard]] std::string toYaml(const T &value, const std::string &key) {
    if constexpr (std::is_same_v<T, std::string> ||
                  std::is_same_v<T, const char *> ||
                  std::is_same_v<T, char *>) {
        return key.empty() ? "\"" + std::string(value) + "\""
                           : key + ": \"" + std::string(value) + "\"\n";
    } else if constexpr (std::is_arithmetic_v<T>) {
        return key.empty() ? std::to_string(value)
                           : key + ": " + std::to_string(value) + "\n";
    } else if constexpr (std::is_pointer_v<T>) {
        if (value == nullptr) [[unlikely]] {
            return key.empty() ? "null" : key + ": null\n";
        } else [[likely]] {
            return toYaml(*value, key);
        }
    } else {
        return key.empty() ? "" : key + ":\n";
    }
}

template <typename... Ts>
[[nodiscard]] std::string toYaml(const std::tuple<Ts...> &tuple,
                                 const std::string &key) {
    std::string result = key.empty() ? "" : key + ":\n";
    std::apply(
        [&result, &key](const Ts &...args) {
            ((result +=
              (key.empty() ? "- " : "  - ") + toYaml(args, "") + "\n"),
             ...);
        },
        tuple);
    return result;
}

template <typename T>
[[nodiscard]] std::string toToml(const T &value, const std::string &key);

template <ContainerLike Container>
[[nodiscard]] std::string toToml(const Container &container,
                                 const std::string &key) {
    std::string result = key + " = [\n";
    for (const auto &item : container) {
        result += "  " + toToml(item, "") + ",\n";
    }
    if (!container.empty()) {
        result.erase(result.length() - 2, 1);  // Remove the last comma
    }
    result += "]\n";
    return result;
}

template <typename K, typename V>
[[nodiscard]] std::string toToml(const std::unordered_map<K, V> &map,
                                 const std::string &key) {
    std::string result = key.empty() ? "" : key + " = {\n";
    for (const auto &pair : map) {
        result += "  " + toToml(pair.second, pair.first);
    }
    if (!map.empty()) {
        result.erase(result.length() - 1);  // Remove the last newline
    }
    result += key.empty() ? "" : "\n}\n";
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] std::string toToml(const std::pair<T1, T2> &pair,
                                 const std::string &key) {
    std::string result = key.empty() ? "" : key + " = {\n";
    result += "  key = " + toToml(pair.first, "") + ",\n";
    result += "  value = " + toToml(pair.second, "") + "\n";
    result += key.empty() ? "" : "}\n";
    return result;
}

template <typename T>
[[nodiscard]] std::string toToml(const T &value, const std::string &key) {
    if constexpr (std::is_same_v<T, std::string> ||
                  std::is_same_v<T, const char *> ||
                  std::is_same_v<T, char *>) {
        return key.empty() ? "\"" + std::string(value) + "\""
                           : key + " = \"" + std::string(value) + "\"\n";
    } else if constexpr (std::is_arithmetic_v<T>) {
        return key.empty() ? std::to_string(value)
                           : key + " = " + std::to_string(value) + "\n";
    } else if constexpr (std::is_pointer_v<T>) {
        if (value == nullptr) [[unlikely]] {
            return key.empty() ? "null" : key + " = null\n";
        } else [[likely]] {
            return toToml(*value, key);
        }
    } else {
        return key.empty() ? "" : key + " = {}\n";
    }
}

template <typename... Ts>
[[nodiscard]] std::string toToml(const std::tuple<Ts...> &tuple,
                                 const std::string &key) {
    std::string result = key.empty() ? "" : key + " = [\n";
    std::apply(
        [&result, &key](const Ts &...args) {
            ((result += "  " + toToml(args, "") + ",\n"), ...);
        },
        tuple);
    if (sizeof...(Ts) > 0) {
        result.erase(result.length() - 2, 1);  // Remove the last comma
    }
    result += key.empty() ? "" : "]\n";
    return result;
}

#endif  // ATOM_EXPERIMENT_ANYUTILS_HPP