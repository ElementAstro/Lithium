
#ifndef CARBON_REGISTER_FUNCTION_HPP
#define CARBON_REGISTER_FUNCTION_HPP

#include <type_traits>

#include "atom/experiment/bind_first.hpp"
#include "function_signature.hpp"
#include "proxy_functions.hpp"

namespace Carbon {
namespace dispatch::detail {
/**
 * @brief Retrieves the first parameter from a function parameter pack.
 *
 * @tparam Obj The type of the object.
 * @tparam Param1 The type of the first parameter.
 * @tparam Rest The remaining parameter types.
 * @param params The function parameters.
 * @param obj The object.
 * @return Param1 The first parameter value.
 */
template <typename Obj, typename Param1, typename... Rest>
Param1 get_first_param(Function_Params<Param1, Rest...>, Obj &&obj) {
    return static_cast<Param1>(std::forward<Obj>(obj));
}

/**
 * @brief Internal implementation to create a callable object from a function or
 * member function.
 *
 * @tparam Func The type of the function.
 * @tparam Is_Noexcept Boolean indicating if the function is noexcept.
 * @tparam Is_Member Boolean indicating if the function is a member function.
 * @tparam Is_MemberObject Boolean indicating if the function is a member
 * object.
 * @tparam Is_Object Boolean indicating if the object is a callable object.
 * @tparam Ret The return type of the function.
 * @tparam Param The parameter types of the function.
 * @param func The function to be made callable.
 * @param fs The function signature.
 * @return A Proxy_Function representing the callable object.
 */
template <typename Func, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject,
          bool Is_Object, typename Ret, typename... Param>
auto make_callable_impl(
    Func &&func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept,
                                    Is_Member, Is_MemberObject, Is_Object>) {
    if constexpr (Is_MemberObject) {
        // we now that the Param pack will have only one element, so we are safe
        // expanding it here
        return Proxy_Function(
            Carbon::make_shared<
                dispatch::Proxy_Function_Base,
                dispatch::Attribute_Access<Ret, std::decay_t<Param>...>>(
                std::forward<Func>(func)));
    } else if constexpr (Is_Member) {
        // TODO some kind of bug is preventing forwarding of this noexcept for
        // the lambda
        auto call =
            [func = std::forward<Func>(func)](
                auto &&obj,
                auto &&...param) /* noexcept(Is_Noexcept) */ -> decltype(auto) {
            return ((get_first_param(Function_Params<Param...>{}, obj).*
                     func)(std::forward<decltype(param)>(param)...));
        };
        return Proxy_Function(
            Carbon::make_shared<dispatch::Proxy_Function_Base,
                                dispatch::Proxy_Function_Callable_Impl<
                                    Ret(Param...), decltype(call)>>(
                std::move(call)));
    } else {
        return Proxy_Function(
            Carbon::make_shared<dispatch::Proxy_Function_Base,
                                dispatch::Proxy_Function_Callable_Impl<
                                    Ret(Param...), std::decay_t<Func>>>(
                std::forward<Func>(func)));
    }
}

template <typename Func, typename Ret, typename Object, typename... Param,
          bool Is_Noexcept>
auto make_callable(Func &&func,
                   Function_Signature<Ret, Function_Params<Object, Param...>,
                                      Is_Noexcept, false, false, true>) {
    return make_callable_impl(
        std::forward<Func>(func),
        Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, false,
                           false, true>{});
}

/**
 * @brief Creates a callable object from a free function, member function, or
 * data member.
 *
 * @tparam Func The type of the function.
 * @tparam Ret The return type of the function.
 * @tparam Object The type of the object.
 * @tparam Param The parameter types of the function.
 * @tparam Is_Noexcept Boolean indicating if the function is noexcept.
 * @param func The function to be made callable.
 * @param fs The function signature.
 * @return A Proxy_Function representing the callable object.
 */
template <typename Func, typename Ret, typename Object, typename... Param,
          bool Is_Noexcept>
auto make_callable(Func &&func,
                   Function_Signature<Ret, Function_Params<Object, Param...>,
                                      Is_Noexcept, false, false, true>
                       fs) {
    return make_callable_impl(std::forward<Func>(func), fs);
}
}  // namespace dispatch::detail
template <typename T>
Proxy_Function fun(T &&t) {
    return dispatch::detail::make_callable(
        std::forward<T>(t), dispatch::detail::function_signature(t));
}

/**
 * @brief Creates a callable object from a free function, member function, or
 * data member and binds the first parameter.
 *
 * @tparam T The type of the function.
 * @tparam Q The type of the value to bind to the first parameter.
 * @param t The function or member to expose.
 * @param q The value to bind to the first parameter.
 * @return A Proxy_Function representing the callable object.
 */
template <typename T, typename Q>
Proxy_Function fun(T &&t, const Q &q) {
    return fun(detail::bind_first(std::forward<T>(t), q));
}
}  // namespace Carbon

#endif
