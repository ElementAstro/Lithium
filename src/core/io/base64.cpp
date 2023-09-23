/*
 * base64.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: Base64

**************************************************/

#include <cstdint>
#include <cstddef>
#include "base64.hpp"
#include "base64_luts.hpp"

/* 
 * as byteswap.h is not available on macos, add macro here
 * Swap bytes in 16-bit value.
 */
//#define bswap_16(x) __builtin_bswap16 (x);
#define bswap_16(x) ((uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))

#include <cstdint>
#ifdef _WIN32
    constexpr bool IS_BIG_ENDIAN = false;
#else
    constexpr bool IS_BIG_ENDIAN = (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
    #include <arpa/inet.h>
#endif

#define  IS_LITTLE_ENDIAN  (!IS_BIG_ENDIAN)

size_t to64frombits_s(unsigned char* out, const unsigned char* in, int inlen, size_t outlen) {
    size_t dlen = (((size_t)inlen + 2) / 3) * 4; /* 4/3, rounded up */

    if (dlen > outlen) {
        return 0;
    }

    uint16_t *b64lut = (uint16_t *)base64lut;
    uint16_t *wbuf   = (uint16_t *)out;

    for (; inlen > 2; inlen -= 3)
    {
        uint32_t n = in[0] << 16 | in[1] << 8 | in[2];

        wbuf[0] = b64lut[n >> 12];
        wbuf[1] = b64lut[n & 0x00000fff];

        wbuf += 2;
        in += 3;
    }

    out = (unsigned char *)wbuf;
    if (inlen > 0)
    {
        unsigned char fragment;
        *out++   = base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;
        if (inlen > 1)
            fragment |= in[1] >> 4;
        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }
    *out = 0; // NULL terminate
    return dlen;
}

int from64tobits(char* out, const char* in) {
    const char* cp = in;
    while (*cp != '\0')
        cp += 4;
    return from64tobits_fast(out, in, cp - in);
}

int from64tobits_fast(char* out, const char* in, int inlen) {
    int outlen = 0;
    uint8_t b1, b2, b3;
    uint16_t s1, s2;
    uint32_t n32;
    int j;
    int n = (inlen / 4) - 1;
    uint16_t* inp = reinterpret_cast<uint16_t*>(const_cast<char*>(in));

    for (j = 0; j < n; j++) {
        if (in[0] == '\n')
            in++;
        inp = reinterpret_cast<uint16_t*>(const_cast<char*>(in));

        if constexpr (IS_BIG_ENDIAN) {
            inp[0] = __builtin_bswap16(inp[0]);
            inp[1] = __builtin_bswap16(inp[1]);
        }

        s1 = rbase64lut[inp[0]];
        s2 = rbase64lut[inp[1]];

        n32 = s1;
        n32 <<= 10;
        n32 |= s2 >> 2;

        b3 = (n32 & 0x00ff);
        n32 >>= 8;
        b2 = (n32 & 0x00ff);
        n32 >>= 8;
        b1 = (n32 & 0x00ff);

        *out++ = b1;
        *out++ = b2;
        *out++ = b3;

        in += 4;
        out += 3;
    }
    outlen = (inlen / 4 - 1) * 3;
    if (in[0] == '\n')
        in++;
    inp = reinterpret_cast<uint16_t*>(const_cast<char*>(in));
    if constexpr (IS_BIG_ENDIAN) {
        inp[0] = __builtin_bswap16(inp[0]);
        inp[1] = __builtin_bswap16(inp[1]);
    }

    s1 = rbase64lut[inp[0]];
    s2 = rbase64lut[inp[1]];

    n32 = s1;
    n32 <<= 10;
    n32 |= s2 >> 2;

    b3 = (n32 & 0x00ff);
    n32 >>= 8;
    b2 = (n32 & 0x00ff);
    n32 >>= 8;
    b1 = (n32 & 0x00ff);

    *out++ = b1;
    outlen++;
    if ((inp[1] & 0x00FF) != 0x003D) {
        *out++ = b2;
        outlen++;
        if ((inp[1] & 0xFF00) != 0x3D00) {
            *out++ = b3;
            outlen++;
        }
    }
    return outlen;
}
