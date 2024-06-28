/*
 * fbase.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Faster Base64 Encode & Decode from pocketpy

**************************************************/

#include <array>

#include "fbase.hpp"

#include "atom/error/exception.hpp"

namespace atom::algorithm {

constexpr char BASE64_PAD = '=';
constexpr char BASE64DE_FIRST = '+';
constexpr char BASE64DE_LAST = 'z';

constexpr std::array<char, 64> BASE64EN = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

constexpr std::array<unsigned char, 256> BASE64DE = [] {
    std::array<unsigned char, 256> arr{};
    arr.fill(255);
    for (unsigned char i = 0; i < 64; ++i) {
        arr[static_cast<unsigned char>(BASE64EN[i])] = i;
    }
    arr['+'] = 62;
    arr['/'] = 63;
    for (unsigned char i = '0'; i <= '9'; ++i) {
        arr[i] = i - '0' + 52;
    }
    for (unsigned char i = 'A'; i <= 'Z'; ++i) {
        arr[i] = i - 'A';
    }
    for (unsigned char i = 'a'; i <= 'z'; ++i) {
        arr[i] = i - 'a' + 26;
    }
    return arr;
}();

auto fbase64Encode(std::span<const unsigned char> input) -> std::string {
    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);

    unsigned int s = 0;
    unsigned char l = 0;

    for (unsigned char c : input) {
        switch (s) {
            case 0:
                output.push_back(BASE64EN[(c >> 2) & 0x3F]);
                s = 1;
                break;
            case 1:
                output.push_back(BASE64EN[((l & 0x3) << 4) | ((c >> 4) & 0xF)]);
                s = 2;
                break;
            case 2:
                output.push_back(BASE64EN[((l & 0xF) << 2) | ((c >> 6) & 0x3)]);
                output.push_back(BASE64EN[c & 0x3F]);
                s = 0;
                break;
        }
        l = c;
    }

    switch (s) {
        case 1:
            output.push_back(BASE64EN[(l & 0x3) << 4]);
            output.push_back(BASE64_PAD);
            output.push_back(BASE64_PAD);
            break;
        case 2:
            output.push_back(BASE64EN[(l & 0xF) << 2]);
            output.push_back(BASE64_PAD);
            break;
    }

    return output;
}

auto fbase64Decode(std::span<const char> input) -> std::vector<unsigned char> {
    if (input.size() % 4 != 0) {
        THROW_INVALID_ARGUMENT("Invalid base64 input length");
    }

    std::vector<unsigned char> output;
    output.reserve(input.size() / 4 * 3);

    unsigned int s = 0;
    unsigned char l = 0;

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == BASE64_PAD) {
            break;
        }

        if (input[i] < BASE64DE_FIRST || input[i] > BASE64DE_LAST) {
            THROW_INVALID_ARGUMENT("Invalid base64 character");
        }

        unsigned char c = BASE64DE[static_cast<unsigned char>(input[i])];
        if (c == 255) {
            THROW_INVALID_ARGUMENT("Invalid base64 character");
        }

        switch (s) {
            case 0:
                output.push_back((c << 2) & 0xFF);
                s = 1;
                break;
            case 1:
                output.back() |= (c >> 4) & 0x3;
                output.push_back((c & 0xF) << 4);
                s = 2;
                break;
            case 2:
                output.back() |= (c >> 2) & 0xF;
                output.push_back((c & 0x3) << 6);
                s = 3;
                break;
            case 3:
                output.back() |= c;
                s = 0;
                break;
        }
    }

    if (s == 1 || s == 2) {
        output.pop_back();
    }
    if (s == 2) {
        output.pop_back();
    }

    return output;
}

}  // namespace atom::algorithm
