/*
 * base16.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Base16 encoding and decoding

**************************************************/

#include "base16.hpp"

#include <iomanip>
#include <iostream>


namespace Atom::Algorithm {
std::string encodeBase16(const std::vector<unsigned char> &data) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    for (unsigned char byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

std::vector<unsigned char> decodeBase16(const std::string &data) {
    std::vector<unsigned char> result;

    for (size_t i = 0; i < data.length(); i += 2) {
        std::string byteStr = data.substr(i, 2);
        unsigned char byte =
            static_cast<unsigned char>(std::stoi(byteStr, nullptr, 16));
        result.push_back(byte);
    }

    return result;
}
}  // namespace Atom::Algorithm