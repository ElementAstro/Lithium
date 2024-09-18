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

#include <cstddef>

namespace atom::utils {
/**
 * @brief A class template to validate aligned storage.
 *
 * This class template is used to validate that the storage size and alignment
 * requirements are met for a given implementation. It uses static assertions
 * to ensure that the storage size is greater than or equal to the
 * implementation size, and that the storage alignment is a multiple of the
 * implementation alignment.
 *
 * @tparam ImplSize The size of the implementation.
 * @tparam ImplAlign The alignment of the implementation.
 * @tparam StorageSize The size of the storage.
 * @tparam StorageAlign The alignment of the storage.
 */
template <std::size_t ImplSize, std::size_t ImplAlign, std::size_t StorageSize,
          std::size_t StorageAlign>
class ValidateAlignedStorage {
    static_assert(StorageSize >= ImplSize,
                  "StorageSize must be greater than or equal to ImplSize");
    static_assert(StorageAlign % ImplAlign == 0,
                  "StorageAlign must be a multiple of ImplAlign");
};

}  // namespace atom::utils

#endif  // ATOM_UTILS_ALIGNED_HPP