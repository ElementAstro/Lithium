/*
 * macro.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Useful Macros

**************************************************/

#pragma once

#if defined(__has_include) && __has_include(<source_location>)
#include <source_location>
#endif

#define ATOM_MACRO_HPP
//-------------------------------------------------------------------------------
// unused
// alignas
// assume
// enable/disable optimization
// inline
// forceinline
// extern c
// export/import/static API
// ptr size
// no vtable
// noexcept
//-------------------------------------------------------------------------------

// UNUSED
#if defined(__cplusplus)
#define ATOM_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
#define ATOM_UNUSED
#endif

#ifdef __cplusplus
#define ATOM_IF_CPP(...) __VA_ARGS__
#else
#define ATOM_IF_CPP(...)
#endif

#if defined(__cplusplus)
#define ATOM_CONSTEXPR constexpr
#else
#define ATOM_CONSTEXPR const
#endif

// ALIGNAS
#if defined(_MSC_VER)
#define ATOM_ALIGNAS(x) __declspec(align(x))
#else
#define ATOM_ALIGNAS(x) __attribute__((aligned(x)))
#endif

// ASSUME
#ifndef ATOM_ASSUME
#if defined(__clang__)
#define ATOM_ASSUME(x) __builtin_assume(x)
#elif defined(_MSC_VER)
#define ATOM_ASSUME(x) __assume(x)
#else
#define ATOM_ASSUME(x)
#endif
#endif

// OPTIMIZATION
#if defined(_MSC_VER)
#define ATOM_DISABLE_OPTIMIZATION __pragma(optimize("", off))
#define ATOM_ENABLE_OPTIMIZATION __pragma(optimize("", on))
#elif defined(__clang__)
#define ATOM_DISABLE_OPTIMIZATION #pragma clang optimize off
#define ATOM_ENABLE_OPTIMIZATION #pragma clang optimize on
#endif

// INLINE
#if defined(__cplusplus)
#define ATOM_INLINE inline
#else
#define ATOM_INLINE
#endif

// FORCEINLINE
#if defined(_MSC_VER) && !defined(__clang__)
#define ATOM_FORCEINLINE __forceinline
#else
#define ATOM_FORCEINLINE inline __attribute__((always_inline))
#endif

#if defined(_MSC_VER)
#define ATOM_NOINLINE __declspec(noinline)
#else
#define ATOM_NOINLINE __attribute__((noinline))
#endif

// EXTERN_C
#ifdef __cplusplus
#define ATOM_EXTERN_C extern "C"
#else
#define ATOM_EXTERN_C extern
#endif

// IMPORT
#ifndef ATOM_IMPORT
#if defined(_MSC_VER)
#define ATOM_IMPORT __declspec(dllimport)
#else
#define ATOM_IMPORT __attribute__((visibility("default")))
#endif
#endif

// EXPORT
#ifndef ATOM_EXPORT
#if defined(_MSC_VER)
// MSVC linker trims symbols, the 'dllexport' attribute prevents this.
// But we are not archiving DLL files with SHIPPING_ONE_ARCHIVE mode.
#define ATOM_EXPORT __declspec(dllexport)
#else
#define ATOM_EXPORT __attribute__((visibility("default")))
#endif
#endif

// STATIC
#define ATOM_STATIC_API

// PTR_SIZE
#ifdef _WIN32
#ifdef _WIN64
constexpr std::size_t ATOM_PTR_SIZE = 8;
#else
constexpr std::size_t ATOM_PTR_SIZE = 4;
#endif
#else
#if INTPTR_MAX == INT64_MAX
constexpr std::size_t ATOM_PTR_SIZE = 8;
#elif INTPTR_MAX == INT32_MAX
constexpr std::size_t ATOM_PTR_SIZE = 4;
#else
static_assert(false, "unsupported platform");
#endif
#endif

// NO_VTABLE
#ifdef _MSC_VER
#define ATOM_NO_VTABLE __declspec(novtable)
#else
#define ATOM_NO_VTABLE
#endif

// NOEXCEPT
#ifdef __cplusplus
// By Default we use cpp-standard above 2011XXL
#define ATOM_NOEXCEPT noexcept
#else
#define ATOM_NOEXCEPT
#endif

#if defined(_MSC_VER)
#if defined(__clang__)
#define ATOM_UNREF_PARAM(x) (void)x
#else
#define ATOM_UNREF_PARAM(x) (x)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_UNREF_PARAM(x) ((void)(x))
#endif

