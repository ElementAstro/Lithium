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
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "abi.hpp"
#include "template_traits.hpp"

namespace atom::meta {

template <typename Func>
struct FunctionTraits;

template <typename Return, typename... Args>
struct FunctionTraitsBase {
    using return_type = Return;
    using argument_types = std::tuple<Args...>;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    using argument_t = std::tuple_element_t<N, argument_types>;

    static constexpr bool is_member_function = false;
    static constexpr bool is_const_member_function = false;
    static constexpr bool is_volatile_member_function = false;
    static constexpr bool is_lvalue_reference_member_function = false;
    static constexpr bool is_rvalue_reference_member_function = false;
    static constexpr bool is_noexcept = false;
    static constexpr bool is_variadic = false;

    static const std::string full_name;
};

template <typename Return, typename... Args>
const std::string FunctionTraitsBase<Return, Args...>::full_name = [] {
    std::string name = typeid(Return(Args...)).name();
    return DemangleHelper::Demangle(name);
}();

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...)> : FunctionTraitsBase<Return, Args...> {};

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...) const>
    : FunctionTraitsBase<Return, Args...> {
    static constexpr bool is_const_member_function = true;
};

template <typename Return, typename... Args>
struct FunctionTraits<std::function<Return(Args...)>>
    : FunctionTraitsBase<Return, Args...> {};

template <typename Return, typename... Args>
struct FunctionTraits<Return (*)(Args...)>
    : FunctionTraitsBase<Return, Args...> {};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...)>
    : FunctionTraitsBase<Return, Args...> {
    static constexpr bool is_member_function = true;
    using class_type = Class;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) volatile>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_volatile_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const volatile>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_volatile_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) &>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_lvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const &>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_lvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) volatile &>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_volatile_member_function = true;
    static constexpr bool is_lvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const volatile &>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_volatile_member_function = true;
    static constexpr bool is_lvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) &&>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_rvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const &&>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_rvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) volatile &&>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_volatile_member_function = true;
    static constexpr bool is_rvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const volatile &&>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_volatile_member_function = true;
    static constexpr bool is_rvalue_reference_member_function = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) noexcept>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_noexcept = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const noexcept>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_noexcept = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) volatile noexcept>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_volatile_member_function = true;
    static constexpr bool is_noexcept = true;
};

template <typename Return, typename Class, typename... Args>
struct FunctionTraits<Return (Class::*)(Args...) const volatile noexcept>
    : FunctionTraits<Return (Class::*)(Args...)> {
    static constexpr bool is_const_member_function = true;
    static constexpr bool is_volatile_member_function = true;
    static constexpr bool is_noexcept = true;
};

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args..., ...)>
    : FunctionTraitsBase<Return, Args...> {
    static constexpr bool is_variadic = true;
};

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args..., ...) noexcept>
    : FunctionTraits<Return(Args..., ...)> {
    static constexpr bool is_noexcept = true;
    static constexpr bool is_variadic = true;
};

// Lambda and function object support
template <typename Func>
struct FunctionTraits
    : FunctionTraits<decltype(&std::remove_cvref_t<Func>::operator())> {};

// Support for function references
template <typename Func>
struct FunctionTraits<Func &> : FunctionTraits<Func> {};

template <typename Func>
struct FunctionTraits<Func &&> : FunctionTraits<Func> {};

// Utility variable templates
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
inline constexpr bool is_lvalue_reference_member_function_v =
    FunctionTraits<Func>::is_lvalue_reference_member_function;

template <typename Func>
inline constexpr bool is_rvalue_reference_member_function_v =
    FunctionTraits<Func>::is_rvalue_reference_member_function;

template <typename Func>
inline constexpr bool is_noexcept_v = FunctionTraits<Func>::is_noexcept;

template <typename Func>
inline constexpr bool is_variadic_v = FunctionTraits<Func>::is_variadic;

}  // namespace atom::meta

#endif
