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
std::mt19937 createRandomGenerator() {
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    return std::mt19937(seed);
}

std::string generateRandomString(int length) {
    const std::string characters =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator = createRandomGenerator();
    std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

    std::string randomString;
    randomString.reserve(length);

    for (int i = 0; i < length; ++i) {
        randomString.push_back(characters[distribution(generator)]);
    }

    return randomString;
}
}  // namespace atom::utils