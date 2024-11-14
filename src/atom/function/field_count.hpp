#ifndef ATOMMETA_FIELD_COUNT_HPP
#define ATOMMETA_FIELD_COUNT_HPP

#include <array>
namespace atom::meta::details {
struct Any;

struct Any {
    constexpr Any(int) {}

    template <typename T>
        requires std::is_copy_constructible_v<T>
    operator T&();

    template <typename T>
        requires std::is_move_constructible_v<T>
    operator T&&();

    struct Empty {};

    template <typename T>
        requires(!std::is_copy_constructible_v<T> &&
                 !std::is_move_constructible_v<T> &&
                 !std::is_constructible_v<T, Empty>)
    operator T();
};

template <typename T, std::size_t N>
consteval auto tryInitializeWithN() -> std::size_t {
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
}  // namespace atom::meta::details

namespace atom::meta::details {

template <typename T, std::size_t N1, std::size_t N2, std::size_t N3>
consteval auto tryInitializeWithThreeParts() -> std::size_t {
    return []<std::size_t... I1, std::size_t... I2, std::size_t... I3>(
               std::index_sequence<I1...>, std::index_sequence<I2...>,
               std::index_sequence<I3...>) {
        return requires { T{Any(I1)..., {Any(I2)...}, Any(I3)...}; };
    }(std::make_index_sequence<N1>{}, std::make_index_sequence<N2>{},
           std::make_index_sequence<N3>{});
}

template <typename T, std::size_t position, std::size_t N>
constexpr auto tryPlaceNInPos() -> bool {
    constexpr auto Total = totalCountOfFields<T>();
    if constexpr (N == 0) {
        return true;
    } else if constexpr (position + N <= Total) {
        return tryInitializeWithThreeParts<T, position, N,
                                           Total - position - N>();
    } else {
        return false;
    }
}

template <typename T, std::size_t pos, std::size_t N = 0, std::size_t Max = 10>
constexpr auto hasExtraElements() -> bool {
    constexpr auto Total = totalCountOfFields<T>();
    if constexpr (tryInitializeWithThreeParts<T, pos, N, Total - pos - 1>()) {
        return false;
    } else if constexpr (N + 1 <= Max) {
        return hasExtraElements<T, pos, N + 1>();
    } else {
        return true;
    }
}

template <typename T, std::size_t pos>
constexpr auto searchMaxInPos() -> std::size_t {
    constexpr auto Total = totalCountOfFields<T>();
    if constexpr (!hasExtraElements<T, pos>()) {
        return 1;
    } else {
        std::size_t result = 0;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((tryPlaceNInPos<T, pos, Is>() ? result = Is : 0), ...);
        }(std::make_index_sequence<Total + 1>());
        return result;
    }
}

template <typename T, std::size_t N = 0>
constexpr auto searchAllExtraIndex(auto&& array) {
    constexpr auto total = totalCountOfFields<T>();
    constexpr auto value = std::max<std::size_t>(searchMaxInPos<T, N>(), 1);
    array[N] = value;
    if constexpr (N + value < total) {
        searchAllExtraIndex<T, N + value>(array);
    }
}

template <typename T>
constexpr auto trueCountOfFields() {
    constexpr auto max = totalCountOfFields<T>();
    if constexpr (max == 0) {
        return 0;
    } else {
        std::array<std::size_t, max> indices = {1};
        searchAllExtraIndex<T>(indices);
        std::size_t result = max;
        std::size_t index = 0;
        while (index < max) {
            auto n = indices[index];
            result -= n - 1;
            index += n;
        }
        return result;
    }
}
}  // namespace atom::meta::details

namespace atom::meta {
template <typename T>
struct type_info;

/**
 *  @brief Retrieve the count of fields of a struct
 *  @warning cannot get the count of fields of a struct which has reference
 * type member in gcc 13 because the internal error occurs in below occasion
 *  @code
 *  struct Number { operator int&(); };
 *  int& x = { Number{} };
 *
 *  internal compiler error: in reference_binding, at cp/call.cc:2020
 *  @endcode
 *
 */
template <typename T>
    requires std::is_aggregate_v<T>
consteval auto fieldCountOf() {
    if constexpr (requires { type_info<T>::count; }) {
        return type_info<T>::count;
    } else {
        return details::trueCountOfFields<T>();
    }
}

template <typename T>
    requires(!std::is_aggregate_v<T>)
consteval auto fieldCountOf() {
    return 0;
}
}  // namespace atom::meta

#endif  // ATOMMETA_FIELD_COUNT_HPP