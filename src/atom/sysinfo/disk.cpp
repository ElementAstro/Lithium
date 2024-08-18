/*
 * disk.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Disk

**************************************************/

#include "atom/sysinfo/disk.hpp"

#include "atom/log/loguru.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <span>
#include <sstream>
#include <unordered_set>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <dirent.h>
#include <limits.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#elif __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <mntent.h>
#endif

namespace fs = std::filesystem;

namespace atom::system {
std::vector<std::pair<std::string, float>> getDiskUsage() {
    std::vector<std::pair<std::string, float>> diskUsage;

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
                diskUsage.push_back(std::make_pair(drive_path, usage));
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
        std::string device;
        std::string path;
        iss >> device >> path;

        struct statfs stats {};
        if (statfs(path.c_str(), &stats) == 0) {
            unsigned long long totalSpace =
                static_cast<unsigned long long>(stats.f_blocks) * stats.f_bsize;
            unsigned long long freeSpace =
                static_cast<unsigned long long>(stats.f_bfree) * stats.f_bsize;

            unsigned long long usedSpace = totalSpace - freeSpace;
            float usage = static_cast<float>(usedSpace) / totalSpace * 100.0;
            diskUsage.emplace_back(path, usage);
        } else {
            LOG_F(ERROR, "GetDiskUsage error: statfs error");
        }
    }

#endif

    return diskUsage;
}

std::string getDriveModel(const std::string& drivePath) {
    std::string model;

#ifdef _WIN32
    HANDLE hDevice =
        CreateFileA(drivePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDevice != INVALID_HANDLE_VALUE) {
        STORAGE_PROPERTY_QUERY query = {};
        std::array<char, 1024> buffer = {};
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;
        DWORD bytesReturned = 0;
        if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query,
                            sizeof(query), buffer.data(), buffer.size(),
                            &bytesReturned, nullptr)) {
            auto desc =
                reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer.data());
            std::string_view vendorId(buffer.data() + desc->VendorIdOffset);
            std::string_view productId(buffer.data() + desc->ProductIdOffset);
            std::string_view productRevision(buffer.data() +
                                             desc->ProductRevisionOffset);
            model = std::string(vendorId) + " " + std::string(productId) + " " +
                    std::string(productRevision);
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
                    CFStringRef modelRef =
                        static_cast<CFStringRef>(CFDictionaryGetValue(
                            desc, kDADiskDescriptionDeviceModelKey));
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
    }
#endif

    return model;
}

std::vector<std::pair<std::string, std::string>> getStorageDeviceModels() {
    std::vector<std::pair<std::string, std::string>> storageDeviceModels;

#ifdef _WIN32
    std::array<char, 1024> driveStrings = {};
    DWORD length =
        GetLogicalDriveStringsA(driveStrings.size(), driveStrings.data());
    if (length > 0 && length <= driveStrings.size()) {
        std::span driveSpan(driveStrings.data(), length);
        for (const auto& drive :
             std::ranges::views::split(driveSpan, '\0') |
                 std::views::filter(
                     [](const std::span<char>& s) { return !s.empty(); })) {
            std::string drivePath(drive.data(), drive.size());
            UINT driveType = GetDriveTypeA(drivePath.c_str());
            if (driveType == DRIVE_FIXED) {
                std::string model = getDriveModel(drivePath);
                if (!model.empty()) {
                    storageDeviceModels.emplace_back(drivePath,
                                                     std::move(model));
                }
            }
        }
    }
#else
    fs::path sysBlockDir("/sys/block/");
    if (fs::exists(sysBlockDir) && fs::is_directory(sysBlockDir)) {
        for (const auto& entry : fs::directory_iterator(sysBlockDir)) {
            if (entry.is_directory() && entry.path().filename() != "." &&
                entry.path().filename() != "..") {
                std::string devicePath = entry.path().filename().string();
                std::string model = getDriveModel(devicePath);
                if (!model.empty()) {
                    storageDeviceModels.emplace_back(devicePath,
                                                     std::move(model));
                }
            }
        }
    }
#endif

    return storageDeviceModels;
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
    drives.emplace_back("/");
#elif __APPLE__
    struct statfs* mounts;
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

std::unordered_set<std::string> whiteList = {"SD1234", "SD5678"};

// Function to check file type and possible malicious behavior
bool checkForMaliciousFiles(const std::string& path) {
    LOG_F(INFO, "Checking for malicious files on: {}", path);
    bool maliciousFound = false;
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(path.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            std::string filename(ent->d_name);
            std::string filePath = path + "/" + filename;

            struct stat fileStat;
            if (stat(filePath.c_str(), &fileStat) == 0 &&
                S_ISREG(fileStat.st_mode)) {
                // Check file extension and report any suspicious files
                if (filename.find(".exe") != std::string::npos ||
                    filename.find(".sh") != std::string::npos) {
                    LOG_F(WARNING, "Suspicious file found: {} ({} bytes)",
                          filename, fileStat.st_size);
                    maliciousFound = true;
                }
            }
        }
        closedir(dir);
    }
    return maliciousFound;
}

