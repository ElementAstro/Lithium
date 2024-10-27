/*!
 * \file concept.hpp
 * \brief C++ Concepts
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef ATOM_META_CONCEPT_HPP
#define ATOM_META_CONCEPT_HPP

#include <concepts>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string_view>
#include <type_traits>

#if __cplusplus < 202002L
#error "C++20 is required for this library"
#endif

// -----------------------------------------------------------------------------
// Function Concepts
// -----------------------------------------------------------------------------

template <typename F, typename... Args>
concept Invocable = requires(F func, Args&&... args) {
    { std::invoke(func, std::forward<Args>(args)...) };
};

template <typename F, typename R, typename... Args>
concept InvocableR = requires(F func, Args&&... args) {
    {
        std::invoke(func, std::forward<Args>(args)...)
    } -> std::convertible_to<R>;
};

template <typename F, typename... Args>
concept NothrowInvocable = requires(F func, Args&&... args) {
    { std::invoke(func, std::forward<Args>(args)...) } noexcept;
};

template <typename F, typename R, typename... Args>
concept NothrowInvocableR = requires(F func, Args&&... args) {
    {
        std::invoke(func, std::forward<Args>(args)...)
    } noexcept -> std::convertible_to<R>;
};

template <typename T>
concept FunctionPointer = std::is_function_v<std::remove_pointer_t<T>>;

template <typename T>
concept Callable = requires(T obj) {
    { std::function{std::declval<T>()} };
};

template <typename T, typename Ret, typename... Args>
concept CallableReturns = std::is_invocable_r_v<Ret, T, Args...>;

template <typename T, typename... Args>
concept CallableNoexcept = requires(T obj, Args&&... args) {
    { obj(std::forward<Args>(args)...) } noexcept;
};

template <typename T>
concept StdFunction = requires {
    typename T::result_type;
    requires std::is_same_v<
        T, std::function<typename T::result_type(typename T::argument_type)>>;
};

// -----------------------------------------------------------------------------
// Object Concepts
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
// Type Concepts
// -----------------------------------------------------------------------------

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template <typename T>
concept SignedInteger = std::is_integral_v<T> && std::is_signed_v<T>;

template <typename T>
concept UnsignedInteger = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
concept Number = Arithmetic<T> || Integral<T> || FloatingPoint<T>;

#if __has_include(<complex>)
#include <complex>
template <typename T>
concept ComplexNumber = requires(T obj) {
    typename T::value_type;
    requires std::is_same_v<T, std::complex<typename T::value_type>>;
};
#endif

template <typename T>
concept Char = std::is_same_v<T, char>;

template <typename T>
concept WChar = std::is_same_v<T, wchar_t>;

template <typename T>
concept Char16 = std::is_same_v<T, char16_t>;

template <typename T>
concept Char32 = std::is_same_v<T, char32_t>;

template <typename T>
concept AnyChar = Char<T> || WChar<T> || Char16<T> || Char32<T>;

template <typename T>
concept StringType =
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> ||
    std::is_same_v<T, std::wstring> || std::is_same_v<T, std::u8string> ||
    std::is_same_v<T, std::u16string> || std::is_same_v<T, std::u32string>;

template <typename T>
concept IsBuiltIn = std::is_fundamental_v<T> || StringType<T>;

template <typename T>
concept Enum = std::is_enum_v<T>;

template <typename T>
concept Pointer = std::is_pointer_v<T>;

template <typename T>
concept UniquePointer = requires(T obj) {
    requires std::is_same_v<T, std::unique_ptr<typename T::element_type>>;
};

template <typename T>
concept SharedPointer = requires(T obj) {
    requires std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

template <typename T>
concept WeakPointer = requires(T obj) {
    requires std::is_same_v<T, std::weak_ptr<typename T::element_type>>;
};

template <typename T>
concept SmartPointer = UniquePointer<T> || SharedPointer<T> || WeakPointer<T>;

template <typename T>
concept Reference = std::is_reference_v<T>;

template <typename T>
concept LvalueReference = std::is_lvalue_reference_v<T>;

template <typename T>
concept RvalueReference = std::is_rvalue_reference_v<T>;

template <typename T>
concept Const = std::is_const_v<std::remove_reference_t<T>>;

template <typename T>
concept Trivial = std::is_trivial_v<T>;

template <typename T>
concept TriviallyConstructible = std::is_trivially_constructible_v<T>;

template <typename T>
concept TriviallyCopyable =
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

// -----------------------------------------------------------------------------
// Container Concepts
// -----------------------------------------------------------------------------

#if __has_include(<iterator>)
#include <iterator>

template <typename T>
concept Iterable = requires(T obj) {
    { obj.begin() } -> std::forward_iterator;
    { obj.end() } -> std::forward_iterator;
};

template <typename T>
concept Container = requires(T obj) {
    { obj.size() } -> std::convertible_to<std::size_t>;
    requires Iterable<T>;
};

template <typename T>
concept StringContainer = requires(T obj) {
    typename T::value_type;
    requires StringType<T> || Char<T>;
    { obj.push_back(std::declval<typename T::value_type>()) };
};

template <typename T>
concept NumberContainer = requires(T obj) {
    typename T::value_type;
    requires Number<typename T::value_type>;
    { obj.push_back(std::declval<typename T::value_type>()) };
};

template <typename T>
concept AssociativeContainer = requires(T obj) {
    typename T::key_type;
    typename T::mapped_type;
    requires Container<T>;
};

template <typename T>
concept Iterator = requires(T iter) {
    {
        *iter
    } -> std::convertible_to<typename std::iterator_traits<T>::value_type>;
    { ++iter } -> std::same_as<T&>;
    { iter++ } -> std::convertible_to<const T&>;
};

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
concept String = NotSequenceContainer<T> && requires(T obj) {
    { obj.size() } -> std::convertible_to<std::size_t>;
    { obj.empty() } -> std::convertible_to<bool>;
    { obj.begin() } -> std::convertible_to<typename T::iterator>;
    { obj.end() } -> std::convertible_to<typename T::iterator>;
};

// -----------------------------------------------------------------------------
// Multi-threading Concepts
// -----------------------------------------------------------------------------

template <typename T>
concept Lockable = requires(T obj) {
    { obj.lock() } -> std::same_as<void>;
    { obj.unlock() } -> std::same_as<void>;
};

template <typename T>
concept SharedLockable = requires(T obj) {
    { obj.lock_shared() } -> std::same_as<void>;
    { obj.unlock_shared() } -> std::same_as<void>;
};

template <typename T>
concept Mutex = Lockable<T> && requires(T obj) {
    { obj.try_lock() } -> std::same_as<bool>;
};

template <typename T>
concept SharedMutex = SharedLockable<T> && requires(T obj) {
    { obj.try_lock_shared() } -> std::same_as<bool>;
};

// -----------------------------------------------------------------------------
// Asynchronous Concepts
// -----------------------------------------------------------------------------

template <typename T>
concept Future = requires(T obj) {
    { obj.get() } -> std::same_as<typename T::value_type>;
    { obj.wait() } -> std::same_as<void>;
};

template <typename T>
concept Promise = requires(T obj) {
    {
        obj.set_value(std::declval<typename T::value_type>())
    } -> std::same_as<void>;
    {
        obj.set_exception(std::declval<std::exception_ptr>())
    } -> std::same_as<void>;
};

template <typename T>
concept AsyncResult = Future<T> || Promise<T>;

#endif

#endif
