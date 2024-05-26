/*!
 * \file template_traits.hpp
 * \brief Template Traits
 * \author Max Qian <lightapt.com>
 * \date 2024-05-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TEMPLATE_TRAITS
#define ATOM_META_TEMPLATE_TRAITS

#include <tuple>
#include <type_traits>

namespace atom::meta {
// Identity template
template <typename T, T... Value>
struct identity {
    using type = T;
    constexpr static bool has_value = false;
};

template <typename T, T Value>
struct identity<T, Value> {
    using type = T;
    constexpr static T value = Value;
    constexpr static bool has_value = true;
};

// Check if a type is a template instantiation
template <typename T, typename = void>
struct is_template : std::false_type {};

template <template <typename...> class Template, typename... Args>
struct is_template<Template<Args...>, std::void_t<Template<Args...>>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_template_v = is_template<T>::value;

// Extract template parameters
template <typename T, typename = void>
struct template_traits;

template <template <typename...> class Template, typename... Args>
struct template_traits<Template<Args...>, std::void_t<Template<Args...>>> {
    using args_type = std::tuple<Args...>;
};

template <typename T>
using args_type_of = typename template_traits<T>::args_type;

template <typename T>
inline constexpr std::size_t template_arity_v =
    std::tuple_size_v<args_type_of<T>>;

// Check if a type is a specialization of a given template
template <template <typename...> class Template, typename T>
struct is_specialization_of : std::false_type {};

template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template, Template<Args...>> : std::true_type {};

template <template <typename...> class Template, typename T>
inline constexpr bool is_specialization_of_v =
    is_specialization_of<Template, T>::value;

// Extract the N-th template parameter type
template <std::size_t N, typename T>
using template_arg_t = std::tuple_element_t<N, args_type_of<T>>;

// Check if a type is derived from multiple base classes
template <typename Derived, typename... Bases>
struct is_derived_from_all
    : std::conjunction<std::is_base_of<Bases, Derived>...> {};

template <typename Derived, typename... Bases>
inline constexpr bool is_derived_from_all_v =
    is_derived_from_all<Derived, Bases...>::value;

// Check if a type is a partial specialization of a given template
template <typename T, template <typename, typename...> class Template>
struct is_partial_specialization_of : std::false_type {};

template <template <typename, typename...> class Template, typename Arg,
          typename... Args>
struct is_partial_specialization_of<Template<Arg, Args...>, Template>
    : std::true_type {};

template <typename T, template <typename, typename...> class Template>
inline constexpr bool is_partial_specialization_of_v =
    is_partial_specialization_of<T, Template>::value;
}  // namespace atom::meta

#endif  // ATOM_META_TEMPLATE_TRAITS
