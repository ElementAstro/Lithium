#ifndef ATOM_UTILS_BIT_HPP
#define ATOM_UTILS_BIT_HPP

#include <bit>
#include <concepts>
#include <cstdint>
#include <numeric>
#include <type_traits>

#include "macro.hpp"

namespace atom::utils {

template <std::unsigned_integral T>
constexpr T CreateMask(uint32_t bits) ATOM_NOEXCEPT {
    if (bits >= std::numeric_limits<T>::digits) {
        return std::numeric_limits<T>::max();
    }
    return static_cast<T>((T{1} << bits) - 1);
}

template <std::unsigned_integral T>
constexpr uint32_t CountBytes(T value) ATOM_NOEXCEPT {
    if constexpr (sizeof(T) <= sizeof(unsigned int)) {
        return static_cast<uint32_t>(std::popcount(value));
    } else {
        return static_cast<uint32_t>(
            std::popcount(static_cast<unsigned long long>(value)));
    }
}

template <std::unsigned_integral T>
constexpr T ReverseBits(T value) ATOM_NOEXCEPT {
    T reversed = 0;
    for (int i = 0; i < std::numeric_limits<T>::digits; ++i) {
        reversed |= ((value >> i) & T{1})
                    << (std::numeric_limits<T>::digits - i - 1);
    }
    return reversed;
}

template <std::unsigned_integral T>
constexpr T RotateLeft(T value, int shift) ATOM_NOEXCEPT {
    const int bits = std::numeric_limits<T>::digits;
    shift %= bits;
    return (value << shift) | (value >> (bits - shift));
}

template <std::unsigned_integral T>
constexpr T RotateRight(T value, int shift) ATOM_NOEXCEPT {
    const int bits = std::numeric_limits<T>::digits;
    shift %= bits;
    return (value >> shift) | (value << (bits - shift));
}
}  // namespace atom::utils

#endif