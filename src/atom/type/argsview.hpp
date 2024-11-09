/*
 * argsview.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-28

Description: Argument View for C++20

**************************************************/

#ifndef ATOM_TYPE_ARGSVIEW_HPP
#define ATOM_TYPE_ARGSVIEW_HPP

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * @brief A class that provides a view over a set of arguments.
 *
 * @tparam Args Types of the arguments.
 */
template <typename... Args>
class ArgsView {
public:
    /**
     * @brief Construct a new ArgsView object.
     *
     * @param args Arguments to be stored in the view.
     */
    constexpr explicit ArgsView(Args&&... args) noexcept
        : args_(std::forward<Args>(args)...) {}

    /**
     * @brief Construct a new ArgsView object from a tuple.
     *
     * @tparam OtherArgs Types of the arguments in the tuple.
     * @param other_tuple Tuple containing the arguments.
     */
    template <typename... OtherArgs>
    constexpr explicit ArgsView(const std::tuple<OtherArgs...>& other_tuple)
        : args_(std::apply(
              [](const auto&... args) { return std::tuple<Args...>(args...); },
              other_tuple)) {}

    /**
     * @brief Construct a new ArgsView object from another ArgsView.
     *
     * @tparam OtherArgs Types of the arguments in the other ArgsView.
     * @param other_args_view The other ArgsView.
     */
    template <typename... OtherArgs>
    constexpr explicit ArgsView(ArgsView<OtherArgs...> other_args_view)
        : args_(std::apply(
              [](const auto&... args) { return std::tuple<Args...>(args...); },
              other_args_view.args_)) {}

    template <typename... OptionalArgs>

    constexpr explicit ArgsView(std::optional<OptionalArgs>... optional_args)
        : args_(std::make_tuple(optional_args.value_or(Args{})...)) {}

    /**
     * @brief Get the argument at the specified index.
     *
     * @tparam I Index of the argument.
     * @return decltype(auto) The argument at the specified index.
     */
    template <std::size_t I>
    constexpr decltype(auto) get() const noexcept {
        return std::get<I>(args_);
    }

    /**
     * @brief Get the number of arguments.
     *
     * @return std::size_t The number of arguments.
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return sizeof...(Args);
    }

    /**
     * @brief Check if there are no arguments.
     *
     * @return true If there are no arguments.
     * @return false Otherwise.
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    /**
     * @brief Apply a function to each argument.
     *
     * @tparam Func Type of the function.
     * @param func The function to apply.
     */
    template <typename Func>
    constexpr void forEach(Func&& func) const {
        std::apply([&func](const auto&... args) { (func(args), ...); }, args_);
    }

    /**
     * @brief Transform the arguments using a function.
     *
     * @tparam Func Type of the function.
     * @param func The function to apply.
     * @return ArgsView<std::invoke_result_t<Func, Args>...> A new ArgsView with
     * the transformed arguments.
     */
    template <typename F>
    auto transform(F&& f) const {
        return ArgsView<std::decay_t<decltype(f(std::declval<Args>()))>...>(
            std::apply(
                [&](const auto&... args) {
                    return std::make_tuple(f(args)...);
                },
                args_));
    }

    std::tuple<Args...> toTuple() const { return std::tuple<Args...>(args_); }

    /**
     * @brief Accumulate the arguments using a function and an initial value.
     *
     * @tparam Func Type of the function.
     * @tparam Init Type of the initial value.
     * @param func The function to apply.
     * @param init The initial value.
     * @return decltype(auto) The accumulated result.
     */
    template <typename Func, typename Init>
    constexpr auto accumulate(Func&& func, Init init) const {
        return std::apply(
            [&func, init = std::move(init)](const auto&... args) mutable {
                ((init = func(init, args)), ...);
                return init;
            },
            args_);
    }

