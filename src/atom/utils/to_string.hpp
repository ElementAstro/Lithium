/*
 * to_string.hpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

#ifndef ATOM_UTILS_TO_STRING_HPP
#define ATOM_UTILS_TO_STRING_HPP

#include <array>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace atom::utils {

// StringType 概念
template <typename T>
concept StringType = std::is_same_v<std::decay_t<T>, std::string> ||
                     std::is_same_v<std::decay_t<T>, const char*> ||
                     std::is_same_v<std::decay_t<T>, char*>;

// Container 概念
template <typename T>
concept Container = requires(T container) {
    std::begin(container);
    std::end(container);
};

// MapType 概念
template <typename T>
concept MapType = requires(T map) {
    typename T::key_type;
    typename T::mapped_type;
    std::begin(map);
    std::end(map);
};

// PointerType 概念
template <typename T>
concept PointerType = std::is_pointer_v<T> && !StringType<T>;

// EnumType 概念
template <typename T>
concept EnumType = std::is_enum_v<T>;

// SmartPointer 概念
template <typename T>
concept SmartPointer = requires(T smartPtr) {
    *smartPtr;
    smartPtr.get();
};

// 将字符串类型转换为 std::string
template <StringType T>
auto toString(T&& value) -> std::string {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        return value;
    } else {
        return std::string(value);
    }
}

// 将 char 类型转换为 std::string
auto toString(char value) -> std::string { return std::string(1, value); }

// 将枚举类型转换为 std::string
template <EnumType T>
auto toString(T value) -> std::string {
    return std::to_string(static_cast<std::underlying_type_t<T>>(value));
}

// 将指针类型转换为 std::string
template <PointerType T>
auto toString(T ptr) -> std::string {
    if (ptr) {
        return "Pointer(" + toString(*ptr) + ")";
    }
    return "nullptr";
}

// 将智能指针类型转换为 std::string
template <SmartPointer T>
auto toString(const T& ptr) -> std::string {
    if (ptr) {
        return "SmartPointer(" + toString(*ptr) + ")";
    }
    return "nullptr";
}

// 将容器类型转换为 std::string
template <Container T>
auto toString(const T& container,
              const std::string& separator = ", ") -> std::string {
    std::ostringstream oss;
    if constexpr (MapType<T>) {
        oss << "{";
        bool first = true;
        for (const auto& [key, value] : container) {
            if (!first) {
                oss << separator;
            }
            oss << toString(key) << ": " << toString(value);
            first = false;
        }
        oss << "}";
    } else {
        oss << "[";
        auto iter = std::begin(container);
        auto end = std::end(container);
        while (iter != end) {
            oss << toString(*iter);
            ++iter;
            if (iter != end) {
                oss << separator;
            }
        }
        oss << "]";
    }
    return oss.str();
}

// 将一般类型转换为 std::string
template <typename T>
    requires(!StringType<T> && !Container<T> && !PointerType<T> &&
             !EnumType<T> && !SmartPointer<T>)
auto toString(const T& value) -> std::string {
    if constexpr (requires { std::to_string(value); }) {
        return std::to_string(value);
    } else {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
}

// 将多个参数连接成一个命令行字符串
template <typename... Args>
auto joinCommandLine(const Args&... args) -> std::string {
    std::ostringstream oss;
    ((oss << toString(args) << ' '), ...);
    std::string result = oss.str();
    if (!result.empty()) {
        result.pop_back();  // 移除尾部空格
    }
    return result;
}

// 将数组转换为 std::string
template <Container T>
auto toStringArray(const T& array,
                   const std::string& separator = " ") -> std::string {
    std::ostringstream oss;
    bool first = true;
    for (const auto& item : array) {
        if (!first) {
            oss << separator;
        }
        oss << toString(item);
        first = false;
    }
    return oss.str();
}

// 将范围转换为 std::string
template <typename Iterator>
auto toStringRange(Iterator begin, Iterator end,
                   const std::string& separator = ", ") -> std::string {
    std::ostringstream oss;
    oss << "[";
    for (auto iter = begin; iter != end; ++iter) {
        oss << toString(*iter);
        if (std::next(iter) != end) {
            oss << separator;
        }
    }
    oss << "]";
    return oss.str();
}

// 将 std::array 转换为 std::string
template <typename T, std::size_t N>
auto toString(const std::array<T, N>& array) -> std::string {
    return toStringRange(array.begin(), array.end());
}

// 将元组转换为 std::string
template <typename Tuple, std::size_t... I>
auto tupleToStringImpl(const Tuple& tpl, std::index_sequence<I...>,
                       const std::string& separator) -> std::string {
    std::ostringstream oss;
    oss << "(";
    ((oss << toString(std::get<I>(tpl))
          << (I < sizeof...(I) - 1 ? separator : "")),
     ...);
    oss << ")";
    return oss.str();
}

// 将 std::tuple 转换为 std::string
template <typename... Args>
auto toString(const std::tuple<Args...>& tpl,
              const std::string& separator = ", ") -> std::string {
    return tupleToStringImpl(tpl, std::index_sequence_for<Args...>(),
                             separator);
}

// 将 std::optional 转换为 std::string
template <typename T>
auto toString(const std::optional<T>& opt) -> std::string {
    if (opt.has_value()) {
        return "Optional(" + toString(*opt) + ")";
    }
    return "nullopt";
}

// 将 std::variant 转换为 std::string
template <typename... Ts>
auto toString(const std::variant<Ts...>& var) -> std::string {
    return std::visit(
        [](const auto& value) -> std::string { return toString(value); }, var);
}

}  // namespace atom::utils

#endif  // ATOM_UTILS_TO_STRING_HPP
