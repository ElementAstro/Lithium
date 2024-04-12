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
#include <bit>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif

namespace Atom::Algorithm {
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

std::string base64Encode(const std::vector<unsigned char> &bytes_to_encode) {
    static const std::string kBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    for (const auto &byte : bytes_to_encode) {
        char_array_3[i++] = byte;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) +
                              ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) +
                              ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; i < 4; i++) {
                ret += kBase64Chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; j < i + 1; j++) {
            ret += kBase64Chars[char_array_4[j]];
        }
        while ((i++ < 3)) {
            ret += '=';
        }
    }
    return ret;
}

std::vector<unsigned char> base64Decode(const std::string &encoded_string) {
    static const std::string kBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in_] != '=') &&
           (kBase64Chars.find(encoded_string[in_]) != std::string::npos)) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = static_cast<unsigned char>(
                    kBase64Chars.find(char_array_4[i]));
            }
            char_array_3[0] =
                (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) +
                              ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; i < 3; i++) {
                ret.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }
    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }
        for (j = 0; j < 4; j++) {
            char_array_4[j] =
                static_cast<unsigned char>(kBase64Chars.find(char_array_4[j]));
        }
        char_array_3[0] =
            (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; j < i - 1; j++) {
            ret.push_back(char_array_3[j]);
        }
    }
    return ret;
}

std::string base64EncodeEnhance(const std::vector<uint8_t> &bytes_to_encode) {
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string encoded_data;
    size_t input_length = bytes_to_encode.size();

    for (size_t i = 0; i < input_length; i += 3) {
        uint32_t padded_value = 0;
        int padding_count = 0;

        for (size_t j = 0; j < 3; ++j) {
            if (i + j < input_length) {
                padded_value <<= 8;
                padded_value |= bytes_to_encode[i + j];
            } else {
                padded_value <<= 8;
                ++padding_count;
            }
        }

        for (int k = 0; k < 4 - padding_count; ++k) {
            uint8_t index = (padded_value >> (6 * (3 - k))) & 0x3F;
            encoded_data += base64_chars[index];
        }

        for (int k = 0; k < padding_count; ++k) {
            encoded_data += '=';
        }
    }

    return encoded_data;
}

std::vector<uint8_t> base64DecodeEnhance(const std::string &encoded_string) {
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::vector<uint8_t> decoded_data;
    size_t input_length = encoded_string.length();
    size_t padding_count =
        std::count(encoded_string.begin(), encoded_string.end(), '=');
    size_t output_length = (3 * input_length) / 4 - padding_count;

    uint32_t padded_value = 0;
    int padding_index = 0;

    for (size_t i = 0; i < input_length; ++i) {
        if (encoded_string[i] == '=') {
            padded_value <<= 6;
            ++padding_index;
        } else {
            uint8_t value = base64_chars.find(encoded_string[i]);
            padded_value <<= 6;
            padded_value |= value;
        }

        if ((i + 1) % 4 == 0 && padding_index < padding_count) {
            for (int j = 0; j < 3 - padding_index; ++j) {
                uint8_t byte = (padded_value >> (16 - (j + 1) * 8)) & 0xFF;
                decoded_data.push_back(byte);
            }
            padding_index = 0;
            padded_value = 0;
        }
    }

    return decoded_data;
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
}  // namespace Atom::Algorithm