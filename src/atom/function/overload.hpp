/*
 * overload.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Function Overload for specific types

**************************************************/

#ifndef ATOM_FUNCTION_OVERLOAD_HPP
#define ATOM_FUNCTION_OVERLOAD_HPP

template <typename... Args>
struct OverloadCast {
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(
        ReturnType (ClassType::*func)(Args...)) const noexcept {
        return func;
    }

    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const) const noexcept {
        return func;
    }

    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(
        ReturnType (ClassType::*func)(Args...) volatile) const noexcept {
        return func;
    }

    template <typename ReturnType>
    constexpr auto operator()(ReturnType (*func)(Args...)) const noexcept {
        return func;
    }

    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(
        ReturnType (ClassType::*func)(Args...) noexcept) const noexcept {
        return func;
    }

    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const noexcept) const noexcept {
        return func;
    }

    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(
        Args...) volatile noexcept) const noexcept {
        return func;
    }

    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const volatile noexcept) const noexcept {
        return func;
    }

    template <typename ReturnType>
    constexpr auto operator()(ReturnType (*func)(Args...)) const noexcept {
        return func;
    }

    template <typename ReturnType>
    constexpr auto operator()(
        ReturnType (*func)(Args...) noexcept) const noexcept {
        return func;
    }
};

// Helper function to create an OverloadCast object
template <typename... Args>
constexpr auto overload_cast = OverloadCast<Args...>{};

#endif
