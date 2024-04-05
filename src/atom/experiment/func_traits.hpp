/*
 * func_traits.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-2

Description: Func Traits for C++20

**************************************************/

#ifndef ATOM_EXPERIMENT_FUNC_TRAITS_HPP
#define ATOM_EXPERIMENT_FUNC_TRAITS_HPP

#include <tuple>

template <typename Func>
struct FunctionTraits;

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...)> {
    using return_type = Return;
    using argument_types = std::tuple<Args...>;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    struct argument {
        static_assert(N < arity, "Invalid argument index.");
        using type = typename std::tuple_element<N, argument_types>::type;
    };

    template <std::size_t N>
    using argument_t = typename argument<N>::type;

    static constexpr bool is_member_function = false;
    static constexpr bool is_const_member_function = false;
    static constexpr bool is_volatile_member_function = false;
    static constexpr bool is_noexcept = false;
};

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

template <typename Func>
struct FunctionTraits : FunctionTraits<decltype(&Func::operator())> {};

#endif
