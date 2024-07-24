/*
 * cpu.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - CPU

**************************************************/

#ifndef ATOM_SYSTEM_MODULE_CPU_HPP
#define ATOM_SYSTEM_MODULE_CPU_HPP

#include <string>

#include "macro.hpp"

namespace atom::system {
struct CacheSizes {
    size_t l1d;
    size_t l1i;
    size_t l2;
    size_t l3;
} ATOM_ALIGNAS(32);

/**
 * @brief Get the CPU usage percentage.
 * 获取 CPU 使用率百分比
 *
 * @return The CPU usage percentage.
 *         CPU 使用率百分比
 */
auto getCurrentCpuUsage() -> float;

/**
 * @brief Get the CPU temperature.
 * 获取 CPU 温度
 *
 * @return The CPU temperature.
 *         CPU 温度
 */
auto getCurrentCpuTemperature() -> float;

/**
 * @brief Get the CPU model.
 * 获取 CPU 型号
 *
 * @return The CPU model.
 *         CPU 型号
 */
auto getCPUModel() -> std::string;

/**
 * @brief Get the CPU identifier.
 * 获取 CPU 标识
 *
 * @return The CPU identifier.
 *         CPU 标识
 */
auto getProcessorIdentifier() -> std::string;

/**
 * @brief Get the CPU frequency.
 * 获取 CPU 频率
 *
 * @return The CPU frequency.
 *         CPU 频率
 */
auto getProcessorFrequency() -> double;

/**
 * @brief Get the number of physical CPUs.
 * 获取物理 CPU 数量
 *
 * @return The number of physical CPUs.
 *         物理 CPU 数量
 */
auto getNumberOfPhysicalPackages() -> int;

/**
 * @brief Get the number of logical CPUs.
 * 获取逻辑 CPU 数量
 *
 * @return The number of logical CPUs.
 *         逻辑 CPU 数量
 */
auto getNumberOfPhysicalCPUs() -> int;

/**
 * @brief Get the cache sizes.
 * 获取缓存大小
 *
 * @return The cache sizes.
 *         缓存大小
 */
auto getCacheSizes() -> CacheSizes;

}  // namespace atom::system

#endif
