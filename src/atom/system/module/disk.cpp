/*
 * disk.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Disk

**************************************************/

#include "disk.hpp"

#include "atom/log/loguru.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <pdh.h>
#include <tlhelp32.h>
#include <wincon.h>
#elif __linux__
#include <dirent.h>
#include <limits.h>
#include <signal.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <csignal>
#include <iterator>
#elif __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <mach/mach_init.h>
#include <mach/task_info.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/mount.h>
#include <sys/param.h>
#endif

namespace Atom::System {
std::vector<std::pair<std::string, float>> getDiskUsage() {
    std::vector<std::pair<std::string, float>> disk_usage;

#ifdef _WIN32
    DWORD drives = GetLogicalDrives();
    char drive_letter = 'A';

    while (drives) {
        if (drives & 1) {
            std::string drive_path = std::string(1, drive_letter) + ":\\";
            ULARGE_INTEGER total_space, free_space;

            if (GetDiskFreeSpaceExA(drive_path.c_str(), nullptr, &total_space,
                                    &free_space)) {
                unsigned long long total = total_space.QuadPart / (1024 * 1024);
                unsigned long long free = free_space.QuadPart / (1024 * 1024);

                float usage = 100.0 * static_cast<float>(total - free) / total;
                disk_usage.push_back(std::make_pair(drive_path, usage));
            } else {
                LOG_F(ERROR, "GetDiskUsage error: GetDiskFreeSpaceExA error");
            }
        }

        drives >>= 1;
        drive_letter++;
    }
#elif __linux__ || __APPLE__
    std::ifstream file("/proc/mounts");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string device, path;
        iss >> device >> path;

        struct statfs stats;
        if (statfs(path.c_str(), &stats) == 0) {
            unsigned long long totalSpace =
                static_cast<unsigned long long>(stats.f_blocks) * stats.f_bsize;
            unsigned long long freeSpace =
                static_cast<unsigned long long>(stats.f_bfree) * stats.f_bsize;

            unsigned long long usedSpace = totalSpace - freeSpace;
            float usage = static_cast<float>(usedSpace) / totalSpace * 100.0;
            disk_usage.push_back({path, usage});
        } else {
            LOG_F(ERROR, "GetDiskUsage error: statfs error");
        }
    }

#endif

    return disk_usage;
}

std::string getDriveModel(const std::string &drivePath) {
    std::string model;

#ifdef _WIN32
    HANDLE hDevice =
        CreateFileA(drivePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice != INVALID_HANDLE_VALUE) {
        STORAGE_PROPERTY_QUERY query;
        char buffer[1024];
        ZeroMemory(&query, sizeof(query));
        ZeroMemory(buffer, sizeof(buffer));
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;
        DWORD bytesReturned = 0;
        if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query,
                            sizeof(query), buffer, sizeof(buffer),
                            &bytesReturned, NULL)) {
            STORAGE_DEVICE_DESCRIPTOR *desc =
                (STORAGE_DEVICE_DESCRIPTOR *)buffer;
            char *vendorId = (char *)(buffer + desc->VendorIdOffset);
            char *productId = (char *)(buffer + desc->ProductIdOffset);
            char *productRevision =
                (char *)(buffer + desc->ProductRevisionOffset);
            model = vendorId + std::string(" ") + productId + std::string(" ") +
                    productRevision;
        }
        CloseHandle(hDevice);
    }
#elif __APPLE__
    DASessionRef session = DASessionCreate(kCFAllocatorDefault);
    if (session != nullptr) {
        CFURLRef url = CFURLCreateWithFileSystemPath(
            kCFAllocatorDefault,
            CFStringCreateWithCString(kCFAllocatorDefault, drivePath.c_str(),
                                      kCFStringEncodingUTF8),
            kCFURLPOSIXPathStyle, false);
        if (url != nullptr) {
            DADiskRef disk =
                DADiskCreateFromBSDName(kCFAllocatorDefault, session,
                                        CFURLGetFileSystemRepresentation(url));
            if (disk != nullptr) {
                CFDictionaryRef desc = DADiskCopyDescription(disk);
                if (desc != nullptr) {
                    CFStringRef modelRef = (CFStringRef)CFDictionaryGetValue(
                        desc, kDADiskDescriptionDeviceModelKey);
                    if (modelRef != nullptr) {
                        char buffer[256];
                        CFStringGetCString(modelRef, buffer, 256,
                                           kCFStringEncodingUTF8);
                        model = buffer;
                    }
                    CFRelease(desc);
                }
                CFRelease(disk);
            }
            CFRelease(url);
        }
        CFRelease(session);
    }
#elif __linux__
    std::ifstream inFile("/sys/block/" + drivePath + "/device/model");
    if (inFile.is_open()) {
        std::getline(inFile, model);
        inFile.close();
    }
#endif

    return model;
}

std::vector<std::pair<std::string, std::string>> getStorageDeviceModels() {
    std::vector<std::pair<std::string, std::string>> storage_device_models;
#ifdef _WIN32
    char driveStrings[1024];
    DWORD length = GetLogicalDriveStringsA(sizeof(driveStrings), driveStrings);
    if (length > 0 && length <= sizeof(driveStrings)) {
        char *drive = driveStrings;
        while (*drive) {
            UINT driveType = GetDriveTypeA(drive);
            if (driveType == DRIVE_FIXED) {
                std::string drivePath = drive;
                std::string model = getDriveModel(drivePath);
                if (!model.empty()) {
                    storage_device_models.push_back(
                        std::make_pair(drivePath, model));
                }
            }
            drive += strlen(drive) + 1;
        }
    }
#else
    DIR *dir = opendir("/sys/block/");
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            std::string deviceName = ent->d_name;
            if (deviceName != "." && deviceName != "..") {
                std::string devicePath = deviceName;
                std::string model = GetDriveModel(devicePath);
                if (!model.empty()) {
                    storage_device_models.push_back(
                        std::make_pair(devicePath, model));
                }
            }
        }
        closedir(dir);
    }
#endif
    return storage_device_models;
}

std::vector<std::string> getAvailableDrives() {
    std::vector<std::string> drives;

#ifdef _WIN32
    DWORD drivesBitMask = GetLogicalDrives();
    for (char i = 'A'; i <= 'Z'; ++i) {
        if (drivesBitMask & 1) {
            std::string drive(1, i);
            drives.push_back(drive + ":\\");
        }
        drivesBitMask >>= 1;
    }
#elif __linux__
    // Linux 下不需要获取可用驱动器列表
    drives.push_back("/");
#elif __APPLE__
    struct statfs *mounts;
    int numMounts = getmntinfo(&mounts, MNT_NOWAIT);
    for (int i = 0; i < numMounts; ++i) {
        drives.push_back(mounts[i].f_mntonname);
    }
#endif

    return drives;
}

double calculateDiskUsagePercentage(unsigned long totalSpace,
                                    unsigned long freeSpace) {
    return ((static_cast<double>(totalSpace) - static_cast<double>(freeSpace)) /
            totalSpace) *
           100.0;
}
}  // namespace Atom::System
