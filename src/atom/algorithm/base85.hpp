/*
 * base85.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Base85 encoding and decoding

**************************************************/

#ifndef ATOM_ALGORITHM_BASE85_HPP
#define ATOM_ALGORITHM_BASE85_HPP

#include <vector>

namespace Atom::Algorithm {
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
[[nodiscard]] std::string encodeBase85(const std::vector<unsigned char> &data);

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
[[nodiscard]] std::vector<unsigned char> decodeBase85(const std::string &data);
}  // namespace Atom::Algorithm

#endif
