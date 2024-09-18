/*!
 * \file overload.hpp
 * \brief Simplified Function Overload Helper with Better Type Deduction
 * \author Max Qian <lightapt.com>
 * \date 2024-04-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_OVERLOAD_HPP
#define ATOM_META_OVERLOAD_HPP

#include <type_traits>
#include <utility>

namespace atom::meta {
/// @brief A utility to simplify the casting of overloaded member functions and
/// free functions.
/// @tparam Args The argument types of the function to be cast.
template <typename... Args>
struct OverloadCast {
    /// @brief Casts a non-const member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The member function pointer.
    /// @return The casted member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(
        ReturnType (ClassType::*func)(Args...)) const noexcept {
        return func;
    }

    /// @brief Casts a const member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The const member function pointer.
    /// @return The casted const member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const) const noexcept {
        return func;
    }

    /// @brief Casts a volatile member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The volatile member function pointer.
    /// @return The casted volatile member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(
        ReturnType (ClassType::*func)(Args...) volatile) const noexcept {
        return func;
    }

    /// @brief Casts a const volatile member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The const volatile member function pointer.
    /// @return The casted const volatile member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const volatile) const noexcept {
        return func;
    }

    /// @brief Casts a free function.
    /// @tparam ReturnType The return type of the free function.
    /// @param func The free function pointer.
    /// @return The casted free function pointer.
    template <typename ReturnType>
    constexpr auto operator()(ReturnType (*func)(Args...)) const noexcept {
        return func;
    }

    // Added noexcept overloads

    /// @brief Casts a non-const noexcept member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The noexcept member function pointer.
    /// @return The casted noexcept member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(
        ReturnType (ClassType::*func)(Args...) noexcept) const noexcept {
        return func;
    }

    /// @brief Casts a const noexcept member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The const noexcept member function pointer.
    /// @return The casted const noexcept member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const noexcept) const noexcept {
        return func;
    }

    /// @brief Casts a volatile noexcept member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The volatile noexcept member function pointer.
    /// @return The casted volatile noexcept member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(
        Args...) volatile noexcept) const noexcept {
        return func;
    }

    /// @brief Casts a const volatile noexcept member function.
    /// @tparam ReturnType The return type of the member function.
    /// @tparam ClassType The class type of the member function.
    /// @param func The const volatile noexcept member function pointer.
    /// @return The casted const volatile noexcept member function pointer.
    template <typename ReturnType, typename ClassType>
    constexpr auto operator()(ReturnType (ClassType::*func)(Args...)
                                  const volatile noexcept) const noexcept {
        return func;
    }

    /// @brief Casts a noexcept free function.
    /// @tparam ReturnType The return type of the free function.
    /// @param func The noexcept free function pointer.
    /// @return The casted noexcept free function pointer.
    template <typename ReturnType>
    constexpr auto operator()(
        ReturnType (*func)(Args...) noexcept) const noexcept {
        return func;
    }
};

/// @brief Helper function to instantiate OverloadCast, simplified to improve
/// usability.
/// @tparam Args The argument types of the function to be cast.
/// @return An instance of OverloadCast with the specified argument types.
template <typename... Args>
constexpr auto overload_cast = OverloadCast<Args...>{};

template <class T>
constexpr auto decayCopy(T&& value) noexcept(
    std::is_nothrow_convertible_v<T, std::decay_t<T>>) -> std::decay_t<T> {
    return std::forward<T>(value);
}

}  // namespace atom::meta

#endif  // ATOM_META_OVERLOAD_HPP
