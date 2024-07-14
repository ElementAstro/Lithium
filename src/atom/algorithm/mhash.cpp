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
#include <limits>
#include <random>
#include <span>
#include <sstream>

#include "atom/error/exception.hpp"
#include "atom/utils/random.hpp"
#include "macro.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace atom::algorithm {
void dataFromData(const void *data, size_t len, char *output) {
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

auto dataFromData(const std::string &data) -> std::string {
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

auto hexstringFromData(const std::string &data) -> std::string {
    const char *hexChars = "0123456789ABCDEF";
    std::string output;
    output.reserve(data.size() * 2);  // Reserve space for the hex string

    for (unsigned char byte : data) {
        output.push_back(hexChars[(byte >> 4) & 0x0F]);
        output.push_back(hexChars[byte & 0x0F]);
    }

    return output;
}

auto dataFromHexstring(const std::string &data) -> std::string {
    if (data.size() % 2 != 0) {
        THROW_INVALID_ARGUMENT("Hex string length must be even");
    }

    std::string result;
    result.resize(data.size() / 2);

    size_t outputIndex = 0;
    for (size_t i = 0; i < data.size(); i += 2) {
        int byte = 0;
        auto [ptr, ec] =
            std::from_chars(data.data() + i, data.data() + i + 2, byte, 16);

        if (ec == std::errc::invalid_argument || ptr != data.data() + i + 2) {
            THROW_INVALID_ARGUMENT("Invalid hex character");
        }

        result[outputIndex++] = static_cast<char>(byte);
    }

    return result;
}

MinHash::MinHash(size_t num_hashes) {
    hash_functions_.reserve(num_hashes);
    for (size_t i = 0; i < num_hashes; ++i) {
        hash_functions_.emplace_back(generateHashFunction());
    }
}

auto MinHash::generateHashFunction() -> HashFunction {
    utils::Random<std::mt19937, std::uniform_int_distribution<>> rand(
        0, std::numeric_limits<int>::max());

    size_t a = rand();
    size_t b = rand();
    size_t p = std::numeric_limits<size_t>::max();

    return [a, b, p](size_t x) -> size_t { return (a * x + b) % p; };
}

auto MinHash::jaccardIndex(const std::vector<size_t> &sig1,
                           const std::vector<size_t> &sig2) -> double {
    size_t equalCount = 0;

    for (size_t i = 0; i < sig1.size(); ++i) {
        if (sig1[i] == sig2[i]) {
            ++equalCount;
        }
    }

    return static_cast<double>(equalCount) / sig1.size();
}

}  // namespace atom::algorithm
