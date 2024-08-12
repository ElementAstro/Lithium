/*
 * math.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Extra Math Library

**************************************************/

#ifndef ATOM_ALGORITHM_MATH_HPP
#define ATOM_ALGORITHM_MATH_HPP

#include <cstdint>

namespace atom::algorithm {
/**
 * @brief Performs a 64-bit multiplication followed by division.
 *
 * This function calculates the result of (operant * multiplier) / divider.
 *
 * @param operant The first operand for multiplication.
 * @param multiplier The second operand for multiplication.
 * @param divider The divisor for the division operation.
 * @return The result of (operant * multiplier) / divider.
 */
auto mulDiv64(uint64_t operant, uint64_t multiplier,
              uint64_t divider) -> uint64_t;

/**
 * @brief Performs a safe addition operation.
 *
 * This function adds two unsigned 64-bit integers, handling potential overflow.
 *
 * @param a The first operand for addition.
 * @param b The second operand for addition.
 * @return The result of a + b, or 0 if there is an overflow.
 */
auto safeAdd(uint64_t a, uint64_t b) -> uint64_t;

/**
 * @brief Performs a safe multiplication operation.
 *
 * This function multiplies two unsigned 64-bit integers, handling potential
 * overflow.
 *
 * @param a The first operand for multiplication.
 * @param b The second operand for multiplication.
 * @return The result of a * b, or 0 if there is an overflow.
 */
auto safeMul(uint64_t a, uint64_t b) -> uint64_t;

/**
 * @brief Rotates a 64-bit integer to the left.
 *
 * This function rotates a 64-bit integer to the left by a specified number of
 * bits.
 *
 * @param n The 64-bit integer to rotate.
 * @param c The number of bits to rotate.
 * @return The rotated 64-bit integer.
 */
auto rotl64(uint64_t n, unsigned int c) -> uint64_t;

/**
 * @brief Rotates a 64-bit integer to the right.
 *
 * This function rotates a 64-bit integer to the right by a specified number of
 * bits.
 *
 * @param n The 64-bit integer to rotate.
 * @param c The number of bits to rotate.
 * @return The rotated 64-bit integer.
 */
auto rotr64(uint64_t n, unsigned int c) -> uint64_t;

/**
 * @brief Counts the leading zeros in a 64-bit integer.
 *
 * This function counts the number of leading zeros in a 64-bit integer.
 *
 * @param x The 64-bit integer to count leading zeros in.
 * @return The number of leading zeros in the 64-bit integer.
 */
auto clz64(uint64_t x) -> int;

/**
 * @brief Normalizes a 64-bit integer.
 *
 * This function normalizes a 64-bit integer by shifting it to the right until
 * the most significant bit is set.
 *
 * @param x The 64-bit integer to normalize.
 * @return The normalized 64-bit integer.
 */
auto normalize(uint64_t x) -> uint64_t;

/**
 * @brief Performs a safe subtraction operation.
 *
 * This function subtracts two unsigned 64-bit integers, handling potential
 * underflow.
 *
 * @param a The first operand for subtraction.
 * @param b The second operand for subtraction.
 * @return The result of a - b, or 0 if there is an underflow.
 */
auto safeSub(uint64_t a, uint64_t b) -> uint64_t;

/**
 * @brief Performs a safe division operation.
 *
 * This function divides two unsigned 64-bit integers, handling potential
 * division by zero.
 *
 * @param a The numerator for division.
 * @param b The denominator for division.
 * @return The result of a / b, or 0 if there is a division by zero.
 */
auto safeDiv(uint64_t a, uint64_t b) -> uint64_t;

/**
 * @brief Calculates the bitwise reverse of a 64-bit integer.
 *
 * This function calculates the bitwise reverse of a 64-bit integer.
 *
 * @param n The 64-bit integer to reverse.
 * @return The bitwise reverse of the 64-bit integer.
 */
auto bitReverse64(uint64_t n) -> uint64_t;

/**
 * @brief Approximates the square root of a 64-bit integer.
 *
 * This function approximates the square root of a 64-bit integer using a fast
 * algorithm.
 *
 * @param n The 64-bit integer for which to approximate the square root.
 * @return The approximate square root of the 64-bit integer.
 */
auto approximateSqrt(uint64_t n) -> uint64_t;

/**
 * @brief Calculates the greatest common divisor (GCD) of two 64-bit integers.
 *
 * This function calculates the greatest common divisor (GCD) of two 64-bit
 * integers.
 *
 * @param a The first 64-bit integer.
 * @param b The second 64-bit integer.
 * @return The greatest common divisor of the two 64-bit integers.
 */
auto gcd64(uint64_t a, uint64_t b) -> uint64_t;

/**
 * @brief Calculates the least common multiple (LCM) of two 64-bit integers.
 *
 * This function calculates the least common multiple (LCM) of two 64-bit
 * integers.
 *
 * @param a The first 64-bit integer.
 * @param b The second 64-bit integer.
 * @return The least common multiple of the two 64-bit integers.
 */
auto lcm64(uint64_t a, uint64_t b) -> uint64_t;

/**
 * @brief Checks if a 64-bit integer is a power of two.
 *
 * This function checks if a 64-bit integer is a power of two.
 *
 * @param n The 64-bit integer to check.
 * @return True if the 64-bit integer is a power of two, false otherwise.
 */
auto isPowerOfTwo(uint64_t n) -> bool;

/**
 * @brief Calculates the next power of two for a 64-bit integer.
 *
 * This function calculates the next power of two for a 64-bit integer.
 *
 * @param n The 64-bit integer for which to calculate the next power of two.
 * @return The next power of two for the 64-bit integer.
 */
auto nextPowerOfTwo(uint64_t n) -> uint64_t;
}  // namespace atom::algorithm

#endif
