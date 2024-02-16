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

#ifdef _WIN32
#include <Windows.h>
#include <dbt.h>
#elif __linux__
#include <sys/statvfs.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <libudev.h>
#endif

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

namespace Atom::System
{
    void StorageMonitor::registerCallback(std::function<void(const std::string &)> callback)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks.push_back(callback);
    }

    bool StorageMonitor::startMonitoring()
    {
        try
        {
            m_isRunning = true;
            DLOG_F(INFO, "Storage monitor started.");
#if ENABLE_DEBUG
            listAllStorage();
#endif
            while (m_isRunning)
            {
                for (const auto &path : m_storagePaths)
                {
                    if (isNewMediaInserted(path))
                    {
                        DLOG_F(INFO, "New storage media inserted. Path: {}", path);
#if ENABLE_DEBUG
                        listFiles(path);
#endif
                        triggerCallbacks(path);
                    }
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Storage monitor failed to start.");
            return false;
        }

        return true;
    }

    void StorageMonitor::stopMonitoring()
    {
        m_isRunning = false;
        DLOG_F(INFO, "Storage monitor stopped.");
    }

    void StorageMonitor::triggerCallbacks(const std::string &path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto &callback : m_callbacks)
        {
            callback(path);
        }
    }

    bool StorageMonitor::isNewMediaInserted(const std::string &path)
    {
        fs::space_info currentSpace = fs::space(path);

        std::lock_guard<std::mutex> lock(m_mutex);
        if (currentSpace.capacity != m_lastCapacity[path] || currentSpace.free != m_lastFree[path])
        {
            m_lastCapacity[path] = currentSpace.capacity;
            m_lastFree[path] = currentSpace.free;

            // Check if it is a USB media insertion event
            if (fs::exists(path + "/autorun.inf") || fs::exists(path + "/AutoRun.inf"))
            {
                return true;
            }
        }
        DLOG_F(INFO, "No new storage media inserted.");
        return false;
    }

#if ENABLE_DEBUG
    void StorageMonitor::listAllStorage()
    {
        DLOG_F(INFO, "List all storage devices.");

        for (const auto &entry : fs::directory_iterator("/"))
        {
            if (fs::is_directory(entry) && fs::space(entry).capacity > 0)
            {
                std::string path = entry.path().string();
                m_storagePaths.push_back(path);
                std::cout << "- " << path << std::endl;
            }
        }

        std::cout << std::endl;
    }

    void StorageMonitor::listFiles(const std::string &path)
    {
        std::cout << "Files in " << path << ":" << std::endl;

        for (const auto &entry : fs::directory_iterator(path))
        {
            std::cout << "- " << entry.path().filename() << std::endl;
        }

        std::cout << std::endl;
    }
#endif
}

#ifdef _WIN32
void monitorUdisk()
{
    DEV_BROADCAST_DEVICEINTERFACE devInterface;
    ZeroMemory(&devInterface, sizeof(devInterface));
    devInterface.dbcc_size = sizeof(devInterface);
    devInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    HDEVNOTIFY hDevNotify = RegisterDeviceNotification(
        GetConsoleWindow(),
        &devInterface,
        DEVICE_NOTIFY_WINDOW_HANDLE);

    if (hDevNotify == nullptr)
    {
        LOG_F(ERROR, "Failed to register device notification");
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_DEVICECHANGE)
        {
            PDEV_BROADCAST_HDR hdr = reinterpret_cast<PDEV_BROADCAST_HDR>(msg.lParam);
            if (hdr != nullptr && hdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME volume = reinterpret_cast<PDEV_BROADCAST_VOLUME>(hdr);
                if (volume->dbcv_flags == DBT_DEVICEARRIVAL)
                {
                    char driveLetter = 'A';
                    DWORD mask = volume->dbcv_unitmask;
                    while (mask != 0)
                    {
                        if ((mask & 1) != 0)
                        {
                            std::string drivePath = std::string(1, driveLetter) + ":\\";
                            DLOG_F(INFO, "U disk inserted. Drive path: {}", drivePath);
                            // TODO: Handle U disk insertion event
                        }
                        mask >>= 1;
                        ++driveLetter;
                    }
                }
                else if (volume->dbcv_flags == DBT_DEVICEREMOVECOMPLETE)
                {
                    char driveLetter = 'A';
                    DWORD mask = volume->dbcv_unitmask;
                    while (mask != 0)
                    {
                        if ((mask & 1) != 0)
                        {
                            std::string drivePath = std::string(1, driveLetter) + ":\\";
                            DLOG_F(INFO, "U disk removed. Drive path: {}", drivePath);
                            // TODO: Handle U disk removal event
                        }
                        mask >>= 1;
                        ++driveLetter;
                    }
                }
            }
        }
    }
    UnregisterDeviceNotification(hDevNotify);
}
#else
static void monitorUdisk(Atom::System::StorageMonitor &monitor)
{
    struct udev *udev = udev_new();
    if (!udev)
    {
        LOG_F(ERROR, "Failed to initialize udev");
        return;
    }

    struct udev_monitor *udevMon = udev_monitor_new_from_netlink(udev, "udev");
    if (!udevMon)
    {
        LOG_F(ERROR, "Failed to create udev monitor");
        udev_unref(udev);
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(udevMon, "block", "disk");
    udev_monitor_enable_receiving(udevMon);

    int fd = udev_monitor_get_fd(udevMon);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    while (true)
    {
        int ret = select(fd + 1, &fds, nullptr, nullptr, nullptr);
        if (ret < 0)
        {
            LOG_F(ERROR, "Select failed: {}", strerror(errno));
            break;
        }
        else if (ret > 0 && FD_ISSET(fd, &fds))
        {
            struct udev_device *dev = udev_monitor_receive_device(udevMon);
            if (dev)
            {
                std::string action = udev_device_get_action(dev);
                std::string devNode = udev_device_get_devnode(dev);
                if (action == "add" && !devNode.empty())
                {
                    LOG_F(INFO, "New disk found: {}", devNode);
                    monitor.triggerCallbacks(devNode);
                }
                else if (action == "remove" && !devNode.empty())
                {
                    LOG_F(INFO, "Removed disk: {}", devNode);
                    // TODO: Handle USB media removal event
                }
                udev_device_unref(dev);
            }
        }
    }

    udev_monitor_unref(udevMon);
    udev_unref(udev);
}
#endif
