/*
 * constructors.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: C++ Function Constructors

**************************************************/

#ifndef ATOM_FUNTION_CONSTRUCTORS_HPP
#define ATOM_FUNTION_CONSTRUCTORS_HPP

#include "proxy.hpp"

#include "atom/error/exception.hpp"

template <typename MemberFunc, typename ClassType>
ProxyFunction<MemberFunc> bind_member_function(
    MemberFunc ClassType::*member_func) {
    auto call = [member_func](ClassType &obj, auto &&...params) {
        return (obj.*member_func)(std::forward<decltype(params)>(params)...);
    };
    return ProxyFunction<MemberFunc>(call);
}

template <typename Func>
ProxyFunction<Func> bind_static_function(Func func) {
    return ProxyFunction<Func>(func);
}

template <typename MemberType, typename ClassType>
ProxyFunction<MemberType &(ClassType &)> bind_member_variable(
    MemberType ClassType::*member_var) {
    auto call = [member_var](ClassType &obj) -> MemberType & {
        return obj.*member_var;
    };
    return ProxyFunction<MemberType &(ClassType &)>(call);
}

template <typename Class, typename... Params>
ProxyFunction<std::shared_ptr<Class>(Params...)> build_shared_constructor_(
    Class (*)(Params...)) {
    auto call = [](auto &&...params) {
        return std::make_shared<Class>(
            std::forward<decltype(params)>(params)...);
    };
    return ProxyFunction<std::shared_ptr<Class>(Params...)>(call);
}

template <typename Class, typename... Params>
ProxyFunction<Class(Params...)> build_copy_constructor_(Class (*)(Params...)) {
    auto call = [](auto &&...params) {
        return Class(std::forward<decltype(params)>(params)...);
    };
    return ProxyFunction<Class(Params...)>(call);
}

template <typename Class, typename... Params>
ProxyFunction<Class(Params...)> build_plain_constructor_(Class (*)(Params...)) {
    auto call = [](auto &&...params) {
        return Class(std::forward<decltype(params)>(params)...);
    };
    return ProxyFunction<Class(Params...)>(call);
}

template <typename Class>
ProxyFunction<Class()> build_default_constructor_() {
    auto call = []() { return Class(); };
    return ProxyFunction<Class()>(call);
}

template <typename T>
auto constructor() {
    T *f = nullptr;
    using ClassType = typename FunctionTraits<T>::class_type;

    if constexpr (!std::is_copy_constructible_v<ClassType>) {
        return build_shared_constructor_(f);
    } else {
        return build_copy_constructor_(f);
    }
}

template <typename Class>
auto default_constructor() {
    if constexpr (std::is_default_constructible_v<Class>) {
        return build_default_constructor_<Class>();
    } else {
        THROW_EXCEPTION("Class is not default constructible");
    }
}

#endif  // ATOM_FUNTION_CONSTRUCTORS_HPP
