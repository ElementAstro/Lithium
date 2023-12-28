/*
 * dirw.hpp
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

Date: 2023-10-27

Description: Folder Watcher

**************************************************/

#pragma once

#include <functional>
#include <string>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/inotify.h>
#include <unistd.h>
#endif

namespace Atom::System
{

    /**
     * @class FolderMonitor
     * @brief 文件夹监视器类，用于监视指定文件夹的变化并触发回调函数。
     */
    class FolderMonitor
    {
    public:
        /**
         * @brief 定义文件变化事件回调函数类型。
         * @param filePath 变化的文件路径。
         */
        using FileChangeEventCallback = std::function<void(const std::string &)>;

        /**
         * @brief 构造函数。
         * @param folderPath 要监视的文件夹路径。
         */
        FolderMonitor(const std::string &folderPath);

        /**
         * @brief 开始监视文件夹变化。
         */
        void StartMonitoring();

        /**
         * @brief 停止监视文件夹变化。
         */
        void StopMonitoring();

        /**
         * @brief 注册文件变化事件回调函数。
         * @param callback 文件变化事件回调函数。
         */
        void RegisterFileChangeEventCallback(FileChangeEventCallback callback);

    private:
        std::string m_folderPath;                          /**< 要监视的文件夹路径 */
        bool m_isMonitoring;                               /**< 是否正在监视文件夹变化 */
        std::thread m_monitorThread;                       /**< 监视线程 */
        FileChangeEventCallback m_fileChangeEventCallback; /**< 文件变化事件回调函数 */

        /**
         * @brief 监视文件夹变化的内部方法。
         */
        void MonitorFolderChanges();
    };
}

/*

// 示例用法
int main()
{
    FolderMonitor folderMonitor("test");

    // 注册文件变更事件的处理函数
    folderMonitor.RegisterFileChangeEventCallback([](const std::string &filePath)
                                                  { std::cout << "File changed: " << filePath << std::endl; });

    // 启动监视器
    folderMonitor.StartMonitoring();

    // 持续运行一段时间
    std::this_thread::sleep_for(std::chrono::seconds(60));

    // 停止监视器
    folderMonitor.StopMonitoring();

    return 0;
}

*/
