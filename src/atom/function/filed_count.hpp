/*!
 * \file field_count.hpp
 * \brief Field Count
 * \author Max Qian <lightapt.com>
 * \date 2024-05-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_FIELD_COUNT_HPP
#define ATOM_META_FIELD_COUNT_HPP

#define ATOM_META_C_ARRAY_SUPPORT 1

#include <array>
#include <type_traits>
#include <utility>

namespace atom::meta {
struct Any {
    constexpr Any(int) {}

    template <typename T>
        requires std::is_copy_constructible_v<T>
    explicit operator T&();

    template <typename T>
        requires std::is_move_constructible_v<T>
    explicit operator T&&();

    struct Empty {};

    template <typename T>
        requires(!std::is_copy_constructible_v<T> &&
                 !std::is_move_constructible_v<T> &&
                 !std::is_constructible_v<T, Empty>)
    explicit operator T();
};

template <typename T, std::size_t N>
consteval auto tryInitializeWithN() -> bool {
    return []<std::size_t... Is>(std::index_sequence<Is...>) {
        return requires { T{Any(Is)...}; };
    }(std::make_index_sequence<N>{});
}

template <typename T, std::size_t N = 0>
consteval auto totalCountOfFields() -> std::size_t {
    if constexpr (try_initialize_with_n<T, N>() &&
                  !try_initialize_with_n<T, N + 1>()) {
        return N;
    } else {
        return total_count_of_fields<T, N + 1>();
    }
}

#if ATOM_META_C_ARRAY_SUPPORT
template <typename T, std::size_t N1, std::size_t N2, std::size_t N3>
consteval auto tryInitializeWithThreeParts() -> bool {
    return []<std::size_t... I1, std::size_t... I2, std::size_t... I3>(
               std::index_sequence<I1...>, std::index_sequence<I2...>,
               std::index_sequence<I3...>) {
        return requires { T{Any(I1)..., {Any(I2)...}, Any(I3)...}; };
    }(std::make_index_sequence<N1>{}, std::make_index_sequence<N2>{},
           std::make_index_sequence<N3>{});
}

template <typename T, std::size_t Position, std::size_t N>
consteval auto tryPlaceNInPos() -> bool {
    constexpr auto TOTAL = total_count_of_fields<T>();
    if constexpr (N == 0) {
        return true;
    } else if constexpr (Position + N <= TOTAL) {
        return try_initialize_with_three_parts<T, Position, N,
                                               TOTAL - Position - N>();
    } else {
        return false;
    }
}

template <typename T, std::size_t Pos, std::size_t N = 0, std::size_t Max = 10>
consteval auto hasExtraElements() -> bool {
    constexpr auto TOTAL = total_count_of_fields<T>();
    if constexpr (try_initialize_with_three_parts<T, Pos, N,
                                                  TOTAL - Pos - 1>()) {
        return false;
    } else if constexpr (N + 1 <= Max) {
        return has_extra_elements<T, Pos, N + 1>();
    } else {
        return true;
    }
}

template <typename T, std::size_t Pos>
consteval auto searchMaxInPos() -> std::size_t {
    constexpr auto TOTAL = total_count_of_fields<T>();
    if constexpr (!has_extra_elements<T, Pos>()) {
        return 1;
    } else {
        std::size_t result = 0;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((try_place_n_in_pos<T, Pos, Is>() ? result = Is : 0), ...);
        }(std::make_index_sequence<TOTAL + 1>());
        return result;
    }
}

template <typename T, std::size_t N = 0>
consteval auto searchAllExtraIndex(auto&& array) {
    constexpr auto TOTAL = total_count_of_fields<T>();
    constexpr auto VALUE = std::max<std::size_t>(search_max_in_pos<T, N>(), 1);
    array[N] = VALUE;
    if constexpr (N + VALUE < TOTAL) {
        search_all_extra_index<T, N + VALUE>(array);
    }
}

template <typename T>
consteval auto trueCountOfFields() {
    constexpr auto MAX = total_count_of_fields<T>();
    if constexpr (MAX == 0) {
        return 0;
    } else {
        std::array<std::size_t, MAX> indices = {1};
        search_all_extra_index<T>(indices);
        std::size_t result = MAX;
        std::size_t index = 0;
        while (index < MAX) {
            auto n = indices[index];
            result -= n - 1;
            index += n;
        }
        return result;
    }
}
#endif

template <typename T>
struct TypeInfo;

/**
 *  @brief Retrieve the count of fields of a struct
 */
template <typename T>
    requires std::is_aggregate_v<T>
consteval auto fieldCountOf() {
    if constexpr (requires { TypeInfo<T>::count; }) {
        return TypeInfo<T>::count;
    } else {
#if ATOM_META_C_ARRAY_SUPPORT
        return true_count_of_fields<T>();
#else
        return total_count_of_fields<T>();
#endif
    }
}
}  // namespace atom::meta

#endif  // ATOM_META_FIELD_COUNT_HPP
