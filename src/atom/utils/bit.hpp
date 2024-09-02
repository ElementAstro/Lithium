#ifndef ATOM_UTILS_BIT_HPP
#define ATOM_UTILS_BIT_HPP

#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>

#include "macro.hpp"

namespace atom::utils {

/**
 * @brief Creates a bitmask with the specified number of bits set to 1.
 *
 * This function generates a bitmask of type `T` where the lower `bits` number
 * of bits are set to 1. If the `bits` parameter is greater than or equal to
 * the number of bits in type `T`, the function returns the maximum value that
 * type `T` can represent.
 *
 * @tparam T The unsigned integral type of the bitmask.
 * @param bits The number of bits to set to 1.
 * @return T The bitmask with `bits` number of bits set to 1.
 */
template <std::unsigned_integral T>
constexpr auto createMask(uint32_t bits) ATOM_NOEXCEPT -> T {
    if (bits >= std::numeric_limits<T>::digits) {
        return std::numeric_limits<T>::max();
    }
    return static_cast<T>((T{1} << bits) - 1);
}

/**
 * @brief Counts the number of set bits (1s) in the given value.
 *
 * This function counts and returns the number of set bits in the unsigned
 * integral value `value`. The number of bits is counted using the
 * `std::popcount` function, which is available in C++20 and later.
 *
 * @tparam T The unsigned integral type of the value.
 * @param value The value whose set bits are to be counted.
 * @return uint32_t The number of set bits in the value.
 */
template <std::unsigned_integral T>
constexpr auto countBytes(T value) ATOM_NOEXCEPT -> uint32_t {
    if constexpr (sizeof(T) <= sizeof(unsigned int)) {
        return static_cast<uint32_t>(std::popcount(value));
    } else {
        return static_cast<uint32_t>(
            std::popcount(static_cast<unsigned long long>(value)));
    }
}

/**
 * @brief Reverses the bits in the given value.
 *
 * This function reverses the bits of the unsigned integral value `value`. For
 * instance, if the value is `0b00000001` (1 in decimal) and the type has 8
 * bits, the function will return `0b10000000` (128 in decimal).
 *
 * @tparam T The unsigned integral type of the value.
 * @param value The value whose bits are to be reversed.
 * @return T The value with its bits reversed.
 */
template <std::unsigned_integral T>
constexpr auto reverseBits(T value) ATOM_NOEXCEPT -> T {
    T reversed = 0;
    for (int i = 0; i < std::numeric_limits<T>::digits; ++i) {
        reversed |= ((value >> i) & T{1})
                    << (std::numeric_limits<T>::digits - i - 1);
    }
    return reversed;
}

/**
 * @brief Performs a left rotation on the bits of the given value.
 *
 * This function performs a bitwise left rotation on the unsigned integral value
 * `value` by the specified number of `shift` positions. The rotation is
 * circular, meaning that bits shifted out on the left will reappear on the
 * right.
 *
 * @tparam T The unsigned integral type of the value.
 * @param value The value to rotate.
 * @param shift The number of positions to rotate left.
 * @return T The value after left rotation.
 */
template <std::unsigned_integral T>
constexpr auto rotateLeft(T value, int shift) ATOM_NOEXCEPT -> T {
    const int BITS = std::numeric_limits<T>::digits;
    shift %= BITS;
    return (value << shift) | (value >> (BITS - shift));
}

/**
 * @brief Performs a right rotation on the bits of the given value.
 *
 * This function performs a bitwise right rotation on the unsigned integral
 * value `value` by the specified number of `shift` positions. The rotation is
 * circular, meaning that bits shifted out on the right will reappear on the
 * left.
 *
 * @tparam T The unsigned integral type of the value.
 * @param value The value to rotate.
 * @param shift The number of positions to rotate right.
 * @return T The value after right rotation.
 */
template <std::unsigned_integral T>
constexpr auto rotateRight(T value, int shift) ATOM_NOEXCEPT -> T {
    const int BITS = std::numeric_limits<T>::digits;
    shift %= BITS;
    return (value >> shift) | (value << (BITS - shift));
}

}  // namespace atom::utils

#endif
