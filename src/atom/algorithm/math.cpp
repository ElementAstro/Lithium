/*
 * mathutils.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Extra Math Library with SIMD support

**************************************************/

#include "math.hpp"

#include <bit>        // For std::bit_width
#include <cmath>      // For std::sqrt
#include <numeric>    // For std::gcd
#ifdef _MSC_VER
#include <stdexcept>  // For std::runtime_error
#endif

#include "atom/error/exception.hpp"

// SIMD headers
#ifdef USE_SIMD
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif
#endif

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

auto safeMul(uint64_t a, uint64_t b) -> uint64_t {
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
#ifdef USE_SIMD
#if defined(__x86_64__) || defined(_M_X64)
    return _byteswap_uint64(n);
#elif defined(__ARM_NEON)
    return vrev64_u8(vcreate_u8(n));
#else
    // Fallback to non-SIMD implementation
#endif
#endif
    n = ((n & 0xAAAAAAAAAAAAAAAA) >> 1) | ((n & 0x5555555555555555) << 1);
    n = ((n & 0xCCCCCCCCCCCCCCCC) >> 2) | ((n & 0x3333333333333333) << 2);
    n = ((n & 0xF0F0F0F0F0F0F0F0) >> 4) | ((n & 0x0F0F0F0F0F0F0F0F) << 4);
    n = ((n & 0xFF00FF00FF00FF00) >> 8) | ((n & 0x00FF00FF00FF00FF) << 8);
    n = ((n & 0xFFFF0000FFFF0000) >> 16) | ((n & 0x0000FFFF0000FFFF) << 16);
    n = (n >> 32) | (n << 32);
    return n;
}

auto approximateSqrt(uint64_t n) -> uint64_t {
#ifdef USE_SIMD
#if defined(__x86_64__) || defined(_M_X64)
    return _mm_cvtsd_si64(
        _mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(static_cast<double>(n))));
#elif defined(__ARM_NEON)
    float32x2_t x = vdup_n_f32(static_cast<float>(n));
    float32x2_t sqrt_reciprocal = vrsqrte_f32(x);
    float32x2_t result = vmul_f32(x, sqrt_reciprocal);
    return static_cast<uint64_t>(vget_lane_f32(result, 0));
#else
    // Fallback to non-SIMD implementation
#endif
#endif
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
#ifdef USE_SIMD
#if defined(__x86_64__) || defined(_M_X64)
    if (n == 0)
        return 1;
    unsigned long index;
    _BitScanReverse64(&index, n);
    return 1ULL << (index + 1);
#elif defined(__ARM_NEON)
    if (n == 0)
        return 1;
    return 1ULL << (64 - __builtin_clzll(n - 1));
#else
    // Fallback to non-SIMD implementation
#endif
#endif
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

// New SIMD-optimized functions

#ifdef USE_SIMD

template <typename T, size_t N>
void vectorAdd(const T* a, const T* b, T* result, size_t size) {
#if defined(__x86_64__) || defined(_M_X64)
    for (size_t i = 0; i < size; i += N) {
        __m256i va =
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
        __m256i vb =
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + i));
        __m256i vr = _mm256_add_epi32(va, vb);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(result + i), vr);
    }
#elif defined(__ARM_NEON)
    for (size_t i = 0; i < size; i += N) {
        int32x4_t va = vld1q_s32(reinterpret_cast<const int32_t*>(a + i));
        int32x4_t vb = vld1q_s32(reinterpret_cast<const int32_t*>(b + i));
        int32x4_t vr = vaddq_s32(va, vb);
        vst1q_s32(reinterpret_cast<int32_t*>(result + i), vr);
    }
#endif
}

template <typename T, size_t N>
void vectorMul(const T* a, const T* b, T* result, size_t size) {
#if defined(__x86_64__) || defined(_M_X64)
    for (size_t i = 0; i < size; i += N) {
        __m256i va =
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
        __m256i vb =
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + i));
        __m256i vr = _mm256_mullo_epi32(va, vb);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(result + i), vr);
    }
#elif defined(__ARM_NEON)
    for (size_t i = 0; i < size; i += N) {
        int32x4_t va = vld1q_s32(reinterpret_cast<const int32_t*>(a + i));
        int32x4_t vb = vld1q_s32(reinterpret_cast<const int32_t*>(b + i));
        int32x4_t vr = vmulq_s32(va, vb);
        vst1q_s32(reinterpret_cast<int32_t*>(result + i), vr);
    }
#endif
}

// Explicit instantiations for common types
template void vectorAdd<int32_t, 8>(const int32_t*, const int32_t*, int32_t*,
                                    size_t);
template void vectorMul<int32_t, 8>(const int32_t*, const int32_t*, int32_t*,
                                    size_t);

#endif  // USE_SIMD

}  // namespace atom::algorithm
