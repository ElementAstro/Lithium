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

namespace Atom::System
{
    /**
     * @brief Get the disk usage for all disks.
     * 获取所有磁盘的使用情况
     *
     * @return A vector of pairs containing the disk name and its usage percentage.
     *         包含磁盘名称和使用率百分比的一对对的向量
     */
    std::vector<std::pair<std::string, float>> getDiskUsage();

    /**
     * @brief Get the drive model.
     * 获取驱动器型号
     *
     * @param drivePath The path of the drive. 驱动器路径
     * @return The drive model.
     *         驱动器型号
     */
    std::string getDriveModel(const std::string &drivePath);

    /**
     * @brief Get the storage device models.
     * 获取存储设备型号
     *
     * @return A vector of pairs containing the storage device name and its model.
     *         包含存储设备名称和型号的一对对的向量
     */
    std::vector<std::pair<std::string, std::string>> getStorageDeviceModels();

}
#endif