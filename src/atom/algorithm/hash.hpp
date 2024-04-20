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

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Atom::Algorithm {
/**
 * @brief Concept to check if a type is hashable.
 *
 * A type is considered hashable if it can be used as a key in hash-based
 * containers.
 */
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

/**
 * @brief Computes the hash value of a single value.
 *
 * This function computes the hash value of a single value using std::hash.
 *
 * @param value The value for which to compute the hash.
 * @return The hash value of the input value.
 */
template <Hashable T>
std::size_t computeHash(const T& value) {
    return std::hash<T>{}(value);
}

/**
 * @brief Computes the hash value of a vector of hashable values.
 *
 * This function computes the hash value of a vector of hashable values by
 * combining the hash values of individual elements using a bitwise XOR
 * operation.
 *
 * @param values The vector of hashable values.
 * @return The hash value of the vector.
 */
template <Hashable T>
std::size_t computeHash(const std::vector<T>& values) {
    std::size_t result = 0;
    for (const auto& value : values) {
        result ^=
            computeHash(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}

/**
 * @brief Computes the hash value of a tuple of hashable values.
 *
 * This function computes the hash value of a tuple of hashable values by
 * applying the computeHash function to each element of the tuple and combining
 * the hash values using a bitwise XOR operation.
 *
 * @param tuple The tuple of hashable values.
 * @return The hash value of the tuple.
 */
template <Hashable... Ts>
std::size_t computeHash(const std::tuple<Ts...>& tuple) {
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
 * @brief Computes the hash value of an array of hashable values.
 *
 * This function computes the hash value of an array of hashable values by
 * combining the hash values of individual elements using a bitwise XOR
 * operation.
 *
 * @param array The array of hashable values.
 * @return The hash value of the array.
 */
template <Hashable T, std::size_t N>
std::size_t computeHash(const std::array<T, N>& array) {
    std::size_t result = 0;
    for (const auto& value : array) {
        result ^=
            computeHash(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}

/**
 * @brief Computes the FNV-1a hash value of a range.
 *
 * This function computes the FNV-1a hash value of a range defined by iterators.
 *
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to the end of the range.
 * @return The FNV-1a hash value of the range.
 */
template <typename Itr>
constexpr std::uint32_t fnv1a_hash(Itr begin, Itr end) noexcept {
    std::uint32_t h = 0x811c9dc5;

    while (begin != end) {
        h = (h ^ static_cast<std::uint8_t>(*begin)) * 0x01000193;
        ++begin;
    }
    return h;
}

/**
 * @brief Computes the FNV-1a hash value of a null-terminated string literal.
 *
 * This function computes the FNV-1a hash value of a null-terminated string
 * literal.
 *
 * @param str The null-terminated string literal.
 * @return The FNV-1a hash value of the string.
 */
template <size_t N>
constexpr std::uint32_t fnv1a_hash(const char (&str)[N]) noexcept {
    return fnv1a_hash(std::begin(str), std::end(str) - 1);
}

/**
 * @brief Computes the FNV-1a hash value of a string view.
 *
 * This function computes the FNV-1a hash value of a string view.
 *
 * @param sv The string view.
 * @return The FNV-1a hash value of the string view.
 */
constexpr std::uint32_t fnv1a_hash(std::string_view sv) noexcept {
    return fnv1a_hash(sv.begin(), sv.end());
}

/**
 * @brief Computes the FNV-1a hash value of a string.
 *
 * This function computes the FNV-1a hash value of a string.
 *
 * @param s The string.
 * @return The FNV-1a hash value of the string.
 */
inline std::uint32_t fnv1a_hash(const std::string& s) noexcept {
    return fnv1a_hash(std::string_view{s});
}

/**
 * @brief Computes the Jenkins One-at-a-Time hash value of a range.
 *
 * This function computes the Jenkins One-at-a-Time hash value of a range
 * defined by iterators.
 *
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to the end of the range.
 * @return The Jenkins One-at-a-Time hash value of the range.
 */
template <typename Itr>
constexpr std::uint32_t jenkins_one_at_a_time_hash(Itr begin,
                                                   Itr end) noexcept {
    std::uint32_t hash = 0;

    while (begin != end) {
        hash += static_cast<std::uint32_t>(*begin);
        hash += hash << 10;
        hash ^= hash >> 6;
        ++begin;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

/**
 * @brief Computes the Jenkins One-at-a-Time hash value of a null-terminated
 * string literal.
 *
 * This function computes the Jenkins One-at-a-Time hash value of a
 * null-terminated string literal.
 *
 * @param str The null-terminated string literal.
 * @return The Jenkins One-at-a-Time hash value of the string.
 */
template <size_t N>
constexpr std::uint32_t jenkins_one_at_a_time_hash(
    const char (&str)[N]) noexcept {
    return jenkins_one_at_a_time_hash(std::begin(str), std::end(str) - 1);
}

/**
 * @brief Computes the Jenkins One-at-a-Time hash value of a string view.
 *
 * This function computes the Jenkins One-at-a-Time hash value of a string view.
 *
 * @param sv The string view.
 * @return The Jenkins One-at-a-Time hash value of the string view.
 */
constexpr std::uint32_t jenkins_one_at_a_time_hash(
    std::string_view sv) noexcept {
    return jenkins_one_at_a_time_hash(sv.begin(), sv.end());
}

/**
 * @brief Computes the Jenkins One-at-a-Time hash value of a string.
 *
 * This function computes the Jenkins One-at-a-Time hash value of a string.
 *
 * @param s The string.
 * @return The Jenkins One-at-a-Time hash value of the string.
 */
inline std::uint32_t jenkins_one_at_a_time_hash(const std::string& s) noexcept {
    return jenkins_one_at_a_time_hash(std::string_view{s});
}

inline uint32_t quickHash(const char* str) {
    if (!str)
        return 0;

    unsigned int h = 0;
    for (; *str; str++) {
        h = 31 * h + *str;
    }
    return h;
}

inline uint32_t quickHash(const void* tmp, uint32_t size) {
    if (!tmp)
        return 0;

    const char* str = static_cast<const char*>(tmp);
    unsigned int h = 0;
    for (uint32_t i = 0; i < size; ++i, ++str) {
        h = 31 * h + *str;
    }
    return h;
}

}  // namespace Atom::Algorithm

#endif
