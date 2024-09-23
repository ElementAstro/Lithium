/*
 * stringutils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_UTILS_TO_STRING_HPP
#define ATOM_UTILS_TO_STRING_HPP

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "atom/function/concept.hpp"

namespace atom::utils {

// -----------------------------------------------------------------------------
// Concepts
// -----------------------------------------------------------------------------

template <typename T>
concept StringType = String<T> || Char<T> || std::is_same_v<T, std::string>;

template <typename T>
concept Container = requires(T container) {
    container.begin();
    container.end();
    container.size();
};

template <typename T>
concept MapType = requires(T map) {
    typename T::key_type;
    typename T::mapped_type;
    requires Container<T>;
};

template <typename T>
concept PointerType = std::is_pointer_v<T>;

template <typename T>
concept EnumType = std::is_enum_v<T>;

// -----------------------------------------------------------------------------
// toString Implementation
// -----------------------------------------------------------------------------

/**
 * @brief Convert a non-container, non-map, non-pointer, non-enum type to a
 * string.
 * @tparam T The type of the value.
 * @param value The value to convert.
 * @return A string representation of the value.
 */
template <typename T>
    requires(!Container<T> && !MapType<T> && !StringType<T> &&
             !PointerType<T> && !EnumType<T> && !SmartPointer<T>)
auto toString(const T& value) -> std::string {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

/**
 * @brief Convert a string type to a string.
 * @tparam T The type of the string (e.g., std::string, const char*).
 * @param value The string to convert.
 * @return The string itself.
 */
template <StringType T>
    requires(!Container<T>)
auto toString(const T& value) -> std::string {
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else if constexpr (std::is_same_v<T, const char*>) {
        return std::string(value);
    } else {
        return std::string(1, value);
    }
}

/**
 * @brief Convert an enum type to a string by casting to its underlying type.
 * @tparam Enum The enum type.
 * @param value The enum value to convert.
 * @return A string representation of the enum's underlying type.
 */
template <EnumType Enum>
auto toString(const Enum& value) -> std::string {
    return std::to_string(static_cast<std::underlying_type_t<Enum>>(value));
}

/**
 * @brief Convert a pointer type to a string.
 * @tparam T The pointer type.
 * @param ptr The pointer to convert.
 * @return A string representation of the pointer address or value.
 */
template <PointerType T>
auto toString(T ptr) -> std::string {
    if (ptr) {
        return "Pointer(" + toString(*ptr) + ")";
    }
    return "nullptr";
}

/**
 * @brief Convert a smart pointer type (unique_ptr/shared_ptr) to a string.
 * @tparam SmartPtr The smart pointer type.
 * @param ptr The smart pointer to convert.
 * @return A string representation of the smart pointer.
 */
template <SmartPointer SmartPtr>
auto toString(const SmartPtr& ptr) -> std::string {
    if (ptr) {
        return "SmartPointer(" + toString(*ptr) + ")";
    }
    return "nullptr";
}

/**
 * @brief Convert a key-value pair to a string.
 * @tparam Key The type of the key.
 * @tparam Value The type of the value.
 * @param keyValue The key-value pair to convert.
 * @return A string representation of the key-value pair.
 */
template <typename Key, typename Value>
auto toString(const std::pair<Key, Value>& keyValue) -> std::string {
    return "(" + toString(keyValue.first) + ", " + toString(keyValue.second) +
           ")";
}

/**
 * @brief Convert a map (or unordered_map) to a string representation.
 * @tparam Map The map type.
 * @param map The map to convert.
 * @return A string representation of the map.
 */
template <MapType Map>
auto toString(const Map& map) -> std::string {
    std::ostringstream oss;
    oss << "{";
    bool isFirst = true;
    for (const auto& [key, value] : map) {
        if (!isFirst) {
            oss << ", ";
        }
        oss << toString(key) << ": " << toString(value);
        isFirst = false;
    }
    oss << "}";
    return oss.str();
}

/**
 * @brief Convert a container (e.g., vector) to a string representation.
 * @tparam ContainerType The container type.
 * @param container The container to convert.
 * @return A string representation of the container.
 */
template <Container ContainerType>
    requires(!MapType<ContainerType> &&
             !StringType<typename ContainerType::value_type>)
auto toString(const ContainerType& container) -> std::string {
    std::ostringstream oss;
    oss << "[";
    auto iterator = container.begin();
    while (iterator != container.end()) {
        oss << toString(*iterator);
        ++iterator;
        if (iterator != container.end()) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

template <Container ContainerType>
    requires(Container<ContainerType> &&
             StringType<typename ContainerType::value_type>)
auto toString(const ContainerType& container) -> std::string {
    std::ostringstream oss;
    oss << "[";
    auto iterator = container.begin();
    while (iterator != container.end()) {
        oss << toString(*iterator);
        ++iterator;
        if (iterator != container.end()) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

/**
 * @brief Join multiple values into a single command line string.
 * @tparam Args The types of the arguments.
 * @param args The arguments to join.
 * @return A string representation of the command line arguments.
 */
template <typename... Args>
auto joinCommandLine(const Args&... args) -> std::string {
    std::ostringstream oss;
    bool isFirst = true;
    ((oss << (isFirst ? (isFirst = false, "") : " ") << toString(args)), ...);
    return oss.str();
}

/**
 * @brief Convert a vector to a space-separated string representation.
 * @tparam ContainerType The type of the elements in the vector.
 * @param array The vector to convert.
 * @return A space-separated string representation of the vector.
 */
template <Container ContainerType>
auto toStringArray(const ContainerType& array) -> std::string {
    std::ostringstream oss;
    for (size_t index = 0; index < array.size(); ++index) {
        oss << toString(array[index]);
        if (index < array.size() - 1) {
            oss << " ";
        }
    }
    return oss.str();
}

/**
 * @brief Convert an iterator range to a string representation.
 * @tparam Iterator The type of the iterator.
 * @param begin The beginning of the range.
 * @param end The end of the range.
 * @return A string representation of the range.
 */
template <typename Iterator>
auto toStringRange(Iterator begin, Iterator end) -> std::string {
    std::ostringstream oss;
    oss << "[";
    while (begin != end) {
        oss << toString(*begin);
        ++begin;
        if (begin != end) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

/**
 * @brief Convert an array to a string.
 * @tparam T The type of the array elements.
 * @tparam N The size of the array.
 * @param array The array to convert.
 * @return A string representation of the array.
 */
template <typename T, std::size_t N>
auto toString(const std::array<T, N>& array) -> std::string {
    return toStringRange(array.begin(), array.end());
}

}  // namespace atom::utils

#endif  // ATOM_UTILS_TO_STRING_HPP