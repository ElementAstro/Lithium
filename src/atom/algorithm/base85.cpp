/*
 * base85.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Base85 encoding and decoding

**************************************************/

#include "base85.hpp"

#include <iostream>

namespace Atom::Algorithm {
const std::string base85_chars =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<"
    "=>?@^_`{|}~";

std::string encodeBase85(const std::vector<unsigned char> &data) {
    std::string result;

    unsigned int value = 0;
    int count = 0;

    for (unsigned char byte : data) {
        value = value * 256 + byte;
        count += 8;

        while (count >= 5) {
            int index = (value >> (count - 5)) & 0x1F;
            result += base85_chars[index];
            count -= 5;
        }
    }

    if (count > 0) {
        value <<= (5 - count);
        int index = value & 0x1F;
        result += base85_chars[index];
    }

    return result;
}

std::vector<unsigned char> decodeBase85(const std::string &data) {
    std::vector<unsigned char> result;

    unsigned int value = 0;
    int count = 0;

    for (char c : data) {
        if (c >= '!' && c <= 'u') {
            value = value * 85 + (c - '!');
            count += 5;

            if (count >= 8) {
                result.push_back((value >> (count - 8)) & 0xFF);
                count -= 8;
            }
        }
    }

    return result;
}
}  // namespace Atom::Algorithm
