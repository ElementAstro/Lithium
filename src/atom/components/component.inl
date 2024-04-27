#ifndef ATOM_COMPONENT_INL
#define ATOM_COMPONENT_INL

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

#endif
