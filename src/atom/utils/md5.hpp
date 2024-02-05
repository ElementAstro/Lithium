/*
 * md5.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Self implemented MD5 algorithm.

**************************************************/

#ifndef ATOM_UTILS_MD5_HPP
#define ATOM_UTILS_MD5_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace Atom::Utils
{
    class MD5
    {
    public:
        static std::string encrypt(const std::string &input);

    private:
        void init();
        void update(const std::string &input);
        std::string finalize();
        void processBlock(const uint8_t *block);
        static uint32_t F(uint32_t x, uint32_t y, uint32_t z);
        static uint32_t G(uint32_t x, uint32_t y, uint32_t z);
        static uint32_t H(uint32_t x, uint32_t y, uint32_t z);
        static uint32_t I(uint32_t x, uint32_t y, uint32_t z);
        static uint32_t leftRotate(uint32_t x, uint32_t n);
        static uint32_t reverseBytes(uint32_t x);

        uint32_t _a, _b, _c, _d;
        uint64_t _count;
        std::vector<uint8_t> _buffer;
    };
}

#endif // MD5_H
