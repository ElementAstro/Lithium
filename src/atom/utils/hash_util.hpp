/*
 * hash_util.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-16

Description: Implementation of murmur3 hash and quick hash

**************************************************/

#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace Atom::Utils
{

    /**
     * @brief Calculates the MurmurHash3 hash value for a given string.
     *
     * @param str The input string.
     * @param seed The seed value (optional, default is 1060627423).
     * @return uint32_t The calculated hash value.
     */
    uint32_t murmur3_hash(const char *str, const uint32_t &seed = 1060627423);

    /**
     * @brief Calculates the 64-bit MurmurHash3 hash value for a given string.
     *
     * @param str The input string.
     * @param seed The first seed value (optional, default is 1060627423).
     * @param seed2 The second seed value (optional, default is 1050126127).
     * @return uint64_t The calculated hash value.
     */
    uint64_t murmur3_hash64(const char *str, const uint32_t &seed = 1060627423, const uint32_t &seed2 = 1050126127);

    /**
     * @brief Calculates the MurmurHash3 hash value for a given data buffer.
     *
     * @param str The input data buffer.
     * @param size The size of the data buffer.
     * @param seed The seed value (optional, default is 1060627423).
     * @return uint32_t The calculated hash value.
     */
    uint32_t murmur3_hash(const void *str, const uint32_t &size, const uint32_t &seed = 1060627423);

    /**
     * @brief Calculates the 64-bit MurmurHash3 hash value for a given data buffer.
     *
     * @param str The input data buffer.
     * @param size The size of the data buffer.
     * @param seed The first seed value (optional, default is 1060627423).
     * @param seed2 The second seed value (optional, default is 1050126127).
     * @return uint64_t The calculated hash value.
     */
    uint64_t murmur3_hash64(const void *str, const uint32_t &size, const uint32_t &seed = 1060627423, const uint32_t &seed2 = 1050126127);

    /**
     * @brief Calculates the quick hash value for a given string.
     *
     * @param str The input string.
     * @return uint32_t The calculated hash value.
     */
    uint32_t quick_hash(const char *str);

    /**
     * @brief Calculates the quick hash value for a given data buffer.
     *
     * @param str The input data buffer.
     * @param size The size of the data buffer.
     * @return uint32_t The calculated hash value.
     */
    uint32_t quick_hash(const void *str, uint32_t size);

    /**
     * @brief Converts binary data to a hexadecimal string representation.
     *
     * @param data The input data buffer.
     * @param len The length of the data buffer.
     * @param output The output buffer to store the hexadecimal string (length must be len * 2).
     */
    void hexstring_from_data(const void *data, size_t len, char *output);

    /**
     * @brief Converts binary data to a hexadecimal string representation.
     *
     * @param data The input data buffer.
     * @param len The length of the data buffer.
     * @return std::string The hexadecimal string representation.
     */
    std::string hexstring_from_data(const void *data, size_t len);

    /**
     * @brief Converts a string to a hexadecimal string representation.
     *
     * @param data The input string.
     * @return std::string The hexadecimal string representation.
     */
    std::string hexstring_from_data(const std::string &data);

    /**
     * @brief Converts a hexadecimal string representation to binary data.
     *
     * @param hexstring The input hexadecimal string.
     * @param length The length of the hexadecimal string.
     * @param output The output buffer to store the binary data (length must be length / 2).
     */
    void data_from_hexstring(const char *hexstring, size_t length, void *output);

    /**
     * @brief Converts a hexadecimal string representation to binary data.
     *
     * @param hexstring The input hexadecimal string.
     * @param length The length of the hexadecimal string.
     * @return std::string The binary data.
     * @throw std::invalid_argument If the input hexstring is not a valid hexadecimal string.
     */
    std::string data_from_hexstring(const char *hexstring, size_t length);

    /**
     * @brief Converts a hexadecimal string representation to binary data.
     *
     * @param data The input hexadecimal string.
     * @return std::string The binary data.
     * @throw std::invalid_argument If the input hexstring is not a valid hexadecimal string.
     */
    std::string data_from_hexstring(const std::string &data);

    /**
     * @brief Replaces all occurrences of a character in a string with another character.
     *
     * @param str The input string.
     * @param find The character to find.
     * @param replaceWith The character to replace with.
     * @return std::string The modified string.
     */
    std::string replace(const std::string &str, char find, char replaceWith);

    /**
     * @brief Replaces all occurrences of a character in a string with another string.
     *
     * @param str The input string.
     * @param find The character to find.
     * @param replaceWith The string to replace with.
     * @return std::string The modified string.
     */
    std::string replace(const std::string &str, char find, const std::string &replaceWith);

    /**
     * @brief Replaces all occurrences of a substring in a string with another substring.
     *
     * @param str The input string.
     * @param find The substring to find.
     * @param replaceWith The substring to replace with.
     * @return std::string The modified string.
     */
    std::string replace(const std::string &str, const std::string &find, const std::string &replaceWith);

    /**
     * @brief Splits a string into a vector of substrings using a delimiter character.
     *
     * @param str The input string.
     * @param delim The delimiter character.
     * @param max The maximum number of splits (optional).
     * @return std::vector<std::string> The vector of substrings.
     */
    std::vector<std::string> split(const std::string &str, char delim, size_t max = ~0);

    /**
     * @brief Splits a string into a vector of substrings using multiple delimiter characters.
     *
     * @param str The input string.
     * @param delims The delimiter characters.
     * @param max The maximum number of splits (optional).
     * @return std::vector<std::string> The vector of substrings.
     */
    std::vector<std::string> split(const std::string &str, const char *delims, size_t max = ~0);

    /**
     * @brief Generates a random string of a specified length.
     *
     * @param len The length of the random string.
     * @param chars The characters to choose from (optional, default is alphanumeric).
     * @return std::string The random string.
     */
    std::string random_string(size_t len, const std::string &chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
}
