/*
 * storage.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-11-5

Description: Storage Monitor

**************************************************/

#include "storage.hpp"

#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
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
            std::cout << "Storage monitor started." << std::endl;
            listAllStorage();

            while (m_isRunning)
            {
                for (const auto &path : m_storagePaths)
                {
                    if (isNewMediaInserted(path))
                    {
                        std::cout << "New storage media inserted. Path: " << path << std::endl;
                        listFiles(path);
                        triggerCallbacks(path);
                    }
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    void StorageMonitor::stopMonitoring()
    {
        m_isRunning = false;
        std::cout << "Storage monitor stopped." << std::endl;
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

        return false;
    }

    void StorageMonitor::listAllStorage()
    {
        std::cout << "All storage media: " << std::endl;

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

#ifdef __linux__
static void monitorUdisk(StorageMonitor &monitor)
{
    struct udev *udev = udev_new();
    if (!udev)
    {
        std::cerr << "Failed to create udev context" << std::endl;
        return;
    }

    struct udev_monitor *udevMon = udev_monitor_new_from_netlink(udev, "udev");
    if (!udevMon)
    {
        std::cerr << "Failed to create udev monitor" << std::endl;
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
            std::cerr << "Error in select function" << std::endl;
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
                    std::cout << "U disk inserted. Device node: " << devNode << std::endl;
                    monitor.triggerCallbacks(devNode);
                }
                else if (action == "remove" && !devNode.empty())
                {
                    std::cout << "U disk removed. Device node: " << devNode << std::endl;
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

/*

int main()
{
    if (!checkIsRoot())
    {
        std::cerr << "This program requires root/administrator privileges." << std::endl;
        return 1;
    }

    StorageMonitor monitor;

    monitor.registerCallback([](const std::string &path)
                             {
                                 std::cout << "Callback triggered. Storage path: " << path << std::endl;
                                 if (fs::exists(path + "/autorun.inf") || fs::exists(path + "/AutoRun.inf"))
                                 {
                                     std::cout << "U disk inserted." << std::endl;
                                     // TODO: Handle USB media insertion event
                                 } });

    // Start monitoring
    if (monitor.startMonitoring())
    {
        // Perform other tasks in the main thread...

#ifdef __linux__
        std::thread udiskThread(monitorUdisk, std::ref(monitor));
#endif

        // Stop monitoring
        monitor.stopMonitoring();

#ifdef __linux__
        udiskThread.join();
#endif
    }

    return 0;
}

*/