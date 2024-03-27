/*
 * mathutils.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Extra Math Library

**************************************************/

#include "mathutils.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace Atom::Algorithm {

uint64_t mulDiv64(uint64_t operant, uint64_t multiplier,
                        uint64_t divider) {
#if defined(__GNUC__) && defined(__SIZEOF_INT128__)
    __uint128_t a = operant;
    __uint128_t b = multiplier;
    __uint128_t c = divider;

    return (uint64_t)(a * b / c);
#elif defined(_MSC_VER)
#if defined(_M_IX86)
    // Your x86 specific code here
#elif defined(_M_X64)
#pragma warning(push)
#pragma warning(disable : 4244)  // C4244: 'conversion' conversion from 'type1'
                                 // to 'type2', possible loss of data
    uint64_t a = operant;
    uint64_t b = multiplier;
    uint64_t c = divider;

    //  Normalize divisor
    unsigned long shift;
    _BitScanReverse64(&shift, c);
    shift = 63 - shift;

    c <<= shift;

    // Multiply
    a = _umul128(a, b, &b);
    if (((b << shift) >> shift) != b) {
        // Overflow
        return 0xFFFFFFFFFFFFFFFF;
    }
    b = __shiftleft128(a, b, shift);
    a <<= shift;

    uint32_t div;
    uint32_t q0, q1;
    uint64_t t0, t1;

    // 1st Reduction
    div = (uint32_t)(c >> 32);
    t0 = b / div;
    if (t0 > 0xFFFFFFFF)
        t0 = 0xFFFFFFFF;
    q1 = (uint32_t)t0;
    while (1) {
        t0 = _umul128(c, (uint64_t)q1 << 32, &t1);
        if (t1 < b || (t1 == b && t0 <= a))
            break;
        q1--;
    }
    b -= t1;
    if (t0 > a)
        b--;
    a -= t0;

    if (b > 0xFFFFFFFF) {
        // Overflow
        return 0xFFFFFFFFFFFFFFFF;
    }

    // 2nd reduction
    t0 = ((b << 32) | (a >> 32)) / div;
    if (t0 > 0xFFFFFFFF)
        t0 = 0xFFFFFFFF;
    q0 = (uint32_t)t0;

    while (1) {
        t0 = _umul128(c, q0, &t1);
        if (t1 < b || (t1 == b && t0 <= a))
            break;
        q0--;
    }

    return ((uint64_t)q1 << 32) | q0;
#pragma warning(pop)
#endif
#else
#error MulDiv64 is no supported!
#endif
}

}  // namespace Atom::Algorithm
