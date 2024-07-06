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

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <span>
#include <sstream>
#include <stdexcept>
#include "error/exception.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace atom::algorithm {
auto fmix32(uint32_t h) ATOM_NOEXCEPT -> uint32_t {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

auto rotl(uint32_t x, int8_t r) ATOM_NOEXCEPT -> uint32_t {
    return (x << r) | (x >> (32 - r));
}

auto murmur3Hash(std::string_view data, uint32_t seed) ATOM_NOEXCEPT -> uint32_t {
    uint32_t hash = seed;
    const uint32_t SEED1 = 0xcc9e2d51;
    const uint32_t SEED2 = 0x1b873593;

    // Process 4-byte chunks
    const auto *blocks = reinterpret_cast<const uint32_t *>(data.data());
    size_t nblocks = data.size() / 4;
    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= SEED1;
        k = rotl(k, 15);
        k *= SEED2;

        hash ^= k;
        hash = rotl(hash, 13);
        hash = hash * 5 + 0xe6546b64;
    }

    // Handle the tail
    const auto *tail =
        reinterpret_cast<const uint8_t *>(data.data() + nblocks * 4);
    uint32_t tailVal = 0;
    switch (data.size() & 3) {
        case 3:
            tailVal |= tail[2] << 16;
            [[fallthrough]];
        case 2:
            tailVal |= tail[1] << 8;
            [[fallthrough]];
        case 1:
            tailVal |= tail[0];
            tailVal *= SEED1;
            tailVal = rotl(tailVal, 15);
            tailVal *= SEED2;
            hash ^= tailVal;
    }

    return fmix32(hash ^ static_cast<uint32_t>(data.size()));
}

auto murmur3Hash64(std::string_view str, uint32_t seed,
                   uint32_t seed2) -> uint64_t {
    return (static_cast<uint64_t>(murmur3Hash(str, seed)) << 32) |
           murmur3Hash(str, seed2);
}

void hexstringFromData(const void *data, size_t len, char *output) {
    const auto *buf = static_cast<const unsigned char *>(data);
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

auto hexstringFromData(const std::string &data) -> std::string {
    if (data.empty()) {
        return {};
    }

    std::string result;
    result.reserve(data.size() * 2);

    for (unsigned char c : data) {
        std::array<char, 3>
            buf{};  // buffer for two hex chars and null terminator
        std::to_chars_result convResult =
            std::to_chars(buf.data(), buf.data() + buf.size(), c, 16);

        if (convResult.ec == std::errc{}) {
            if (buf[1] == '\0') {
                result += '0';  // pad single digit hex numbers
            }
            result.append(buf.data(), convResult.ptr);
        }
    }

    return result;
}

auto dataFromHexstring(const std::string &hexstring) -> std::string {
    if (hexstring.size() % 2 != 0) {
        THROW_INVALID_ARGUMENT("Hex string length must be even");
    }

    std::string result;
    result.resize(hexstring.size() / 2);

    size_t outputIndex = 0;
    for (size_t i = 0; i < hexstring.size(); i += 2) {
        int byte = 0;
        auto [ptr, ec] = std::from_chars(hexstring.data() + i,
                                         hexstring.data() + i + 2, byte, 16);

        if (ec == std::errc::invalid_argument ||
            ptr != hexstring.data() + i + 2) {
            THROW_INVALID_ARGUMENT("Invalid hex character");
        }

        result[outputIndex++] = static_cast<char>(byte);
    }

    return result;
}

}  // namespace atom::algorithm
