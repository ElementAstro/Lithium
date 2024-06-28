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

#include "atom/error/exception.hpp"

namespace atom::algorithm {

#if defined(__GNUC__) && defined(__SIZEOF_INT128__)
auto mulDiv64(uint64_t operand, uint64_t multiplier,
              uint64_t divider) -> uint64_t {
    if (divider == 0) {
        THROW_INVALID_ARGUMENT("Division by zero");
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
        THROW_INVALID_ARGUMENT("Division by zero");
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

auto safeAdd(uint64_t a, uint64_t b) -> uint64_t {
    uint64_t result;
    if (__builtin_add_overflow(a, b, &result)) {
        THROW_OVERFLOW("Overflow in addition");
    }
    return result;
}

uint64_t safeMul(uint64_t a, uint64_t b) {
    uint64_t result;
    if (__builtin_mul_overflow(a, b, &result)) {
        THROW_OVERFLOW("Overflow in multiplication");
    }
    return result;
}

auto rotl64(uint64_t n, unsigned int c) -> uint64_t { return std::rotl(n, c); }

auto rotr64(uint64_t n, unsigned int c) -> uint64_t { return std::rotr(n, c); }

auto clz64(uint64_t x) -> int {
    if (x == 0) {
        return 64;
    }
    return __builtin_clzll(x);
}

auto normalize(uint64_t x) -> uint64_t {
    if (x == 0) {
        return 0;
    }
    int n = clz64(x);
    return x << n;
}

auto safeSub(uint64_t a, uint64_t b) -> uint64_t {
    uint64_t result;
    if (__builtin_sub_overflow(a, b, &result)) {
        THROW_UNDERFLOW("Underflow in subtraction");
    }
    return result;
}

auto safeDiv(uint64_t a, uint64_t b) -> uint64_t {
    if (b == 0) {
        THROW_INVALID_ARGUMENT("Division by zero");
    }
    return a / b;
}

auto bitReverse64(uint64_t n) -> uint64_t {
    n = ((n & 0xAAAAAAAAAAAAAAAA) >> 1) | ((n & 0x5555555555555555) << 1);
    n = ((n & 0xCCCCCCCCCCCCCCCC) >> 2) | ((n & 0x3333333333333333) << 2);
    n = ((n & 0xF0F0F0F0F0F0F0F0) >> 4) | ((n & 0x0F0F0F0F0F0F0F0F) << 4);
    n = ((n & 0xFF00FF00FF00FF00) >> 8) | ((n & 0x00FF00FF00FF00FF) << 8);
    n = ((n & 0xFFFF0000FFFF0000) >> 16) | ((n & 0x0000FFFF0000FFFF) << 16);
    n = (n >> 32) | (n << 32);
    return n;
}

auto approximateSqrt(uint64_t n) -> uint64_t {
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

auto gcd64(uint64_t a, uint64_t b) -> uint64_t { return std::gcd(a, b); }

auto lcm64(uint64_t a, uint64_t b) -> uint64_t { return a / gcd64(a, b) * b; }

auto isPowerOfTwo(uint64_t n) -> bool { return n != 0 && (n & (n - 1)) == 0; }

auto nextPowerOfTwo(uint64_t n) -> uint64_t {
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
