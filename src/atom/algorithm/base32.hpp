/*
 * base32.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Base32

**************************************************/

#ifndef ATOM_ALGORITHM_BASE32_HPP
#define ATOM_ALGORITHM_BASE32_HPP

#include <string>

namespace Atom::Algorithm {
/**
 * @brief Encodes a string to Base32
 * @param input The string to encode
 * @return The encoded string
 */
[[nodiscard]] std::string encodeBase32(const std::string &input);

/**
 * @brief Decodes a Base32 string
 * @param input The string to decode
 * @return The decoded string
 */
[[nodiscard]] std::string decodeBase32(const std::string &input);
}  // namespace Atom::Algorithm

#endif