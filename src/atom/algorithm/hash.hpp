/*
 * hash.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-28

Description: A collection of hash algorithms

**************************************************/

#ifndef ATOM_ALGORITHM_HASH_HPP
#define ATOM_ALGORITHM_HASH_HPP

#include <any>
#include <array>
#include <functional>
#include <tuple>
#include <vector>

namespace atom::algorithm {

/**
 * @brief Concept for types that can be hashed.
 *
 * A type is Hashable if it supports hashing via std::hash and the result is
 * convertible to std::size_t.
 */
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

/**
 * @brief Computes the hash value for a single Hashable value.
 *
 * @tparam T Type of the value to hash, must satisfy Hashable concept.
 * @param value The value to hash.
 * @return std::size_t Hash value of the input value.
 */
template <Hashable T>
auto computeHash(const T& value) -> std::size_t {
    return std::hash<T>{}(value);
}

/**
 * @brief Computes the hash value for a vector of Hashable values.
 *
 * @tparam T Type of the elements in the vector, must satisfy Hashable concept.
 * @param values The vector of values to hash.
 * @return std::size_t Hash value of the vector of values.
 */
template <Hashable T>
auto computeHash(const std::vector<T>& values) -> std::size_t {
    std::size_t result = 0;
    for (const auto& value : values) {
        result ^=
            computeHash(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}

/**
 * @brief Computes the hash value for a tuple of Hashable values.
 *
 * @tparam Ts Types of the elements in the tuple, all must satisfy Hashable
 * concept.
 * @param tuple The tuple of values to hash.
 * @return std::size_t Hash value of the tuple of values.
 */
template <Hashable... Ts>
auto computeHash(const std::tuple<Ts...>& tuple) -> std::size_t {
    std::size_t result = 0;
    std::apply(
        [&result](const Ts&... values) {
            ((result ^=
              computeHash(values) + 0x9e3779b9 + (result << 6) + (result >> 2)),
             ...);
        },
        tuple);
    return result;
}

/**
 * @brief Computes the hash value for an array of Hashable values.
 *
 * @tparam T Type of the elements in the array, must satisfy Hashable concept.
 * @tparam N Size of the array.
 * @param array The array of values to hash.
 * @return std::size_t Hash value of the array of values.
 */
template <Hashable T, std::size_t N>
auto computeHash(const std::array<T, N>& array) -> std::size_t {
    std::size_t result = 0;
    for (const auto& value : array) {
        result ^=
            computeHash(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}

inline auto computeHash(const std::any& value) -> std::size_t {
    if (value.has_value()) {
        return value.type().hash_code();
    }
    return 0;
}
}  // namespace atom::algorithm

/**
 * @brief Computes a hash value for a null-terminated string using FNV-1a
 * algorithm.
 *
 * @param str Pointer to the null-terminated string to hash.
 * @param basis Initial basis value for hashing.
 * @return constexpr unsigned int Hash value of the string.
 */
constexpr auto hash(const char* str,
                    unsigned int basis = 2166136261U) -> unsigned int {
    while (*str != 0) {
        basis = (basis ^ static_cast<unsigned int>(*str)) * 16777619u;
        ++str;
    }
    return basis;
}

/**
 * @brief User-defined literal for computing hash values of string literals.
 *
 * Example usage: "example"_hash
 *
 * @param str Pointer to the string literal to hash.
 * @param size Size of the string literal (unused).
 * @return constexpr unsigned int Hash value of the string literal.
 */
constexpr auto operator""_hash(const char* str,
                               std::size_t size) -> unsigned int {
    // The size parameter is not used in this implementation
    static_cast<void>(size);
    return hash(str);
}

#endif  // ATOM_ALGORITHM_HASH_HPP
