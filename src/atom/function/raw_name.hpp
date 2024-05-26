/*!
 * \file raw_name.hpp
 * \brief Get raw name of a type
 * \author Max Qian <lightapt.com>
 * \date 2024-5-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_RAW_NAME_HPP
#define ATOM_META_RAW_NAME_HPP

#include <string_view>

#include "macro.hpp"
#include "template_traits.hpp"

namespace atom::meta {
constexpr std::string_view extract_raw_name(std::string_view name) {
#if __GNUC__ || __clang__
    std::size_t start = name.find('=') + 2;
    std::size_t end = name.size() - 1;
    return std::string_view{name.data() + start, end - start};
#elif _MSC_VER
    std::size_t start = name.find('<') + 1;
    std::size_t end = name.rfind(">(");
    name = std::string_view{name.data() + start, end - start};
    start = name.find(' ');
    return start == std::string_view::npos
               ? name
               : std::string_view{name.data() + start + 1,
                                  name.size() - start - 1};
#else
    static_assert(false, "Unsupported compiler");
#endif
}

template <typename T>
constexpr auto raw_name_of() {
    return extract_raw_name(MAGIC_CPP_FUNCTION_NAME);
}

template <typename T>
constexpr auto raw_name_of_template() {
    std::string_view name = template_traits<T>::full_name;
    return extract_raw_name(name);
}

template <auto Value>
constexpr auto raw_name_of() {
    return extract_raw_name(MAGIC_CPP_FUNCTION_NAME);
}

template <auto Value>
constexpr auto raw_name_of_enum() {
    std::string_view name = extract_raw_name(MAGIC_CPP_FUNCTION_NAME);
    std::size_t start = name.rfind("::");
    return start == std::string_view::npos
               ? name
               : std::string_view{name.data() + start + 2,
                                  name.size() - start - 2};
}

#ifdef MAGIC_CPP_20_SUPPORT
template <typename T>
struct Wrapper {
    T a;
    constexpr Wrapper(T value) : a(value) {}
};

template <Wrapper T>
constexpr auto raw_name_of_member() {
    std::string_view name = MAGIC_CPP_FUNCTION_NAME;
#if __GNUC__ && (!__clang__) && (!_MSC_VER)
    std::size_t start = name.rfind("::") + 2;
    std::size_t end = name.rfind(')');
    return name.substr(start, end - start);
#elif __clang__
    std::size_t start = name.rfind(".") + 1;
    std::size_t end = name.rfind('}');
    return name.substr(start, end - start);
#elif _MSC_VER
    std::size_t start = name.rfind("->") + 2;
    std::size_t end = name.rfind('}');
    return name.substr(start, end - start);
#else
#error "Unsupported compiler"
#endif
}
#endif  // MAGIC_CPP_20_SUPPORT

template <typename T>
using args_type_of = args_type_of<T>;

template <typename Derived, typename... Bases>
constexpr bool is_derived_from_all_v = is_derived_from_all_v<Derived, Bases...>;

template <typename T, template <typename, typename...> class Template>
constexpr bool is_partial_specialization_of_v =
    is_partial_specialization_of_v<T, Template>;
}  // namespace atom::meta

#endif  // ATOM_META_RAW_NAME_HPP
