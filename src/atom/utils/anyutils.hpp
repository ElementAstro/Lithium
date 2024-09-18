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

#include <concepts>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "atom/function/concept.hpp"

template <typename T>
concept CanBeStringified = requires(T t) {
    { toString(t) } -> std::convertible_to<std::string>;
};

template <typename T>
concept CanBeStringifiedToJson = requires(T t) {
    { toJson(t) } -> std::convertible_to<std::string>;
};

template <typename T>
[[nodiscard]] auto toString(const T &value,
                            bool prettyPrint = false) -> std::string;

template <std::ranges::input_range Container>
[[nodiscard]] auto toString(const Container &container,
                            bool prettyPrint = false) -> std::string {
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
[[nodiscard]] auto toString(const std::unordered_map<K, V> &map,
                            bool prettyPrint = false) -> std::string {
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
[[nodiscard]] auto toString(const std::pair<T1, T2> &pair,
                            bool prettyPrint = false) -> std::string {
    return "(" + toString(pair.first, prettyPrint) + ", " +
           toString(pair.second, prettyPrint) + ")";
}

template <typename T>
[[nodiscard]] auto toString(const T &value, bool prettyPrint) -> std::string {
    if constexpr (String<T> || Char<T>) {
        return value;
    } else if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr (Number<T>) {
        return std::to_string(value);
    } else if constexpr (Pointer<T> || SmartPointer<T>) {
        if (value == nullptr) {
            return "nullptr";
        }
        return toString(*value, prettyPrint);
    } else {
        return "unknown type";
    }
}

template <typename T>
[[nodiscard]] auto toJson(const T &value,
                          bool prettyPrint = false) -> std::string;

template <std::ranges::input_range Container>
[[nodiscard]] auto toJson(const Container &container,
                          bool prettyPrint = false) -> std::string {
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
[[nodiscard]] auto toJson(const std::unordered_map<K, V> &map,
                          bool prettyPrint = false) -> std::string {
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
[[nodiscard]] auto toJson(const std::pair<T1, T2> &pair,
                          bool prettyPrint = false) -> std::string {
    return "{" + toJson(pair.first, prettyPrint) + ", " +
           toJson(pair.second, prettyPrint) + "}";
}

template <typename T>
[[nodiscard]] auto toJson(const T &value, bool prettyPrint) -> std::string {
    if constexpr (String<T>) {
        return "\"" + value + "\"";
    } else if constexpr (Char<T>) {
        return "\"" + std::string(value) + "\"";
    } else if constexpr (Number<T>) {
        return std::to_string(value);
    } else if constexpr (Pointer<T> || SmartPointer<T>) {
        if (value == nullptr) {
            return "null";
        }
        return toJson(*value, prettyPrint);
    } else {
        return "{}";
    }
}

template <typename T>
[[nodiscard]] auto toXml(const T &value,
                         const std::string &tagName) -> std::string;

template <std::ranges::input_range Container>
[[nodiscard]] auto toXml(const Container &container,
                         const std::string &tagName) -> std::string {
    std::string result;
    for (const auto &item : container) {
        result += toXml(item, tagName);
    }
    return result;
}

template <typename K, typename V>
[[nodiscard]] auto toXml(const std::unordered_map<K, V> &map,
                         [[maybe_unused]] const std::string &tagName)
    -> std::string {
    std::string result;
    for (const auto &pair : map) {
        result += toXml(pair.second, pair.first);
    }
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] auto toXml(const std::pair<T1, T2> &pair,
                         const std::string &tagName) -> std::string {
    std::string result = "<" + tagName + ">";
    result += toXml(pair.first, "key");
    result += toXml(pair.second, "value");
    result += "</" + tagName + ">";
    return result;
}

template <typename T>
[[nodiscard]] auto toXml(const T &value,
                         const std::string &tagName) -> std::string {
    if constexpr (String<T> || Char<T>) {
        return "<" + tagName + ">" + value + "</" + tagName + ">";
    } else if constexpr (Number<T>) {
        return "<" + tagName + ">" + std::to_string(value) + "</" + tagName +
               ">";
    } else if constexpr (Pointer<T> || SmartPointer<T>) {
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
[[nodiscard]] auto toYaml(const T &value,
                          const std::string &key) -> std::string;

template <std::ranges::input_range Container>
[[nodiscard]] auto toYaml(const Container &container,
                          const std::string &key) -> std::string {
    std::string result = key.empty() ? "" : key + ":\n";
    for (const auto &item : container) {
        result += (key.empty() ? "- " : "  - ") + toYaml(item, "") + "\n";
    }
    return result;
}

template <typename K, typename V>
[[nodiscard]] auto toYaml(const std::unordered_map<K, V> &map,
                          const std::string &key) -> std::string {
    std::string result = key.empty() ? "" : key + ":\n";
    for (const auto &pair : map) {
        result += (key.empty() ? "" : "  ") + toYaml(pair.second, pair.first);
    }
    return result;
}

template <typename T1, typename T2>
[[nodiscard]] auto toYaml(const std::pair<T1, T2> &pair,
                          const std::string &key) -> std::string {
    std::string result = key.empty() ? "" : key + ":\n";
    result += std::string((key.empty() ? "" : "  ")) +
              "key: " + toYaml(pair.first, "");
    result += std::string((key.empty() ? "" : "  ")) +
              "value: " + toYaml(pair.second, "");
    return result;
}

template <typename T>
[[nodiscard]] auto toYaml(const T &value,
                          const std::string &key) -> std::string {
    if constexpr (String<T> || Char<T>) {
        return key.empty() ? "\"" + std::string(value) + "\""
                           : key + ": \"" + std::string(value) + "\"\n";
    } else if constexpr (Number<T>) {
        return key.empty() ? std::to_string(value)
                           : key + ": " + std::to_string(value) + "\n";
    } else if constexpr (Pointer<T> || SmartPointer<T>) {
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
[[nodiscard]] auto toYaml(const std::tuple<Ts...> &tuple,
                          const std::string &key) -> std::string {
    std::string result = key.empty() ? "" : key + ":\n";
    std::apply(
        [&result](const Ts &...args) {
            ((result += "- " + toYaml(args, "") + "\n"), ...);
        },
        tuple);
    return result;
}

template <typename T>
[[nodiscard]] auto toToml(const T &value,
                          const std::string &key) -> std::string;

template <std::ranges::input_range Container>
[[nodiscard]] auto toToml(const Container &container,
                          const std::string &key) -> std::string {
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
[[nodiscard]] auto toToml(const std::unordered_map<K, V> &map,
                          const std::string &key) -> std::string {
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
[[nodiscard]] auto toToml(const std::pair<T1, T2> &pair,
                          const std::string &key) -> std::string {
    std::string result = key.empty() ? "" : key + " = {\n";
    result += "  key = " + toToml(pair.first, "") + ",\n";
    result += "  value = " + toToml(pair.second, "") + "\n";
    result += key.empty() ? "" : "}\n";
    return result;
}

template <typename T>
[[nodiscard]] auto toToml(const T &value,
                          const std::string &key) -> std::string {
    if constexpr (String<T> || Char<T>) {
        return key.empty() ? "\"" + std::string(value) + "\""
                           : key + " = \"" + std::string(value) + "\"\n";
    } else if constexpr (Number<T>) {
        return key.empty() ? std::to_string(value)
                           : key + " = " + std::to_string(value) + "\n";
    } else if constexpr (Pointer<T> || SmartPointer<T>) {
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
[[nodiscard]] auto toToml(const std::tuple<Ts...> &tuple,
                          const std::string &key) -> std::string {
    std::string result = key.empty() ? "" : key + " = [\n";
    std::apply(
        [&result](const Ts &...args) {
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
