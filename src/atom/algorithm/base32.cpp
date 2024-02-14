/*
 * base32.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Base32

**************************************************/

#include "base32.hpp"

#include <vector>
#include <cstdint>

namespace Atom::Algorithm
{
    std::string base32Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

    std::string encodeBase32(const std::string &input)
    {
        std::string output;

        auto getChunk = [&](size_t index)
        {
            uint64_t chunk = 0;
            for (size_t i = 0; i < 5; ++i)
            {
                if (index * 5 + i < input.size())
                {
                    chunk |= static_cast<uint64_t>(input[index * 5 + i]) << (32 - i * 8);
                }
            }
            return chunk;
        };

        for (size_t i = 0; i < (input.size() + 4) / 5; ++i)
        {
            uint64_t chunk = getChunk(i);

            for (size_t j = 0; j < 8; ++j)
            {
                uint8_t index = (chunk >> (35 - j * 5)) & 0x1f;
                output += base32Chars[index];
            }

            if (input.size() - i * 5 < 5)
            {
                output.replace(output.size() - (5 - input.size() % 5), 5 - input.size() % 5, 5 - input.size() % 5, '=');
            }
        }

        return output;
    }

    std::string decodeBase32(const std::string &input)
    {
        std::string output;
        std::vector<uint8_t> bytes;

        auto getChunk = [&](size_t index)
        {
            uint64_t chunk = 0;
            for (size_t i = 0; i < 8; ++i)
            {
                char c = input[index * 8 + i];
                if (c == '=')
                {
                    break;
                }

                uint8_t base32Index = base32Chars.find(c);
                if (base32Index == std::string::npos)
                {
                    throw std::invalid_argument("Invalid Base32 character: " + c);
                }

                chunk |= static_cast<uint64_t>(base32Index) << (35 - i * 5);
            }
            return chunk;
        };

        for (size_t i = 0; i < input.size() / 8; ++i)
        {
            uint64_t chunk = getChunk(i);

            for (size_t j = 0; j < 5; ++j)
            {
                uint8_t byte = (chunk >> (32 - j * 8)) & 0xff;
                if (byte != 0)
                {
                    bytes.push_back(byte);
                }
            }
        }

        output.assign(bytes.begin(), bytes.end());

        return output;
    }
}
