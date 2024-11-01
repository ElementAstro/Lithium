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

/**
 * @class MD5
 * @brief A class that implements the MD5 hashing algorithm.
 */
class MD5 {
public:
    /**
     * @brief Encrypts the input string using the MD5 algorithm.
     * @param input The input string to be hashed.
     * @return The MD5 hash of the input string.
     */
    static auto encrypt(const std::string &input) -> std::string;

private:
    /**
     * @brief Initializes the MD5 context.
     */
    void init();

    /**
     * @brief Updates the MD5 context with a new input string.
     * @param input The input string to update the context with.
     */
    void update(const std::string &input);

    /**
     * @brief Finalizes the MD5 hash and returns the result.
     * @return The finalized MD5 hash as a string.
     */
    auto finalize() -> std::string;

    /**
     * @brief Processes a 512-bit block of the input.
     * @param block A pointer to the 512-bit block.
     */
    void processBlock(const uint8_t *block);

    /**
     * @brief MD5 auxiliary function F.
     * @param x Input value.
     * @param y Input value.
     * @param z Input value.
     * @return The result of the function.
     */
    static auto F(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief MD5 auxiliary function G.
     * @param x Input value.
     * @param y Input value.
     * @param z Input value.
     * @return The result of the function.
     */
    static auto G(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief MD5 auxiliary function H.
     * @param x Input value.
     * @param y Input value.
     * @param z Input value.
     * @return The result of the function.
     */
    static auto H(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief MD5 auxiliary function I.
     * @param x Input value.
     * @param y Input value.
     * @param z Input value.
     * @return The result of the function.
     */
    static auto I(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief Rotates the bits of x to the left by n positions.
     * @param x The value to be rotated.
     * @param n The number of positions to rotate.
     * @return The rotated value.
     */
    static auto leftRotate(uint32_t x, uint32_t n) -> uint32_t;

    /**
     * @brief Reverses the byte order of a 32-bit value.
     * @param x The value to reverse.
     * @return The byte-reversed value.
     */
    static auto reverseBytes(uint32_t x) -> uint32_t;

    uint32_t a_, b_, c_, d_;       ///< MD5 state variables.
    uint64_t count_;               ///< Number of bits processed.
    std::vector<uint8_t> buffer_;  ///< Input buffer.
};

}  // namespace atom::algorithm

#endif  // ATOM_UTILS_MD5_HPP
