/*
 * gpu.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - GPU

**************************************************/

#ifndef ATOM_SYSTEM_MODULE_GPU_HPP
#define ATOM_SYSTEM_MODULE_GPU_HPP

#include <string>

namespace atom::system {
/**
 * @brief Get GPU information
 * @return std::string GPU information
 */
[[nodiscard]] std::string getGPUInfo();
}  // namespace atom::system

#endif
