/*!
 * \file template_traits.hpp
 * \brief Template Traits (Optimized with C++20)
 * \author Max Qian <lightapt.com>
 * \date 2024-05-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TEMPLATE_TRAITS_HPP
#define ATOM_META_TEMPLATE_TRAITS_HPP

#include <concepts>
#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "abi.hpp"

namespace atom::meta {

// Identity template
template <typename T, auto... Value>
struct identity {
    using type = T;
    static constexpr bool has_value = sizeof...(Value) > 0;
    static constexpr auto value =
        (has_value ? std::get<0>(std::tuple{Value...}) : 0);
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
    static const inline std::string full_name =
        DemangleHelper::demangle(typeid(Template<Args...>).name());
};

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

template <typename T>
struct is_alias_template : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct is_alias_template<Template<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_alias_template_v = is_alias_template<T>::value;

// Check if a type is a class or function template
template <typename T>
concept is_class_template =
    requires { typename template_traits<T>::args_type; };

template <typename T>
concept is_function_template =
    requires(T) { typename template_traits<T>::args_type; };

// Extract the template arguments as a tuple of types
template <typename T>
using template_args_as_tuple_t = typename template_traits<T>::args_type;

// Count the number of occurrences of a type in a parameter pack
template <typename T, typename... Args>
constexpr std::size_t count_occurrences_v = (0 + ... + std::is_same_v<T, Args>);

// Find the index of the first occurrence of a type in a parameter pack
template <typename T, typename... Args>
constexpr std::size_t find_first_index_v = []() {
    constexpr auto idx = []<std::size_t... I>(std::index_sequence<I...>) {
        return ((std::is_same_v<T, Args>
                     ? I
                     : std::numeric_limits<std::size_t>::max()) +
                ...);
    }(std::index_sequence_for<Args...>{});
    return idx < sizeof...(Args) ? idx
                                 : std::numeric_limits<std::size_t>::max();
}();

// Extract reference wrapper or pointer types
template <typename T>
using extract_reference_wrapper_type_t = std::remove_reference_t<T>;

template <typename T>
using extract_pointer_type_t = std::remove_pointer_t<T>;

// Extract function return type and parameter types
template <typename T>
struct extract_function_traits;

template <typename R, typename... Args>
struct extract_function_traits<R(Args...)> {
    using return_type = R;
    using parameter_types = std::tuple<Args...>;
};

template <typename T>
using extract_function_return_type_t =
    typename extract_function_traits<T>::return_type;

template <typename T>
using extract_function_parameters_t =
    typename extract_function_traits<T>::parameter_types;

template <class T, std::size_t I>
concept has_tuple_element = requires { typename std::tuple_element_t<I, T>; };

template <class Expr>
consteval bool is_consteval(Expr) {
    return requires { typename std::bool_constant<(Expr{}(), false)>; };
}

template <class T>
consteval bool is_tuple_like_well_formed() {
    if constexpr (requires {
                      {
                          std::tuple_size<T>::value
                      } -> std::same_as<const std::size_t&>;
                  }) {
        if constexpr (is_consteval([] { return std::tuple_size<T>::value; })) {
            return []<std::size_t... I>(std::index_sequence<I...>) {
                return (has_tuple_element<T, I> && ...);
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    }
    return false;
}

// Check copyability and relocatability constraints using C++20 features
enum class constraint_level { none, nontrivial, nothrow, trivial };

template <typename T>
consteval bool has_copyability(constraint_level level) {
    switch (level) {
        case constraint_level::none:
            return true;
        case constraint_level::nontrivial:
            return std::is_copy_constructible_v<T>;
        case constraint_level::nothrow:
            return std::is_nothrow_copy_constructible_v<T>;
        case constraint_level::trivial:
            return std::is_trivially_copy_constructible_v<T> &&
                   std::is_trivially_destructible_v<T>;
        default:
            return false;
    }
}

template <typename T>
consteval bool has_relocatability(constraint_level level) {
    switch (level) {
        case constraint_level::none:
            return true;
        case constraint_level::nontrivial:
            return std::is_move_constructible_v<T> && std::is_destructible_v<T>;
        case constraint_level::nothrow:
            return std::is_nothrow_move_constructible_v<T> &&
                   std::is_nothrow_destructible_v<T>;
        case constraint_level::trivial:
            return std::is_trivially_move_constructible_v<T> &&
                   std::is_trivially_destructible_v<T>;
        default:
            return false;
    }
}

template <typename T>
consteval bool has_destructibility(constraint_level level) {
    switch (level) {
        case constraint_level::none:
            return true;
        case constraint_level::nontrivial:
            return std::is_destructible_v<T>;
        case constraint_level::nothrow:
            return std::is_nothrow_destructible_v<T>;
        case constraint_level::trivial:
            return std::is_trivially_destructible_v<T>;
        default:
            return false;
    }
}

template <template <typename...> class Base, typename Derived>
struct is_base_of_template_impl {
private:
    // 使用SFINAE机制，判断Derived是否可以转换为Base<U...>*
    template <typename... U>
    static constexpr std::true_type test(const Base<U...>*);
    static constexpr std::false_type test(...);

public:
    // value为true_type或false_type，取决于test的返回类型
    static constexpr bool value =
        decltype(test(std::declval<Derived*>()))::value;
};

// 支持多重继承检查
template <typename Derived, template <typename...> class... Bases>
struct is_base_of_any_template {
    static constexpr bool value =
        (is_base_of_template_impl<Bases, Derived>::value || ...);
};

// 变量模板简化接口
template <template <typename...> class Base, typename Derived>
inline constexpr bool is_base_of_template_v =
    is_base_of_template_impl<Base, Derived>::value;

template <typename Derived, template <typename...> class... Bases>
inline constexpr bool is_base_of_any_template_v =
    is_base_of_any_template<Derived, Bases...>::value;

// 使用概念定义
template <template <typename...> class Base, typename Derived>
concept is_delivered_from_template = is_base_of_template_v<Base, Derived>;

template <typename Derived, template <typename...> class... Bases>
concept is_delivered_from_any_template =
    is_base_of_any_template_v<Derived, Bases...>;

}  // namespace atom::meta

#endif