/*
 * base64.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-5

Description: Base64

**************************************************/

#ifndef BASE64_ENC_DEC_H_
#define BASE64_ENC_DEC_H_
#include <string>
#include <vector>
#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif
/**
 * @brief Base64编码函数
 *
 * @param bytes_to_encode 待编码数据
 * @return std::string 编码后的字符串
 */
std::string base64Encode(const std::vector<unsigned char> &bytes_to_encode)
{
    static const std::string kBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    for (const auto &byte : bytes_to_encode)
    {
        char_array_3[i++] = byte;
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; i < 4; i++)
            {
                ret += kBase64Chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    if (i)
    {
        for (j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; j < i + 1; j++)
        {
            ret += kBase64Chars[char_array_4[j]];
        }
        while ((i++ < 3))
        {
            ret += '=';
        }
    }
    return ret;
}
/**
 * @brief Base64解码函数
 *
 * @param encoded_string 待解码字符串
 * @return std::vector<unsigned char> 解码后的数据
 */
std::vector<unsigned char> base64Decode(const std::string &encoded_string)
{
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

    while (in_len-- && (encoded_string[in_] != '=') && (kBase64Chars.find(encoded_string[in_]) != std::string::npos))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                char_array_4[i] = static_cast<unsigned char>(kBase64Chars.find(char_array_4[i]));
            }
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; i < 3; i++)
            {
                ret.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }
    if (i)
    {
        for (j = i; j < 4; j++)
        {
            char_array_4[j] = 0;
        }
        for (j = 0; j < 4; j++)
        {
            char_array_4[j] = static_cast<unsigned char>(kBase64Chars.find(char_array_4[j]));
        }
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; j < i - 1; j++)
        {
            ret.push_back(char_array_3[j]);
        }
    }
    return ret;
}

#endif // BASE64_ENC_DEC_H_