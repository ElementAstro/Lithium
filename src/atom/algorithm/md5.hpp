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

#include <cstdint>
#include <string>
#include <vector>

namespace atom::algorithm {

class MD5 {
public:
    static auto encrypt(const std::string &input) -> std::string;

private:
    void init();
    void update(const std::string &input);
    auto finalize() -> std::string;
    void processBlock(const uint8_t *block);

    static auto F(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;
    static auto G(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;
    static auto H(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;
    static auto I(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;
    static auto leftRotate(uint32_t x, uint32_t n) -> uint32_t;
    static auto reverseBytes(uint32_t x) -> uint32_t;

    uint32_t a_, b_, c_, d_;
    uint64_t count_;
    std::vector<uint8_t> buffer_;
};

}  // namespace atom::algorithm

#endif  // ATOM_UTILS_MD5_HPP
