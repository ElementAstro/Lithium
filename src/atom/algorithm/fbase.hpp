/*
 * fbase.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Faster Base64 Encode & Decode from pocketpy

**************************************************/

#ifndef ATOM_ALGORITHM_FBASE_HPP
#define ATOM_ALGORITHM_FBASE_HPP

#include <span>
#include <string>
#include <vector>

namespace atom::algorithm {
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
}  // namespace atom::algorithm

#endif
