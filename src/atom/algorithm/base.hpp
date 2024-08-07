/*
 * base.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: A collection of algorithms for C++

**************************************************/

#ifndef ATOM_ALGORITHM_BASE16_HPP
#define ATOM_ALGORITHM_BASE16_HPP

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "atom/type/static_string.hpp"

namespace atom::algorithm {
namespace detail {
constexpr std::string_view BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
}
/**
 * @brief Base64编码函数
 *
 * @param bytes_to_encode 待编码数据
 * @return std::string 编码后的字符串
 */
[[nodiscard("The result of base64Encode is not used.")]] auto base64Encode(
    std::string_view bytes_to_encode) -> std::string;

/**
 * @brief Base64解码函数
 *
 * @param encoded_string 待解码字符串
 * @return std::vector<unsigned char> 解码后的数据
 */
[[nodiscard("The result of base64Decode is not used.")]] auto base64Decode(
    std::string_view encoded_string) -> std::string;

/**
 * @brief Faster Base64 Encode
 *
 * @param input
 * @return std::string
 */
auto fbase64Encode(std::span<const unsigned char> input) -> std::string;

/**
 * @brief Faster Base64 Decode
 *
 * @param input
 * @return std::vector<unsigned char>
 */
auto fbase64Decode(std::span<const char> input) -> std::vector<unsigned char>;

/**
 * @brief Encrypts a string using the XOR algorithm.
 *
 * @param data The string to be encrypted.
 * @param key The encryption key.
 * @return The encrypted string.
 */
[[nodiscard("The result of xorEncrypt is not used.")]] auto xorEncrypt(
    std::string_view plaintext, uint8_t key) -> std::string;

/**
 * @brief Decrypts a string using the XOR algorithm.
 *
 * @param data The string to be decrypted.
 * @param key The decryption key.
 * @return The decrypted string.
 */
[[nodiscard("The result of xorDecrypt is not used.")]] auto xorDecrypt(
    std::string_view ciphertext, uint8_t key) -> std::string;

ATOM_INLINE constexpr auto findBase64Char(char c) -> size_t {
    for (size_t i = 0; i < 64; ++i) {
        if (detail::BASE64_CHARS[i] == c) {
            return i;
        }
    }
    return 64;  // Indicates not found, should not happen with valid input
}

template <size_t N>
constexpr auto cbase64Encode(const StaticString<N> &input) {
    constexpr size_t ENCODED_SIZE = ((N + 2) / 3) * 4;
    StaticString<ENCODED_SIZE> ret;

    auto addChar = [&](char c) constexpr { ret += c; };

    std::array<unsigned char, 3> charArray3{};
    std::array<unsigned char, 4> charArray4{};

    size_t i = 0;
    for (auto it = input.begin(); it != input.end(); ++it, ++i) {
        charArray3[i % 3] = static_cast<unsigned char>(*it);
        if (i % 3 == 2) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] =
                ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] =
                ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (int j = 0; j < 4; ++j) {
                addChar(detail::BASE64_CHARS[charArray4[j]]);
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
            addChar(detail::BASE64_CHARS[charArray4[j]]);
        }

        while (i++ % 3 != 0) {
            addChar('=');
        }
    }

    return ret;
}

template <size_t N>
constexpr auto cbase64Decode(const StaticString<N> &input) {
    constexpr size_t DECODED_SIZE = (N / 4) * 3;
    StaticString<DECODED_SIZE> ret;

    auto addChar = [&](char c) constexpr { ret += c; };

    std::array<unsigned char, 4> charArray4{};
    std::array<unsigned char, 3> charArray3{};

    size_t i = 0;
    for (auto it = input.begin(); it != input.end() && *it != '='; ++it) {
        charArray4[i++] = static_cast<unsigned char>(findBase64Char(*it));
        if (i == 4) {
            charArray3[0] =
                (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] =
                ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; i < 3; ++i) {
                addChar(static_cast<char>(charArray3[i]));
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
            addChar(static_cast<char>(charArray3[j]));
        }
    }

    return ret;
}
}  // namespace atom::algorithm

#endif
