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

/**
 * @brief Concept for string types.
 */
template <typename T>
concept StringType = std::is_same_v<std::decay_t<T>, std::string> ||
                     std::is_same_v<std::decay_t<T>, const char*> ||
                     std::is_same_v<std::decay_t<T>, char*>;

/**
 * @brief Concept for container types.
 */
template <typename T>
concept Container = requires(T container) {
    std::begin(container);
    std::end(container);
};

/**
 * @brief Concept for map types.
 */
template <typename T>
concept MapType = requires(T map) {
    typename T::key_type;
    typename T::mapped_type;
    std::begin(map);
    std::end(map);
};

/**
 * @brief Concept for pointer types excluding string types.
 */
template <typename T>
concept PointerType = std::is_pointer_v<T> && !StringType<T>;

/**
 * @brief Concept for enum types.
 */
template <typename T>
concept EnumType = std::is_enum_v<T>;

/**
 * @brief Concept for smart pointer types.
 */
template <typename T>
concept SmartPointer = requires(T smartPtr) {
    *smartPtr;
    smartPtr.get();
};

/**
 * @brief Converts a string type to std::string.
 *
 * @tparam T The type of the string.
 * @param value The string value.
 * @return std::string The converted string.
 */
template <StringType T>
auto toString(T&& value) -> std::string {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        return value;
    } else {
        return std::string(value);
    }
}

/**
 * @brief Converts an enum type to std::string.
 *
 * @tparam T The enum type.
 * @param value The enum value.
 * @return std::string The converted string.
 */
template <EnumType T>
auto toString(T value) -> std::string {
    return std::to_string(static_cast<std::underlying_type_t<T>>(value));
}

/**
 * @brief Converts a pointer type to std::string.
 *
 * @tparam T The pointer type.
 * @param ptr The pointer value.
 * @return std::string The converted string.
 */
template <PointerType T>
auto toString(T ptr) -> std::string {
    if (ptr) {
        return "Pointer(" + toString(*ptr) + ")";
    }
    return "nullptr";
}

/**
 * @brief Converts a smart pointer type to std::string.
 *
 * @tparam T The smart pointer type.
 * @param ptr The smart pointer value.
 * @return std::string The converted string.
 */
template <SmartPointer T>
auto toString(const T& ptr) -> std::string {
    if (ptr) {
        return "SmartPointer(" + toString(*ptr) + ")";
    }
    return "nullptr";
}

/**
 * @brief Converts a container type to std::string.
 *
 * @tparam T The container type.
 * @param container The container value.
 * @param separator The separator between elements.
 * @return std::string The converted string.
 */
template <Container T>
auto toString(const T& container,
              const std::string& separator = ", ") -> std::string {
    std::ostringstream oss;
    if constexpr (MapType<T>) {
        oss << "{";
        bool first = true;
#pragma unroll
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
#pragma unroll
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

/**
 * @brief Converts a general type to std::string.
 *
 * @tparam T The general type.
 * @param value The value.
 * @return std::string The converted string.
 */
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

/**
 * @brief Joins multiple arguments into a single command line string.
 *
 * @tparam Args The types of the arguments.
 * @param args The arguments.
 * @return std::string The joined command line string.
 */
template <typename... Args>
auto joinCommandLine(const Args&... args) -> std::string {
    std::ostringstream oss;
    ((oss << toString(args) << ' '), ...);
    std::string result = oss.str();
    if (!result.empty()) {
        result.pop_back();  // Remove trailing space
    }
    return result;
}

/**
 * @brief Converts an array to std::string.
 *
 * @tparam T The container type.
 * @param array The array value.
 * @param separator The separator between elements.
 * @return std::string The converted string.
 */
template <Container T>
auto toStringArray(const T& array,
                   const std::string& separator = " ") -> std::string {
    std::ostringstream oss;
    bool first = true;
#pragma unroll
    for (const auto& item : array) {
        if (!first) {
            oss << separator;
        }
        oss << toString(item);
        first = false;
    }
    return oss.str();
}

/**
 * @brief Converts a range to std::string.
 *
 * @tparam Iterator The iterator type.
 * @param begin The beginning iterator.
 * @param end The ending iterator.
 * @param separator The separator between elements.
 * @return std::string The converted string.
 */
template <typename Iterator>
auto toStringRange(Iterator begin, Iterator end,
                   const std::string& separator = ", ") -> std::string {
    std::ostringstream oss;
    oss << "[";
#pragma unroll
    for (auto iter = begin; iter != end; ++iter) {
        oss << toString(*iter);
        if (std::next(iter) != end) {
            oss << separator;
        }
    }
    oss << "]";
    return oss.str();
}

/**
 * @brief Converts a std::array to std::string.
 *
 * @tparam T The type of the elements.
 * @tparam N The size of the array.
 * @param array The array value.
 * @return std::string The converted string.
 */
template <typename T, std::size_t N>
auto toString(const std::array<T, N>& array) -> std::string {
    return toStringRange(array.begin(), array.end());
}

/**
 * @brief Converts a tuple to std::string.
 *
 * @tparam Tuple The tuple type.
 * @tparam I The indices of the tuple elements.
 * @param tpl The tuple value.
 * @param separator The separator between elements.
 * @return std::string The converted string.
 */
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

/**
 * @brief Converts a std::tuple to std::string.
 *
 * @tparam Args The types of the tuple elements.
 * @param tpl The tuple value.
 * @param separator The separator between elements.
 * @return std::string The converted string.
 */
template <typename... Args>
auto toString(const std::tuple<Args...>& tpl,
              const std::string& separator = ", ") -> std::string {
    return tupleToStringImpl(tpl, std::index_sequence_for<Args...>(),
                             separator);
}

/**
 * @brief Converts a std::optional to std::string.
 *
 * @tparam T The type of the optional value.
 * @param opt The optional value.
 * @return std::string The converted string.
 */
template <typename T>
auto toString(const std::optional<T>& opt) -> std::string {
    if (opt.has_value()) {
        return "Optional(" + toString(*opt) + ")";
    }
    return "nullopt";
}

/**
 * @brief Converts a std::variant to std::string.
 *
 * @tparam Ts The types of the variant alternatives.
 * @param var The variant value.
 * @return std::string The converted string.
 */
template <typename... Ts>
auto toString(const std::variant<Ts...>& var) -> std::string {
    return std::visit(
        [](const auto& value) -> std::string { return toString(value); }, var);
}

}  // namespace atom::utils

#endif  // ATOM_UTILS_TO_STRING_HPP