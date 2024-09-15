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
 * @brief Retrieves the disk usage information for all available disks.
 *
 * This function scans the system for all connected disks and calculates the
 * disk usage for each one. It returns a vector of pairs where the first element
 * is the name of the disk (e.g., "/dev/sda1") and the second element is the
 * disk's usage percentage. The usage percentage is calculated based on the
 * total space and available space on the disk.
 *
 * @return A vector of pairs where each pair consists of:
 *         - A string representing the disk name.
 *         - A float representing the usage percentage of the disk.
 */
[[nodiscard]] auto getDiskUsage() -> std::vector<std::pair<std::string, float>>;

/**
 * @brief Retrieves the model of a specified drive.
 *
 * Given the path to a drive (e.g., "/dev/sda"), this function returns the model
 * information of the drive. This information is typically extracted from system
 * files or using system-level APIs that provide details about the storage
 * hardware.
 *
 * @param drivePath A string representing the path of the drive.
 *                  For example, "/dev/sda" or "C:\\" on Windows.
 * @return A string containing the model name of the drive.
 */
[[nodiscard]] auto getDriveModel(const std::string& drivePath) -> std::string;

/**
 * @brief Retrieves the models of all connected storage devices.
 *
 * This function queries the system for all connected storage devices (e.g.,
 * hard drives, SSDs) and returns a list of their names along with their
 * respective models. Each element in the returned vector is a pair, where the
 * first element is the name of the storage device (e.g., "/dev/sda" or "C:\\"
 * on Windows) and the second element is the device's model name.
 *
 * @return A vector of pairs where each pair consists of:
 *         - A string representing the storage device name.
 *         - A string representing the model name of the storage device.
 */
[[nodiscard]] auto getStorageDeviceModels()
    -> std::vector<std::pair<std::string, std::string>>;

/**
 * @brief Retrieves a list of all available drives on the system.
 *
 * This function returns a list of all available drives currently recognized by
 * the system. The drives are represented by their mount points or device paths.
 * For example, on Linux, the returned list may contain paths such as
 * "/dev/sda1", while on Windows it may contain drive letters like "C:\\".
 *
 * @return A vector of strings where each string represents an available drive.
 */
[[nodiscard]] auto getAvailableDrives() -> std::vector<std::string>;

/**
 * @brief Calculates the disk usage percentage.
 *
 * Given the total and free space on a disk, this function computes the disk
 * usage percentage. The calculation is performed using the formula:
 * \f$ \text{Usage Percentage} = \left( \frac{\text{Total Space} - \text{Free
 * Space}}{\text{Total Space}} \right) \times 100 \f$ This percentage represents
 * how much of the disk space is currently used.
 *
 * @param totalSpace The total space on the disk, in bytes.
 * @param freeSpace The free (available) space on the disk, in bytes.
 * @return A double representing the disk usage percentage. The result is
 *         a value between 0.0 and 100.0.
 */
[[nodiscard]] auto calculateDiskUsagePercentage(
    unsigned long totalSpace, unsigned long freeSpace) -> double;

/**
 * @brief Retrieves the file system type for a specified path.
 *
 * This function determines the type of the file system used by the disk at the
 * specified path. The file system type could be, for example, "ext4" for Linux
 * systems, "NTFS" for Windows, or "APFS" for macOS. The function queries the
 * system to retrieve this information and returns the file system type as a
 * string.
 *
 * @param path A string representing the path to the disk or mount point.
 *             For example, "/dev/sda1" or "C:\\".
 * @return A string containing the file system type (e.g., "ext4", "NTFS",
 * "APFS").
 */
[[nodiscard]] auto getFileSystemType(const std::string& path) -> std::string;

}  // namespace atom::system

#endif
