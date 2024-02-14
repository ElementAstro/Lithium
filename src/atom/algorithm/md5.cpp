/*
 * md5.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Self implemented MD5 algorithm.

**************************************************/

#include "md5.hpp"

#include <sstream>
#include <iomanip>
#include <cstring>

namespace Atom::Utils
{
    constexpr uint32_t T[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

    constexpr uint32_t s[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

    void MD5::init()
    {
        _a = 0x67452301;
        _b = 0xefcdab89;
        _c = 0x98badcfe;
        _d = 0x10325476;
        _count = 0;
        _buffer.clear();
    }

    void MD5::update(const std::string &input)
    {
        for (char ch : input)
        {
            _buffer.push_back(static_cast<uint8_t>(ch));
            if (_buffer.size() == 64)
            {
                processBlock(_buffer.data());
                _count += 512;
                _buffer.clear();
            }
        }

        // Padding
        size_t bitLen = _count + _buffer.size() * 8;
        _buffer.push_back(0x80);
        while (_buffer.size() < 56)
            _buffer.push_back(0x00);

        for (int i = 0; i < 8; ++i)
        {
            _buffer.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xff));
        }

        processBlock(_buffer.data());
    }

    std::string MD5::finalize()
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(8) << reverseBytes(_a);
        ss << std::setw(8) << reverseBytes(_b);
        ss << std::setw(8) << reverseBytes(_c);
        ss << std::setw(8) << reverseBytes(_d);
        return ss.str();
    }

    void MD5::processBlock(const uint8_t *block)
    {
        uint32_t M[16];
        for (int i = 0; i < 16; ++i)
        {
            // 将block的每四个字节转换为32位整数（考虑到小端字节序）
            M[i] = (uint32_t)block[i * 4] | ((uint32_t)block[i * 4 + 1] << 8) |
                   ((uint32_t)block[i * 4 + 2] << 16) | ((uint32_t)block[i * 4 + 3] << 24);
        }

        uint32_t a = _a;
        uint32_t b = _b;
        uint32_t c = _c;
        uint32_t d = _d;

        // 主循环
        for (uint32_t i = 0; i < 64; ++i)
        {
            uint32_t f, g;
            if (i < 16)
            {
                f = F(b, c, d);
                g = i;
            }
            else if (i < 32)
            {
                f = G(b, c, d);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48)
            {
                f = H(b, c, d);
                g = (3 * i + 5) % 16;
            }
            else
            {
                f = I(b, c, d);
                g = (7 * i) % 16;
            }

            uint32_t temp = d;
            d = c;
            c = b;
            b = b + leftRotate((a + f + T[i] + M[g]), s[i]);
            a = temp;
        }

        _a += a;
        _b += b;
        _c += c;
        _d += d;
    }

    uint32_t MD5::F(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & y) | (~x & z);
    }

    uint32_t MD5::G(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & z) | (y & ~z);
    }

    uint32_t MD5::H(uint32_t x, uint32_t y, uint32_t z)
    {
        return x ^ y ^ z;
    }

    uint32_t MD5::I(uint32_t x, uint32_t y, uint32_t z)
    {
        return y ^ (x | ~z);
    }

    uint32_t MD5::leftRotate(uint32_t x, uint32_t n)
    {
        return (x << n) | (x >> (32 - n));
    }

    uint32_t MD5::reverseBytes(uint32_t x)
    {
        return ((x & 0x000000FF) << 24) |
               ((x & 0x0000FF00) << 8) |
               ((x & 0x00FF0000) >> 8) |
               ((x & 0xFF000000) >> 24);
    }

    std::string MD5::encrypt(const std::string &input)
    {
        MD5 md5;
        md5.init();
        md5.update(input);
        return md5.finalize();
    }
}