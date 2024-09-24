/*!
 * \file concept.hpp
 * \brief C++ Concepts
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_CONCEPT_HPP
#define ATOM_META_CONCEPT_HPP

#include <deque>
#include <list>
#include <map>
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

// Checks if a type can be invoked with specified argument types
template <typename F, typename... Args>
concept Invocable = requires(F func, Args&&... args) {
    { std::invoke(func, std::forward<Args>(args)...) };
};

// Checks if a type can be invoked with specified argument types and returns a
// result convertible to R
template <typename F, typename R, typename... Args>
concept InvocableR = requires(F func, Args&&... args) {
    {
        std::invoke(func, std::forward<Args>(args)...)
    } -> std::convertible_to<R>;
};

// Similar to Invocable but checks for noexcept
template <typename F, typename... Args>
concept NothrowInvocable = requires(F func, Args&&... args) {
    { std::invoke(func, std::forward<Args>(args)...) } noexcept;
};

// Similar to InvocableR but checks for noexcept
template <typename F, typename R, typename... Args>
concept NothrowInvocableR = requires(F func, Args&&... args) {
    {
        std::invoke(func, std::forward<Args>(args)...)
    } noexcept -> std::convertible_to<R>;
};

// Checks if a type is a function pointer
template <typename T>
concept FunctionPointer = std::is_function_v<std::remove_pointer_t<T>>;

template <typename T>
concept Callable = requires(T obj) {
    { std::function{std::declval<T>()} };
};

// Checks if a callable type has a specific return type
template <typename T, typename Ret, typename... Args>
concept CallableReturns = std::is_invocable_r_v<Ret, T, Args...>;

// Checks if a callable type can be invoked with a given set of arguments and is
// noexcept
template <typename T, typename... Args>
concept CallableNoexcept = requires(T obj, Args&&... args) {
    { obj(std::forward<Args>(args)...) } noexcept;
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
concept Relocatable = requires(T obj) {
    { std::is_nothrow_move_constructible_v<T> } -> std::convertible_to<bool>;
    { std::is_nothrow_move_assignable_v<T> } -> std::convertible_to<bool>;
};

template <typename T>
concept DefaultConstructible = requires(T obj) {
    { T() } -> std::same_as<T>;
};

template <typename T>
concept CopyConstructible = requires(T obj) {
    { T(obj) } -> std::same_as<T>;
};

template <typename T>
concept CopyAssignable = requires(T obj) {
    { obj = obj } -> std::same_as<T&>;
};

template <typename T>
concept MoveAssignable = requires(T obj) {
    { obj = std::move(obj) } -> std::same_as<T&>;
};

template <typename T>
concept EqualityComparable = requires(T obj) {
    { obj == obj } -> std::convertible_to<bool>;
    { obj != obj } -> std::convertible_to<bool>;
};

template <typename T>
concept LessThanComparable = requires(T obj) {
    { obj < obj } -> std::convertible_to<bool>;
};

template <typename T>
concept Hashable = requires(T obj) {
    { std::hash<T>{}(obj) } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Swappable = requires(T obj) { std::swap(obj, obj); };

template <typename T>
concept Copyable =
    std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;

template <typename T>
concept Destructible = requires(T obj) {
    { obj.~T() } -> std::same_as<void>;
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

template <typename T>
concept Number = Arithmetic<T> || Integral<T> || FloatingPoint<T>;

#if __has_include(<complex>)
#include <complex>
// Checks if a type is a complex number (in <complex> header)
template <typename T>
concept ComplexNumber = requires(T obj) {
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

template <typename T>
concept NotSequenceContainer =
    !std::is_same_v<T, std::vector<typename T::value_type>> &&
    !std::is_same_v<T, std::list<typename T::value_type>> &&
    !std::is_same_v<T, std::deque<typename T::value_type>>;

template <typename T>
concept NotAssociativeOrSequenceContainer =
    !std::is_same_v<T,
                    std::map<typename T::key_type, typename T::mapped_type>> &&
    !std::is_same_v<
        T, std::unordered_map<typename T::key_type, typename T::mapped_type>> &&
    !std::is_same_v<
        T, std::multimap<typename T::key_type, typename T::mapped_type>> &&
    !std::is_same_v<T, std::unordered_multimap<typename T::key_type,
                                               typename T::mapped_type>> &&
    !NotSequenceContainer<T>;

template <typename T>
concept String = NotAssociativeOrSequenceContainer<T> && requires(T obj) {
    { obj.size() } -> std::convertible_to<std::size_t>;
    { obj.empty() } -> std::convertible_to<bool>;
    { obj.begin() } -> std::convertible_to<typename T::iterator>;
    { obj.end() } -> std::convertible_to<typename T::iterator>;
};

template <typename T>
concept IsBuiltIn = std::is_fundamental_v<T> || String<T>;

// Checks if a type is an enum
template <typename T>
concept Enum = std::is_enum_v<T>;

// Checks if a type is a pointer
template <typename T>
concept Pointer = std::is_pointer_v<T>;

// Checks if a type is a std::unique_ptr of any type
template <typename T>
concept UniquePointer = requires(T obj) {
    requires std::is_same_v<T, std::unique_ptr<typename T::element_type>>;
};

// Checks if a type is a std::shared_ptr of any type
template <typename T>
concept SharedPointer = requires(T obj) {
    requires std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

// Checks if a type is a std::weak_ptr of any type
template <typename T>
concept WeakPointer = requires(T obj) {
    requires std::is_same_v<T, std::weak_ptr<typename T::element_type>>;
};

template <typename T>
concept SmartPointer = UniquePointer<T> || SharedPointer<T> || WeakPointer<T>;

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
concept Iterable = requires(T obj) {
    { obj.begin() } -> std::forward_iterator;
    { obj.end() } -> std::forward_iterator;
};

// Checks if a type is a standard container with size, begin, and end
template <typename T>
concept Container = requires(T obj) {
    { obj.size() } -> std::convertible_to<std::size_t>;
    requires Iterable<T>;
};

template <typename T>
concept StringContainer = requires(T obj) {
    typename T::value_type;
    requires String<T> || Char<T>;
    { obj.push_back(std::declval<typename T::value_type>()) };
};

template <typename T>
concept NumberContainer = requires(T obj) {
    typename T::value_type;
    requires Number<typename T::value_type>;
    { obj.push_back(std::declval<typename T::value_type>()) };
};

// Checks if a type is an associative container like map or set
template <typename T>
concept AssociativeContainer = requires(T obj) {
    typename T::key_type;
    typename T::mapped_type;
    requires Container<T>;
};

// Concept to check if a type is an iterator
template <typename T>
concept Iterator = requires(T iter) {
    {
        *iter
    } -> std::convertible_to<typename std::iterator_traits<T>::value_type>;
    { ++iter } -> std::same_as<T&>;
    { iter++ } -> std::convertible_to<const T&>;
};
#endif

#endif
