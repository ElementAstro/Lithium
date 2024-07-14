/*
 * base.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: A collection of algorithms for C++

**************************************************/

#include "base.hpp"

#include <array>
#include <string_view>

#include "atom/error/exception.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#endif

namespace atom::algorithm {
namespace detail {
template <typename InputIt, typename OutputIt>
void base64Encode(InputIt begin, InputIt end, OutputIt dest) {
    std::array<unsigned char, 3> charArray3{};
    std::array<unsigned char, 4> charArray4{};

    size_t i = 0;
    for (auto it = begin; it != end; ++it, ++i) {
        charArray3[i % 3] = static_cast<unsigned char>(*it);
        if (i % 3 == 2) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] =
                ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] =
                ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (int j = 0; j < 4; ++j) {
                *dest++ = BASE64_CHARS[charArray4[j]];
            }
        }
    }

    if (i % 3 != 0) {
        for (size_t j = i % 3; j < 3; ++j) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] =
            ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] =
            ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;

        for (size_t j = 0; j < i % 3 + 1; ++j) {
            *dest++ = BASE64_CHARS[charArray4[j]];
        }

        while (i++ % 3 != 0) {
            *dest++ = '=';
        }
    }
}

template <typename InputIt, typename OutputIt>
void base64Decode(InputIt begin, InputIt end, OutputIt dest) {
    std::array<unsigned char, 4> charArray4{};
    std::array<unsigned char, 3> charArray3{};

    size_t i = 0;
    for (auto it = begin; it != end && *it != '='; ++it) {
        charArray4[i++] = static_cast<unsigned char>(BASE64_CHARS.find(*it));
        if (i == 4) {
            charArray3[0] =
                (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] =
                ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; i < 3; ++i) {
                *dest++ = charArray3[i];
            }
            i = 0;
        }
    }

    if (i != 0) {
        for (size_t j = i; j < 4; ++j) {
            charArray4[j] = 0;
        }

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] =
            ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);

        for (size_t j = 0; j < i - 1; ++j) {
            *dest++ = charArray3[j];
        }
    }
}
}  // namespace detail

auto base64Encode(std::string_view bytes_to_encode) -> std::string {
    std::string ret;
    ret.reserve((bytes_to_encode.size() + 2) / 3 * 4);
    detail::base64Encode(bytes_to_encode.begin(), bytes_to_encode.end(),
                         std::back_inserter(ret));
    return ret;
}

auto base64Decode(std::string_view encoded_string) -> std::string {
    std::string ret;
    ret.reserve(encoded_string.size() / 4 * 3);
    detail::base64Decode(encoded_string.begin(), encoded_string.end(),
                         std::back_inserter(ret));
    return ret;
}

auto fbase64Encode(std::span<const unsigned char> input) -> std::string {
    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);
    detail::base64Encode(input.begin(), input.end(),
                         std::back_inserter(output));
    return output;
}

auto fbase64Decode(std::span<const char> input) -> std::vector<unsigned char> {
    if (input.size() % 4 != 0) {
        THROW_INVALID_ARGUMENT("Invalid base64 input length");
    }

    std::vector<unsigned char> output;
    output.reserve(input.size() / 4 * 3);
    detail::base64Decode(input.begin(), input.end(),
                         std::back_inserter(output));
    return output;
}

auto xorEncrypt(std::string_view plaintext, uint8_t key) -> std::string {
    std::string ciphertext;
    ciphertext.reserve(plaintext.size());
    for (char c : plaintext) {
        ciphertext.push_back(static_cast<char>(static_cast<uint8_t>(c) ^ key));
    }
    return ciphertext;
}

auto xorDecrypt(std::string_view ciphertext, uint8_t key) -> std::string {
    return xorEncrypt(ciphertext, key);
}
}  // namespace atom::algorithm
