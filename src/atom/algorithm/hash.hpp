/*
 * hash.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-28

Description: A collection of optimized and enhanced hash algorithms

**************************************************/

#ifndef ATOM_ALGORITHM_HASH_HPP
#define ATOM_ALGORITHM_HASH_HPP

#include <any>
#include <array>
#include <functional>
#include <optional>
#include <tuple>
#include <variant>
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
 * @brief Combines two hash values into one.
 *
 * This function implements the hash combining technique proposed by Boost.
 *
 * @param seed The initial hash value.
 * @param hash The hash value to combine with the seed.
 * @return std::size_t The combined hash value.
 */
inline auto hashCombine(std::size_t seed,
                        std::size_t hash) noexcept -> std::size_t {
    // Magic number from Boost library
    return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

/**
 * @brief Computes the hash value for a single Hashable value.
 *
 * @tparam T Type of the value to hash, must satisfy Hashable concept.
 * @param value The value to hash.
 * @return std::size_t Hash value of the input value.
 */
template <Hashable T>
inline auto computeHash(const T& value) noexcept -> std::size_t {
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
inline auto computeHash(const std::vector<T>& values) noexcept -> std::size_t {
    std::size_t result = 0;
    for (const auto& value : values) {
        result = hashCombine(result, computeHash(value));
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
inline auto computeHash(const std::tuple<Ts...>& tuple) noexcept
    -> std::size_t {
    std::size_t result = 0;
    std::apply(
        [&result](const Ts&... values) {
            ((result = hashCombine(result, computeHash(values))), ...);
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
inline auto computeHash(const std::array<T, N>& array) noexcept -> std::size_t {
    std::size_t result = 0;
    for (const auto& value : array) {
        result = hashCombine(result, computeHash(value));
    }
    return result;
}

/**
 * @brief Computes the hash value for a std::pair of Hashable values.
 *
 * @tparam T1 Type of the first element in the pair, must satisfy Hashable
 * concept.
 * @tparam T2 Type of the second element in the pair, must satisfy Hashable
 * concept.
 * @param pair The pair of values to hash.
 * @return std::size_t Hash value of the pair of values.
 */
template <Hashable T1, Hashable T2>
inline auto computeHash(const std::pair<T1, T2>& pair) noexcept -> std::size_t {
    std::size_t seed = computeHash(pair.first);
    seed = hashCombine(seed, computeHash(pair.second));
    return seed;
}

/**
 * @brief Computes the hash value for a std::optional of a Hashable value.
 *
 * @tparam T Type of the value inside the optional, must satisfy Hashable
 * concept.
 * @param opt The optional value to hash.
 * @return std::size_t Hash value of the optional value.
 */
template <Hashable T>
inline auto computeHash(const std::optional<T>& opt) noexcept -> std::size_t {
    if (opt.has_value()) {
        return computeHash(*opt) +
               1;  // Adding 1 to differentiate from std::nullopt
    }
    return 0;
}

/**
 * @brief Computes the hash value for a std::variant of Hashable types.
 *
 * @tparam Ts Types contained in the variant, all must satisfy Hashable concept.
 * @param var The variant of values to hash.
 * @return std::size_t Hash value of the variant value.
 */
template <Hashable... Ts>
inline auto computeHash(const std::variant<Ts...>& var) noexcept
    -> std::size_t {
    return std::visit(
        [](const auto& value) -> std::size_t { return computeHash(value); },
        var);
}

/**
 * @brief Computes the hash value for a std::any value.
 *
 * This function attempts to hash the contained value if it is Hashable.
 * If the contained type is not Hashable, it hashes the type information
 * instead.
 *
 * @param value The std::any value to hash.
 * @return std::size_t Hash value of the std::any value.
 */
inline auto computeHash(const std::any& value) noexcept -> std::size_t {
    if (value.has_value()) {
        const std::type_info& type = value.type();
        // Hashing the type information as a fallback
        return type.hash_code();
    }
    return 0;
}

/**
 * @brief Computes a hash value for a null-terminated string using FNV-1a
 * algorithm.
 *
 * @param str Pointer to the null-terminated string to hash.
 * @param basis Initial basis value for hashing.
 * @return constexpr std::size_t Hash value of the string.
 */
constexpr auto hash(const char* str,
                    std::size_t basis = 2166136261u) noexcept -> std::size_t {
    std::size_t hash = basis;
    while (*str != '\0') {
        hash ^= static_cast<std::size_t>(*str);
        hash *= 16777619u;
        ++str;
    }
    return hash;
}

/**
 * @brief User-defined literal for computing hash values of string literals.
 *
 * Example usage: "example"_hash
 *
 * @param str Pointer to the string literal to hash.
 * @param size Size of the string literal (unused).
 * @return constexpr std::size_t Hash value of the string literal.
 */
constexpr auto operator""_hash(const char* str,
                               std::size_t size) noexcept -> std::size_t {
    // The size parameter is not used in this implementation
    static_cast<void>(size);
    return hash(str);
}

}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_HASH_HPP