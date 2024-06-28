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
#include <vector>

namespace atom::algorithm {
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
auto computeHash(const T& value) -> std::size_t {
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
auto computeHash(const std::vector<T>& values) -> std::size_t {
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
auto computeHash(const std::array<T, N>& array) -> std::size_t {
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
constexpr auto fnv1aHash(Itr begin, Itr end) noexcept -> std::uint32_t {
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
constexpr auto fnv1aHash(const char (&str)[N]) noexcept -> std::uint32_t {
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
constexpr auto fnv1aHash(std::string_view sv) noexcept -> std::uint32_t {
    return fnv1aHash(sv.begin(), sv.end());
}

/**
 * @brief Computes the FNV-1a hash value of a string.
 *
 * This function computes the FNV-1a hash value of a string.
 *
 * @param s The string.
 * @return The FNV-1a hash value of the string.
 */
inline auto fnv1aHash(const std::string& s) noexcept -> std::uint32_t {
    return fnv1aHash(std::string_view{s});
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
constexpr auto jenkinsOneAtATimeHash(Itr begin,
                                     Itr end) noexcept -> std::uint32_t {
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
constexpr auto jenkinsOneAtATimeHash(const char (&str)[N]) noexcept
    -> std::uint32_t {
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
constexpr auto jenkinsOneAtATimeHash(std::string_view sv) noexcept
    -> std::uint32_t {
    return jenkinsOneAtATimeHash(sv.begin(), sv.end());
}

/**
 * @brief Computes the Jenkins One-at-a-Time hash value of a string.
 *
 * This function computes the Jenkins One-at-a-Time hash value of a string.
 *
 * @param s The string.
 * @return The Jenkins One-at-a-Time hash value of the string.
 */
inline auto jenkinsOneAtATimeHash(const std::string& s) noexcept
    -> std::uint32_t {
    return jenkinsOneAtATimeHash(std::string_view{s});
}

inline auto quickHash(std::string_view str) -> uint32_t {
    uint32_t h = 0;
    for (char c : str) {
        h = 31 * h + static_cast<unsigned char>(c);
    }
    return h;
}

inline auto quickHash(const void* data, size_t size) -> uint32_t {
    if (data == nullptr || size == 0) {
        return 0;
    }

    const auto* str = static_cast<const unsigned char*>(data);
    uint32_t h = 0;
    for (size_t i = 0; i < size; ++i) {
        h = 31 * h + str[i];
    }
    return h;
}

}  // namespace atom::algorithm

/**
 * @brief Computes the hash value of a string during compile time.
 *
 * This function computes the hash value of a string using the FNV-1a algorithm.
 *
 * @param str The string.
 * @return The hash value of the string.
 */
constexpr auto hash(const char* str,
                    unsigned int basis = 2166136261U) -> unsigned int {
    return (*str != 0)
               ? hash(str + 1,
                      (basis ^ static_cast<unsigned int>(*str)) * 16777619u)
               : basis;
}

/**
 * @brief Computes the hash value of a string during compile time.
 *
 * This function computes the hash value of a string using the FNV-1a algorithm.
 *
 * @param str The string.
 * @return The hash value of the string.
 */
constexpr auto operator""_hash(const char* str, std::size_t) -> unsigned int {
    return hash(str);
}

#endif
