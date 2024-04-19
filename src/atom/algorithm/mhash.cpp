/*
 * mhash.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Implementation of murmur3 hash and quick hash

**************************************************/

#include "mhash.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <string.h>
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

#define ROTL(x, r) ((x << r) | (x >> (32 - r)))

namespace Atom::Utils {
static inline uint32_t fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

uint32_t murmur3Hash(const char *str, const uint32_t &seed = 1060627423) {
    if (!str)
        return 0;

    int len = strlen(str);
    return murmur3Hash(str, len, seed);
}

uint32_t murmur3Hash(const void *data, const uint32_t &size,
                     const uint32_t &seed = 1060627423) {
    if (!data)
        return 0;

    const char *str = (const char *)data;
    int len = size;

    uint32_t s, h = seed, seed1 = 0xcc9e2d51, seed2 = 0x1b873593,
                *ptr = (uint32_t *)str;

    // handle begin blocks
    int blk = len / 4;
    for (int i = 0; i < blk; i++) {
        s = ptr[i];
        s *= seed1;
        s = ROTL(s, 15);
        s *= seed2;

        h ^= s;
        h = ROTL(h, 13);
        h *= 5;
        h += 0xe6546b64;
    }

    // handle tail
    s = 0;
    uint8_t *tail = (uint8_t *)(str + blk * 4);
    switch (len & 3) {
        case 3:
            s |= tail[2] << 16;
        case 2:
            s |= tail[1] << 8;
        case 1:
            s |= tail[0];

            s *= seed1;
            s = ROTL(s, 15);
            s *= seed2;
            h ^= s;
    };

    return fmix32(h ^ len);
}

uint64_t murmur3Hash64(const void *str, const uint32_t &size,
                       const uint32_t &seed = 1060627423,
                       const uint32_t &seed2 = 1050126127) {
    return (((uint64_t)murmur3Hash(str, size, seed)) << 32 |
            murmur3Hash(str, size, seed2));
}

uint64_t murmur3Hash64(const char *str, const uint32_t &seed = 1060627423,
                       const uint32_t &seed2 = 1050126127) {
    return (((uint64_t)murmur3Hash(str, seed)) << 32 | murmur3Hash(str, seed2));
}

void hexstringFromData(const void *data, size_t len, char *output) {
    const unsigned char *buf = (const unsigned char *)data;
    size_t i, j;
    for (i = j = 0; i < len; ++i) {
        char c;
        c = (buf[i] >> 4) & 0xf;
        c = (c > 9) ? c + 'a' - 10 : c + '0';
        output[j++] = c;
        c = (buf[i] & 0xf);
        c = (c > 9) ? c + 'a' - 10 : c + '0';
        output[j++] = c;
    }
}

std::string hexstringFromData(const char *data, size_t len) {
    if (len == 0) {
        return std::string();
    }
    std::string result;
    result.resize(len * 2);
    hexstringFromData(data, len, &result[0]);
    return result;
}

std::string hexstringFromData(const std::string &data) {
    return hexstringFromData(data.c_str(), data.size());
}

void dataFromHexstring(const char *hexstring, size_t length, void *output) {
    unsigned char *buf = (unsigned char *)output;
    unsigned char byte;
    if (length % 2 != 0) {
        throw std::invalid_argument("dataFromHexstring length % 2 != 0");
    }
    for (size_t i = 0; i < length; ++i) {
        switch (hexstring[i]) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                byte = (hexstring[i] - 'a' + 10) << 4;
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                byte = (hexstring[i] - 'A' + 10) << 4;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                byte = (hexstring[i] - '0') << 4;
                break;
            default:
                throw std::invalid_argument(
                    "dataFromHexstring invalid hexstring");
        }
        ++i;
        switch (hexstring[i]) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                byte |= hexstring[i] - 'a' + 10;
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                byte |= hexstring[i] - 'A' + 10;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                byte |= hexstring[i] - '0';
                break;
            default:
                throw std::invalid_argument(
                    "dataFromHexstring invalid hexstring");
        }
        *buf++ = byte;
    }
}

std::string dataFromHexstring(const char *hexstring, size_t length) {
    if (length % 2 != 0) {
        throw std::invalid_argument("dataFromHexstring length % 2 != 0");
    }
    if (length == 0) {
        return std::string();
    }
    std::string result;
    result.resize(length / 2);
    dataFromHexstring(hexstring, length, &result[0]);
    return result;
}

std::string dataFromHexstring(const std::string &hexstring) {
    return dataFromHexstring(hexstring.c_str(), hexstring.size());
}

}  // namespace Atom::Utils
