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
auto generateRandomString(int length) -> std::string {
    if (length <= 0) {
        THROW_INVALID_ARGUMENT("Length must be a positive integer.");
    }
    const std::string CHARACTERS =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    Random<std::mt19937, std::uniform_int_distribution<int>> rng(
        0, static_cast<int>(CHARACTERS.size() - 1));

    std::string randomString(length, '\0');
    std::generate(randomString.begin(), randomString.end(),
                  [&]() { return CHARACTERS[rng()]; });

    return randomString;
}
}  // namespace atom::utils
