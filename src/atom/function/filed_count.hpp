/*!
 * \file field_count.hpp
 * \brief Field Count
 * \author Max Qian <lightapt.com>
 * \date 2024-05-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_FIELD_COUNT_HPP
#define ATOM_META_FIELD_COUNT_HPP

#include <type_traits>
#include <utility>

namespace atom::meta {

struct Any {
    template <typename T>
    operator T() const;
};

template <typename T, std::size_t... I>
consteval auto isBracesConstructible(std::index_sequence<I...>) -> bool {
    return requires { T{((void)I, std::declval<Any>())...}; };
}

template <typename T, std::size_t N = 0>
consteval auto fieldCount() -> std::size_t {
    if constexpr (!isBracesConstructible<T>(
                      std::make_index_sequence<N + 1>{})) {
        return N;
    } else {
        return fieldCount<T, N + 1>();
    }
}

template <typename T>
struct TypeInfo;

/**
 *  @brief Retrieve the count of fields of a struct
 */
template <typename T>
consteval auto fieldCountOf() {
    if constexpr (std::is_aggregate_v<T>) {
        if constexpr (requires { TypeInfo<T>::count; }) {
            return TypeInfo<T>::count;
        } else {
            return fieldCount<T>();
        }
    } else {
        return 0;  // Non-aggregate types are considered to have 0 fields
    }
}

// Overload for arrays
template <typename T, std::size_t N>
consteval auto fieldCountOf() {
    return N;
}

}  // namespace atom::meta

#endif  // ATOM_META_FIELD_COUNT_HPP