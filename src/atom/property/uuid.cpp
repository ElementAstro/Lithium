/*
 * uuid.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-4-5

Description: UUID Generator

**************************************************/

#include "uuid.hpp"

namespace Atom::Property
{
    UUIDGenerator::UUIDGenerator() : gen_(std::random_device{}()), dis_(0, 15)
    {
    }

    void UUIDGenerator::seed(unsigned int seed_value)
    {
        gen_.seed(seed_value);
    }

    unsigned int UUIDGenerator::getRandomNumber()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> dis(0, 15);
        return dis(gen);
    }

    std::string UUIDGenerator::generateUUID()
    {
        std::stringstream ss;
        for (int i = 0; i < 32; ++i)
        {
            if (i == 8 || i == 12 || i == 16 || i == 20)
                ss << '-';
            ss << std::hex << getRandomNumber();
        }
        return ss.str();
    }

    std::string UUIDGenerator::generateUUIDWithFormat(bool use_braces, bool use_hyphens)
    {
        std::stringstream ss;
        if (use_braces)
            ss << '{';
        ss << std::hex << getRandomNumber();
        if (use_hyphens)
            ss << '-';
        ss << std::hex << getRandomNumber();
        ss << '-';
        ss << std::hex << getRandomNumber();
        if (use_hyphens)
            ss << '-';
        ss << std::hex << getRandomNumber();
        ss << '-';
        for (int i = 0; i < 12; ++i)
            ss << std::hex << getRandomNumber();
        if (use_braces)
            ss << '}';
        return ss.str();
    }

    std::string UUIDGenerator::generateEnhancedUUID()
    {
        std::stringstream ss;

        for (int i = 0; i < 8; ++i)
            ss << std::hex << getRandomNumber();

        ss << '-';

        for (int i = 0; i < 4; ++i)
            ss << std::hex << getRandomNumber();

        ss << '-';

        for (int i = 0; i < 4; ++i)
            ss << std::hex << getRandomNumber();

        ss << '-';

        for (int i = 0; i < 4; ++i)
            ss << std::hex << getRandomNumber();

        ss << '-';

        for (int i = 0; i < 12; ++i)
            ss << std::hex << getRandomNumber();

        return ss.str();
    }
}
