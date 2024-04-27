/*
 * leak.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Memory Leak Detection

**************************************************/

#ifndef ATOM_UTILS_LEAK_HPP
#define ATOM_UTILS_LEAK_HPP

#if defined(__clang__)
#pragma clang system_header
#elif defined(__GNUC__)
#pragma GCC system_header
#elif defined(_MSC_VER)
#pragma system_header
#endif

//! @cond INTERNALS
#if defined(_MSC_VER)
#define VLD_FORCE_ENABLE
#include <vld.h>
#endif
//! @endcond

#endif  // ATOM_UTILS_LEAK_HPP
