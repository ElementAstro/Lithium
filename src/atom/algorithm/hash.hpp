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
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template <Hashable T>
std::size_t computeHash(const T& value) {
    return std::hash<T>{}(value);
}

template <Hashable T>
std::size_t computeHash(const std::vector<T>& values) {
    std::size_t result = 0;
    for (const auto& value : values) {
        result ^=
            computeHash(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}

template <Hashable... Ts>
std::size_t computeHash(const std::tuple<Ts...>& tuple) {
    std::size_t result = 0;
    apply(
        [&result](const Ts&... values) {
            ((result ^=
              computeHash(values) + 0x9e3779b9 + (result << 6) + (result >> 2)),
             ...);
        },
        tuple);
    return result;
}

template <Hashable T, std::size_t N>
std::size_t computeHash(const std::array<T, N>& array) {
    std::size_t result = 0;
    for (const auto& value : array) {
        result ^=
            computeHash(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}

template <typename Itr>
constexpr std::uint32_t fnv1a_hash(Itr begin, Itr end) noexcept {
    std::uint32_t h = 0x811c9dc5;

    while (begin != end) {
        h = (h ^ static_cast<std::uint8_t>(*begin)) * 0x01000193;
        ++begin;
    }
    return h;
}

template <size_t N>
constexpr std::uint32_t fnv1a_hash(const char (&str)[N]) noexcept {
    return fnv1a_hash(std::begin(str), std::end(str) - 1);
}

constexpr std::uint32_t fnv1a_hash(std::string_view sv) noexcept {
    return fnv1a_hash(sv.begin(), sv.end());
}

inline std::uint32_t fnv1a_hash(const std::string& s) noexcept {
    return fnv1a_hash(std::string_view{s});
}

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

template <size_t N>
constexpr std::uint32_t jenkins_one_at_a_time_hash(
    const char (&str)[N]) noexcept {
    return jenkins_one_at_a_time_hash(std::begin(str), std::end(str) - 1);
}

constexpr std::uint32_t jenkins_one_at_a_time_hash(
    std::string_view sv) noexcept {
    return jenkins_one_at_a_time_hash(sv.begin(), sv.end());
}

inline std::uint32_t jenkins_one_at_a_time_hash(const std::string& s) noexcept {
    return jenkins_one_at_a_time_hash(std::string_view{s});
}
}  // namespace Atom::Algorithm

#endif
