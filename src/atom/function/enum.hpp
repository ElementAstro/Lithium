/*!
 * \file enum.hpp
 * \brief Enum Utilities
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ENUM_HPP
#define ATOM_META_ENUM_HPP

#include "raw_name.hpp"

#ifdef ATOM_META_CPP_20_SUPPORT
#include <array>

#define ENUM_SEARCH_RANGE 32
namespace atom::meta {
template <typename T>
struct type_info;

namespace details {
template <typename T, std::ptrdiff_t N = 0>
consteval std::ptrdiff_t search_possible_enum_start() {
    if constexpr (!raw_name_of_enum<static_cast<T>(N)>().empty()) {
        return N;
    } else if constexpr (std::is_signed_v<std::underlying_type_t<T>>) {
        if constexpr (!raw_name_of_enum<static_cast<T>(-N)>().empty()) {
            return -N;
        } else {
            return search_possible_enum_start<T, N + 1>();
        }
    } else {
        return search_possible_enum_start<T, N + 1>();
    }
}

template <typename T, auto N = search_possible_enum_start<T>()>
consteval std::ptrdiff_t search_possible_continuous_enum_max() {
    constexpr auto is_end = []<std::size_t... Is>(std::index_sequence<Is...>) {
        return (raw_name_of_enum<static_cast<T>(N + Is)>().empty() && ...);
    }(std::make_index_sequence<8>{});

    if constexpr (is_end) {
        return N - 1;
    } else {
        return search_possible_continuous_enum_max<T, N + 1>();
    }
}
}  // namespace details

template <typename T>
    requires std::is_enum_v<T>
consteval std::ptrdiff_t enum_start() {
    if constexpr (requires { type_info<T>::start; }) {
        return type_info<T>::start;
    } else {
        return details::search_possible_enum_start<T>();
    }
}

template <typename T>
    requires std::is_enum_v<T>
consteval std::ptrdiff_t enum_max() {
    if constexpr (requires { type_info<T>::max; }) {
        return type_info<T>::max;
    } else {
        return details::search_possible_continuous_enum_max<T>();
    }
}

template <typename T>
consteval bool is_bit_field_enum() {
    if constexpr (requires { type_info<T>::bit; }) {
        return type_info<T>::bit;
    } else {
        return false;
    }
}

namespace details {
template <typename T>
constexpr auto search_possible_bit_field_enum_length() {
    constexpr auto max = sizeof(T) * 8;
    std::size_t result = !raw_name_of_enum<static_cast<T>(0)>().empty();
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ((raw_name_of_enum<static_cast<T>(1 << Is)>().empty() ? (void)0
                                                              : ++result),
         ...);
    }(std::make_index_sequence<max>{});
    return result;
}

template <typename T, std::size_t length>
constexpr void split_initialize(auto&& f) {
    constexpr auto group_num = length / 64;
    constexpr auto rest = length % 64;

    []<std::size_t... Is>(std::index_sequence<Is...>, auto&& f) {
        (
            []<std::size_t I, std::size_t... Js>(std::index_sequence<Js...>,
                                                 auto&& f) {
                (f.template operator()<I * 64 + Js>(), ...);
            }.template operator()<Is>(std::make_index_sequence<64>{}, f),
            ...);
    }(std::make_index_sequence<group_num>{}, f);

    []<std::size_t... Is>(std::index_sequence<Is...>, auto&& f) {
        (f.template operator()<group_num * 64 + Is>(), ...);
    }(std::make_index_sequence<rest>{}, f);
}

template <typename T>
constexpr auto enum_names_of_impl() {
    if constexpr (!magic::is_bit_field_enum<T>()) {
        constexpr auto start = magic::enum_start<T>();
        constexpr auto max = magic::enum_max<T>();
        constexpr auto length = max - start + 1;
        std::array<std::string_view, length> names{};
        split_initialize<T, length>([&]<std::size_t I>() {
            names[I] = raw_name_of_enum<static_cast<T>(start + I)>();
        });
        return names;
    } else {
        constexpr auto length = search_possible_bit_field_enum_length<T>();
        std::array<std::string_view, length> names{};
        constexpr auto has_zero =
            !raw_name_of_enum<static_cast<T>(0)>().empty();
        if constexpr (has_zero) {
            names[0] = raw_name_of_enum<static_cast<T>(0)>();
            split_initialize<T, length - 1>([&]<std::size_t I>() {
                names[I + 1] = raw_name_of_enum<static_cast<T>(1 << I)>();
            });
        } else {
            split_initialize<T, length>([&]<std::size_t I>() {
                names[I] = raw_name_of_enum<static_cast<T>(1 << I)>();
            });
        }
        return names;
    }
}

template <typename T>
struct enum_names_storage {
    constexpr static auto storage = enum_names_of_impl<T>();
    constexpr static auto& names = storage;
};

template <typename T, std::ptrdiff_t N>
    requires std::is_enum_v<T>
struct Field {
    constexpr static std::string_view name() {
        return raw_name_of_enum<static_cast<T>(N)>();
    }

    using type = T;

    constexpr static std::ptrdiff_t value() { return N; }

    constexpr static std::string_view type_name() { return raw_name_of<T>(); }
};

template <typename T>
    requires std::is_enum_v<T>
constexpr void foreach (auto&& f) {
    if constexpr (!magic::is_bit_field_enum<T>()) {
        constexpr auto start = search_possible_enum_start<T>();
        constexpr auto max = search_possible_continuous_enum_max<T>();
        constexpr auto length = max - start + 1;

        split_initialize<T, length>(
            [&]<std::size_t I>() { f(Field<T, start + I>{}); });
    } else {
        constexpr auto length = search_possible_bit_field_enum_length<T>();
        constexpr auto has_zero =
            !raw_name_of_enum<static_cast<T>(0)>().empty();
        if constexpr (has_zero) {
            f(Field<T, 0>{});
            split_initialize<T, length - 1>(
                [&]<std::size_t I>() { f(Field<T, 1 << I>{}); });
        } else {
            split_initialize<T, length>(
                [&]<std::size_t I>() { f(Field<T, 1 << I>{}); });
        }
    }
}

}  // namespace details

using details::foreach;

template <typename T>
    requires std::is_enum_v<T>
constexpr auto& enum_names_of() {
    return details::enum_names_storage<T>::names;
}

template <typename T>
    requires std::is_enum_v<T>
constexpr auto& raw_name_of_enum(T value) {
    constexpr auto start = enum_start<T>();
    return details::enum_names_storage<T>::names
        [static_cast<std::size_t>(value) - static_cast<std::size_t>(start)];
}

}  // namespace atom::meta
#endif
#endif  // ATOM_META_ENUM_HPP
