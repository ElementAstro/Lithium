/*
 * callable.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: make a callabel object

**************************************************/

#ifndef ATOM_FUNCTION_CALLABLE_HPP
#define ATOM_FUNCTION_CALLABLE_HPP

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename Class, typename... Param>
struct Constructor {
    template <typename... Inner>
    auto operator()(Inner&&... inner) const -> std::shared_ptr<Class> {
        return std::make_shared<Class>(std::forward<Inner>(inner)...);
    }
};

template <typename Ret, typename Class, typename... Param>
struct Const_Caller {
    explicit constexpr Const_Caller(Ret (Class::*t_func)(Param...)
                                        const) noexcept
        : m_func(t_func) {}

    template <typename... Inner>
    constexpr auto operator()(const Class& o, Inner&&... inner) const
        noexcept(noexcept((o.*m_func)(std::forward<Inner>(inner)...))) -> Ret {
        return (o.*m_func)(std::forward<Inner>(inner)...);
    }

    Ret (Class::*m_func)(Param...) const;
};

template <typename Ret, typename... Param>
struct Fun_Caller {
    explicit constexpr Fun_Caller(Ret (*t_func)(Param...)) noexcept
        : m_func(t_func) {}

    template <typename... Inner>
    constexpr auto operator()(Inner&&... inner) const
        noexcept(noexcept(m_func(std::forward<Inner>(inner)...))) -> Ret {
        return m_func(std::forward<Inner>(inner)...);
    }

    Ret (*m_func)(Param...);
};

template <typename Ret, typename Class, typename... Param>
struct Caller {
    explicit constexpr Caller(Ret (Class::*t_func)(Param...)) noexcept
        : m_func(t_func) {}

    template <typename... Inner>
    constexpr auto operator()(Class& o, Inner&&... inner) const
        noexcept(noexcept((o.*m_func)(std::forward<Inner>(inner)...))) -> Ret {
        return (o.*m_func)(std::forward<Inner>(inner)...);
    }

    Ret (Class::*m_func)(Param...);
};

template <typename T>
struct Arity
    : std::integral_constant<std::size_t, std::tuple_size_v<std::tuple<T>>> {};

template <typename T>
struct Function_Signature;

template <typename Ret, typename... Params>
struct Function_Signature<Ret(Params...)> {
    using Return_Type = Ret;
    using Signature = Ret (*)(Params...);
};

template <typename Ret, typename T, typename... Params>
struct Function_Signature<Ret (T::*)(Params...) const> {
    using Return_Type = Ret;
    using Signature = Ret (*)(Params...);
};

template <typename T>
struct Callable_Traits {
    using Signature = typename Function_Signature<
        decltype(&std::remove_reference_t<T>::operator())>::Signature;
    using Return_Type = typename Function_Signature<
        decltype(&std::remove_reference_t<T>::operator())>::Return_Type;
};

template <typename Ret, typename... Param>
struct Fun_Caller_Noexcept {
    explicit constexpr Fun_Caller_Noexcept(Ret (*t_func)(Param...) noexcept)
        : m_func(t_func) {}

    template <typename... Inner>
    constexpr auto operator()(Inner&&... inner) const noexcept -> Ret {
        return m_func(std::forward<Inner>(inner)...);
    }
    Ret (*m_func)(Param...) noexcept;
};

template <typename Ret, typename Class, typename... Param>
struct Caller_Noexcept {
    explicit constexpr Caller_Noexcept(Ret (Class::*t_func)(Param...) noexcept)
        : m_func(t_func) {}

    template <typename... Inner>
    constexpr auto operator()(Class& o,
                              Inner&&... inner) const noexcept -> Ret {
        return (o.*m_func)(std::forward<Inner>(inner)...);
    }

    Ret (Class::*m_func)(Param...) noexcept;
};

template <typename T>
struct Is_Noexcept_Callable : std::false_type {};

template <typename Ret, typename... Param>
struct Is_Noexcept_Callable<Ret (*)(Param...) noexcept> : std::true_type {};

template <typename Ret, typename Class, typename... Param>
struct Is_Noexcept_Callable<Ret (Class::*)(Param...) noexcept>
    : std::true_type {};

template <typename T>
constexpr bool Is_Noexcept_Callable_v = Is_Noexcept_Callable<T>::value;

#endif  // ATOM_FUNCTION_CALLABLE_HPP
