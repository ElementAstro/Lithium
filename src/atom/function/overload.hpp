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
};

// Helper function to create an OverloadCast object
template <typename... Args>
constexpr auto overload_cast = OverloadCast<Args...>{};

#endif
