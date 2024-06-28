/*!
 * \file constructors.hpp
 * \brief C++ Function Constructors
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_CONSTRUCTOR_HPP
#define ATOM_META_CONSTRUCTOR_HPP

#include <utility>

#include "func_traits.hpp"

#include "atom/error/exception.hpp"

namespace atom::meta {
template <typename MemberFunc, typename ClassType>
auto bindMemberFunction(MemberFunc ClassType::*member_func) {
    return [member_func](ClassType &obj, auto &&...params) {
        if constexpr (FunctionTraits<MemberFunc>::is_const_member_function) {
            return (std::as_const(obj).*
                    member_func)(std::forward<decltype(params)>(params)...);
        } else {
            return (obj.*
                    member_func)(std::forward<decltype(params)>(params)...);
        }
    };
}

template <typename Func>
auto bindStaticFunction(Func func) {
    return func;
}

template <typename MemberType, typename ClassType>
auto bindMemberVariable(MemberType ClassType::*member_var) {
    return [member_var](ClassType &instance) -> MemberType & {
        return instance.*member_var;
    };
}

template <typename Class, typename... Params>
auto buildSharedConstructor(Class (* /*unused*/)(Params...)) {
    return [](auto &&...params) {
        return std::make_shared<Class>(
            std::forward<decltype(params)>(params)...);
    };
}

template <typename Class, typename... Params>
auto buildCopyConstructor(Class (* /*unused*/)(Params...)) {
    return [](auto &&...params) {
        return Class(std::forward<decltype(params)>(params)...);
    };
}

template <typename Class, typename... Params>
auto buildPlainConstructor(Class (*)(Params...)) {
    return [](auto &&...params) {
        return Class(std::forward<decltype(params)>(params)...);
    };
}

template <typename Class, typename... Args>
auto buildConstructor() {
    return [](Args... args) -> std::shared_ptr<Class> {
        return std::make_shared<Class>(std::forward<Args>(args)...);
    };
}

template <typename Class>
auto buildDefaultConstructor() {
    return []() { return Class(); };
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

template <typename Class, typename... Args>
auto constructor() {
    return build_constructor_<Class, Args...>();
}

template <typename Class>
auto defaultConstructor() {
    if constexpr (std::is_default_constructible_v<Class>) {
        return build_default_constructor_<Class>();
    } else {
        THROW_EXCEPTION("Class is not default constructible");
    }
}
}  // namespace atom::meta
#endif  // ATOM_META_CONSTRUCTOR_HPP
