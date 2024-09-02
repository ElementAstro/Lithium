/*!
 * \file overload.hpp
 * \brief Simplified Function Overload Helper with Better Type Deduction
 * \author Max Qian <lightapt.com>
 * \date 2024-04-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_OVERLOAD_HPP
#define ATOM_META_OVERLOAD_HPP

namespace atom::meta {

// Simplified OverloadCast with improved type deduction and usage
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

    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const volatile) const noexcept {
        return func;
    }

    template <typename ReturnType>
    constexpr auto operator()(ReturnType (*func)(Args...)) const noexcept {
        return func;
    }

    // Added noexcept overloads
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
    constexpr auto operator()(
        ReturnType (*func)(Args...) noexcept) const noexcept {
        return func;
    }
};

// Helper function to instantiate OverloadCast, simplified to improve usability
template <typename... Args>
constexpr auto overload_cast = OverloadCast<Args...>{};

}  // namespace atom::meta

#endif  // ATOM_META_OVERLOAD_HPP
