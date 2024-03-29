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
#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>


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

#endif