#if defined(_MSC_VER)
#define ATOM_CALLCONV __cdecl
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_CALLCONV
#endif

#if defined(__cplusplus)
#define ATOM_DECLARE_ZERO(type, var)                            \
    static_assert(std::is_trivially_constructible<type>::value, \
                  "not trival, 0 init is invalid!");            \
    type var = {};
#else
#define ATOM_DECLARE_ZERO(type, var) type var = {0};
#endif

// VLA
#ifndef __cplusplus
#if defined(_MSC_VER) && !defined(__clang__)
#define ATOM_DECLARE_ZERO_VLA(type, var, num)         \
    type* var = (type*)_alloca(sizeof(type) * (num)); \
    memset((void*)(var), 0, sizeof(type) * (num));
#else
#define ATOM_DECLARE_ZERO_VLA(type, var, num) \
    type var[(num)];                          \
    memset((void*)(var), 0, sizeof(type) * (num));
#endif
#endif

#if defined(__cplusplus)
#define ATOM_ENUM(inttype) : inttype
#else
#define ATOM_ENUM(inttype)
#endif

#pragma region stringizing

#ifndef ATOM_STRINGIZING
#define ATOM_STRINGIZING(...) #__VA_ARGS__
#endif

#ifndef ATOM_MAKE_STRING
#define ATOM_MAKE_STRING(...) ATOM_STRINGIZING(__VA_ARGS__)
#endif

#if defined(__has_include) && __has_include(<source_location>)
#define ATOM_FILE_LINE std::source_location::current().line()
#define ATOM_FILE_NAME std::source_location::current().file_name()
#define ATOM_FILE_LINE_NAME                                         \
    std::source_location::current().file_name() ":" std::to_string( \
        std::source_location::current().line())
#define ATOM_FUNC_NAME std::source_location::current().function_name()
#else
#define ATOM_FILE_LINE ATOM_MAKE_STRING(__LINE__)
#define ATOM_FILE_NAME __FILE__
#define ATOM_FILE_LINE_NAME __FILE__ ":" ATOM_MAKE_STRING(__LINE__)
#define ATOM_FUNC_NAME __func__
#endif

#pragma endregion

#pragma region utf-8

#if __cplusplus >= 201100L
#define ATOM_UTF8(str) u8##str
#else
#define ATOM_UTF8(str) str
#endif

#if __cpp_char8_t
#define CHAR8_T_DEFINED
#define OSTR_USE_CXX20_CHAR8_TYPE
#endif

#ifndef CHAR8_T_DEFINED  // If the user hasn't already defined these...
#if defined(EA_PLATFORM_APPLE)
#define char8_t \
    char  // The Apple debugger is too stupid to realize char8_t is typedef'd to
          // char, so we #define it.
#else
typedef char char8_t;
#endif
#define CHAR8_T_DEFINED
#endif

#pragma endregion

#pragma region typedecl

#ifdef __cplusplus
#define ATOM_DECLARE_TYPE_ID_FWD(ns, type, ctype) \
    namespace ns {                                \
    struct type;                                  \
    }                                             \
    using ctype##_t = ns::type;                   \
    using ctype##_id = ns::type*;
#else
#define ATOM_DECLARE_TYPE_ID_FWD(ns, type, ctype) \
    typedef struct ctype##_t ctype##_t;           \
    typedef struct ctype* ctype##_id;
#endif

#ifdef __cplusplus
#define ATOM_DECLARE_TYPE_ID(type, ctype) \
    typedef struct type ctype##_t;        \
    typedef type* ctype##_id;
#else
#define ATOM_DECLARE_TYPE_ID(type, ctype) \
    typedef struct ctype##_t ctype##_t;   \
    typedef ctype##_t* ctype##_id;
#endif

#pragma endregion

constexpr bool ATOM_IS_BIG_ENDIAN = false;
constexpr bool ATOM_IS_LITTLE_ENDIAN = true;

#pragma region deprecated

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define ATOM_DEPRECATED(msg) [[deprecated(msg)]]
#endif
#endif

#if !defined(ATOM_DEPRECATED)
#if defined(__GNUC__) || defined(__clang__)
#define ATOM_DEPRECATED(msg) __attribute__((deprecated))
#elif defined(_MSC_VER)
#define ATOM_DEPRECATED(msg) __declspec(deprecated)
#else
#define ATOM_DEPRECATED(msg)
#endif
#endif

#pragma endregion
