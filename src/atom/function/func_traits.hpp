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
#include "concept.hpp"

#if ENABLE_DEBUG
#include <iostream>
#endif

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

    static const inline std::string full_name =
        DemangleHelper::demangle(typeid(Return(Args...)).name());
};

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
    using class_type = Class;
    static constexpr bool is_member_function = true;
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

#if ENABLE_DEBUG
// Helper function to print tuple types
template <typename Tuple>
void print_tuple_types() {
    std::apply(
        [](auto &&...args) {
            ((std::cout << (typeid(decltype(args)).name()) << ", "), ...);
        },
        std::tuple<>{});
}

// Helper function to print function information
template <typename F>
void print_function_info(const std::string &name, F &&) {
    using traits = FunctionTraits<std::decay_t<F>>;
    std::cout << name << " info:\n";
    std::cout << "  Return type: "
              << typeid(typename traits::return_type).name() << "\n";
    std::cout << "  Arity: " << traits::arity << "\n";
    std::cout << "  Parameter types: (";
    print_tuple_types<typename traits::argument_types>();
    std::cout << ")\n";
    if constexpr (is_member_function_v<F>) {
        std::cout << "  Class type: "
                  << typeid(typename traits::class_type).name() << "\n";
    }
    if constexpr (is_const_member_function_v<F>) {
        std::cout << "  Is const: " << std::boolalpha
                  << traits::is_const_member_function << "\n";
    }
    if constexpr (is_volatile_member_function_v<F>) {
        std::cout << "  Is volatile: " << std::boolalpha
                  << traits::is_volatile_member_function << "\n";
    }
    if constexpr (is_lvalue_reference_member_function_v<F>) {
        std::cout << "  Is lvalue reference qualified: " << std::boolalpha
                  << traits::is_lvalue_reference_member_function << "\n";
    }
    if constexpr (is_rvalue_reference_member_function_v<F>) {
        std::cout << "  Is rvalue reference qualified: " << std::boolalpha
                  << traits::is_rvalue_reference_member_function << "\n";
    }
    if constexpr (is_noexcept_v<F>) {
        std::cout << "  Is noexcept: " << std::boolalpha << traits::is_noexcept
                  << "\n";
    }
    if constexpr (is_variadic_v<F>) {
        std::cout << "  Is variadic: " << std::boolalpha << traits::is_variadic
                  << "\n";
    }
    std::cout << "\n";
}

#endif

// Definition of the function_pipe class
template <typename Func>
class function_pipe;

// Specialization for specific function signatures
template <typename R, typename Arg0, typename... Args>
class function_pipe<R(Arg0, Args...)> {
    std::function<R(Arg0, Args...)> _func;
    std::tuple<Args...> _args;

public:
    // Constructor accepting any callable type
    template <Callable T>
    explicit function_pipe(T &&f) : _func(std::forward<T>(f)) {}

    // Overload operator() to capture arguments by storing them in a tuple
    auto operator()(Args... args) -> auto & {
        _args = std::make_tuple(args...);
        return *this;
    }

    // Friend operator| to invoke the function with the stored arguments
    friend auto operator|(Arg0 arg0, const function_pipe &pf) -> R {
        return std::apply(pf._func,
                          std::tuple_cat(std::make_tuple(arg0), pf._args));
    }
};

// Deduction guide using function traits to determine the function signature
template <Callable T>
function_pipe(T) -> function_pipe<typename FunctionTraits<T>::result_type(
                     typename std::tuple_element<
                         0, typename FunctionTraits<T>::argument_types>::type,
                     typename std::tuple_element<
                         1, typename FunctionTraits<T>::argument_types>::type)>;

}  // namespace atom::meta

#endif  // ATOM_META_FUNC_TRAITS_HPP
