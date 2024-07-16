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
    if constexpr (tryInitializeWithN<T, N>() &&
                  !tryInitializeWithN<T, N + 1>()) {
        return N;
    } else {
        return totalCountOfFields<T, N + 1>();
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
    constexpr auto TOTAL = totalCountOfFields<T>();
    if constexpr (N == 0) {
        return true;
    } else if constexpr (Position + N <= TOTAL) {
        return tryInitializeWithN<T, Position, N, TOTAL - Position - N>();
    } else {
        return false;
    }
}

template <typename T, std::size_t Pos, std::size_t N = 0, std::size_t Max = 10>
consteval auto hasExtraElements() -> bool {
    constexpr auto TOTAL = totalCountOfFields<T>();
    if constexpr (tryInitializeWithThreeParts<T, Pos, N, TOTAL - Pos - 1>()) {
        return false;
    } else if constexpr (N + 1 <= Max) {
        return hasExtraElements<T, Pos, N + 1>();
    } else {
        return true;
    }
}

template <typename T, std::size_t Pos>
consteval auto searchMaxInPos() -> std::size_t {
    constexpr auto TOTAL = totalCountOfFields<T>();
    if constexpr (!hasExtraElements<T, Pos>()) {
        return 1;
    } else {
        std::size_t result = 0;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((tryPlaceNInPos<T, Pos, Is>() ? result = Is : 0), ...);
        }(std::make_index_sequence<TOTAL + 1>());
        return result;
    }
}

template <typename T, std::size_t N = 0>
consteval auto searchAllExtraIndex(auto&& array) {
    constexpr auto TOTAL = totalCountOfFields<T>();
    constexpr auto VALUE = std::max<std::size_t>(searchMaxInPos<T, N>(), 1);
    array[N] = VALUE;
    if constexpr (N + VALUE < TOTAL) {
        search_all_extra_index<T, N + VALUE>(array);
    }
}

template <typename T>
consteval auto trueCountOfFields() {
    constexpr auto MAX = totalCountOfFields<T>();
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
        return trueCountOfFields<T>();
#else
        return totalCountOfFields<T>();
#endif
    }
}
}  // namespace atom::meta

#endif  // ATOM_META_FIELD_COUNT_HPP
