/*
 * mhash.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Implementation of murmur3 hash and quick hash

**************************************************/

#ifndef ATOM_ALGORITHM_MHASH_HPP
#define ATOM_ALGORITHM_MHASH_HPP

#include <stdint.h>
#include <cstdint>
#include <string>
#include <vector>

namespace atom::algorithm {
/**
 * @brief Calculates the MurmurHash3 hash value for a given string.
 *
 * @param str The input string.
 * @param seed The seed value (optional, default is 1060627423).
 * @return uint32_t The calculated hash value.
 */
[[nodiscard]] uint32_t murmur3Hash(std::string_view data,
                                   uint32_t seed = 1060627423) noexcept;

/**
 * @brief Calculates the 64-bit MurmurHash3 hash value for a given string.
 *
 * @param str The input string.
 * @param seed The first seed value (optional, default is 1060627423).
 * @param seed2 The second seed value (optional, default is 1050126127).
 * @return uint64_t The calculated hash value.
 */
[[nodiscard]] uint64_t murmur3Hash64(std::string_view str,
                                     uint32_t seed = 1060627423,
                                     uint32_t seed2 = 1050126127);

/**
 * @brief Converts binary data to a hexadecimal string representation.
 *
 * @param data The input data buffer.
 * @param len The length of the data buffer.
 * @param output The output buffer to store the hexadecimal string (length must
 * be len * 2).
 */
void hexstringFromData(const void *data, size_t len, char *output);

/**
 * @brief Converts a string to a hexadecimal string representation.
 *
 * @param data The input string.
 * @return std::string The hexadecimal string representation.
 */
[[nodiscard]] std::string hexstringFromData(const std::string &data);

/**
 * @brief Converts a hexadecimal string representation to binary data.
 *
 * @param data The input hexadecimal string.
 * @return std::string The binary data.
 * @throw std::invalid_argument If the input hexstring is not a valid
 * hexadecimal string.
 */
[[nodiscard]] std::string dataFromHexstring(const std::string &data);
}  // namespace atom::algorithm

#endif