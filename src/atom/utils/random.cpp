/*
 * random.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-25

Description: Simple random number generator

**************************************************/

#include "random.hpp"

namespace atom::utils {
std::string generateRandomString(int length) {
    const std::string CHARACTERS =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    Random<std::mt19937, std::uniform_int_distribution<int>> random(
        0, CHARACTERS.size() - 1);

    std::string randomString;
    randomString.reserve(length);

    for (int i = 0; i < length; ++i) {
        randomString.push_back(CHARACTERS[random()]);
    }

    return randomString;
}
}  // namespace atom::utils