    /**
     * @brief Apply a function to the arguments.
     *
     * @tparam Func Type of the function.
     * @param func The function to apply.
     * @return decltype(auto) The result of applying the function.
     */
    template <typename Func>
    constexpr auto apply(Func&& func) const {
        return std::apply(std::forward<Func>(func), args_);
    }

    /**
     * @brief Assign the arguments from a tuple.
     *
     * @tparam OtherArgs Types of the arguments in the tuple.
     * @param other_tuple The tuple containing the arguments.
     * @return ArgsView& Reference to this ArgsView.
     */
    template <typename... OtherArgs>
    constexpr auto operator=(const std::tuple<OtherArgs...>& other_tuple)
        -> ArgsView& {
        args_ = std::apply(
            [](const auto&... args) { return std::tuple<Args...>(args...); },
            other_tuple);
        return *this;
    }

    /**
     * @brief Assign the arguments from another ArgsView.
     *
     * @tparam OtherArgs Types of the arguments in the other ArgsView.
     * @param other_args_view The other ArgsView.
     * @return ArgsView& Reference to this ArgsView.
     */
    template <typename... OtherArgs>
    constexpr auto operator=(ArgsView<OtherArgs...> other_args_view)
        -> ArgsView& {
        args_ = std::apply(
            [](const auto&... args) { return std::tuple<Args...>(args...); },
            other_args_view.args_);
        return *this;
    }

    /**
     * @brief Filter the arguments using a predicate.
     *
     * @tparam Pred Type of the predicate.
     * @param pred The predicate to apply.
     * @return ArgsView<std::decay_t<Args>...> A new ArgsView with the filtered
     * arguments.
     */
    template <typename Pred>
    auto filter(Pred&& pred) const {
        return std::apply(
            [&](const auto&... args) {
                return ArgsView{
                    (pred(args) ? std::optional{args} : std::nullopt)...};
            },
            args_);
    }

    /**
     * @brief Find the first argument that satisfies a predicate.
     *
     * @tparam Pred Type of the predicate.
     * @param pred The predicate to apply.
     * @return std::optional<std::decay_t<Args>> The first argument that
     * satisfies the predicate, or std::nullopt if none do.
     */
    template <typename Pred>
    auto find(Pred&& pred) const {
        return std::apply(
            [&](const auto&... args)
                -> std::optional<std::common_type_t<Args...>> {
                return ((pred(args)
                             ? std::optional<std::common_type_t<Args...>>{args}
                             : std::nullopt) ||
                        ...);
            },
            args_);
    }

    /**
     * @brief Check if the arguments contain a specific value.
     *
     * @tparam T Type of the value.
     * @param value The value to check for.
     * @return true If the value is found.
     * @return false Otherwise.
     */
    template <typename T>
    auto contains(const T& value) const -> bool {
        return std::apply(
            [&](const auto&... args) { return ((args == value) || ...); },
            args_);
    }

private:
    std::tuple<Args...> args_;
};

/**
 * @brief Deduction guide for ArgsView.
 *
 * @tparam Args Types of the arguments.
 */
template <typename... Args>
ArgsView(Args&&...) -> ArgsView<std::decay_t<Args>...>;

/**
 * @brief Alias for ArgsView with decayed argument types.
 *
 * @tparam Args Types of the arguments.
 */
template <typename... Args>
using ArgsViewT = ArgsView<std::decay_t<Args>...>;

/**
 * @brief Sum the arguments.
 *
 * @tparam Args Types of the arguments.
 * @param args The arguments to sum.
 * @return int The sum of the arguments.
 */
template <typename... Args>
auto sum(Args&&... args) -> int {
    return ArgsView{std::forward<Args>(args)...}.accumulate(
        [](int a, int b) { return a + b; }, 0);
}

/**
 * @brief Concatenate the arguments into a string.
 *
 * @tparam Args Types of the arguments.
 * @param args The arguments to concatenate.
 * @return std::string The concatenated string.
 */
