/*
 * base.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: A collection of algorithms for C++

**************************************************/

#include "base.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include "atom/error/exception.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#endif

namespace atom::algorithm {
auto base16Encode(const std::vector<unsigned char> &data) -> std::string {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    for (unsigned char byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

auto base16Decode(const std::string &data) -> std::vector<unsigned char> {
    std::vector<unsigned char> result;

    for (size_t i = 0; i < data.length(); i += 2) {
        std::string byteStr = data.substr(i, 2);
        auto byte = static_cast<unsigned char>(std::stoi(byteStr, nullptr, 16));
        result.push_back(byte);
    }

    return result;
}

static constexpr std::string_view BASE32_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

auto base32Encode(const uint8_t *data, size_t length) -> std::string {
    std::string result;
    result.reserve((length + 4) / 5 * 8);

    size_t bits = 0;
    int numBits = 0;
    for (size_t i = 0; i < length; ++i) {
        bits = (bits << 8) | data[i];
        numBits += 8;
        while (numBits >= 5) {
            result.push_back(BASE32_CHARS[(bits >> (numBits - 5)) & 0x1F]);
            numBits -= 5;
        }
    }

    if (numBits > 0) {
        bits <<= (5 - numBits);
        result.push_back(BASE32_CHARS[bits & 0x1F]);
    }

    int paddingChars = (8 - result.size() % 8) % 8;
    result.append(paddingChars, '=');

    return result;
}

auto base32Decode(std::string_view encoded) -> std::string {
    std::string result;
    result.reserve(encoded.size() * 5 / 8);

    size_t bits = 0;
    int numBits = 0;
    for (char c : encoded) {
        if (c == '=') {
            break;
        }
        auto pos = BASE32_CHARS.find(c);
        if (pos == std::string_view::npos) {
            THROW_INVALID_ARGUMENT(
                "Invalid character in Base32 encoded string");
        }
        bits = (bits << 5) | pos;
        numBits += 5;
        if (numBits >= 8) {
            result.push_back(static_cast<char>(bits >> (numBits - 8)));
            numBits -= 8;
        }
    }

    return result;
}

auto base64Encode(std::string_view bytes_to_encode) -> std::string {
    static constexpr std::string_view K_BASE64_CHARS =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    ret.reserve((bytes_to_encode.size() + 2) / 3 * 4);

    std::array<unsigned char, 3> charArray3{};
    std::array<unsigned char, 4> charArray4{};

    const auto *it = bytes_to_encode.begin();
    const auto *end = bytes_to_encode.end();

    while (it != end) {
        int i = 0;
        for (; i < 3 && it != end; ++i, ++it) {
            charArray3[i] = static_cast<unsigned char>(*it);
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] =
            ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] =
            ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;

        for (int j = 0; j < i + 1; ++j) {
            ret.push_back(K_BASE64_CHARS[charArray4[j]]);
        }

        if (i < 3) {
            for (int j = i; j < 3; ++j) {
                ret.push_back('=');
            }
            break;
        }
    }

    return ret;
}

auto base64Decode(std::string_view encoded_string) -> std::string {
    static constexpr std::string_view K_BASE64_CHARS =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    ret.reserve(encoded_string.size() / 4 * 3);

    std::array<unsigned char, 4> charArray4{};
    std::array<unsigned char, 3> charArray3{};

    const auto *it = encoded_string.begin();
    const auto *end = encoded_string.end();

    while (it != end) {
        int i = 0;
        for (; i < 4 && it != end && *it != '='; ++i, ++it) {
            charArray4[i] =
                static_cast<unsigned char>(K_BASE64_CHARS.find(*it));
        }

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] =
            ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
        charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

        for (int j = 0; j < i - 1; ++j) {
            ret.push_back(static_cast<char>(charArray3[j]));
        }
    }

    return ret;
}

static constexpr std::string_view BASE85_CHARS =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<"
    "=>?@^_`{|}~";

auto base85Encode(const std::vector<unsigned char> &data) -> std::string {
    std::string result;

    unsigned int value = 0;
    int count = 0;

    for (unsigned char byte : data) {
        value = value * 256 + byte;
        count += 8;

        while (count >= 5) {
            int index = (value >> (count - 5)) & 0x1F;
            result += BASE85_CHARS[index];
            count -= 5;
        }
    }

    if (count > 0) {
        value <<= (5 - count);
        int index = value & 0x1F;
        result += BASE85_CHARS[index];
    }

    return result;
}

auto base85Decode(const std::string &data) -> std::vector<unsigned char> {
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

constexpr std::array<char, 91> K_ENCODE_TABLE = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '#', '$',
    '%', '&', '(', ')', '*', '+', ',', '.', '/', ':', ';', '<', '=',
    '>', '?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', '"'};

constexpr std::array<int, 256> K_DECODE_TABLE = []() {
    std::array<int, 256> table{};
    table.fill(-1);
    for (int i = 0; i < K_ENCODE_TABLE.size(); ++i) {
        table[K_ENCODE_TABLE[i]] = i;
    }
    return table;
}();

auto base91Encode(std::string_view data) -> std::string {
    std::string result;
    result.reserve(data.size() * 2);

    int ebq = 0;
    int en = 0;
    for (char c : data) {
        ebq |= static_cast<unsigned char>(c) << en;
        en += 8;
        if (en > 13) {
            int ev = ebq & 8191;
            if (ev > 88) {
                ebq >>= 13;
                en -= 13;
            } else {
                ev = ebq & 16383;
                ebq >>= 14;
                en -= 14;
            }
            result += K_ENCODE_TABLE[ev % 91];
            result += K_ENCODE_TABLE[ev / 91];
        }
    }

    if (en > 0) {
        result += K_ENCODE_TABLE[ebq % 91];
        if (en > 7 || ebq > 90) {
            result += K_ENCODE_TABLE[ebq / 91];
        }
    }

    return result;
}

auto base91Decode(std::string_view data) -> std::string {
    std::string result;
    result.reserve(data.size());

    int dbq = 0;
    int dn = 0;
    int dv = -1;

    for (char c : data) {
        if (c == '"') {
            continue;
        }
        if (dv == -1) {
            dv = K_DECODE_TABLE[c];
        } else {
            dv += K_DECODE_TABLE[c] * 91;
            dbq |= dv << dn;
            dn += (dv & 8191) > 88 ? 13 : 14;
            do {
                result += static_cast<char>(dbq & 0xFF);
                dbq >>= 8;
                dn -= 8;
            } while (dn > 7);
            dv = -1;
        }
    }

    if (dv != -1) {
        result += static_cast<char>((dbq | dv << dn) & 0xFF);
    }

    return result;
}

auto base128Encode(const uint8_t *data, size_t length) -> std::string {
    std::string result;
    result.reserve((length * 8 + 6) / 7);

    size_t bits = 0;
    int numBits = 0;
    for (size_t i = 0; i < length; ++i) {
        bits = (bits << 8) | data[i];
        numBits += 8;
        while (numBits >= 7) {
            result.push_back(static_cast<char>((bits >> (numBits - 7)) & 0x7F));
            numBits -= 7;
        }
    }

    if (numBits > 0) {
        bits <<= (7 - numBits);
        result.push_back(static_cast<char>(bits & 0x7F));
    }

    return result;
}

auto base128Decode(std::string_view encoded) -> std::string {
    std::string result;
    result.reserve(encoded.size() * 7 / 8);

    size_t bits = 0;
    int numBits = 0;
    for (char c : encoded) {
        if (static_cast<uint8_t>(c) > 127) {
            THROW_INVALID_ARGUMENT(
                "Invalid character in Base128 encoded string");
        }
        bits = (bits << 7) | static_cast<uint8_t>(c);
        numBits += 7;
        if (numBits >= 8) {
            result.push_back(static_cast<char>(bits >> (numBits - 8)));
            numBits -= 8;
        }
    }

    return result;
}

auto xorEncrypt(std::string_view plaintext, uint8_t key) -> std::string {
    std::string ciphertext;
    ciphertext.reserve(plaintext.size());
    for (char c : plaintext) {
        ciphertext.push_back(static_cast<char>(static_cast<uint8_t>(c) ^ key));
    }
    return ciphertext;
}

auto xorDecrypt(std::string_view ciphertext, uint8_t key) -> std::string {
    return xorEncrypt(ciphertext, key);
}
}  // namespace atom::algorithm
