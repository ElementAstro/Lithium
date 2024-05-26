/*!
 * \file macro.hpp
 * \brief Some utility macros
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_MACRO_HPP
#define ATOM_META_MACRO_HPP

#if __has_include(<source_location>)
#include <source_location>
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#define std experimental
#else
#error "No source_location support"
#endif

#if __cplusplus >= 202002L || _MSVC_LANG >= 202002L
#define ATOM_META_CPP_20_SUPPORT
#else
#error "No C++20 support"
#endif

#if __has_include(<version>)
#include <version>
#else
#error "No version support"
#endif

#if __clang__ && _MSC_VER
#define ATOM_META_FUNCTION_NAME (__FUNCSIG__)
#elif __cpp_lib_source_location
#define ATOM_META_FUNCTION_NAME \
    (std::source_location::current().function_name())
#elif (__clang__ || __GNUC__) && (!_MSC_VER)
#define ATOM_META_FUNCTION_NAME (__PRETTY_FUNCTION__)
#elif _MSC_VER
#define ATOM_META_FUNCTION_NAME (__FUNCSIG__)
#else
staic_assert(false, "Unsupported compiler");
#endif  // source_location

namespace atom::meta {
inline void unreachable [[noreturn]] () {
#if __GNUC__ || __clang__  // GCC, Clang, ICC
    __builtin_unreachable();
#elif _MSC_VER  // MSVC
    __assume(false);
#endif
}
}  // namespace atom::meta

#endif  // ATOM_META_MACRO_HPP