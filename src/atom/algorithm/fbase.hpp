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
std::string fbase64Encode(std::span<const unsigned char> input);
std::vector<unsigned char> fbase64Decode(std::span<const char> input);
}  // namespace atom::algorithm

#endif
