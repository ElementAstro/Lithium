/*!
 * \file god.hpp
 * \brief Some useful functions, inspired by Coost
 * \author Max Qian <lightapt.com>
 * \date 2023-06-17
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_GOD_HPP
#define ATOM_META_GOD_HPP

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

namespace atom::meta {

// No-op function for blessing with no bugs
inline void blessNoBugs() {}

template <typename To, typename From>
constexpr To cast(From&& f) {
    return static_cast<To>(std::forward<From>(f));
}

template <std::size_t A, typename X>
constexpr X alignUp(X x) {
    static_assert((A & (A - 1)) == 0, "A must be power of 2");
    return (x + static_cast<X>(A - 1)) & ~static_cast<X>(A - 1);
}

template <std::size_t A, typename X>
constexpr X* alignUp(X* x) {
    return reinterpret_cast<X*>(alignUp<A>(reinterpret_cast<std::size_t>(x)));
}

template <typename X, typename A>
constexpr X alignUp(X x, A a) {
    static_assert(std::is_integral<A>::value, "A must be integral type");
    return (x + static_cast<X>(a - 1)) & ~static_cast<X>(a - 1);
}

template <typename X, typename A>
constexpr X* alignUp(X* x, A a) {
    return reinterpret_cast<X*>(alignUp(reinterpret_cast<std::size_t>(x), a));
}

template <std::size_t A, typename X>
constexpr X alignDown(X x) {
    static_assert((A & (A - 1)) == 0, "A must be power of 2");
    return x & ~static_cast<X>(A - 1);
}

template <std::size_t A, typename X>
constexpr X* alignDown(X* x) {
    return reinterpret_cast<X*>(alignDown<A>(reinterpret_cast<std::size_t>(x)));
}

template <typename X, typename A>
constexpr X alignDown(X x, A a) {
    static_assert(std::is_integral<A>::value, "A must be integral type");
    return x & ~static_cast<X>(a - 1);
}

template <typename X, typename A>
constexpr X* alignDown(X* x, A a) {
    return reinterpret_cast<X*>(alignDown(reinterpret_cast<std::size_t>(x), a));
}

template <typename T>
constexpr T log2(T x) {
    static_assert(std::is_integral<T>::value, "T must be integral type");
    return x <= 1 ? 0 : 1 + atom::meta::log2(x >> 1);
}

template <std::size_t N, typename X>
constexpr X nb(X x) {
    static_assert((N & (N - 1)) == 0, "N must be power of 2");
    return (x >> atom::meta::log2(static_cast<X>(N))) + !!(x & static_cast<X>(N - 1));
}

template <typename T>
inline bool eq(const void* p, const void* q) {
    return *reinterpret_cast<const T*>(p) == *reinterpret_cast<const T*>(q);
}

template <std::size_t N>
inline void copy(void* dst, const void* src) {
    if constexpr (N > 0) {
        std::memcpy(dst, src, N);
    }
}

template <>
inline void copy<0>(void*, const void*) {}

template <typename T, typename V>
inline T swap(T* p, V v) {
    T x = *p;
    *p = static_cast<T>(v);
    return x;
}

template <typename T, typename V>
inline T fetchAdd(T* p, V v) {
    T x = *p;
    *p += v;
    return x;
}

template <typename T, typename V>
inline T fetchSub(T* p, V v) {
    T x = *p;
    *p -= v;
    return x;
}

template <typename T, typename V>
inline T fetchAnd(T* p, V v) {
    T x = *p;
    *p &= static_cast<T>(v);
    return x;
}

template <typename T, typename V>
inline T fetchOr(T* p, V v) {
    T x = *p;
    *p |= static_cast<T>(v);
    return x;
}

template <typename T, typename V>
inline T fetchXor(T* p, V v) {
    T x = *p;
    *p ^= static_cast<T>(v);
    return x;
}

template <bool C, typename T = void>
using if_t = std::enable_if_t<C, T>;

template <typename T>
using rmRefT = std::remove_reference_t<T>;

template <typename T>
using rmCvT = std::remove_cv_t<T>;

template <typename T>
using rmCvRefT = rmCvT<rmRefT<T>>;

template <typename T>
using rmArrT = std::remove_extent_t<T>;

template <typename T>
using constT = std::add_const_t<T>;

template <typename T>
using constRefT = std::add_lvalue_reference_t<constT<rmRefT<T>>>;

namespace detail {
template <typename... T>
struct isSame {
    static constexpr bool value = false;
};

template <typename T, typename U, typename... X>
struct isSame<T, U, X...> {
    static constexpr bool value =
        std::is_same_v<T, U> || isSame<T, X...>::value;
};
}  // namespace detail

template <typename T, typename U, typename... X>
constexpr bool isSame() {
    return detail::isSame<T, U, X...>::value;
}

template <typename T>
constexpr bool isRef() {
    return std::is_reference_v<T>;
}

template <typename T>
constexpr bool isArray() {
    return std::is_array_v<T>;
}

template <typename T>
constexpr bool isClass() {
    return std::is_class_v<T>;
}

template <typename T>
constexpr bool isScalar() {
    return std::is_scalar_v<T>;
}

template <typename T>
constexpr bool isTriviallyCopyable() {
    return std::is_trivially_copyable_v<T>;
}

template <typename T>
constexpr bool isTriviallyDestructible() {
    return std::is_trivially_destructible_v<T>;
}

template <typename B, typename D>
constexpr bool isBaseOf() {
    return std::is_base_of_v<B, D>;
}

template <typename T>
constexpr bool hasVirtualDestructor() {
    return std::has_virtual_destructor_v<T>;
}

}  // namespace atom::meta

#endif