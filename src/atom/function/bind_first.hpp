/*
 * bind_first.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: An easy way to bind a function to a object

**************************************************/

#ifndef ATOM_FUNCTION_BIND_FIRST_HPP
#define ATOM_FUNCTION_BIND_FIRST_HPP

#include <concepts>
#include <functional>
#include <type_traits>

template <typename T>
constexpr T *get_pointer(T *t) noexcept {
    return t;
}

template <typename T>
T *get_pointer(const std::reference_wrapper<T> &t) noexcept {
    return &t.get();
}

template <typename T>
constexpr const T *get_pointer(const T &t) noexcept {
    return &t;
}

template <typename T>
constexpr T *remove_const_pointer(const T *t) noexcept {
    return const_cast<T *>(t);
}

template <typename F, typename... Args>
concept invocable = std::is_invocable_v<F, Args...>;

template <typename F, typename... Args>
concept nothrow_invocable = std::is_nothrow_invocable_v<F, Args...>;

template <typename F, typename... Args>
constexpr bool is_invocable_v = invocable<F, Args...>;

template <typename F, typename... Args>
constexpr bool is_nothrow_invocable_v = std::is_nothrow_invocable_v<F, Args...>;

template <typename O, typename Ret, typename P1, typename... Param>
constexpr auto bind_first(Ret (*f)(P1, Param...), O &&o)
    requires invocable<Ret (*)(P1, Param...), O, Param...>
{
    return [f, o = std::forward<O>(o)](Param... param) -> Ret {
        return f(o, std::forward<Param>(param)...);
    };
}

template <typename O, typename Ret, typename Class, typename... Param>
constexpr auto bind_first(Ret (Class::*f)(Param...), O &&o)
    requires invocable<Ret (Class::*)(Param...), O, Param...>
{
    return [f, o = std::forward<O>(o)](Param... param) -> Ret {
        return (remove_const_pointer(get_pointer(o))->*f)(
            std::forward<Param>(param)...);
    };
}

template <typename O, typename Ret, typename Class, typename... Param>
constexpr auto bind_first(Ret (Class::*f)(Param...) const, O &&o)
    requires invocable<Ret (Class::*)(Param...) const, O, Param...>
{
    return [f, o = std::forward<O>(o)](Param... param) -> Ret {
        return (get_pointer(o)->*f)(std::forward<Param>(param)...);
    };
}

template <typename O, typename Ret, typename P1, typename... Param>
auto bind_first(const std::function<Ret(P1, Param...)> &f, O &&o)
    requires invocable<std::function<Ret(P1, Param...)>, O, Param...>
{
    return [f, o = std::forward<O>(o)](Param... param) -> Ret {
        return f(o, std::forward<Param>(param)...);
    };
}

template <typename F, typename O, typename Ret, typename Class, typename P1,
          typename... Param>
constexpr auto bind_first(const F &fo, O &&o,
                          Ret (Class::*f)(P1, Param...) const)
    requires invocable<F, O, P1, Param...>
{
    return [fo, o = std::forward<O>(o), f](Param... param) -> Ret {
        return (fo.*f)(o, std::forward<Param>(param)...);
    };
}

template <typename F, typename O>
constexpr auto bind_first(const F &f, O &&o)
    requires invocable<F, O>
{
    return bind_first(f, std::forward<O>(o), &F::operator());
}

template <typename F, typename O>
constexpr auto bind_first(F &&f, O &&o)
    requires std::invocable<F, O>
{
    return [f = std::forward<F>(f),
            o = std::forward<O>(o)](auto &&...param) -> decltype(auto) {
        return std::invoke(f, o, std::forward<decltype(param)>(param)...);
    };
}

#endif  // ATOM_FUNCTION_BIND_FIRST_HPP
