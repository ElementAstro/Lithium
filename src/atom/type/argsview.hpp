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
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename... Args>
class ArgsView {
public:
    constexpr explicit ArgsView(Args&&... args) noexcept
        : args_(std::forward<Args>(args)...) {}

    template <typename... OtherArgs>
    constexpr explicit ArgsView(const std::tuple<OtherArgs...>& other_tuple)
        : args_(std::apply(
              [](const auto&... args) { return std::tuple<Args...>(args...); },
              other_tuple)) {}

    template <typename... OtherArgs>
    constexpr explicit ArgsView(ArgsView<OtherArgs...> other_args_view)
        : args_(std::apply(
              [](const auto&... args) { return std::tuple<Args...>(args...); },
              other_args_view.args_)) {}

    template <std::size_t I>
    constexpr decltype(auto) get() const noexcept {
        return std::get<I>(args_);
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return sizeof...(Args);
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    template <typename Func>
    constexpr void forEach(Func&& func) const {
        std::apply([&func](const auto&... args) { (func(args), ...); }, args_);
    }

    template <typename Func>
    constexpr auto transform(Func&& func) const {
        return ArgsView<std::invoke_result_t<Func, Args>...>(std::apply(
            [&func](const auto&... args) {
                return std::make_tuple(func(args)...);
            },
            args_));
    }

    template <typename Func, typename Init>
    constexpr auto accumulate(Func&& func, Init init) const {
        return std::apply(
            [&func, init = std::move(init)](const auto&... args) mutable {
                ((init = func(init, args)), ...);
                return init;
            },
            args_);
    }

    template <typename Func>
    constexpr auto apply(Func&& func) const {
        return std::apply(std::forward<Func>(func), args_);
    }

    template <typename... OtherArgs>
    constexpr auto operator=(const std::tuple<OtherArgs...>& other_tuple)
        -> ArgsView& {
        args_ = std::apply(
            [](const auto&... args) { return std::tuple<Args...>(args...); },
            other_tuple);
        return *this;
    }

    template <typename... OtherArgs>
    constexpr auto operator=(ArgsView<OtherArgs...> other_args_view)
        -> ArgsView& {
        args_ = std::apply(
            [](const auto&... args) { return std::tuple<Args...>(args...); },
            other_args_view.args_);
        return *this;
    }

private:
    std::tuple<Args...> args_;
};

template <typename... Args>
ArgsView(Args&&...) -> ArgsView<std::decay_t<Args>...>;

template <typename... Args>
using ArgsViewT = ArgsView<std::decay_t<Args>...>;

template <typename... Args>
auto sum(Args&&... args) -> int {
    return ArgsView{std::forward<Args>(args)...}.accumulate(
        [](int a, int b) { return a + b; }, 0);
}

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

template <typename Func, typename... Args>
constexpr auto apply(Func&& func, ArgsViewT<Args...> args_view) {
    return args_view.apply(std::forward<Func>(func));
}

template <typename Func, typename... Args>
constexpr void forEach(Func&& func, ArgsView<Args...> args_view) {
    args_view.forEach(std::forward<Func>(func));
}

template <typename Func, typename Init, typename... Args>
constexpr auto accumulate(Func&& func, Init init,
                          ArgsViewT<Args...> args_view) {
    return args_view.accumulate(std::forward<Func>(func), std::move(init));
}

template <typename... Args>
constexpr auto makeArgsView(Args&&... args) -> ArgsViewT<Args...> {
    return ArgsViewT<Args...>(std::forward<Args>(args)...);
}

template <std::size_t I, typename... Args>
constexpr auto get(ArgsView<Args...> args_view) -> decltype(auto) {
    return args_view.template get<I>();
}

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

template <typename... Args1, typename... Args2>
constexpr auto operator!=(ArgsView<Args1...> lhs,
                          ArgsView<Args2...> rhs) -> bool {
    return !(lhs == rhs);
}

template <typename... Args1, typename... Args2>
constexpr auto operator<(ArgsView<Args1...> lhs,
                         ArgsView<Args2...> rhs) -> bool {
    return lhs.apply([&rhs](const auto&... lhs_args) {
        return rhs.apply([&lhs_args...](const auto&... rhs_args) {
            return std::tie(lhs_args...) < std::tie(rhs_args...);
        });
    });
}

template <typename... Args1, typename... Args2>
constexpr auto operator<=(ArgsView<Args1...> lhs,
                          ArgsView<Args2...> rhs) -> bool {
    return !(rhs < lhs);
}

template <typename... Args1, typename... Args2>
constexpr auto operator>(ArgsView<Args1...> lhs,
                         ArgsView<Args2...> rhs) -> bool {
    return rhs < lhs;
}

template <typename... Args1, typename... Args2>
constexpr auto operator>=(ArgsView<Args1...> lhs,
                          ArgsView<Args2...> rhs) -> bool {
    return !(lhs < rhs);
}

namespace std {
template <typename... Args>
struct hash<ArgsView<Args...>> {
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
template <typename... Args>
void print(Args&&... args) {
    ArgsView{std::forward<Args>(args)...}.forEach(
        [](const auto& arg) { std::cout << arg << ' '; });
    std::cout << '\n';
}
#endif

#endif
