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

#include <bit>        // For std::bit_width
#include <cmath>      // For std::sqrt
#include <numeric>    // For std::gcd
#include <stdexcept>  // For std::runtime_error

namespace atom::algorithm {

#if defined(__GNUC__) && defined(__SIZEOF_INT128__)
uint64_t mulDiv64(uint64_t operand, uint64_t multiplier, uint64_t divider) {
    if (divider == 0) {
        throw std::runtime_error("Division by zero");
    }

    __uint128_t a = operand;
    __uint128_t b = multiplier;
    __uint128_t c = divider;

    return static_cast<uint64_t>((a * b) / c);
}
#elif defined(_MSC_VER)
#include <intrin.h>  // For _umul128 and _BitScanReverse

uint64_t mulDiv64(uint64_t operand, uint64_t multiplier, uint64_t divider) {
    if (divider == 0) {
        throw std::runtime_error("Division by zero");
    }

    uint64_t highProd;
    uint64_t lowProd = _umul128(operand, multiplier, &highProd);

    unsigned long shift = 63 - std::bit_width(divider - 1);
    uint64_t normDiv = divider << shift;

    highProd = (highProd << shift) | (lowProd >> (64 - shift));
    lowProd <<= shift;

    uint64_t quotient;
    _udiv128(highProd, lowProd, normDiv, &quotient);

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

uint64_t rotl64(uint64_t n, unsigned int c) { return std::rotl(n, c); }

uint64_t rotr64(uint64_t n, unsigned int c) { return std::rotr(n, c); }

int clz64(uint64_t x) {
    if (x == 0)
        return 64;
    return __builtin_clzll(x);
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

uint64_t bitReverse64(uint64_t n) {
    n = ((n & 0xAAAAAAAAAAAAAAAA) >> 1) | ((n & 0x5555555555555555) << 1);
    n = ((n & 0xCCCCCCCCCCCCCCCC) >> 2) | ((n & 0x3333333333333333) << 2);
    n = ((n & 0xF0F0F0F0F0F0F0F0) >> 4) | ((n & 0x0F0F0F0F0F0F0F0F) << 4);
    n = ((n & 0xFF00FF00FF00FF00) >> 8) | ((n & 0x00FF00FF00FF00FF) << 8);
    n = ((n & 0xFFFF0000FFFF0000) >> 16) | ((n & 0x0000FFFF0000FFFF) << 16);
    n = (n >> 32) | (n << 32);
    return n;
}

uint64_t approximateSqrt(uint64_t n) {
    if (n == 0 || n == 1) {
        return n;
    }
    double x = n;
    double y = 1;
    double e = 0.000001;
    while (x - y > e) {
        x = (x + y) / 2;
        y = n / x;
    }
    return static_cast<uint64_t>(x);
}

uint64_t gcd64(uint64_t a, uint64_t b) { return std::gcd(a, b); }

uint64_t lcm64(uint64_t a, uint64_t b) { return a / gcd64(a, b) * b; }

bool isPowerOfTwo(uint64_t n) { return n != 0 && (n & (n - 1)) == 0; }

uint64_t nextPowerOfTwo(uint64_t n) {
    if (n == 0) {
        return 1;
    }
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

}  // namespace atom::algorithm
