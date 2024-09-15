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
#include "atom/macro.hpp"

namespace atom::meta {
/*!
 * \brief No-op function for blessing with no bugs.
 */
ATOM_INLINE void blessNoBugs() {}

/*!
 * \brief Casts a value from one type to another.
 * \tparam To The type to cast to.
 * \tparam From The type to cast from.
 * \param f The value to cast.
 * \return The casted value.
 */
template <typename To, typename From>
constexpr auto cast(From&& f) -> To {
    return static_cast<To>(std::forward<From>(f));
}

/*!
 * \brief Aligns a value up to the nearest multiple of A.
 * \tparam A The alignment value, must be a power of 2.
 * \tparam X The type of the value to align.
 * \param x The value to align.
 * \return The aligned value.
 */
template <std::size_t A, typename X>
constexpr auto alignUp(X x) -> X {
    static_assert((A & (A - 1)) == 0, "A must be power of 2");
    return (x + static_cast<X>(A - 1)) & ~static_cast<X>(A - 1);
}

/*!
 * \brief Aligns a pointer up to the nearest multiple of A.
 * \tparam A The alignment value, must be a power of 2.
 * \tparam X The type of the pointer to align.
 * \param x The pointer to align.
 * \return The aligned pointer.
 */
template <std::size_t A, typename X>
constexpr auto alignUp(X* x) -> X* {
    return reinterpret_cast<X*>(alignUp<A>(reinterpret_cast<std::size_t>(x)));
}

/*!
 * \brief Aligns a value up to the nearest multiple of a.
 * \tparam X The type of the value to align.
 * \tparam A The type of the alignment value, must be integral.
 * \param x The value to align.
 * \param a The alignment value.
 * \return The aligned value.
 */
template <typename X, typename A>
constexpr auto alignUp(X x, A a) -> X {
    static_assert(std::is_integral<A>::value, "A must be integral type");
    return (x + static_cast<X>(a - 1)) & ~static_cast<X>(a - 1);
}

/*!
 * \brief Aligns a pointer up to the nearest multiple of a.
 * \tparam X The type of the pointer to align.
 * \tparam A The type of the alignment value, must be integral.
 * \param x The pointer to align.
 * \param a The alignment value.
 * \return The aligned pointer.
 */
template <typename X, typename A>
constexpr auto alignUp(X* x, A a) -> X* {
    return reinterpret_cast<X*>(alignUp(reinterpret_cast<std::size_t>(x), a));
}

/*!
 * \brief Aligns a value down to the nearest multiple of A.
 * \tparam A The alignment value, must be a power of 2.
 * \tparam X The type of the value to align.
 * \param x The value to align.
 * \return The aligned value.
 */
template <std::size_t A, typename X>
constexpr auto alignDown(X x) -> X {
    static_assert((A & (A - 1)) == 0, "A must be power of 2");
    return x & ~static_cast<X>(A - 1);
}

/*!
 * \brief Aligns a pointer down to the nearest multiple of A.
 * \tparam A The alignment value, must be a power of 2.
 * \tparam X The type of the pointer to align.
 * \param x The pointer to align.
 * \return The aligned pointer.
 */
template <std::size_t A, typename X>
constexpr auto alignDown(X* x) -> X* {
    return reinterpret_cast<X*>(alignDown<A>(reinterpret_cast<std::size_t>(x)));
}

/*!
 * \brief Aligns a value down to the nearest multiple of a.
 * \tparam X The type of the value to align.
 * \tparam A The type of the alignment value, must be integral.
 * \param x The value to align.
 * \param a The alignment value.
 * \return The aligned value.
 */
template <typename X, typename A>
constexpr auto alignDown(X x, A a) -> X {
    static_assert(std::is_integral<A>::value, "A must be integral type");
    return x & ~static_cast<X>(a - 1);
}

/*!
 * \brief Aligns a pointer down to the nearest multiple of a.
 * \tparam X The type of the pointer to align.
 * \tparam A The type of the alignment value, must be integral.
 * \param x The pointer to align.
 * \param a The alignment value.
 * \return The aligned pointer.
 */
template <typename X, typename A>
constexpr auto alignDown(X* x, A a) -> X* {
    return reinterpret_cast<X*>(alignDown(reinterpret_cast<std::size_t>(x), a));
}

/*!
 * \brief Computes the base-2 logarithm of an integral value.
 * \tparam T The type of the value, must be integral.
 * \param x The value to compute the logarithm of.
 * \return The base-2 logarithm of the value.
 */
template <typename T>
constexpr auto log2(T x) -> T {
    static_assert(std::is_integral<T>::value, "T must be integral type");
    return x <= 1 ? 0 : 1 + atom::meta::log2(x >> 1);
}

/*!
 * \brief Computes the number of blocks of size N needed to cover a value.
 * \tparam N The block size, must be a power of 2.
 * \tparam X The type of the value.
 * \param x The value to compute the number of blocks for.
 * \return The number of blocks needed to cover the value.
 */
template <std::size_t N, typename X>
constexpr auto nb(X x) -> X {
    static_assert((N & (N - 1)) == 0, "N must be power of 2");
    return (x >> atom::meta::log2(static_cast<X>(N))) +
           !!(x & static_cast<X>(N - 1));
}

/*!
 * \brief Compares two values for equality.
 * \tparam T The type of the values.
 * \param p Pointer to the first value.
 * \param q Pointer to the second value.
 * \return True if the values are equal, false otherwise.
 */
template <typename T>
ATOM_INLINE auto eq(const void* p, const void* q) -> bool {
    return *reinterpret_cast<const T*>(p) == *reinterpret_cast<const T*>(q);
}

/*!
 * \brief Copies N bytes from src to dst.
 * \tparam N The number of bytes to copy.
 * \param dst Pointer to the destination.
 * \param src Pointer to the source.
 */
template <std::size_t N>
ATOM_INLINE void copy(void* dst, const void* src) {
    if constexpr (N > 0) {
        std::memcpy(dst, src, N);
    }
}

/*!
 * \brief Specialization of copy for N = 0, does nothing.
 */
template <>
ATOM_INLINE void copy<0>(void*, const void*) {}

/*!
 * \brief Swaps the value pointed to by p with v.
 * \tparam T The type of the value pointed to by p.
 * \tparam V The type of the value v.
 * \param p Pointer to the value to swap.
 * \param v The value to swap with.
 * \return The original value pointed to by p.
 */
template <typename T, typename V>
ATOM_INLINE auto swap(T* p, V v) -> T {
    T x = *p;
    *p = static_cast<T>(v);
    return x;
}

/*!
 * \brief Adds v to the value pointed to by p and returns the original value.
 * \tparam T The type of the value pointed to by p.
 * \tparam V The type of the value v.
 * \param p Pointer to the value to add to.
 * \param v The value to add.
 * \return The original value pointed to by p.
 */
template <typename T, typename V>
ATOM_INLINE auto fetchAdd(T* p, V v) -> T {
    T x = *p;
    *p += v;
    return x;
}

/*!
 * \brief Subtracts v from the value pointed to by p and returns the original
 * value. \tparam T The type of the value pointed to by p. \tparam V The type of
 * the value v. \param p Pointer to the value to subtract from. \param v The
 * value to subtract. \return The original value pointed to by p.
 */
template <typename T, typename V>
ATOM_INLINE auto fetchSub(T* p, V v) -> T {
    T x = *p;
    *p -= v;
    return x;
}

/*!
 * \brief Performs a bitwise AND between the value pointed to by p and v, and
 * returns the original value. \tparam T The type of the value pointed to by p.
 * \tparam V The type of the value v.
 * \param p Pointer to the value to AND.
 * \param v The value to AND with.
 * \return The original value pointed to by p.
 */
template <typename T, typename V>
ATOM_INLINE auto fetchAnd(T* p, V v) -> T {
    T x = *p;
    *p &= static_cast<T>(v);
    return x;
}

/*!
 * \brief Performs a bitwise OR between the value pointed to by p and v, and
 * returns the original value. \tparam T The type of the value pointed to by p.
 * \tparam V The type of the value v.
 * \param p Pointer to the value to OR.
 * \param v The value to OR with.
 * \return The original value pointed to by p.
 */
template <typename T, typename V>
ATOM_INLINE auto fetchOr(T* p, V v) -> T {
    T x = *p;
    *p |= static_cast<T>(v);
    return x;
}

/*!
 * \brief Performs a bitwise XOR between the value pointed to by p and v, and
 * returns the original value. \tparam T The type of the value pointed to by p.
 * \tparam V The type of the value v.
 * \param p Pointer to the value to XOR.
 * \param v The value to XOR with.
 * \return The original value pointed to by p.
 */
template <typename T, typename V>
ATOM_INLINE auto fetchXor(T* p, V v) -> T {
    T x = *p;
    *p ^= static_cast<T>(v);
    return x;
}

/*!
 * \brief Alias for std::enable_if_t.
 * \tparam C The condition.
 * \tparam T The type to enable if the condition is true.
 */
template <bool C, typename T = void>
using if_t = std::enable_if_t<C, T>;

/*!
 * \brief Alias for std::remove_reference_t.
 * \tparam T The type to remove reference from.
 */
template <typename T>
using rmRefT = std::remove_reference_t<T>;

/*!
 * \brief Alias for std::remove_cv_t.
 * \tparam T The type to remove const and volatile qualifiers from.
 */
template <typename T>
using rmCvT = std::remove_cv_t<T>;

/*!
 * \brief Alias for removing both const, volatile qualifiers and reference.
 * \tparam T The type to remove const, volatile qualifiers and reference from.
 */
template <typename T>
using rmCvRefT = rmCvT<rmRefT<T>>;

/*!
 * \brief Alias for std::remove_extent_t.
 * \tparam T The type to remove extent from.
 */
template <typename T>
using rmArrT = std::remove_extent_t<T>;

/*!
 * \brief Alias for std::add_const_t.
 * \tparam T The type to add const qualifier to.
 */
template <typename T>
using constT = std::add_const_t<T>;

/*!
 * \brief Alias for adding const qualifier and lvalue reference.
 * \tparam T The type to add const qualifier and lvalue reference to.
 */
template <typename T>
using constRefT = std::add_lvalue_reference_t<constT<rmRefT<T>>>;

namespace detail {

/*!
 * \brief Helper struct to check if all types are the same.
 * \tparam T The types to check.
 */
template <typename... T>
struct isSame {
    static constexpr bool value = false;
};

/*!
 * \brief Specialization of isSame for two or more types.
 * \tparam T The first type.
 * \tparam U The second type.
 * \tparam X The remaining types.
 */
template <typename T, typename U, typename... X>
struct isSame<T, U, X...> {
    static constexpr bool value =
        std::is_same_v<T, U> || isSame<T, X...>::value;
};

}  // namespace detail

/*!
 * \brief Checks if all types are the same.
 * \tparam T The first type.
 * \tparam U The second type.
 * \tparam X The remaining types.
 * \return True if all types are the same, false otherwise.
 */
template <typename T, typename U, typename... X>
constexpr auto isSame() -> bool {
    return detail::isSame<T, U, X...>::value;
}

/*!
 * \brief Checks if a type is a reference.
 * \tparam T The type to check.
 * \return True if the type is a reference, false otherwise.
 */
template <typename T>
constexpr auto isRef() -> bool {
    return std::is_reference_v<T>;
}

/*!
 * \brief Checks if a type is an array.
 * \tparam T The type to check.
 * \return True if the type is an array, false otherwise.
 */
template <typename T>
constexpr auto isArray() -> bool {
    return std::is_array_v<T>;
}

/*!
 * \brief Checks if a type is a class.
 * \tparam T The type to check.
 * \return True if the type is a class, false otherwise.
 */
template <typename T>
constexpr auto isClass() -> bool {
    return std::is_class_v<T>;
}

/*!
 * \brief Checks if a type is a scalar.
 * \tparam T The type to check.
 * \return True if the type is a scalar, false otherwise.
 */
template <typename T>
constexpr auto isScalar() -> bool {
    return std::is_scalar_v<T>;
}

/*!
 * \brief Checks if a type is trivially copyable.
 * \tparam T The type to check.
 * \return True if the type is trivially copyable, false otherwise.
 */
template <typename T>
constexpr auto isTriviallyCopyable() -> bool {
    return std::is_trivially_copyable_v<T>;
}

/*!
 * \brief Checks if a type is trivially destructible.
 * \tparam T The type to check.
 * \return True if the type is trivially destructible, false otherwise.
 */
template <typename T>
constexpr auto isTriviallyDestructible() -> bool {
    return std::is_trivially_destructible_v<T>;
}

/*!
 * \brief Checks if a type is a base of another type.
 * \tparam B The base type.
 * \tparam D The derived type.
 * \return True if B is a base of D, false otherwise.
 */
template <typename B, typename D>
constexpr auto isBaseOf() -> bool {
    return std::is_base_of_v<B, D>;
}

/*!
 * \brief Checks if a type has a virtual destructor.
 * \tparam T The type to check.
 * \return True if the type has a virtual destructor, false otherwise.
 */
template <typename T>
constexpr auto hasVirtualDestructor() -> bool {
    return std::has_virtual_destructor_v<T>;
}

}  // namespace atom::meta

#endif
