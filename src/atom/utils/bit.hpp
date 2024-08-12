#ifndef ATOM_UTILS_BIT_HPP
#define ATOM_UTILS_BIT_HPP

#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>

#include "macro.hpp"

namespace atom::utils {

template <std::unsigned_integral T>
constexpr auto createMask(uint32_t bits) ATOM_NOEXCEPT -> T {
    if (bits >= std::numeric_limits<T>::digits) {
        return std::numeric_limits<T>::max();
    }
    return static_cast<T>((T{1} << bits) - 1);
}

template <std::unsigned_integral T>
constexpr auto countBytes(T value) ATOM_NOEXCEPT -> uint32_t {
    if constexpr (sizeof(T) <= sizeof(unsigned int)) {
        return static_cast<uint32_t>(std::popcount(value));
    } else {
        return static_cast<uint32_t>(
            std::popcount(static_cast<unsigned long long>(value)));
    }
}

template <std::unsigned_integral T>
constexpr auto reverseBits(T value) ATOM_NOEXCEPT -> T {
    T reversed = 0;
    for (int i = 0; i < std::numeric_limits<T>::digits; ++i) {
        reversed |= ((value >> i) & T{1})
                    << (std::numeric_limits<T>::digits - i - 1);
    }
    return reversed;
}

template <std::unsigned_integral T>
constexpr auto rotateLeft(T value, int shift) ATOM_NOEXCEPT -> T {
    const int BITS = std::numeric_limits<T>::digits;
    shift %= BITS;
    return (value << shift) | (value >> (BITS - shift));
}

template <std::unsigned_integral T>
constexpr auto rotateRight(T value, int shift) ATOM_NOEXCEPT -> T {
    const int BITS = std::numeric_limits<T>::digits;
    shift %= BITS;
    return (value >> shift) | (value << (BITS - shift));
}
}  // namespace atom::utils

#endif