template <typename... Args>
auto concat(Args&&... args) -> std::string {
    return ArgsView{std::forward<Args>(args)...}
        .transform([](const auto& arg) {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>,
                                         const char*>) {
                return std::string(arg);
            } else {
                return std::to_string(arg);
            }
        })
        .accumulate([](std::string a, std::string b) { return a + b; },
                    std::string{});
}

/**
 * @brief Apply a function to the arguments in an ArgsView.
 *
 * @tparam Func Type of the function.
 * @tparam Args Types of the arguments.
 * @param func The function to apply.
 * @param args_view The ArgsView containing the arguments.
 * @return decltype(auto) The result of applying the function.
 */
template <typename Func, typename... Args>
constexpr auto apply(Func&& func, ArgsViewT<Args...> args_view) {
    return args_view.apply(std::forward<Func>(func));
}

/**
 * @brief Apply a function to each argument in an ArgsView.
 *
 * @tparam Func Type of the function.
 * @tparam Args Types of the arguments.
 * @param func The function to apply.
 * @param args_view The ArgsView containing the arguments.
 */
template <typename Func, typename... Args>
constexpr void forEach(Func&& func, ArgsView<Args...> args_view) {
    args_view.forEach(std::forward<Func>(func));
}

/**
 * @brief Accumulate the arguments in an ArgsView using a function and an
 * initial value.
 *
 * @tparam Func Type of the function.
 * @tparam Init Type of the initial value.
 * @tparam Args Types of the arguments.
 * @param func The function to apply.
 * @param init The initial value.
 * @param args_view The ArgsView containing the arguments.
 * @return decltype(auto) The accumulated result.
 */
template <typename Func, typename Init, typename... Args>
constexpr auto accumulate(Func&& func, Init init,
                          ArgsViewT<Args...> args_view) {
    return args_view.accumulate(std::forward<Func>(func), std::move(init));
}

/**
 * @brief Create an ArgsView from the given arguments.
 *
 * @tparam Args Types of the arguments.
 * @param args The arguments.
 * @return ArgsViewT<Args...> The created ArgsView.
 */
template <typename... Args>
constexpr auto makeArgsView(Args&&... args) -> ArgsViewT<Args...> {
    return ArgsViewT<Args...>(std::forward<Args>(args)...);
}

/**
 * @brief Get the argument at the specified index in an ArgsView.
 *
 * @tparam I Index of the argument.
 * @tparam Args Types of the arguments.
 * @param args_view The ArgsView containing the arguments.
 * @return decltype(auto) The argument at the specified index.
 */
template <std::size_t I, typename... Args>
constexpr auto get(ArgsView<Args...> args_view) -> decltype(auto) {
    return args_view.template get<I>();
}

/**
 * @brief Equality operator for ArgsView.
 *
 * @tparam Args1 Types of the first ArgsView.
 * @tparam Args2 Types of the second ArgsView.
 * @param lhs The left-hand side ArgsView.
 * @param rhs The right-hand side ArgsView.
 * @return true if lhs is equal to rhs.
 * @return false otherwise.
 */
template <typename... Args1, typename... Args2>
constexpr auto operator==(ArgsView<Args1...> lhs,
                          ArgsView<Args2...> rhs) -> bool {
    return lhs.size() == rhs.size() &&
           lhs.apply([&rhs](const auto&... lhs_args) {
               return rhs.apply([&lhs_args...](const auto&... rhs_args) {
                   return ((lhs_args == rhs_args) && ...);
               });
           });
}

/**
 * @brief Inequality operator for ArgsView.
 *
 * @tparam Args1 Types of the first ArgsView.
 * @tparam Args2 Types of the second ArgsView.
 * @param lhs The left-hand side ArgsView.
 * @param rhs The right-hand side ArgsView.
 * @return true if lhs is not equal to rhs.
 * @return false if lhs is equal to rhs.
 */
