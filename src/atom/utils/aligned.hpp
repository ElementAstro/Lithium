/*
 * aligned.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Validate aligned storage

**************************************************/

#ifndef ATOM_UTILS_ALIGNED_HPP
#define ATOM_UTILS_ALIGNED_HPP

#include <cstdint>
#include <type_traits>

namespace atom::utils {

//! Aligned storage validator
template <size_t ImplSize, size_t ImplAlign, size_t StorageSize, size_t StorageAlign>
class ValidateAlignedStorage {
    static_assert(StorageSize >= ImplSize, "StorageSize must be greater than or equal to ImplSize");
    static_assert(StorageAlign % ImplAlign == 0, "StorageAlign must be a multiple of ImplAlign");
};

} // namespace atom::utils

#endif // ATOM_UTILS_ALIGNED_HPP