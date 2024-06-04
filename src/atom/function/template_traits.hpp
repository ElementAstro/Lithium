/*!
 * \file template_traits.hpp
 * \brief Template Traits
 * \author Max Qian <lightapt.com>
 * \date 2024-05-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TEMPLATE_TRAITS_HPP
#define ATOM_META_TEMPLATE_TRAITS_HPP

#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "abi.hpp"

namespace atom::meta {

// Identity template
template <typename T, auto... Value>
struct identity {
    using type = T;
    static constexpr bool has_value = false;
};

template <typename T, auto Value>
struct identity<T, Value> {
    using type = T;
    static constexpr auto value = Value;
    static constexpr bool has_value = true;
};

// Check if a type is a template instantiation
template <typename T>
struct is_template : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct is_template<Template<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_template_v = is_template<T>::value;

// Extract template parameters and full name
template <typename T>
struct template_traits;

template <template <typename...> typename Template, typename... Args>
struct template_traits<Template<Args...>> {
    using args_type = std::tuple<Args...>;
    static const std::string full_name;
};

template <template <typename...> typename Template, typename... Args>
const std::string template_traits<Template<Args...>>::full_name = [] {
    std::string name = typeid(Template<Args...>).name();
    return DemangleHelper::Demangle(name);
}();

// Helper alias templates
template <typename T>
using args_type_of = typename template_traits<T>::args_type;

template <typename T>
inline constexpr std::size_t template_arity_v =
    std::tuple_size_v<args_type_of<T>>;

// Check if a type is a specialization of a given template
template <template <typename...> typename Template, typename T>
struct is_specialization_of : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct is_specialization_of<Template, Template<Args...>> : std::true_type {};

template <template <typename...> typename Template, typename T>
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
template <typename T, template <typename, typename...> typename Template>
struct is_partial_specialization_of : std::false_type {};

template <template <typename, typename...> typename Template, typename Arg,
          typename... Args>
struct is_partial_specialization_of<Template<Arg, Args...>, Template>
    : std::true_type {};

template <typename T, template <typename, typename...> typename Template>
inline constexpr bool is_partial_specialization_of_v =
    is_partial_specialization_of<T, Template>::value;

// Check if a type is a class template
template <typename T>
struct is_class_template : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct is_class_template<Template<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_class_template_v = is_class_template<T>::value;

// Check if a type is a function template
template <typename T>
struct is_function_template : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct is_function_template<Template<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_function_template_v = is_function_template<T>::value;

// Check if a type is a variable template
template <typename T>
struct is_variable_template : std::false_type {};

template <template <auto...> typename Template, auto... Args>
struct is_variable_template<Template<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_variable_template_v = is_variable_template<T>::value;

// Check if a type is an alias template
template <typename T>
struct is_alias_template : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct is_alias_template<Template<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_alias_template_v = is_alias_template<T>::value;

// Extract the template arguments as a tuple of types
template <typename T>
struct template_args_as_tuple;

template <template <typename...> typename Template, typename... Args>
struct template_args_as_tuple<Template<Args...>> {
    using type = std::tuple<Args...>;
};

template <typename T>
using template_args_as_tuple_t = typename template_args_as_tuple<T>::type;

// Extract the template arguments as a tuple of values
template <typename T>
struct template_args_as_value_tuple;

template <template <auto...> typename Template, auto... Args>
struct template_args_as_value_tuple<Template<Args...>> {
    using type = std::tuple<std::integral_constant<decltype(Args), Args>...>;
};

template <typename T>
using template_args_as_value_tuple_t =
    typename template_args_as_value_tuple<T>::type;

// Count the number of occurrences of a type in a parameter pack
template <typename T, typename... Args>
struct count_occurrences;

template <typename T>
struct count_occurrences<T> {
    static constexpr std::size_t value = 0;
};

template <typename T, typename U, typename... Args>
struct count_occurrences<T, U, Args...> {
    static constexpr std::size_t value =
        std::is_same_v<T, U> + count_occurrences<T, Args...>::value;
};

template <typename T, typename... Args>
inline constexpr std::size_t count_occurrences_v =
    count_occurrences<T, Args...>::value;

// Find the index of the first occurrence of a type in a parameter pack
template <typename T, typename... Args>
struct find_first_index;

template <typename T, typename U, typename... Args>
struct find_first_index<T, U, Args...> {
    static constexpr std::size_t value =
        std::is_same_v<T, U> ? 0 : 1 + find_first_index<T, Args...>::value;
};

template <typename T>
struct find_first_index<T> {
    static constexpr std::size_t value =
        std::numeric_limits<std::size_t>::max();
};

template <typename T, typename... Args>
inline constexpr std::size_t find_first_index_v =
    find_first_index<T, Args...>::value;

// Find the index of the last occurrence of a type in a parameter pack
template <typename T, typename... Args>
struct find_last_index;

template <typename T, typename U, typename... Args>
struct find_last_index<T, U, Args...> {
    static constexpr std::size_t value =
        std::is_same_v<T, U> ? sizeof...(Args)
                             : find_last_index<T, Args...>::value;
};

template <typename T>
struct find_last_index<T> {
    static constexpr std::size_t value =
        std::numeric_limits<std::size_t>::max();
};

template <typename T, typename... Args>
inline constexpr std::size_t find_last_index_v =
    find_last_index<T, Args...>::value;

template <typename T>
struct extract_reference_wrapper_type {
    using type = T;
};

template <typename U>
struct extract_reference_wrapper_type<std::reference_wrapper<U>> {
    using type = U;
};

template <typename T>
using extract_reference_wrapper_type_t =
    typename extract_reference_wrapper_type<T>::type;

template <typename T>
struct extract_pointer_type {
    using type = T;
};

template <typename U>
struct extract_pointer_type<U*> {
    using type = U;
};

template <typename T>
using extract_pointer_type_t = typename extract_pointer_type<T>::type;

template <typename T>
struct extract_function_return_type;

template <typename R, typename... Args>
struct extract_function_return_type<R(Args...)> {
    using type = R;
};

template <typename T>
using extract_function_return_type_t =
    typename extract_function_return_type<T>::type;

template <typename T>
struct extract_function_parameters;

template <typename R, typename... Args>
struct extract_function_parameters<R(Args...)> {
    using type = std::tuple<Args...>;
};

template <typename T>
using extract_function_parameters_t =
    typename extract_function_parameters<T>::type;

template <typename T>
struct extract_array_element_type {
    using type = T;
};

template <typename U, std::size_t N>
struct extract_array_element_type<U[N]> {
    using type = U;
};

template <typename T>
using extract_array_element_type_t =
    typename extract_array_element_type<T>::type;

}  // namespace atom::meta

#endif
