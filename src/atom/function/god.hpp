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

#include "atom/atom/macro.hpp"

namespace atom::meta {
/*!
 * \brief No-op function for blessing with no bugs.
 */
ATOM_INLINE void blessNoBugs() {}

/*!
 * \brief Casts a value from one type to another.
 * \tparam To The type to cast to.
 * \tparam From The type to cast from.
 * \param fromValue The value to cast.
 * \return The casted value.
 */
template <typename To, typename From>
constexpr auto cast(From&& fromValue) -> To {
    return static_cast<To>(std::forward<From>(fromValue));
}

/*!
 * \brief Aligns a value up to the nearest multiple of A.
 * \tparam Alignment The alignment value, must be a power of 2.
 * \tparam ValueType The type of the value to align.
 * \param value The value to align.
 * \return The aligned value.
 */
template <std::size_t Alignment, typename ValueType>
constexpr auto alignUp(ValueType value) -> ValueType {
    static_assert((Alignment & (Alignment - 1)) == 0,
                  "Alignment must be power of 2");
    return (value + static_cast<ValueType>(Alignment - 1)) &
           ~static_cast<ValueType>(Alignment - 1);
}

/*!
 * \brief Aligns a pointer up to the nearest multiple of A.
 * \tparam Alignment The alignment value, must be a power of 2.
 * \tparam PointerType The type of the pointer to align.
 * \param pointer The pointer to align.
 * \return The aligned pointer.
 */
template <std::size_t Alignment, typename PointerType>
constexpr auto alignUp(PointerType* pointer) -> PointerType* {
    return reinterpret_cast<PointerType*>(
        alignUp<Alignment>(reinterpret_cast<std::size_t>(pointer)));
}

/*!
 * \brief Aligns a value up to the nearest multiple of alignment.
 * \tparam ValueType The type of the value to align.
 * \tparam AlignmentType The type of the alignment value, must be integral.
 * \param value The value to align.
 * \param alignment The alignment value.
 * \return The aligned value.
 */
template <typename ValueType, typename AlignmentType>
constexpr auto alignUp(ValueType value, AlignmentType alignment) -> ValueType {
    static_assert(std::is_integral<AlignmentType>::value,
                  "Alignment must be integral type");
    return (value + static_cast<ValueType>(alignment - 1)) &
           ~static_cast<ValueType>(alignment - 1);
}

/*!
 * \brief Aligns a pointer up to the nearest multiple of alignment.
 * \tparam PointerType The type of the pointer to align.
 * \tparam AlignmentType The type of the alignment value, must be integral.
 * \param pointer The pointer to align.
 * \param alignment The alignment value.
 * \return The aligned pointer.
 */
template <typename PointerType, typename AlignmentType>
constexpr auto alignUp(PointerType* pointer,
                       AlignmentType alignment) -> PointerType* {
    return reinterpret_cast<PointerType*>(
        alignUp(reinterpret_cast<std::size_t>(pointer), alignment));
}

/*!
 * \brief Aligns a value down to the nearest multiple of A.
 * \tparam Alignment The alignment value, must be a power of 2.
 * \tparam ValueType The type of the value to align.
 * \param value The value to align.
 * \return The aligned value.
 */
template <std::size_t Alignment, typename ValueType>
constexpr auto alignDown(ValueType value) -> ValueType {
    static_assert((Alignment & (Alignment - 1)) == 0,
                  "Alignment must be power of 2");
    return value & ~static_cast<ValueType>(Alignment - 1);
}

/*!
 * \brief Aligns a pointer down to the nearest multiple of A.
 * \tparam Alignment The alignment value, must be a power of 2.
 * \tparam PointerType The type of the pointer to align.
 * \param pointer The pointer to align.
 * \return The aligned pointer.
 */
template <std::size_t Alignment, typename PointerType>
constexpr auto alignDown(PointerType* pointer) -> PointerType* {
    return reinterpret_cast<PointerType*>(
        alignDown<Alignment>(reinterpret_cast<std::size_t>(pointer)));
}

/*!
 * \brief Aligns a value down to the nearest multiple of alignment.
 * \tparam ValueType The type of the value to align.
 * \tparam AlignmentType The type of the alignment value, must be integral.
 * \param value The value to align.
 * \param alignment The alignment value.
 * \return The aligned value.
 */
template <typename ValueType, typename AlignmentType>
constexpr auto alignDown(ValueType value,
                         AlignmentType alignment) -> ValueType {
    static_assert(std::is_integral<AlignmentType>::value,
                  "Alignment must be integral type");
    return value & ~static_cast<ValueType>(alignment - 1);
}

/*!
 * \brief Aligns a pointer down to the nearest multiple of alignment.
 * \tparam PointerType The type of the pointer to align.
 * \tparam AlignmentType The type of the alignment value, must be integral.
 * \param pointer The pointer to align.
 * \param alignment The alignment value.
 * \return The aligned pointer.
 */
template <typename PointerType, typename AlignmentType>
constexpr auto alignDown(PointerType* pointer,
                         AlignmentType alignment) -> PointerType* {
    return reinterpret_cast<PointerType*>(
        alignDown(reinterpret_cast<std::size_t>(pointer), alignment));
}

/*!
 * \brief Computes the base-2 logarithm of an integral value.
 * \tparam IntegralType The type of the value, must be integral.
 * \param value The value to compute the logarithm of.
 * \return The base-2 logarithm of the value.
 */
template <typename IntegralType>
constexpr auto log2(IntegralType value) -> IntegralType {
    static_assert(std::is_integral<IntegralType>::value,
                  "IntegralType must be integral type");
    return value <= 1 ? 0 : 1 + atom::meta::log2(value >> 1);
}

/*!
 * \brief Computes the number of blocks of size N needed to cover a value.
 * \tparam BlockSize The block size, must be a power of 2.
 * \tparam ValueType The type of the value.
 * \param value The value to compute the number of blocks for.
 * \return The number of blocks needed to cover the value.
 */
template <std::size_t BlockSize, typename ValueType>
constexpr auto nb(ValueType value) -> ValueType {
    static_assert((BlockSize & (BlockSize - 1)) == 0,
                  "BlockSize must be power of 2");
    return (value >> atom::meta::log2(static_cast<ValueType>(BlockSize))) +
           !!(value & static_cast<ValueType>(BlockSize - 1));
}

/*!
 * \brief Compares two values for equality.
 * \tparam ValueType The type of the values.
 * \param first Pointer to the first value.
 * \param second Pointer to the second value.
 * \return True if the values are equal, false otherwise.
 */
template <typename ValueType>
ATOM_INLINE auto eq(const void* first, const void* second) -> bool {
    return *reinterpret_cast<const ValueType*>(first) ==
           *reinterpret_cast<const ValueType*>(second);
}

/*!
 * \brief Copies N bytes from src to dst.
 * \tparam NumBytes The number of bytes to copy.
 * \param destination Pointer to the destination.
 * \param source Pointer to the source.
 */
template <std::size_t NumBytes>
ATOM_INLINE void copy(void* destination, const void* source) {
    if constexpr (NumBytes > 0) {
        std::memcpy(destination, source, NumBytes);
    }
}

/*!
 * \brief Specialization of copy for NumBytes = 0, does nothing.
 */
template <>
ATOM_INLINE void copy<0>(void*, const void*) {}

/*!
 * \brief Swaps the value pointed to by pointer with value.
 * \tparam PointerType The type of the value pointed to by pointer.
 * \tparam ValueType The type of the value.
 * \param pointer Pointer to the value to swap.
 * \param value The value to swap with.
 * \return The original value pointed to by pointer.
 */
template <typename PointerType, typename ValueType>
ATOM_INLINE auto swap(PointerType* pointer, ValueType value) -> PointerType {
    PointerType originalValue = *pointer;
    *pointer = static_cast<PointerType>(value);
    return originalValue;
}

/*!
 * \brief Adds value to the value pointed to by pointer and returns the original
 * value. \tparam PointerType The type of the value pointed to by pointer.
 * \tparam ValueType The type of the value.
 * \param pointer Pointer to the value to add to.
 * \param value The value to add.
 * \return The original value pointed to by pointer.
 */
template <typename PointerType, typename ValueType>
ATOM_INLINE auto fetchAdd(PointerType* pointer,
                          ValueType value) -> PointerType {
    PointerType originalValue = *pointer;
    *pointer += value;
    return originalValue;
}

/*!
 * \brief Subtracts value from the value pointed to by pointer and returns the
 * original value. \tparam PointerType The type of the value pointed to by
 * pointer. \tparam ValueType The type of the value. \param pointer Pointer to
 * the value to subtract from. \param value The value to subtract. \return The
 * original value pointed to by pointer.
 */
template <typename PointerType, typename ValueType>
ATOM_INLINE auto fetchSub(PointerType* pointer,
                          ValueType value) -> PointerType {
    PointerType originalValue = *pointer;
    *pointer -= value;
    return originalValue;
}

/*!
 * \brief Performs a bitwise AND between the value pointed to by pointer and
 * value, and returns the original value. \tparam PointerType The type of the
 * value pointed to by pointer. \tparam ValueType The type of the value. \param
 * pointer Pointer to the value to AND. \param value The value to AND with.
 * \return The original value pointed to by pointer.
 */
template <typename PointerType, typename ValueType>
ATOM_INLINE auto fetchAnd(PointerType* pointer,
                          ValueType value) -> PointerType {
    PointerType originalValue = *pointer;
    *pointer &= static_cast<PointerType>(value);
    return originalValue;
}

/*!
 * \brief Performs a bitwise OR between the value pointed to by pointer and
 * value, and returns the original value. \tparam PointerType The type of the
 * value pointed to by pointer. \tparam ValueType The type of the value. \param
 * pointer Pointer to the value to OR. \param value The value to OR with.
 * \return The original value pointed to by pointer.
 */
template <typename PointerType, typename ValueType>
ATOM_INLINE auto fetchOr(PointerType* pointer, ValueType value) -> PointerType {
    PointerType originalValue = *pointer;
    *pointer |= static_cast<PointerType>(value);
    return originalValue;
}

/*!
 * \brief Performs a bitwise XOR between the value pointed to by pointer and
 * value, and returns the original value. \tparam PointerType The type of the
 * value pointed to by pointer. \tparam ValueType The type of the value. \param
 * pointer Pointer to the value to XOR. \param value The value to XOR with.
 * \return The original value pointed to by pointer.
 */
template <typename PointerType, typename ValueType>
ATOM_INLINE auto fetchXor(PointerType* pointer,
                          ValueType value) -> PointerType {
    PointerType originalValue = *pointer;
    *pointer ^= static_cast<PointerType>(value);
    return originalValue;
}

/*!
 * \brief Alias for std::enable_if_t.
 * \tparam Condition The condition.
 * \tparam Type The type to enable if the condition is true.
 */
template <bool Condition, typename Type = void>
using if_t = std::enable_if_t<Condition, Type>;

/*!
 * \brief Alias for std::remove_reference_t.
 * \tparam Type The type to remove reference from.
 */
template <typename Type>
using rmRefT = std::remove_reference_t<Type>;

/*!
 * \brief Alias for std::remove_cv_t.
 * \tparam Type The type to remove const and volatile qualifiers from.
 */
template <typename Type>
using rmCvT = std::remove_cv_t<Type>;

/*!
 * \brief Alias for removing both const, volatile qualifiers and reference.
 * \tparam Type The type to remove const, volatile qualifiers and reference
 * from.
 */
template <typename Type>
using rmCvRefT = rmCvT<rmRefT<Type>>;

/*!
 * \brief Alias for std::remove_extent_t.
 * \tparam Type The type to remove extent from.
 */
template <typename Type>
using rmArrT = std::remove_extent_t<Type>;

/*!
 * \brief Alias for std::add_const_t.
 * \tparam Type The type to add const qualifier to.
 */
template <typename Type>
using constT = std::add_const_t<Type>;

/*!
 * \brief Alias for adding const qualifier and lvalue reference.
 * \tparam Type The type to add const qualifier and lvalue reference to.
 */
template <typename Type>
using constRefT = std::add_lvalue_reference_t<constT<rmRefT<Type>>>;

namespace detail {

/*!
 * \brief Helper struct to check if all types are the same.
 * \tparam Types The types to check.
 */
template <typename... Types>
struct IsSame {
    static constexpr bool K_VALUE = false;
};

/*!
 * \brief Specialization of IsSame for two or more types.
 * \tparam FirstType The first type.
 * \tparam SecondType The second type.
 * \tparam RemainingTypes The remaining types.
 */
template <typename FirstType, typename SecondType, typename... RemainingTypes>
struct IsSame<FirstType, SecondType, RemainingTypes...> {
    static constexpr bool K_VALUE =
        std::is_same_v<FirstType, SecondType> ||
        IsSame<FirstType, RemainingTypes...>::K_VALUE;
};

}  // namespace detail

/*!
 * \brief Checks if all types are the same.
 * \tparam FirstType The first type.
 * \tparam SecondType The second type.
 * \tparam RemainingTypes The remaining types.
 * \return True if all types are the same, false otherwise.
 */
template <typename FirstType, typename SecondType, typename... RemainingTypes>
constexpr auto isSame() -> bool {
    return detail::IsSame<FirstType, SecondType, RemainingTypes...>::K_VALUE;
}

/*!
 * \brief Checks if a type is a reference.
 * \tparam Type The type to check.
 * \return True if the type is a reference, false otherwise.
 */
template <typename Type>
constexpr auto isRef() -> bool {
    return std::is_reference_v<Type>;
}

/*!
 * \brief Checks if a type is an array.
 * \tparam Type The type to check.
 * \return True if the type is an array, false otherwise.
 */
template <typename Type>
constexpr auto isArray() -> bool {
    return std::is_array_v<Type>;
}

/*!
 * \brief Checks if a type is a class.
 * \tparam Type The type to check.
 * \return True if the type is a class, false otherwise.
 */
template <typename Type>
constexpr auto isClass() -> bool {
    return std::is_class_v<Type>;
}

/*!
 * \brief Checks if a type is a scalar.
 * \tparam Type The type to check.
 * \return True if the type is a scalar, false otherwise.
 */
template <typename Type>
constexpr auto isScalar() -> bool {
    return std::is_scalar_v<Type>;
}

/*!
 * \brief Checks if a type is trivially copyable.
 * \tparam Type The type to check.
 * \return True if the type is trivially copyable, false otherwise.
 */
template <typename Type>
constexpr auto isTriviallyCopyable() -> bool {
    return std::is_trivially_copyable_v<Type>;
}

/*!
 * \brief Checks if a type is trivially destructible.
 * \tparam Type The type to check.
 * \return True if the type is trivially destructible, false otherwise.
 */
template <typename Type>
constexpr auto isTriviallyDestructible() -> bool {
    return std::is_trivially_destructible_v<Type>;
}

/*!
 * \brief Checks if a type is a base of another type.
 * \tparam BaseType The base type.
 * \tparam DerivedType The derived type.
 * \return True if BaseType is a base of DerivedType, false otherwise.
 */
template <typename BaseType, typename DerivedType>
constexpr auto isBaseOf() -> bool {
    return std::is_base_of_v<BaseType, DerivedType>;
}

/*!
 * \brief Checks if a type has a virtual destructor.
 * \tparam Type The type to check.
 * \return True if the type has a virtual destructor, false otherwise.
 */
template <typename Type>
constexpr auto hasVirtualDestructor() -> bool {
    return std::has_virtual_destructor_v<Type>;
}

}  // namespace atom::meta

#endif  // ATOM_META_GOD_HPP
