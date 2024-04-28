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

#if defined(_WIN32)
#include <objbase.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <uuid/uuid.h>
#endif

namespace atom::utils {
UUIDGenerator::UUIDGenerator() : gen_(rd_()), dis_(0, 0xFFFFFFFF) {}

void UUIDGenerator::seed(unsigned int seed_value) const {
    gen_.seed(seed_value);
}

unsigned int UUIDGenerator::getRandomNumber() const { return dis_(gen_); }

std::string UUIDGenerator::generateUUID(bool use_uppercase, bool use_braces,
                                        bool use_hyphens) const {
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

std::string generateSystemUUID() {
    std::string uuid;

#if defined(_WIN32)
    GUID guid;
    CoCreateGuid(&guid);

    char uuidStr[40] = {0};
    snprintf(
        uuidStr, sizeof(uuidStr),
        "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
        guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
    uuid = uuidStr;

#elif defined(__linux__) || defined(__APPLE__)
    uuid_t id;
    uuid_generate(id);

    char uuidStr[40] = {0};
    uuid_unparse(id, uuidStr);
    uuid = uuidStr;
#endif

    return uuid;
}
}  // namespace atom::utils
