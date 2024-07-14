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

//-------------------------------------------------------------------------------
// Include checks and C++ version support
//-------------------------------------------------------------------------------
#pragma region include_checks

#if __has_include(<source_location>)
#include <source_location>
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#define std experimental
#else
#error "No source_location support"
#endif

#if __cplusplus >= 202002L || _MSVC_LANG >= 202002L
#define ATOM_CPP_20_SUPPORT
#else
#error "No C++20 support"
#endif

#if __has_include(<version>)
#include <version>
#else
#error "No version support"
#endif

#pragma endregion include_checks

//-------------------------------------------------------------------------------
// Compiler and Platform checks
//-------------------------------------------------------------------------------
#pragma region compiler_checks

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
static_assert(false, "Unsupported compiler");
#endif  // source_location

#pragma endregion compiler_checks

//-------------------------------------------------------------------------------
// Common Macros
//-------------------------------------------------------------------------------
#pragma region common_macros

#define ATOM_UNUSED_RESULT(expr)                \
    {                                           \
        ALLOW_UNUSED auto unused_result = expr; \
        (void)unused_result;                    \
    }

// UNUSED
#if defined(__cplusplus)
#define ATOM_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
#define ATOM_UNUSED
#endif

#ifndef ALLOW_UNUSED
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define ALLOW_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
#define ALLOW_UNUSED __pragma(warning(suppress : 4100))
#else
#define ALLOW_UNUSED
#endif
#endif

// NORETURN
#if defined(__cplusplus)
#define ATOM_NORETURN [[noreturn]]
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define ATOM_NORETURN __declspec(noreturn)
#endif

// NODISCARD
#if defined(__cplusplus)
#define ATOM_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
#define ATOM_NODISCARD __declspec(warn_unused_result)
#endif

// C++ Specific
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
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define ATOM_ALIGNAS(x) __declspec(align(x))
#define ATOM_PACKED __pragma(pack(push, 1))
#define ATOM_PACKED_END __pragma(pack(pop))
#else
#define ATOM_ALIGNAS(x) __attribute__((aligned(x)))
#define ATOM_PACKED __attribute__((packed))
#define ATOM_PACKED_END
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
#define ATOM_DISABLE_OPTIMIZATION _Pragma("clang optimize off")
#define ATOM_ENABLE_OPTIMIZATION _Pragma("clang optimize on")
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

// IMPORT/EXPORT
#ifndef ATOM_IMPORT
#if defined(_MSC_VER)
#define ATOM_IMPORT __declspec(dllimport)
#else
#define ATOM_IMPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef ATOM_EXPORT
#if defined(_MSC_VER)
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

// UNREFERENCED PARAMETER
#if defined(_MSC_VER)
#if defined(__clang__)
#define ATOM_UNREF_PARAM(x) (void)x
#else
#define ATOM_UNREF_PARAM(x) (x)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_UNREF_PARAM(x) ((void)(x))
#endif

// CALLCONV
#if defined(_MSC_VER)
#define ATOM_CALLCONV __cdecl
#elif defined(__GNUC__) || defined(__clang__)
#define ATOM_CALLCONV
#endif

// DECLARE_ZERO
#if defined(__cplusplus)
#define ATOM_DECLARE_ZERO(type, var)                            \
    static_assert(std::is_trivially_constructible<type>::value, \
                  "not trivial, 0 init is invalid!");           \
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

// ENUM
#if defined(__cplusplus)
#define ATOM_ENUM(inttype) : inttype
#else
#define ATOM_ENUM(inttype)
#endif

#pragma endregion common_macros

//-------------------------------------------------------------------------------
// Stringizing Macros
//-------------------------------------------------------------------------------
#pragma region stringizing_macros

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

#pragma endregion stringizing_macros

//-------------------------------------------------------------------------------
// UTF-8 Macros
//-------------------------------------------------------------------------------
#pragma region utf8_macros

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

#pragma endregion utf8_macros

//-------------------------------------------------------------------------------
// Type Declaration Macros
//-------------------------------------------------------------------------------
#pragma region typedecl_macros

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
    typedef(type) * ctype##_id;
#else
#define ATOM_DECLARE_TYPE_ID(type, ctype) \
    typedef struct ctype##_t ctype##_t;   \
    typedef ctype##_t* ctype##_id;
#endif

#pragma endregion typedecl_macros

//-------------------------------------------------------------------------------
// Endianness
//-------------------------------------------------------------------------------
constexpr bool ATOM_IS_BIG_ENDIAN = false;
constexpr bool ATOM_IS_LITTLE_ENDIAN = true;

//-------------------------------------------------------------------------------
// Deprecated
//-------------------------------------------------------------------------------
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

#pragma endregion deprecated

//-------------------------------------------------------------------------------
// Unreachable
//-------------------------------------------------------------------------------
#pragma region unreachable

namespace atom::meta {
ATOM_INLINE void unreachable ATOM_NORETURN() {
#if __GNUC__ || __clang__  // GCC, Clang, ICC
    __builtin_unreachable();
#elif _MSC_VER  // MSVC
    __assume(false);
#endif
}
}  // namespace atom::meta

#pragma endregion unreachable

//-------------------------------------------------------------------------------
// More Useful Macros
//-------------------------------------------------------------------------------
#pragma region more_macros

// Likely/Unlikely
#if defined(__GNUC__) || defined(__clang__)
#define ATOM_LIKELY(x) __builtin_expect(!!(x), 1)
#define ATOM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define ATOM_LIKELY(x) (x)
#define ATOM_UNLIKELY(x) (x)
#endif

// Static Assert
#ifndef ATOM_STATIC_ASSERT
#if defined(__cplusplus)
#define ATOM_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
#define ATOM_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif
#endif

// Debug Break
#if defined(_MSC_VER)
#define ATOM_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#include <csignal>
#define ATOM_DEBUG_BREAK() raise(SIGTRAP)
#else
#define ATOM_DEBUG_BREAK()
#endif

#define ATOM_C extern "C"

#pragma endregion more_macros
