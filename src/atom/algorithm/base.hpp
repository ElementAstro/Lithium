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
#include <string>
#include <vector>

namespace atom::algorithm {
/**
 * @brief Encodes a vector of unsigned characters into a Base16 string.
 *
 * This function takes a vector of unsigned characters and encodes it into a
 * Base16 string representation. Base16 encoding, also known as hexadecimal
 * encoding, represents each byte in the data data as a pair of hexadecimal
 * digits (0-9, A-F).
 *
 * @param data The vector of unsigned characters to be encoded.
 * @return The Base16 encoded string.
 */
[[nodiscard("The result of base16Encode is not used.")]] auto base16Encode(
    const std::vector<unsigned char> &data) -> std::string;

/**
 * @brief Decodes a Base16 string into a vector of unsigned characters.
 *
 * This function takes a Base16 encoded string and decodes it into a vector of
 * unsigned characters. Base16 encoding, also known as hexadecimal encoding,
 * represents each byte in the data data as a pair of hexadecimal digits (0-9,
 * A-F).
 *
 * @param data The Base16 encoded string to be decoded.
 * @return The decoded vector of unsigned characters.
 */
[[nodiscard("The result of base16Decode is not used.")]] auto base16Decode(
    const std::string &data) -> std::vector<unsigned char>;

/**
 * @brief Encodes a string to Base32
 * @param data The string to encode
 * @return The encoded string
 */
[[nodiscard("The result of base32Encode is not used.")]] auto base32Encode(
    const uint8_t *data, size_t length) -> std::string;

/**
 * @brief Decodes a Base32 string
 * @param data The string to decode
 * @return The decoded string
 */
[[nodiscard("The result of base32Decode is not used.")]] auto base32Decode(
    std::string_view encoded) -> std::string;

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
 * @brief Encodes a vector of unsigned characters into a Base85 string.
 *
 * This function takes a vector of unsigned characters and encodes it into a
 * Base85 string representation. Base85 encoding is a binary-to-text encoding
 * scheme that encodes 4 bytes into 5 ASCII characters.
 *
 * @param data The vector of unsigned characters to be encoded.
 * @return The Base85 encoded string.
 */
[[nodiscard("The result of base85Encode is not used.")]] auto base85Encode(
    const std::vector<unsigned char> &data) -> std::string;

/**
 * @brief Decodes a Base85 string into a vector of unsigned characters.
 *
 * This function takes a Base85 encoded string and decodes it into a vector of
 * unsigned characters. Base85 encoding is a binary-to-text encoding scheme that
 * encodes 4 bytes into 5 ASCII characters.
 *
 * @param data The Base85 encoded string to be decoded.
 * @return The decoded vector of unsigned characters.
 */
[[nodiscard("The result of base85Decode is not used.")]] auto base85Decode(
    const std::string &data) -> std::vector<unsigned char>;

/**
 * @brief Encodes a string to Base91
 * @param data The string to encode
 * @return The encoded string
 */
[[nodiscard("The result of base91Encode is not used.")]]
auto base91Encode(std::string_view data) -> std::string;

/**
 * @brief Decodes a Base91 string
 * @param data The string to decode
 * @return The decoded string
 */
[[nodiscard("The result of base91Decode is not used.")]] auto base91Decode(
    std::string_view data) -> std::string;

/**
 * @brief Encodes a vector of unsigned characters into a Base128 string.
 *
 * This function takes a vector of unsigned characters and encodes it into a
 * Base128 string representation. Base128 encoding is a binary-to-text encoding
 * scheme that encodes 1 byte into 1 ASCII character.
 *
 * @param data The vector of unsigned characters to be encoded.
 * @return The Base128 encoded string.
 */
[[nodiscard("The result of encodeBase128 is not used.")]] auto base128Encode(
    const uint8_t *data, size_t length) -> std::string;

/**
 * @brief Decodes a Base128 string into a vector of unsigned characters.
 *
 * This function takes a Base128 encoded string and decodes it into a vector of
 * unsigned characters. Base128 encoding is a binary-to-text encoding scheme
 * that encodes 1 byte into 1 ASCII character.
 *
 * @param data The Base128 encoded string to be decoded.
 * @return The decoded vector of unsigned characters.
 */
[[nodiscard("The result of decodeBase128 is not used.")]] auto base128Decode(
    std::string_view encoded) -> std::string;

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
}  // namespace atom::algorithm

#endif
