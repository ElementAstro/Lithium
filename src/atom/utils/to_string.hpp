/*
 * stringutils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Implementation of command line generator.

**************************************************/

#ifndef ATOM_EXPERIMENT_STRINGUTILS_HPP
#define ATOM_EXPERIMENT_STRINGUTILS_HPP

#include <algorithm>
#include <concepts>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace atom::utils {
template <typename T>
constexpr bool is_string_type =
    std::is_same_v<T, char *> || std::is_same_v<T, const char *> ||
    std::is_same_v<T, std::string>;

template <typename T, typename _ = void>
struct is_container : std::false_type {};

template <typename... Ts>
struct is_container_helper {};

template <typename T>
struct is_container<
    T,
    std::conditional_t<false,
                       is_container_helper<decltype(std::declval<T>().begin()),
                                           decltype(std::declval<T>().end()),
                                           decltype(std::declval<T>().size()),
                                           typename T::value_type>,
                       void>>
    : public std::integral_constant<bool, !is_string_type<T>> {};

template <typename T>
struct is_map : std::false_type {};

template <typename... Args>
struct is_map<std::map<Args...>> : std::true_type {};

template <typename... Args>
struct is_map<std::unordered_map<Args...>> : std::true_type {};

#if __cplusplus >= 202002L

template <typename T>
concept BasicType = std::is_arithmetic_v<T>;

/**
 * @brief Check if a type is a basic type.
 * @tparam T The type to check.
 * @return True if the type is a basic type, false otherwise.
 */
template <typename T>
concept StringType = requires(T a) {
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> ||
        std::is_same_v<T, const char *> || std::is_same_v<T, char *>;
};

/**
 * @brief Check if a type is a basic type.
 * @tparam T The type to check.
 * @return True if the type is a basic type, false otherwise.
 */
template <typename T>
concept SequenceContainer = requires(T a) {
    typename T::value_type;
    { a.begin() } -> std::forward_iterator;
    { a.end() } -> std::forward_iterator;
};

/**
 * @brief Check if a type is an associative container.
 * @tparam T The type to check.
 * @return True if the type is an associative container, false otherwise.
 */
template <typename T>
concept AssociativeContainer = requires(T a) {
    typename T::key_type;
    typename T::mapped_type;
    { a.begin() } -> std::forward_iterator;
    { a.end() } -> std::forward_iterator;
};

/**
 * @brief Check if a type is a smart pointer.
 * @tparam T The type to check.
 * @return True if the type is a smart pointer, false otherwise.
 */
template <typename T>
concept SmartPointer = requires(T a) {
    { *a } -> std::convertible_to<typename T::element_type &>;
};

#endif

/**
 * @brief Convert a value to a string representation.
 * @tparam T The type of the value.
 * @param value The value to be converted.
 * @return A string representation of the value.
 */
template <typename T>
auto toString(const T &value)
    -> std::enable_if_t<!is_map<T>::value && !is_container<T>::value,
                        std::string> {
    if constexpr (is_string_type<T>) {
        return std::string(value);
    } else {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
}

/**
 * @brief Join a pair of key-value pairs into a string representation.
 * @tparam Key The type of the key.
 * @tparam Value The type of the value.
 * @param keyValue The pair of key-value pairs.
 * @return A string representation of the pair.
 */
template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue) {
    return "(" + toString(keyValue.first) + ", " + toString(keyValue.second) +
           ")";
}

/**
 * @brief Join a pair of key-value pairs into a string representation.
 * @tparam Key The type of the key.
 * @tparam Value The type of the value.
 * @param keyValue The pair of key-value pairs.
 * @param separator The separator to use between the key and value.
 * @return A string representation of the pair.
 */
template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue,
              const std::string &separator) {
    return toString(keyValue.first) + separator + toString(keyValue.second);
}

/**
 * @brief Join a map of key-value pairs into a string representation.
 * @tparam Container The type of the map.
 * @param container The map of key-value pairs.
 * @return A string representation of the map.
 */
template <typename Container>
std::enable_if_t<is_map<Container>::value, std::string> toString(
    const Container &container) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto &elem : container) {
        if (!first) {
            oss << ", ";
        }
        oss << toString(elem.first) << ": " << toString(elem.second);
        first = false;
    }
    oss << "}";
    return oss.str();
}

/**
 * @brief Join a container of values into a string representation.
 * @tparam Container The type of the container.
 * @param container The container of values.
 * @return A string representation of the container.
 */
template <typename Container>
auto toString(const Container &container)
    -> std::enable_if_t<is_container<Container>::value &&
                            !is_map<Container>::value &&
                            !is_string_type<typename Container::value_type>,
                        std::string> {
    std::ostringstream oss;
    oss << "[";
    auto it = container.begin();
    while (it != container.end()) {
        oss << toString(*it);
        ++it;
        if (it != container.end())
            oss << ", ";
    }
    oss << "]";
    return oss.str();
}

/**
 * @brief Join a vector of values into a string representation.
 * @tparam T The type of the values in the vector.
 * @param value The vector of values.
 * @return A string representation of the vector.
 */
