/*!
 * \file func_traits.hpp
 * \brief Func Traits for C++20
 * \author Max Qian <lightapt.com>
 * \date 2024-04-02
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_FUNC_TRAITS_HPP
#define ATOM_META_FUNC_TRAITS_HPP

#include <functional>
#include <tuple>
#include <type_traits>

namespace atom::meta {

template <typename Func>
struct FunctionTraits;

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...)> {
    using return_type = Return;
    using argument_types = std::tuple<Args...>;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    using argument_t = std::tuple_element_t<N, argument_types>;

    static constexpr bool is_member_function = false;
    static constexpr bool is_const_member_function = false;
    static constexpr bool is_volatile_member_function = false;
    static constexpr bool is_noexcept = false;
    static constexpr bool is_variadic = false;
};

template <typename Return, typename... Args>
struct FunctionTraits<std::function<Return(Args...)>>
    : FunctionTraits<Return(Args...)> {};

template <typename Return, typename... Args>
struct FunctionTraits<Return (*)(Args...)> : FunctionTraits<Return(Args...)> {};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...)>
    : FunctionTraits<Return(Args...)> {
    static constexpr bool is_member_function = true;
    using class_type = Class;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const>
    : FunctionTraits<Return(Args...)> {
    static constexpr bool is_member_function = true;
    static constexpr bool is_const_member_function = true;
    using class_type = Class;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) volatile>
    : FunctionTraits<Return(Args...)> {
    static constexpr bool is_member_function = true;
    static constexpr bool is_volatile_member_function = true;
    using class_type = Class;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const volatile>
    : FunctionTraits<Return(Args...)> {
    static constexpr bool is_member_function = true;
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_volatile_member_function = true;
    using class_type = Class;
};

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...) noexcept>
    : FunctionTraits<Return(Args...)> {
    static constexpr bool is_noexcept = true;
};

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args..., ...)> : FunctionTraits<Return(Args...)> {
    static constexpr bool is_variadic = true;
};

template <typename Func>
struct FunctionTraits : FunctionTraits<std::remove_cvref_t<Func>> {};

template <typename Func>
inline constexpr bool is_member_function_v =
    FunctionTraits<Func>::is_member_function;

template <typename Func>
inline constexpr bool is_const_member_function_v =
    FunctionTraits<Func>::is_const_member_function;

template <typename Func>
inline constexpr bool is_volatile_member_function_v =
    FunctionTraits<Func>::is_volatile_member_function;

template <typename Func>
inline constexpr bool is_noexcept_v = FunctionTraits<Func>::is_noexcept;

template <typename Func>
inline constexpr bool is_variadic_v = FunctionTraits<Func>::is_variadic;
}  // namespace atom::meta

#endif
