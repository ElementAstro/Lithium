/*
 * mathutils.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Extra Math Library

**************************************************/

#include "math.hpp"

#include <bit>
#include <stdexcept>

namespace atom::algorithm {

#if defined(__GNUC__) && defined(__SIZEOF_INT128__)
uint64_t mulDiv64(uint64_t operant, uint64_t multiplier,
                  uint64_t divider) noexcept {
    __uint128_t a = operant;
    __uint128_t b = multiplier;
    __uint128_t c = divider;

    return static_cast<uint64_t>((a * b) / c);
}
#elif defined(_MSC_VER)
#include <intrin.h>  // For _umul128 and _BitScanReverse64
#include <stdexcept>

uint64_t mulDiv64(uint64_t operant, uint64_t multiplier,
                  uint64_t divider) noexcept {
    uint64_t highProd;
    uint64_t lowProd = _umul128(
        operant, multiplier,
        &highProd);  // Directly get the low and high parts of the product

    if (divider == 0) {
        throw std::runtime_error("Division by zero");
    }

    // Normalize divisor
    unsigned long shift = 63 - std::bit_width(divider - 1);
    uint64_t normDiv = divider << shift;

    // Normalize high part
    highProd = (highProd << shift) | (lowProd >> (64 - shift));
    lowProd <<= shift;

    // Division using high and low parts
    uint64_t quotient, remainder;
    _udiv128(highProd, lowProd, normDiv, &remainder);

    return quotient;
}
#else
#error "Platform not supported for mulDiv64 function!"
#endif

uint64_t safeAdd(uint64_t a, uint64_t b) {
    uint64_t result;
    if (__builtin_add_overflow(a, b, &result)) {
        throw std::overflow_error("Overflow in addition");
    }
    return result;
}

uint64_t safeMul(uint64_t a, uint64_t b) {
    uint64_t result;
    if (__builtin_mul_overflow(a, b, &result)) {
        throw std::overflow_error("Overflow in multiplication");
    }
    return result;
}

uint64_t rotl64(uint64_t n, unsigned int c) {
    const unsigned int mask = 63;
    c &= mask;
    return (n << c) | (n >> (-c & mask));
}

uint64_t rotr64(uint64_t n, unsigned int c) {
    const unsigned int mask = 63;
    c &= mask;
    return (n >> c) | (n << (-c & mask));
}

int clz64(uint64_t x) {
    if (x == 0)
        return 64;
    return __builtin_clzll(x);  // GCC built-in
}

uint64_t normalize(uint64_t x) {
    if (x == 0)
        return 0;
    int n = clz64(x);
    return x << n;
}

uint64_t safeSub(uint64_t a, uint64_t b) {
    uint64_t result;
    if (__builtin_sub_overflow(a, b, &result)) {
        throw std::underflow_error("Underflow in subtraction");
    }
    return result;
}

uint64_t safeDiv(uint64_t a, uint64_t b) {
    if (b == 0) {
        throw std::runtime_error("Division by zero");
    }
    return a / b;
}

}  // namespace atom::algorithm
