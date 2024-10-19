/*
 * storage.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-5

Description: Storage Monitor

**************************************************/

#include "storage.hpp"

#include <filesystem>
#include <format>
#include <thread>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbt.h>
// clang-format on
#elif __linux__
#include <fcntl.h>
#include <libudev.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#endif

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

namespace atom::system {
StorageMonitor::~StorageMonitor() {
    LOG_F(INFO, "StorageMonitor destructor called");
    stopMonitoring();
}

void StorageMonitor::registerCallback(
    std::function<void(const std::string&)> callback) {
    LOG_F(INFO, "registerCallback called");
    std::lock_guard lock(m_mutex);
    m_callbacks.push_back(std::move(callback));
    LOG_F(INFO, "Callback registered successfully");
}

auto StorageMonitor::startMonitoring() -> bool {
    LOG_F(INFO, "startMonitoring called");
    m_isRunning = true;
    std::thread([this] {
        try {
            listAllStorage();
            while (m_isRunning) {
                for (const auto& path : m_storagePaths) {
                    if (isNewMediaInserted(path)) {
                        triggerCallbacks(path);
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in storage monitor: {}", e.what());
            m_isRunning = false;
        }
    }).detach();
    LOG_F(INFO, "Monitoring started successfully");
    return true;
}

void StorageMonitor::stopMonitoring() {
    LOG_F(INFO, "stopMonitoring called");
    m_isRunning = false;
    LOG_F(INFO, "Storage monitor stopped");
}

auto StorageMonitor::isRunning() const -> bool {
    LOG_F(INFO, "isRunning called, returning: {}", m_isRunning);
    return m_isRunning;
}

void StorageMonitor::triggerCallbacks(const std::string& path) {
    LOG_F(INFO, "triggerCallbacks called with path: {}", path);
    std::lock_guard lock(m_mutex);
    for (const auto& callback : m_callbacks) {
        callback(path);
    }
    LOG_F(INFO, "Callbacks triggered successfully for path: {}", path);
}

auto StorageMonitor::isNewMediaInserted(const std::string& path) -> bool {
    LOG_F(INFO, "isNewMediaInserted called with path: {}", path);
    auto currentSpace = fs::space(path);
    std::lock_guard lock(m_mutex);
    auto& [lastCapacity, lastFree] = m_storageStats[path];
    if (currentSpace.capacity != lastCapacity ||
        currentSpace.free != lastFree) {
        lastCapacity = currentSpace.capacity;
        lastFree = currentSpace.free;
        LOG_F(INFO, "New media inserted at path: {}", path);
        return true;
    }
    LOG_F(INFO, "No new media inserted at path: {}", path);
    return false;
}

void StorageMonitor::listAllStorage() {
    LOG_F(INFO, "listAllStorage called");
    for (const auto& entry : fs::directory_iterator("/")) {
        if (entry.is_directory()) {
            auto capacity = fs::space(entry).capacity;
            if (capacity > 0) {
                std::string path = entry.path().string();
                m_storagePaths.push_back(path);
                LOG_F(INFO, "Found storage device: {}", path);
            }
        }
    }
    LOG_F(INFO, "listAllStorage completed with {} storage devices found",
          m_storagePaths.size());
}

void StorageMonitor::listFiles(const std::string& path) {
    LOG_F(INFO, "listFiles called with path: {}", path);
    for (const auto& entry : fs::directory_iterator(path)) {
        LOG_F(INFO, "- {}", entry.path().filename().string());
    }
    LOG_F(INFO, "listFiles completed for path: {}", path);
}

#ifdef _WIN32
void monitorUdisk() {
    LOG_F(INFO, "monitorUdisk called");
    DEV_BROADCAST_DEVICEINTERFACE devInterface{};
    devInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    devInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    HDEVNOTIFY hDevNotify = RegisterDeviceNotification(
        GetConsoleWindow(), &devInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

    if (hDevNotify == nullptr) {
        LOG_F(ERROR, "Failed to register device notification");
        return;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_DEVICECHANGE) {
            auto* hdr = reinterpret_cast<PDEV_BROADCAST_HDR>(msg.lParam);
            if ((hdr != nullptr) && hdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                auto* volume = reinterpret_cast<PDEV_BROADCAST_VOLUME>(hdr);
                if (volume->dbcv_flags == DBT_DEVICEARRIVAL) {
                    for (char driveLetter = 'A'; volume->dbcv_unitmask != 0U;
                         volume->dbcv_unitmask >>= 1, ++driveLetter) {
                        if ((volume->dbcv_unitmask & 1) != 0U) {
                            std::string drivePath =
                                std::format("{}:\\", driveLetter);
                            LOG_F(INFO, "U disk inserted. Drive path: {}",
                                  drivePath);
                        }
                    }
                } else if (volume->dbcv_flags == DBT_DEVICEREMOVECOMPLETE) {
                    for (char driveLetter = 'A'; volume->dbcv_unitmask != 0U;
                         volume->dbcv_unitmask >>= 1, ++driveLetter) {
                        if ((volume->dbcv_unitmask & 1) != 0U) {
                            std::string drivePath =
                                std::format("{}:\\", driveLetter);
                            LOG_F(INFO, "U disk removed. Drive path: {}",
                                  drivePath);
                        }
                    }
                }
            }
        }
    }
    UnregisterDeviceNotification(hDevNotify);
    LOG_F(INFO, "monitorUdisk completed");
}
#else
void monitorUdisk(atom::system::StorageMonitor& monitor) {
    LOG_F(INFO, "monitorUdisk called");
    struct udev* udev = udev_new();
    if (!udev) {
        LOG_F(ERROR, "Failed to initialize udev");
        return;
    }

    struct udev_monitor* udevMon = udev_monitor_new_from_netlink(udev, "udev");
    if (!udevMon) {
        udev_unref(udev);
        LOG_F(ERROR, "Failed to create udev monitor");
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(udevMon, "block", "disk");
    udev_monitor_enable_receiving(udevMon);

    int fd = udev_monitor_get_fd(udevMon);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    while (true) {
        if (select(fd + 1, &fds, nullptr, nullptr, nullptr) > 0 &&
            FD_ISSET(fd, &fds)) {
            struct udev_device* dev = udev_monitor_receive_device(udevMon);
            if (dev) {
                std::string action = udev_device_get_action(dev);
                std::string devNode = udev_device_get_devnode(dev);
                if (action == "add") {
                    LOG_F(INFO, "New disk found: {}", devNode);
                    monitor.triggerCallbacks(devNode);
                } else if (action == "remove") {
                    LOG_F(INFO, "Removed disk: {}", devNode);
                }
                udev_device_unref(dev);
            }
        }
    }

    udev_monitor_unref(udevMon);
    udev_unref(udev);
    LOG_F(INFO, "monitorUdisk completed");
}
#endif
}  // namespace atom::system