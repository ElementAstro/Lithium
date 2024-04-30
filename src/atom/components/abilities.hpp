#ifndef ATOM_COMPONENTS_ABILITIES_HPP
#define ATOM_COMPONENTS_ABILITIES_HPP

#include <string>
#include <type_traits>

template <typename T>
struct has_getValue {
    template <typename U>
    static constexpr auto check(int) -> decltype(std::declval<U>().getValue(),
                                                 std::true_type{});

    template <typename>
    static constexpr std::false_type check(...);

    static constexpr bool value = decltype(check<T>(0))::value;
};

template <typename T>
struct has_setValue {
    template <typename U>
    static constexpr auto check(int) -> decltype(std::declval<U>().setValue(
                                                     std::declval<std::string>()),
                                                 std::true_type{});

    template <typename>
    static constexpr std::false_type check(...);

    static constexpr bool value = decltype(check<T>(0))::value;
};

#endif