bool isDeviceInWhiteList(const std::string& deviceID) {
    if (whiteList.find(deviceID) != whiteList.end()) {
        LOG_F(INFO, "Device {} is in the whitelist. Access granted.", deviceID);
        return true;
    }
    LOG_F(ERROR, "Device {} is not in the whitelist. Access denied.", deviceID);
    return false;
}

#ifdef _WIN32

bool setReadOnlyWindows(const std::string& driveLetter) {
    std::string volumePath = "\\\\.\\";
    volumePath += driveLetter;
    volumePath += ":";

    HANDLE hDevice = CreateFile(volumePath.c_str(), GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to access volume: " << GetLastError() << std::endl;
        logEvent("Failed to access volume: " + driveLetter);
        return false;
    }

    DWORD bytesReturned;
    bool result = DeviceIoControl(hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0,
                                  &bytesReturned, NULL);

    if (result) {
        std::cout << "Volume locked as read-only." << std::endl;
        logEvent("Successfully locked volume " + driveLetter +
                 " as read-only.");
    } else {
        std::cerr << "Failed to lock volume: " << GetLastError() << std::endl;
        logEvent("Failed to lock volume: " + driveLetter);
    }

    CloseHandle(hDevice);
    return result;
}

// Disable AutoRun functionality (Windows only)
void disableAutoRun() {
    const char* registryPath =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";
    HKEY hKey;
    DWORD value = 0xFF;  // Disable AutoRun

    if (RegOpenKeyEx(HKEY_CURRENT_USER, registryPath, 0, KEY_SET_VALUE,
                     &hKey) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, "NoDriveTypeAutoRun", 0, REG_DWORD, (BYTE*)&value,
                      sizeof(DWORD));
        RegCloseKey(hKey);
        logEvent("Successfully disabled AutoRun.");
    } else {
        logEvent("Failed to disable AutoRun.");
    }
}

bool setReadOnly(const std::string& path) {
    disableAutoRun();
    return setReadOnlyWindows(path);
}

void monitorDeviceInsertionWindows() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::string deviceID = "SD1234";  // Simulate device ID
        if (isDeviceInWhiteList(deviceID)) {
            std::cout << "Detected device insertion, executing read-only lock "
                         "and security scan..."
                      << std::endl;
            disableAutoRun();
            setReadOnlyWindows("E");  // Assume the SD card is on drive E
            checkForMaliciousFiles("E:/");
        }
    }
}

#elif __linux__ || __ANDROID__ || __APPLE__

auto setReadOnlyLinux(const std::string& mountPoint) -> bool {
    std::string command = "mount -o remount,ro " + mountPoint;
    int result = std::system(command.c_str());

    if (result == 0) {
        LOG_F(INFO, "Successfully mounted SD card {} as read-only.",
              mountPoint);
        return true;
    }
    LOG_F(ERROR, "Failed to mount SD card {} as read-only.", mountPoint);
    return false;
}

void disableAutoRun() {
    LOG_F(INFO, "AutoRun disabled (Linux/macOS/Android).");
}

auto setReadOnly(const std::string& mountPoint) -> bool {
    disableAutoRun();
    return setReadOnlyLinux(mountPoint);
}

void monitorDeviceInsertionUnix() {
    int fd = inotify_init();
    if (fd < 0) {
        LOG_F(ERROR, "inotify initialization failed.");
        return;
    }

    int wd = inotify_add_watch(fd, "/dev", IN_CREATE | IN_ATTRIB);
    if (wd < 0) {
        LOG_F(ERROR, "Failed to add watch for /dev directory.");
        return;
    }

    const int buf_len = 1024;
    char buffer[buf_len];

    while (true) {
        int length = read(fd, buffer, buf_len);
        if (length < 0) {
            LOG_F(ERROR, "Failed to read inotify events.");
            continue;
        }

        for (int i = 0; i < length; i += sizeof(struct inotify_event)) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            if (event->mask & IN_CREATE) {
                std::string deviceID = "SD5678";  // Simulate device ID
                if (isDeviceInWhiteList(deviceID)) {
                    LOG_F(INFO,
                          "Detected device insertion, executing "
                          "read-only lock and security scan...");
                    disableAutoRun();
                    setReadOnlyLinux("/mnt/sdcard");
                    checkForMaliciousFiles("/mnt/sdcard");
                }
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}

#endif
}  // namespace atom::system
