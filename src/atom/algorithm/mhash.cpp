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
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <charconv>

namespace Atom::Utils {
uint32_t fmix32(uint32_t h) noexcept {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

uint32_t ROTL(uint32_t x, int8_t r) noexcept {
    return (x << r) | (x >> (32 - r));
}

uint32_t murmur3Hash(std::string_view data, uint32_t seed) noexcept {
    uint32_t hash = seed;
    const uint32_t seed1 = 0xcc9e2d51;
    const uint32_t seed2 = 0x1b873593;

    // Process 4-byte chunks
    const uint32_t *blocks = reinterpret_cast<const uint32_t *>(data.data());
    size_t nblocks = data.size() / 4;
    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= seed1;
        k = ROTL(k, 15);
        k *= seed2;

        hash ^= k;
        hash = ROTL(hash, 13);
        hash = hash * 5 + 0xe6546b64;
    }

    // Handle the tail
    const uint8_t *tail =
        reinterpret_cast<const uint8_t *>(data.data() + nblocks * 4);
    uint32_t tail_val = 0;
    switch (data.size() & 3) {
        case 3:
            tail_val |= tail[2] << 16;
            [[fallthrough]];
        case 2:
            tail_val |= tail[1] << 8;
            [[fallthrough]];
        case 1:
            tail_val |= tail[0];
            tail_val *= seed1;
            tail_val = ROTL(tail_val, 15);
            tail_val *= seed2;
            hash ^= tail_val;
    }

    return fmix32(hash ^ static_cast<uint32_t>(data.size()));
}

uint64_t murmur3Hash64(std::string_view str, uint32_t seed, uint32_t seed2) {
    return (static_cast<uint64_t>(murmur3Hash(str, seed)) << 32) |
           murmur3Hash(str, seed2);
}

void hexstringFromData(const void *data, size_t len, char *output) {
    const unsigned char *buf = static_cast<const unsigned char *>(data);
    std::span<const unsigned char> bytes(buf, len);
    std::ostringstream stream;

    // Use iomanip to format output
    stream << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        stream << std::setw(2) << static_cast<int>(byte);
    }

    std::string hexstr = stream.str();
    std::copy(hexstr.begin(), hexstr.end(), output);
    output[hexstr.size()] = '\0';  // Null-terminate the output string
}

std::string hexstringFromData(const std::string &data) {
    if (data.empty()) {
        return {};
    }

    std::string result;
    result.reserve(data.size() * 2);

    for (unsigned char c : data) {
        char buf[3];  // buffer for two hex chars and null terminator
        std::to_chars_result conv_result =
            std::to_chars(buf, buf + sizeof(buf), c, 16);

        if (conv_result.ec == std::errc{}) {
            if (buf[1] == '\0') {
                result += '0';  // pad single digit hex numbers
            }
            result.append(buf, conv_result.ptr);
        }
    }

    return result;
}

std::string dataFromHexstring(const std::string &hexstring) {
    if (hexstring.size() % 2 != 0) {
        throw std::invalid_argument("Hex string length must be even");
    }

    std::string result;
    result.resize(hexstring.size() / 2);

    size_t output_index = 0;
    for (size_t i = 0; i < hexstring.size(); i += 2) {
        int byte = 0;
        auto [ptr, ec] = std::from_chars(hexstring.data() + i,
                                         hexstring.data() + i + 2, byte, 16);

        if (ec == std::errc::invalid_argument ||
            ptr != hexstring.data() + i + 2) {
            throw std::invalid_argument("Invalid hex character");
        }

        result[output_index++] = static_cast<char>(byte);
    }

    return result;
}

}  // namespace Atom::Utils
