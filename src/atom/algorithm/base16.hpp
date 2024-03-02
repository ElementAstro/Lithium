/*
 * base16.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Base16 encoding and decoding

**************************************************/

#ifndef ATOM_ALGORITHM_BASE16_HPP
#define ATOM_ALGORITHM_BASE16_HPP

#include <string>
#include <vector>

namespace Atom::Algorithm {
/**
 * @brief Encodes a vector of unsigned characters into a Base16 string.
 *
 * This function takes a vector of unsigned characters and encodes it into a
 * Base16 string representation. Base16 encoding, also known as hexadecimal
 * encoding, represents each byte in the input data as a pair of hexadecimal
 * digits (0-9, A-F).
 *
 * @param data The vector of unsigned characters to be encoded.
 * @return The Base16 encoded string.
 */
[[nodiscard]] std::string encodeBase16(const std::vector<unsigned char> &data);

/**
 * @brief Decodes a Base16 string into a vector of unsigned characters.
 *
 * This function takes a Base16 encoded string and decodes it into a vector of
 * unsigned characters. Base16 encoding, also known as hexadecimal encoding,
 * represents each byte in the input data as a pair of hexadecimal digits (0-9,
 * A-F).
 *
 * @param data The Base16 encoded string to be decoded.
 * @return The decoded vector of unsigned characters.
 */
[[nodiscard]] std::vector<unsigned char> decodeBase16(const std::string &data);
}  // namespace Atom::Algorithm

#endif
