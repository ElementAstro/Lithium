/*
 * memory.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Memory

**************************************************/

#ifndef ATOM_SYSTEM_MODULE_MEMORY_HPP
#define ATOM_SYSTEM_MODULE_MEMORY_HPP

#include <string>
#include <utility>
#include <vector>

#include "macro.hpp"

namespace atom::system {
struct MemoryInfo {
    struct MemorySlot {
        std::string capacity;
        std::string clockSpeed;
        std::string type;

        MemorySlot() = default;
        MemorySlot(std::string capacity, std::string clockSpeed,
                   std::string type)
            : capacity(std::move(capacity)),
              clockSpeed(std::move(clockSpeed)),
              type(std::move(type)) {}
    } ATOM_ALIGNAS(128);

    std::vector<MemorySlot> slots;
    unsigned long long virtualMemoryMax;
    unsigned long long virtualMemoryUsed;
    unsigned long long swapMemoryTotal;
    unsigned long long swapMemoryUsed;
} ATOM_ALIGNAS(64);

/**
 * @brief Get the memory usage percentage.
 * 获取内存使用率百分比
 *
 * @return The memory usage percentage.
 *         内存使用率百分比
 */
float getMemoryUsage();

/**
 * @brief Get the total memory size.
 * 获取总内存大小
 *
 * @return The total memory size.
 *         总内存大小
 */
unsigned long long getTotalMemorySize();

/**
 * @brief Get the available memory size.
 * 获取可用内存大小
 *
 * @return The available memory size.
 *         可用内存大小
 */
unsigned long long getAvailableMemorySize();

/**
 * @brief Get the physical memory slot info.
 * 获取物理内存槽信息
 *
 * @return The physical memory slot info.
 *         物理内存槽信息
 */
MemoryInfo::MemorySlot getPhysicalMemoryInfo();

/**
 * @brief Get the virtual memory max size.
 * 获取虚拟内存最大值
 *
 * @return The virtual memory max size.
 *         虚拟内存最大值
 */
unsigned long long getVirtualMemoryMax();

/**
 * @brief Get the virtual memory used size.
 * 获取虚拟内存已用值
 *
 * @return The virtual memory used size.
 *         虚拟内存已用值
 */
unsigned long long getVirtualMemoryUsed();

/**
 * @brief Get the swap memory total size.
 * 获取交换内存总值
 *
 * @return The swap memory total size.
 *         交换内存总值
 */
unsigned long long getSwapMemoryTotal();

/**
 * @brief Get the swap memory used size.
 * 获取交换内存已用值
 *
 * @return The swap memory used size.
 *         交换内存已用值
 */
unsigned long long getSwapMemoryUsed();
}  // namespace atom::system

#endif
