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

#include <algorithm>
#include <array>
#include <bit>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif

namespace atom::algorithm {
std::string base16Encode(const std::vector<unsigned char> &data) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    for (unsigned char byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

std::vector<unsigned char> base16Decode(const std::string &data) {
    std::vector<unsigned char> result;

    for (size_t i = 0; i < data.length(); i += 2) {
        std::string byteStr = data.substr(i, 2);
        unsigned char byte =
            static_cast<unsigned char>(std::stoi(byteStr, nullptr, 16));
        result.push_back(byte);
    }

    return result;
}

constexpr std::string_view base32_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

std::string base32Encode(const uint8_t *data, size_t length) {
    std::string result;
    result.reserve((length + 4) / 5 * 8);

    size_t bits = 0;
    int num_bits = 0;
    for (size_t i = 0; i < length; ++i) {
        bits = (bits << 8) | data[i];
        num_bits += 8;
        while (num_bits >= 5) {
            result.push_back(base32_chars[(bits >> (num_bits - 5)) & 0x1F]);
            num_bits -= 5;
        }
    }

    if (num_bits > 0) {
        bits <<= (5 - num_bits);
        result.push_back(base32_chars[bits & 0x1F]);
    }

    int padding_chars = (8 - result.size() % 8) % 8;
    result.append(padding_chars, '=');

    return result;
}

std::string base32Decode(std::string_view encoded) {
    std::string result;
    result.reserve(encoded.size() * 5 / 8);

    size_t bits = 0;
    int num_bits = 0;
    for (char c : encoded) {
        if (c == '=') {
            break;
        }
        auto pos = base32_chars.find(c);
        if (pos == std::string_view::npos) {
            throw std::invalid_argument(
                "Invalid character in Base32 encoded string");
        }
        bits = (bits << 5) | pos;
        num_bits += 5;
        if (num_bits >= 8) {
            result.push_back(static_cast<char>(bits >> (num_bits - 8)));
            num_bits -= 8;
        }
    }

    return result;
}

std::string base64Encode(std::string_view bytes_to_encode) {
    static constexpr std::string_view kBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    ret.reserve((bytes_to_encode.size() + 2) / 3 * 4);

    std::array<unsigned char, 3> char_array_3{};
    std::array<unsigned char, 4> char_array_4{};

    auto it = bytes_to_encode.begin();
    auto end = bytes_to_encode.end();

    while (it != end) {
        int i = 0;
        for (; i < 3 && it != end; ++i, ++it) {
            char_array_3[i] = static_cast<unsigned char>(*it);
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; j < i + 1; ++j) {
            ret.push_back(kBase64Chars[char_array_4[j]]);
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

std::string base64Decode(std::string_view encoded_string) {
    static constexpr std::string_view kBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    ret.reserve(encoded_string.size() / 4 * 3);

    std::array<unsigned char, 4> char_array_4{};
    std::array<unsigned char, 3> char_array_3{};

    auto it = encoded_string.begin();
    auto end = encoded_string.end();

    while (it != end) {
        int i = 0;
        for (; i < 4 && it != end && *it != '='; ++i, ++it) {
            char_array_4[i] =
                static_cast<unsigned char>(kBase64Chars.find(*it));
        }

        char_array_3[0] =
            (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int j = 0; j < i - 1; ++j) {
            ret.push_back(static_cast<char>(char_array_3[j]));
        }
    }

    return ret;
}

const std::string base85_chars =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<"
    "=>?@^_`{|}~";

std::string base85Encode(const std::vector<unsigned char> &data) {
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

std::vector<unsigned char> base85Decode(const std::string &data) {
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

constexpr std::array<char, 91> kEncodeTable = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '#', '$',
    '%', '&', '(', ')', '*', '+', ',', '.', '/', ':', ';', '<', '=',
    '>', '?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', '"'};

constexpr std::array<int, 256> kDecodeTable = []() {
    std::array<int, 256> table{};
    table.fill(-1);
    for (int i = 0; i < kEncodeTable.size(); ++i) {
        table[kEncodeTable[i]] = i;
    }
    return table;
}();

std::string base91Encode(std::string_view data) {
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
            result += kEncodeTable[ev % 91];
            result += kEncodeTable[ev / 91];
        }
    }

    if (en > 0) {
        result += kEncodeTable[ebq % 91];
        if (en > 7 || ebq > 90) {
            result += kEncodeTable[ebq / 91];
        }
    }

    return result;
}

std::string base91Decode(std::string_view data) {
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
            dv = kDecodeTable[c];
        } else {
            dv += kDecodeTable[c] * 91;
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

std::string base128Encode(const uint8_t *data, size_t length) {
    std::string result;
    result.reserve((length * 8 + 6) / 7);

    size_t bits = 0;
    int num_bits = 0;
    for (size_t i = 0; i < length; ++i) {
        bits = (bits << 8) | data[i];
        num_bits += 8;
        while (num_bits >= 7) {
            result.push_back(
                static_cast<char>((bits >> (num_bits - 7)) & 0x7F));
            num_bits -= 7;
        }
    }

    if (num_bits > 0) {
        bits <<= (7 - num_bits);
        result.push_back(static_cast<char>(bits & 0x7F));
    }

    return result;
}

std::string base128Decode(std::string_view encoded) {
    std::string result;
    result.reserve(encoded.size() * 7 / 8);

    size_t bits = 0;
    int num_bits = 0;
    for (char c : encoded) {
        if (static_cast<uint8_t>(c) > 127) {
            throw std::invalid_argument(
                "Invalid character in Base128 encoded string");
        }
        bits = (bits << 7) | static_cast<uint8_t>(c);
        num_bits += 7;
        if (num_bits >= 8) {
            result.push_back(static_cast<char>(bits >> (num_bits - 8)));
            num_bits -= 8;
        }
    }

    return result;
}

std::string xorEncrypt(std::string_view plaintext, uint8_t key) {
    std::string ciphertext;
    ciphertext.reserve(plaintext.size());
    for (char c : plaintext) {
        ciphertext.push_back(static_cast<char>(static_cast<uint8_t>(c) ^ key));
    }
    return ciphertext;
}

std::string xorDecrypt(std::string_view ciphertext, uint8_t key) {
    return xorEncrypt(ciphertext, key);
}
}  // namespace atom::algorithm
