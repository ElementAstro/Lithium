/*
 * uuid.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: UUID Generator

**************************************************/

#include "uuid.hpp"

namespace Atom::Utils
{
    UUIDGenerator::UUIDGenerator() : gen_(rd_()), dis_(0, 0xFFFFFFFF) {}

    void UUIDGenerator::seed(unsigned int seed_value) const
    {
        gen_.seed(seed_value);
    }

    unsigned int UUIDGenerator::getRandomNumber() const
    {
        return dis_(gen_);
    }

    std::string UUIDGenerator::generateUUID(bool use_uppercase, bool use_braces, bool use_hyphens) const
    {
        unsigned int time_low = getRandomNumber();
        unsigned int time_mid = getRandomNumber();
        unsigned int time_hi_and_version = getRandomNumber();
        time_hi_and_version &= 0x0FFF;
        time_hi_and_version |= (4 << 12);

        unsigned int clock_seq_hi_and_reserved = getRandomNumber();
        clock_seq_hi_and_reserved &= 0x3FFF;
        clock_seq_hi_and_reserved |= 0x8000;

        unsigned int clock_seq_low = getRandomNumber();

        std::stringstream ss;
        if (use_braces)
            ss << '{';
        if (use_uppercase)
            ss << std::hex << std::uppercase;
        ss << std::hex << time_low;
        if (use_hyphens)
            ss << '-';
        ss << std::hex << time_mid;
        if (use_hyphens)
            ss << '-';
        ss << std::hex << time_hi_and_version;
        if (use_hyphens)
            ss << '-';
        ss << std::hex << clock_seq_hi_and_reserved;
        if (use_hyphens)
            ss << '-';
        ss << std::hex << clock_seq_low;
        if (use_uppercase)
            ss << std::nouppercase;
        if (use_braces)
            ss << '}';
        return ss.str();
    }
}
