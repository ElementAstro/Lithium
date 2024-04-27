/*
 * validate_aligned_storage.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Validate aligned storage

**************************************************/

#ifndef ATOM_UTILS_VALIDATE_ALIGNED_STORAGE_HPP
#define ATOM_UTILS_VALIDATE_ALIGNED_STORAGE_HPP

#include <cstdint>
#include <type_traits>

namespace Atom::Utils {

//! Aligned storage validator
template <size_t ImplSize, size_t ImplAlign, size_t StorageSize, size_t StorageAlign, class Enable = void>
class ValidateAlignedStorage;

//! Aligned storage validator (specialization)
template <size_t ImplSize, size_t ImplAlign, size_t StorageSize, size_t StorageAlign>
class ValidateAlignedStorage<ImplSize, ImplAlign, StorageSize, StorageAlign, typename std::enable_if<(StorageSize >= ImplSize) && ((StorageAlign % ImplAlign) == 0)>::type> {};

} // namespace Atom::Utils

#endif // ATOM_UTILS_VALIDATE_ALIGNED_STORAGE_HPP
