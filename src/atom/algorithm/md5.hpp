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
 * @brief The MD5 class for calculating MD5 hash of input data.
 */
class MD5 {
public:
    /**
     * @brief Encrypts the input string using MD5 algorithm.
     *
     * @param input The input string to be encrypted.
     * @return The MD5 hash of the input string.
     */
    static auto encrypt(const std::string &input) -> std::string;

private:
    /**
     * @brief Initializes internal variables and buffer for MD5 computation.
     */
    void init();

    /**
     * @brief Updates the MD5 computation with additional input data.
     *
     * @param input The input string to be added to the MD5 computation.
     */
    void update(const std::string &input);

    /**
     * @brief Finalizes the MD5 computation and returns the resulting hash.
     *
     * @return The MD5 hash of all the input data provided so far.
     */
    auto finalize() -> std::string;

    /**
     * @brief Processes a 64-byte block of input data.
     *
     * @param block Pointer to the 64-byte block of input data.
     */
    void processBlock(const uint8_t *block);

    /**
     * @brief The F function for MD5 algorithm.
     */
    static auto F(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief The G function for MD5 algorithm.
     */
    static auto G(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief The H function for MD5 algorithm.
     */
    static auto H(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief The I function for MD5 algorithm.
     */
    static auto I(uint32_t x, uint32_t y, uint32_t z) -> uint32_t;

    /**
     * @brief Performs a left rotate operation on the input.
     */
    static auto leftRotate(uint32_t x, uint32_t n) -> uint32_t;

    /**
     * @brief Reverses the bytes in the input.
     */
    static auto reverseBytes(uint32_t x) -> uint32_t;

    uint32_t a_, b_, c_,
        d_;          /**< Internal state variables for MD5 computation. */
    uint64_t count_; /**< Total count of input bits. */
    std::vector<uint8_t> buffer_; /**< Buffer for input data. */
};

}  // namespace atom::algorithm

#endif  // MD5_H
