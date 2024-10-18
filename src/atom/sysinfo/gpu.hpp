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
#include <vector>

namespace atom::system {
/**
 * @brief Get GPU information
 * @return std::string GPU information
 */
[[nodiscard]] auto getGPUInfo() -> std::string;

struct alignas(128) MonitorInfo {
    std::string model;
    std::string identifier;
    int width{0};
    int height{0};
    int refreshRate{0};
};

/**
 * @brief Get all monitors information
 * @return std::vector<MonitorInfo> All monitors information
 */
[[nodiscard]]
auto getAllMonitorsInfo() -> std::vector<MonitorInfo>;
}  // namespace atom::system

#endif
