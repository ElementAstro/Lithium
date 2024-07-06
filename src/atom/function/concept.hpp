/*!
 * \file concept.hpp
 * \brief C++ Concepts
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_CONCEPT_HPP
#define ATOM_META_CONCEPT_HPP

#if __cplusplus < 202002L
#error "C++20 is required for this library"
#endif

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------

template <typename T>
concept Function = requires(T t) {
    { t() } -> std::same_as<void>;
};

// Checks if a type can be invoked with specified argument types
template <typename F, typename... Args>
concept Invocable = requires(F f, Args&&... args) {
    { std::invoke(f, std::forward<Args>(args)...) };
};

// Checks if a type can be invoked with specified argument types and returns a
// result convertible to R
template <typename F, typename R, typename... Args>
concept InvocableR = requires(F f, Args&&... args) {
    { std::invoke(f, std::forward<Args>(args)...) } -> std::convertible_to<R>;
};

// Similar to Invocable but checks for noexcept
template <typename F, typename... Args>
concept NothrowInvocable = requires(F f, Args&&... args) {
    { std::invoke(f, std::forward<Args>(args)...) } noexcept;
};

// Similar to InvocableR but checks for noexcept
template <typename F, typename R, typename... Args>
concept NothrowInvocableR = requires(F f, Args&&... args) {
    {
        std::invoke(f, std::forward<Args>(args)...)
    } noexcept -> std::convertible_to<R>;
};

// Checks if a type is a function pointer
template <typename T>
concept FunctionPointer = std::is_function_v<std::remove_pointer_t<T>>;

// Checks if a callable type has a specific return type
template <typename T, typename Ret, typename... Args>
concept CallableReturns = std::is_invocable_r_v<Ret, T, Args...>;

// Checks if a callable type can be invoked with a given set of arguments and is
// noexcept
template <typename T, typename... Args>
concept CallableNoexcept = requires(T t, Args&&... args) {
    { t(std::forward<Args>(args)...) } noexcept;
};

// Checks if a type is a std::function of any signature
template <typename T>
concept StdFunction = requires {
    typename T::result_type;
    requires std::is_same_v<
        T, std::function<typename T::result_type(typename T::argument_type)>>;
};

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------

template <typename T>
concept Relocatable = requires(T t) {
    { std::is_nothrow_move_constructible_v<T> } -> std::convertible_to<bool>;
    { std::is_nothrow_move_assignable_v<T> } -> std::convertible_to<bool>;
};

template <typename T>
concept DefaultConstructible = requires(T t) {
    { T() } -> std::same_as<T>;
};

template <typename T>
concept CopyConstructible = requires(T t) {
    { T(t) } -> std::same_as<T>;
};

template <typename T>
concept CopyAssignable = requires(T t) {
    { t = t } -> std::same_as<T&>;
};

template <typename T>
concept MoveAssignable = requires(T t) {
    { t = std::move(t) } -> std::same_as<T&>;
};

template <typename T>
concept EqualityComparable = requires(T t) {
    { t == t } -> std::convertible_to<bool>;
    { t != t } -> std::convertible_to<bool>;
};

template <typename T>
concept LessThanComparable = requires(T t) {
    { t < t } -> std::convertible_to<bool>;
};

template <typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Swappable = requires(T t) { std::swap(t, t); };

template <typename T>
concept Copyable =
    std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;

template <typename T>
concept Destructible = requires(T t) {
    { t.~T() } -> std::same_as<void>;
};

// -----------------------------------------------------------------------------
// Type
// -----------------------------------------------------------------------------

// Checks if a type is an arithmetic type (integer or floating point)
template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

// Checks if a type is an integral type
template <typename T>
concept Integral = std::is_integral_v<T>;

// Checks if a type is a floating-point type
template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

// Checks if a type is a signed integer
template <typename T>
concept SignedInteger = std::is_integral_v<T> && std::is_signed_v<T>;

// Checks if a type is an unsigned integer
template <typename T>
concept UnsignedInteger = std::is_integral_v<T> && std::is_unsigned_v<T>;

#if __has_include(<complex>)
#include <complex>
// Checks if a type is a complex number (in <complex> header)
template <typename T>
concept ComplexNumber = requires(T x) {
    typename T::value_type;
    requires std::is_same_v<T, std::complex<typename T::value_type>>;
};
#endif

// Checks if a type is char
template <typename T>
concept Char = std::is_same_v<T, char>;

// Checks if a type is wchar_t
template <typename T>
concept WChar = std::is_same_v<T, wchar_t>;

// Checks if a type is char16_t
template <typename T>
concept Char16 = std::is_same_v<T, char16_t>;

// Checks if a type is char32_t
template <typename T>
concept Char32 = std::is_same_v<T, char32_t>;

// Checks if a type is any character type
template <typename T>
concept AnyChar = Char<T> || WChar<T> || Char16<T> || Char32<T>;

// Checks if a type is an enum
template <typename T>
concept Enum = std::is_enum_v<T>;

// Checks if a type is a pointer
template <typename T>
concept Pointer = std::is_pointer_v<T>;

// Checks if a type is a std::unique_ptr of any type
template <typename T>
concept UniquePointer = requires(T x) {
    requires std::is_same_v<T, std::unique_ptr<typename T::element_type>>;
};

// Checks if a type is a std::shared_ptr of any type
template <typename T>
concept SharedPointer = requires(T x) {
    requires std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

// Checks if a type is a std::weak_ptr of any type
template <typename T>
concept WeakPointer = requires(T x) {
    requires std::is_same_v<T, std::weak_ptr<typename T::element_type>>;
};

// Checks if a type is a reference
template <typename T>
concept Reference = std::is_reference_v<T>;

// Checks if a type is a lvalue reference
template <typename T>
concept LvalueReference = std::is_lvalue_reference_v<T>;

// Checks if a type is a rvalue reference
template <typename T>
concept RvalueReference = std::is_rvalue_reference_v<T>;

// Checks if a type is const-qualified
template <typename T>
concept Const = std::is_const_v<std::remove_reference_t<T>>;

// Checks if a type is trivial
template <typename T>
concept Trivial = std::is_trivial_v<T>;

// Checks if a type is trivially constructible
template <typename T>
concept TriviallyConstructible = std::is_trivially_constructible_v<T>;

template <typename T>
concept TriviallyCopyable =
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

#if __has_include(<iterator>)
#include <iterator>

// Checks if a type supports begin() and end()
template <typename T>
concept Iterable = requires(T x) {
    { x.begin() } -> std::forward_iterator;
    { x.end() } -> std::forward_iterator;
};

// Checks if a type is a standard container with size, begin, and end
template <typename T>
concept Container = requires(T x) {
    { x.size() } -> std::convertible_to<std::size_t>;
    requires Iterable<T>;
};

// Checks if a type is an associative container like map or set
template <typename T>
concept AssociativeContainer = requires(T x) {
    typename T::key_type;
    typename T::mapped_type;
    requires Container<T>;
};

// Concept to check if a type is an iterator
template <typename T>
concept Iterator = requires(T it) {
    {
        *it
    } -> std::convertible_to<typename std::iterator_traits<T>::value_type>;
    { ++it } -> std::same_as<T&>;
    { it++ } -> std::convertible_to<const T&>;
};
#endif

#endif
