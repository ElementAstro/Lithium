/*
 * disk.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Disk

**************************************************/

#ifndef ATOM_SYSTEM_MODULE_DISK_HPP
#define ATOM_SYSTEM_MODULE_DISK_HPP

#include <string>
#include <vector>

namespace atom::system {
/**
 * @brief Get the disk usage for all disks.
 * 获取所有磁盘的使用情况
 *
 * @return A vector of pairs containing the disk name and its usage percentage.
 *         包含磁盘名称和使用率百分比的一对对的向量
 */
[[nodiscard]] std::vector<std::pair<std::string, float>> getDiskUsage();

/**
 * @brief Get the drive model.
 * 获取驱动器型号
 *
 * @param drivePath The path of the drive. 驱动器路径
 * @return The drive model.
 *         驱动器型号
 */
[[nodiscard]] std::string getDriveModel(const std::string &drivePath);

/**
 * @brief Get the storage device models.
 * 获取存储设备型号
 *
 * @return A vector of pairs containing the storage device name and its model.
 *         包含存储设备名称和型号的一对对的向量
 */
[[nodiscard]] std::vector<std::pair<std::string, std::string>> getStorageDeviceModels();

/**
 * @brief Get the available drives.
 * 获取可用驱动器
 *
 * @return A vector of available drives.
 *         可用驱动器的向量
 */
[[nodiscard]] std::vector<std::string> getAvailableDrives();

/**
 * @brief Calculate the disk usage percentage.
 * 计算磁盘使用率
 *
 * @param totalSpace The total space of the disk. 磁盘的总空间
 * @param freeSpace The free space of the disk. 磁盘的可用空间
 * @return The disk usage percentage. 磁盘使用率
 */
[[nodiscard]] double calculateDiskUsagePercentage(unsigned long totalSpace,
                                    unsigned long freeSpace);

}  // namespace atom::system
#endif
