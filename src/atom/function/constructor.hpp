/*!
 * \file constructors.hpp
 * \brief C++ Function Constructors
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_CONSTRUCTOR_HPP
#define ATOM_META_CONSTRUCTOR_HPP

#include <memory>
#include <utility>

#include "atom/error/exception.hpp"
#include "func_traits.hpp"

namespace atom::meta {

/*!
 * \brief Binds a member function to an object.
 * \tparam MemberFunc Type of the member function.
 * \tparam ClassType Type of the class.
 * \param member_func Pointer to the member function.
 * \return A lambda that binds the member function to an object.
 */
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

/*!
 * \brief Binds a static function.
 * \tparam Func Type of the function.
 * \param func The static function.
 * \return The static function itself.
 */
template <typename Func>
auto bindStaticFunction(Func func) {
    return func;
}

/*!
 * \brief Binds a member variable to an object.
 * \tparam MemberType Type of the member variable.
 * \tparam ClassType Type of the class.
 * \param member_var Pointer to the member variable.
 * \return A lambda that binds the member variable to an object.
 */
template <typename MemberType, typename ClassType>
auto bindMemberVariable(MemberType ClassType::*member_var) {
    return [member_var](ClassType &instance) -> MemberType & {
        return instance.*member_var;
    };
}

/*!
 * \brief Builds a shared constructor for a class.
 * \tparam Class Type of the class.
 * \tparam Params Types of the constructor parameters.
 * \param unused Unused parameter to deduce types.
 * \return A lambda that constructs a shared pointer to the class.
 */
template <typename Class, typename... Params>
auto buildSharedConstructor(Class (* /*unused*/)(Params...)) {
    return [](auto &&...params) {
        return std::make_shared<Class>(
            std::forward<decltype(params)>(params)...);
    };
}

/*!
 * \brief Builds a copy constructor for a class.
 * \tparam Class Type of the class.
 * \tparam Params Types of the constructor parameters.
 * \param unused Unused parameter to deduce types.
 * \return A lambda that constructs an instance of the class.
 */
template <typename Class, typename... Params>
auto buildCopyConstructor(Class (* /*unused*/)(Params...)) {
    return [](auto &&...params) {
        return Class(std::forward<decltype(params)>(params)...);
    };
}

/*!
 * \brief Builds a plain constructor for a class.
 * \tparam Class Type of the class.
 * \tparam Params Types of the constructor parameters.
 * \param unused Unused parameter to deduce types.
 * \return A lambda that constructs an instance of the class.
 */
template <typename Class, typename... Params>
auto buildPlainConstructor(Class (* /*unused*/)(Params...)) {
    return [](auto &&...params) {
        return Class(std::forward<decltype(params)>(params)...);
    };
}

/*!
 * \brief Builds a constructor for a class with specified arguments.
 * \tparam Class Type of the class.
 * \tparam Args Types of the constructor arguments.
 * \return A lambda that constructs a shared pointer to the class.
 */
template <typename Class, typename... Args>
auto buildConstructor() {
    return [](Args... args) -> std::shared_ptr<Class> {
        return std::make_shared<Class>(std::forward<Args>(args)...);
    };
}

/*!
 * \brief Builds a default constructor for a class.
 * \tparam Class Type of the class.
 * \return A lambda that constructs an instance of the class.
 */
template <typename Class>
auto buildDefaultConstructor() {
    return []() { return Class(); };
}

/*!
 * \brief Constructs an instance of a class based on its traits.
 * \tparam T Type of the function.
 * \return A lambda that constructs an instance of the class.
 */
template <typename T>
auto constructor() {
    T *func = nullptr;
    using ClassType = typename FunctionTraits<T>::class_type;

    if constexpr (!std::is_copy_constructible_v<ClassType>) {
        return buildSharedConstructor(func);
    } else {
        return buildCopyConstructor(func);
    }
}

/*!
 * \brief Constructs an instance of a class with specified arguments.
 * \tparam Class Type of the class.
 * \tparam Args Types of the constructor arguments.
 * \return A lambda that constructs a shared pointer to the class.
 */
template <typename Class, typename... Args>
auto constructor() {
    return buildConstructor<Class, Args...>();
}

/*!
 * \brief Constructs an instance of a class using the default constructor.
 * \tparam Class Type of the class.
 * \return A lambda that constructs an instance of the class.
 * \throws Exception if the class is not default constructible.
 */
template <typename Class>
auto defaultConstructor() {
    if constexpr (std::is_default_constructible_v<Class>) {
        return buildDefaultConstructor<Class>();
    } else {
        THROW_NOT_FOUND("Class is not default constructible");
    }
}

/*!
 * \brief Constructs an instance of a class using a move constructor.
 * \tparam Class Type of the class.
 * \return A lambda that constructs an instance of the class using a move
 * constructor.
 */
template <typename Class>
auto buildMoveConstructor() {
    return [](Class &&instance) { return Class(std::move(instance)); };
}

/*!
 * \brief Constructs an instance of a class using an initializer list.
 * \tparam Class Type of the class.
 * \tparam T Type of the elements in the initializer list.
 * \return A lambda that constructs an instance of the class using an
 * initializer list.
 */
template <typename Class, typename T>
auto buildInitializerListConstructor() {
    return [](std::initializer_list<T> init_list) { return Class(init_list); };
}

}  // namespace atom::meta

#endif  // ATOM_META_CONSTRUCTOR_HPP