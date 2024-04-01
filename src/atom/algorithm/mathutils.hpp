/*
 * mathutils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Extra Math Library

**************************************************/

#ifndef ATOM_ALGORITHM_MATHUTILS_HPP
#define ATOM_ALGORITHM_MATHUTILS_HPP

#include <cstdint>

namespace Atom::Algorithm {
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
uint64_t mulDiv64(uint64_t operant, uint64_t multiplier, uint64_t divider);
}  // namespace Atom::Algorithm

#endif