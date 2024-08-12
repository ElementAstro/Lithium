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

#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <sstream>

#ifdef USE_OPENMP
#include <omp.h>
#endif

namespace atom::algorithm {

constexpr std::array<uint32_t, 64> T{
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

constexpr std::array<uint32_t, 64> s{
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

void MD5::init() {
    a_ = 0x67452301;
    b_ = 0xefcdab89;
    c_ = 0x98badcfe;
    d_ = 0x10325476;
    count_ = 0;
    buffer_.clear();
}

void MD5::update(const std::string &input) {
    auto update_length = [this](size_t length) { count_ += length * 8; };

    update_length(input.size());

    for (char ch : input) {
        buffer_.push_back(static_cast<uint8_t>(ch));
        if (buffer_.size() == 64) {
            processBlock(buffer_.data());
            buffer_.clear();
        }
    }

    // Padding
    buffer_.push_back(0x80);
    while (buffer_.size() < 56) {
        buffer_.push_back(0x00);
    }

    for (int i = 0; i < 8; ++i) {
        buffer_.push_back(static_cast<uint8_t>((count_ >> (i * 8)) & 0xff));
    }

    processBlock(buffer_.data());
}

auto MD5::finalize() -> std::string {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << std::byteswap(a_);
    ss << std::setw(8) << std::byteswap(b_);
    ss << std::setw(8) << std::byteswap(c_);
    ss << std::setw(8) << std::byteswap(d_);
    return ss.str();
}

void MD5::processBlock(const uint8_t *block) {
    std::array<uint32_t, 16> M;
    for (size_t i = 0; i < 16; ++i) {
        M[i] = std::bit_cast<uint32_t>(
            std::array<uint8_t, 4>{block[i * 4], block[i * 4 + 1],
                                   block[i * 4 + 2], block[i * 4 + 3]});
    }

    uint32_t a = a_;
    uint32_t b = b_;
    uint32_t c = c_;
    uint32_t d = d_;

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
    for (uint32_t i = 0; i < 64; ++i) {
        uint32_t f, g;
        if (i < 16) {
            f = F(b, c, d);
            g = i;
        } else if (i < 32) {
            f = G(b, c, d);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = H(b, c, d);
            g = (3 * i + 5) % 16;
        } else {
            f = I(b, c, d);
            g = (7 * i) % 16;
        }

        uint32_t temp = d;
        d = c;
        c = b;
        b += leftRotate(a + f + T[i] + M[g], s[i]);
        a = temp;
    }

#ifdef USE_OPENMP
#pragma omp critical
#endif
    {
        a_ += a;
        b_ += b;
        c_ += c;
        d_ += d;
    }
}

auto MD5::F(uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
    return (x & y) | (~x & z);
}

auto MD5::G(uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
    return (x & z) | (y & ~z);
}

auto MD5::H(uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
    return x ^ y ^ z;
}

auto MD5::I(uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
    return y ^ (x | ~z);
}

auto MD5::leftRotate(uint32_t x, uint32_t n) -> uint32_t {
    return std::rotl(x, n);
}

auto MD5::encrypt(const std::string &input) -> std::string {
    MD5 md5;
    md5.init();
    md5.update(input);
    return md5.finalize();
}

}  // namespace atom::algorithm
