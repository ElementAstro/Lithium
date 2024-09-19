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

/*!
 * \brief A struct that can be converted to any type.
 */
struct Any {
    /*!
     * \brief Constexpr conversion operator to any type.
     * \tparam T The type to convert to.
     * \return An instance of type T.
     */
    template <typename T>
    explicit consteval operator T() const noexcept;
};

/*!
 * \brief Checks if a type T is constructible with braces.
 * \tparam T The type to check.
 * \tparam I The index sequence.
 * \param[in] indices The index sequence.
 * \return True if T is constructible with braces, false otherwise.
 */
template <typename T, std::size_t... I>
consteval auto isBracesConstructible(
    std::index_sequence<I...> /*indices*/) noexcept -> bool {
    return requires { T{((void)I, std::declval<Any>())...}; };
}

/*!
 * \brief Recursively counts the number of fields in a type T.
 * \tparam T The type to count fields in.
 * \tparam N The current count of fields.
 * \return The number of fields in type T.
 */
template <typename T, std::size_t N = 0>
consteval auto fieldCount() noexcept -> std::size_t {
    if constexpr (!isBracesConstructible<T>(
                      std::make_index_sequence<N + 1>{})) {
        return N;
    } else {
        return fieldCount<T, N + 1>();
    }
}

/*!
 * \brief A template struct to hold type information.
 * \tparam T The type to hold information for.
 */
template <typename T>
struct TypeInfo;

/*!
 * \brief Gets the number of fields in a type T.
 * \tparam T The type to get the field count for.
 * \return The number of fields in type T.
 */
template <typename T>
consteval auto fieldCountOf() noexcept -> std::size_t {
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

/*!
 * \brief Gets the number of elements in an array.
 * \tparam T The type of the array elements.
 * \tparam N The number of elements in the array.
 * \return The number of elements in the array.
 */
template <typename T, std::size_t N>
consteval auto fieldCountOf() noexcept -> std::size_t {
    return N;
}

}  // namespace atom::meta

#endif  // ATOM_META_FIELD_COUNT_HPP
