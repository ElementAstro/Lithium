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

#include "macro.hpp"

namespace atom::algorithm {
namespace detail {
constexpr std::string_view BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

constexpr size_t BASE64_CHAR_COUNT = 64;
constexpr uint8_t MASK_6_BITS = 0x3F;
constexpr uint8_t MASK_4_BITS = 0x0F;
constexpr uint8_t MASK_2_BITS = 0x03;
constexpr uint8_t MASK_8_BITS = 0xFC;
constexpr uint8_t MASK_12_BITS = 0xF0;
constexpr uint8_t MASK_14_BITS = 0xC0;
constexpr uint8_t MASK_16_BITS = 0x30;
constexpr uint8_t MASK_18_BITS = 0x3C;
}  // namespace detail

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

ATOM_INLINE constexpr auto findBase64Char(char character) -> size_t {
#pragma unroll
    for (size_t index = 0; index < detail::BASE64_CHAR_COUNT; ++index) {
        if (detail::BASE64_CHARS[index] == character) {
            return index;
        }
    }
    return detail::BASE64_CHAR_COUNT;  // Indicates not found, should not happen
                                       // with valid input
}

template <size_t N>
constexpr auto cbase64Encode(const StaticString<N> &input) {
    constexpr size_t ENCODED_SIZE = ((N + 2) / 3) * 4;
    StaticString<ENCODED_SIZE> ret;

    auto addCharacter = [&](char character) constexpr { ret += character; };

    std::array<unsigned char, 3> charArray3{};
    std::array<unsigned char, 4> charArray4{};

    size_t index = 0;
    for (auto it = input.begin(); it != input.end(); ++it, ++index) {
        charArray3[index % 3] = static_cast<unsigned char>(*it);
        if (index % 3 == 2) {
            charArray4[0] = (charArray3[0] & detail::MASK_8_BITS) >> 2;
            charArray4[1] = ((charArray3[0] & detail::MASK_2_BITS) << 4) +
                            ((charArray3[1] & detail::MASK_12_BITS) >> 4);
            charArray4[2] = ((charArray3[1] & detail::MASK_4_BITS) << 2) +
                            ((charArray3[2] & detail::MASK_14_BITS) >> 6);
            charArray4[3] = charArray3[2] & detail::MASK_6_BITS;

#pragma unroll
            for (int j = 0; j < 4; ++j) {
                addCharacter(detail::BASE64_CHARS[charArray4[j]]);
            }
        }
    }

    if (index % 3 != 0) {
        for (size_t j = index % 3; j < 3; ++j) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & detail::MASK_8_BITS) >> 2;
        charArray4[1] = ((charArray3[0] & detail::MASK_2_BITS) << 4) +
                        ((charArray3[1] & detail::MASK_12_BITS) >> 4);
        charArray4[2] = ((charArray3[1] & detail::MASK_4_BITS) << 2) +
                        ((charArray3[2] & detail::MASK_14_BITS) >> 6);
        charArray4[3] = charArray3[2] & detail::MASK_6_BITS;

#pragma unroll
        for (size_t j = 0; j < index % 3 + 1; ++j) {
            addCharacter(detail::BASE64_CHARS[charArray4[j]]);
        }

        while (index++ % 3 != 0) {
            addCharacter('=');
        }
    }

    return ret;
}

template <size_t N>
constexpr auto cbase64Decode(const StaticString<N> &input) {
    constexpr size_t DECODED_SIZE = (N / 4) * 3;
    StaticString<DECODED_SIZE> ret;

    auto addCharacter = [&](char character) constexpr { ret += character; };

    std::array<unsigned char, 4> charArray4{};
    std::array<unsigned char, 3> charArray3{};

    size_t index = 0;
    for (auto it = input.begin(); it != input.end() && *it != '='; ++it) {
        charArray4[index++] = static_cast<unsigned char>(findBase64Char(*it));
        if (index == 4) {
            charArray3[0] = (charArray4[0] << 2) +
                            ((charArray4[1] & detail::MASK_16_BITS) >> 4);
            charArray3[1] = ((charArray4[1] & detail::MASK_4_BITS) << 4) +
                            ((charArray4[2] & detail::MASK_18_BITS) >> 2);
            charArray3[2] =
                ((charArray4[2] & detail::MASK_2_BITS) << 6) + charArray4[3];

#pragma unroll
            for (index = 0; index < 3; ++index) {
                addCharacter(static_cast<char>(charArray3[index]));
            }
            index = 0;
        }
    }

    if (index != 0) {
        for (size_t j = index; j < 4; ++j) {
            charArray4[j] = 0;
        }

        charArray3[0] = (charArray4[0] << 2) +
                        ((charArray4[1] & detail::MASK_16_BITS) >> 4);
        charArray3[1] = ((charArray4[1] & detail::MASK_4_BITS) << 4) +
                        ((charArray4[2] & detail::MASK_18_BITS) >> 2);

#pragma unroll
        for (size_t j = 0; j < index - 1; ++j) {
            addCharacter(static_cast<char>(charArray3[j]));
        }
    }

    return ret;
}
}  // namespace atom::algorithm

#endif