template <typename T>
std::string toString(const std::vector<T> &value) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < value.size(); ++i) {
        oss << toString(value[i]);
        if (i != value.size() - 1) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

/**
 * @brief Join a key-value pair into a string representation.
 * @tparam T The type of the value.
 * @param key The key.
 * @param value The value.
 * @param separator The separator to use between the key and value.
 * @return A string representation of the key-value pair.
 */
template <typename T>
std::enable_if_t<is_string_type<T>, std::string> joinKeyValuePair(
    const std::string &key, const T &value, const std::string &separator = "") {
    return key + separator + std::string(value);
}

/**
 * @brief Join a key-value pair into a string representation.
 * @tparam Key The type of the key.
 * @tparam Value The type of the value.
 * @param keyValue The key-value pair to join.
 * @param separator The separator to use between the key and value.
 * @return A string representation of the key-value pair.
 */
template <typename Key, typename Value>
std::string joinKeyValuePair(const std::pair<Key, Value> &keyValue,
                             const std::string &separator = "") {
    return joinKeyValuePair(keyValue.first, keyValue.second, separator);
}

/**
 * @brief Join command line arguments into a single string.
 * @tparam Args The types of the command line arguments.
 * @param args The command line arguments.
 * @return A string representation of the command line arguments.
 */
template <typename... Args>
[[nodiscard]] std::string joinCommandLine(const Args &...args) {
    std::ostringstream oss;
    bool firstArg = true;
    ((oss << (firstArg ? (firstArg = false, "") : " ") << toString(args)), ...);
    return oss.str();
}

/**
 * @brief Convert a vector of elements to a string representation.
 * @tparam T The type of elements in the vector.
 * @param array The vector to convert.
 * @return A string representation of the vector.
 */
template <typename T>
auto toStringArray(const std::vector<T> &array) -> std::string {
    std::ostringstream oss;
    for (size_t i = 0; i < array.size(); ++i) {
        oss << toString(array[i]);
        if (i < array.size() - 1) {
            oss << " ";
        }
    }
    return oss.str();
}

/**
 * @brief Concept to check if a type has begin() and end() member functions.
 * @tparam T The type to check.
 */
template <typename T>
concept HasIterator = requires(T t) {
    t.begin(); /**< The begin() member function. */
    t.end();   /**< The end() member function. */
};

/**
 * @brief Implementation of string equality comparison for types supporting
 * iterators.
 * @tparam LHS Type of the left-hand side.
 * @tparam RHS Type of the right-hand side.
 * @param t_lhs The left-hand side operand.
 * @param t_rhs The right-hand side operand.
 * @return True if the strings are equal, false otherwise.
 */
template <HasIterator LHS, HasIterator RHS>
[[nodiscard]] constexpr bool str_equal_impl(const LHS &t_lhs,
                                            const RHS &t_rhs) noexcept {
    return std::equal(t_lhs.begin(), t_lhs.end(), t_rhs.begin(), t_rhs.end());
}

/**
 * @brief Functor for string equality comparison.
 */
struct str_equal {
    /**
     * @brief Compares two std::string objects for equality.
     * @param t_lhs The left-hand side string.
     * @param t_rhs The right-hand side string.
     * @return True if the strings are equal, false otherwise.
     */
    [[nodiscard]] bool operator()(const std::string &t_lhs,
                                  const std::string &t_rhs) const noexcept {
        return t_lhs == t_rhs;
    }

    /**
     * @brief Compares two objects for equality using iterators.
     * @tparam LHS Type of the left-hand side.
     * @tparam RHS Type of the right-hand side.
     * @param t_lhs The left-hand side operand.
     * @param t_rhs The right-hand side operand.
     * @return True if the strings are equal, false otherwise.
     */
    template <HasIterator LHS, HasIterator RHS>
    [[nodiscard]] constexpr bool operator()(const LHS &t_lhs,
                                            const RHS &t_rhs) const noexcept {
        return str_equal_impl(t_lhs, t_rhs);
    }

    struct is_transparent {}; /**< Enables transparent comparison. */
};

/**
 * @brief Implementation of string less-than comparison for types supporting
 * iterators.
 * @tparam T Type of the operands.
 * @param t_lhs The left-hand side operand.
 * @param t_rhs The right-hand side operand.
 * @return True if t_lhs is less than t_rhs, false otherwise.
 */
template <typename T>
[[nodiscard]] constexpr bool str_less_impl(const T &t_lhs,
                                           const T &t_rhs) noexcept {
    return t_lhs < t_rhs;
}

/**
 * @brief Implementation of string less-than comparison for types supporting
 * iterators.
 * @tparam LHS Type of the left-hand side.
 * @tparam RHS Type of the right-hand side.
 * @param t_lhs The left-hand side operand.
 * @param t_rhs The right-hand side operand.
 * @return True if t_lhs is less than t_rhs, false otherwise.
 */
template <HasIterator LHS, HasIterator RHS>
[[nodiscard]] constexpr bool str_less_impl(const LHS &t_lhs,
                                           const RHS &t_rhs) noexcept {
    return std::lexicographical_compare(t_lhs.begin(), t_lhs.end(),
                                        t_rhs.begin(), t_rhs.end());
}

/**
 * @brief Functor for string less-than comparison.
 */
struct str_less {
    /**
     * @brief Compares two std::string objects.
     * @param t_lhs The left-hand side string.
     * @param t_rhs The right-hand side string.
     * @return True if t_lhs is less than t_rhs, false otherwise.
     */
    [[nodiscard]] bool operator()(const std::string &t_lhs,
                                  const std::string &t_rhs) const noexcept {
        return t_lhs < t_rhs;
    }

    /**
     * @brief Compares two objects using iterators.
     * @tparam LHS Type of the left-hand side.
     * @tparam RHS Type of the right-hand side.
     * @param t_lhs The left-hand side operand.
     * @param t_rhs The right-hand side operand.
     * @return True if t_lhs is less than t_rhs, false otherwise.
     */
    template <HasIterator LHS, HasIterator RHS>
    [[nodiscard]] constexpr bool operator()(const LHS &t_lhs,
                                            const RHS &t_rhs) const noexcept {
        return str_less_impl(t_lhs, t_rhs);
    }

    struct is_transparent {}; /**< Enables transparent comparison. */
};
}  // namespace atom::utils

#endif  // ATOM_STRINGUTILS_HPP
