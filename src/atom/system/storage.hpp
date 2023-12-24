/*
 * storage.hpp
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

#include <functional>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>

/**
 * @brief 监控存储空间变化的类。
 * 
 * 这个类可以监控所有已挂载设备的存储空间使用情况，并在存储空间发生变化时触发注册的回调函数。
 */
class StorageMonitor
{
public:
    /**
     * @brief 默认构造函数。
     */
    StorageMonitor() = default;

    /**
     * @brief 注册回调函数。
     * 
     * @param callback 回调函数，当存储空间发生变化时会被触发。
     */
    void registerCallback(std::function<void(const std::string &)> callback);

    /**
     * @brief 启动存储空间监控。
     * 
     * @return 成功返回true，否则返回false。
     */
    [[nodiscard]] bool startMonitoring();

    /**
     * @brief 停止存储空间监控。
     */
    void stopMonitoring();

    /**
     * @brief 触发所有注册的回调函数。
     * 
     * @param path 存储空间路径。
     */
    void triggerCallbacks(const std::string &path);

private:
    /**
     * @brief 检查指定路径是否有新的存储设备插入。
     * 
     * @param path 存储空间路径。
     * @return 如果有新的存储设备插入则返回true，否则返回false。
     */
    [[nodiscard]] bool isNewMediaInserted(const std::string &path);

#ifdef DEBUG
    /**
     * @brief 列举所有已挂载的存储空间。
     */
    void listAllStorage();

    /**
     * @brief 列举指定路径下的文件列表。
     * 
     * @param path 存储空间路径。
     */
    void listFiles(const std::string &path);
#endif

private:
    std::vector<std::string> m_storagePaths; ///< 所有已挂载的存储空间路径。
    std::unordered_map<std::string, uintmax_t> m_lastCapacity; ///< 上一次记录的存储空间容量。
    std::unordered_map<std::string, uintmax_t> m_lastFree; ///< 上一次记录的存储空间可用空间。
    std::mutex m_mutex; ///< 互斥锁，用于保护数据结构的线程安全。
    std::vector<std::function<void(const std::string &)>> m_callbacks; ///< 注册的回调函数列表。
    bool m_isRunning = false; ///< 标记是否正在运行监控。
};


#ifdef __linux__
static void monitorUdisk(StorageMonitor &monitor);
#endif