template <typename... Args1, typename... Args2>
constexpr auto operator!=(ArgsView<Args1...> lhs,
                          ArgsView<Args2...> rhs) -> bool {
    return !(lhs == rhs);
}

/**
 * @brief Less-than operator for ArgsView.
 *
 * @tparam Args1 Types of the first ArgsView.
 * @tparam Args2 Types of the second ArgsView.
 * @param lhs The left-hand side ArgsView.
 * @param rhs The right-hand side ArgsView.
 * @return true if lhs is less than rhs.
 * @return false otherwise.
 */
template <typename... Args1, typename... Args2>
constexpr auto operator<(ArgsView<Args1...> lhs,
                         ArgsView<Args2...> rhs) -> bool {
    return lhs.apply([&rhs](const auto&... lhs_args) {
        return rhs.apply([&lhs_args...](const auto&... rhs_args) {
            return std::tie(lhs_args...) < std::tie(rhs_args...);
        });
    });
}

/**
 * @brief Less-than-or-equal-to operator for ArgsView.
 *
 * @tparam Args1 Types of the first ArgsView.
 * @tparam Args2 Types of the second ArgsView.
 * @param lhs The left-hand side ArgsView.
 * @param rhs The right-hand side ArgsView.
 * @return true if lhs is less than or equal to rhs.
 * @return false otherwise.
 */
template <typename... Args1, typename... Args2>
constexpr auto operator<=(ArgsView<Args1...> lhs,
                          ArgsView<Args2...> rhs) -> bool {
    return !(rhs < lhs);
}

/**
 * @brief Greater-than operator for ArgsView.
 *
 * @tparam Args1 Types of the first ArgsView.
 * @tparam Args2 Types of the second ArgsView.
 * @param lhs The left-hand side ArgsView.
 * @param rhs The right-hand side ArgsView.
 * @return true if lhs is greater than rhs.
 * @return false otherwise.
 */
template <typename... Args1, typename... Args2>
constexpr auto operator>(ArgsView<Args1...> lhs,
                         ArgsView<Args2...> rhs) -> bool {
    return rhs < lhs;
}

/**
 * @brief Greater-than-or-equal-to operator for ArgsView.
 *
 * @tparam Args1 Types of the first ArgsView.
 * @tparam Args2 Types of the second ArgsView.
 * @param lhs The left-hand side ArgsView.
 * @param rhs The right-hand side ArgsView.
 * @return true if lhs is greater than or equal to rhs.
 * @return false otherwise.
 */
template <typename... Args1, typename... Args2>
constexpr auto operator>=(ArgsView<Args1...> lhs,
                          ArgsView<Args2...> rhs) -> bool {
    return !(lhs < rhs);
}

namespace std {
/**
 * @brief Hash specialization for ArgsView.
 *
 * @tparam Args Types of the arguments.
 */
template <typename... Args>
struct hash<ArgsView<Args...>> {
    /**
     * @brief Compute the hash value for an ArgsView.
     *
     * @param args_view The ArgsView to hash.
     * @return std::size_t The hash value.
     */
    auto operator()(ArgsView<Args...> args_view) const -> std::size_t {
        return args_view.apply([](const auto&... args) {
            std::size_t seed = 0;
            ((seed ^= std::hash<std::decay_t<decltype(args)>>{}(args) +
                      0x9e3779b9 + (seed << 6) + (seed >> 2)),
             ...);
            return seed;
        });
    }
};
}  // namespace std

#ifdef __DEBUG__
#include <iostream>
/**
 * @brief Print the arguments to the standard output.
 *
 * @tparam Args Types of the arguments.
 * @param args The arguments to print.
 */
template <typename... Args>
void print(Args&&... args) {
    ArgsView{std::forward<Args>(args)...}.forEach(
        [](const auto& arg) { std::cout << arg << ' '; });
    std::cout << '\n';
}
#endif

#endif
