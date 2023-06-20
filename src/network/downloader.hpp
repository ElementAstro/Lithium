/*
 * downloader.hpp
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

Date: 2023-4-9

Description: Downloader

**************************************************/

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <string>
#include <vector>
#include <queue>
#include <atomic>
#include <optional>
#include <mutex>

struct DownloadTask
{
    std::string url;
    std::string filepath;
    bool completed{false};
    bool paused{false};
    size_t downloaded_bytes{0};
    int priority{0};
};

inline bool operator<(const DownloadTask &lhs, const DownloadTask &rhs)
{
    return lhs.priority < rhs.priority;
}

/**
 * @brief DownloadManager 类，用于管理下载任务
 */
class DownloadManager
{
public:
    /**
     * @brief 构造函数
     * @param task_file 保存下载任务列表的文件路径
     */
    DownloadManager(const std::string &task_file);

    /**
     * @brief 析构函数，用于释放资源
     */
    ~DownloadManager();

    /**
     * @brief 添加下载任务
     * @param url 下载链接
     * @param filepath 本地保存文件路径
     * @param priority 下载任务优先级，数字越大优先级越高
     */
    void add_task(const std::string &url, const std::string &filepath, int priority = 0);

    /**
     * @brief 删除下载任务
     * @param index 要删除的任务在任务列表中的索引
     * @return 是否成功删除任务
     */
    bool remove_task(size_t index);

    /**
     * @brief 开始下载任务
     * @param thread_count 下载线程数，默认为 CPU 核心数
     * @param download_speed 下载速度限制，单位为字节/秒，0 表示不限制下载速度
     */
    void start(size_t thread_count = std::thread::hardware_concurrency(), size_t download_speed = 0);

    /**
     * @brief 暂停下载任务
     * @param index 要暂停的任务在任务列表中的索引
     */
    void pause_task(size_t index);

    /**
     * @brief 恢复下载任务
     * @param index 要恢复的任务在任务列表中的索引
     */
    void resume_task(size_t index);

    /**
     * @brief 获取已下载的字节数
     * @param index 下载任务在任务列表中的索引
     * @return 已下载的字节数
     */
    size_t get_downloaded_bytes(size_t index);

private:
    /**
     * @brief 获取下一个要下载的任务的索引
     * @return 下一个要下载的任务的索引，如果任务队列为空，则返回空
     */
    std::optional<size_t> get_next_task_index();

    /**
     * @brief 获取下一个要下载的任务
     * @return 下一个要下载的任务，如果任务队列为空，则返回空
     */
    std::optional<DownloadTask> get_next_task();

    /**
     * @brief 启动下载线程
     * @param download_speed 下载速度限制，单位为字节/秒，0 表示不限制下载速度
     */
    void run(size_t download_speed);

    /**
     * @brief 下载指定的任务
     * @param task 要下载的任务
     * @param download_speed 下载速度限制，单位为字节/秒，0 表示不限制下载速度
     */
    void download_task(DownloadTask &task, size_t download_speed);

    /**
     * @brief 保存下载任务列表到文件中
     */
    void save_task_list_to_file();

private:
    std::string task_file_;                        ///< 下载任务列表文件路径
    std::vector<DownloadTask> tasks_;              ///< 下载任务列表
    std::priority_queue<DownloadTask> task_queue_; ///< 任务队列，按照优先级排序
    std::mutex mutex_;                             ///< 互斥量，用于保护任务列表和任务队列
    std::atomic<bool> running_{false};             ///< 是否正在下载中
};

/*
#include <iostream>
#include <string>
#include "downloader.hpp"

int main()
{
    DownloadManager download_manager("tasks.txt");

    while (true)
    {
        std::cout << "1. Add task" << std::endl;
        std::cout << "2. Pause task" << std::endl;
        std::cout << "3. Resume task" << std::endl;
        std::cout << "4. Remove task" << std::endl;
        std::cout << "5. Start downloading" << std::endl;
        std::cout << "6. Exit" << std::endl;

        int choice;
        std::cout << "Please select an option: ";
        std::cin >> choice;

        if (choice == 1)
        {
            std::string url, filepath;
            int priority;

            std::cout << "URL: ";
            std::cin >> url;

            std::cout << "Filepath: ";
            std::cin >> filepath;

            std::cout << "Priority (-1 for default): ";
            std::cin >> priority;

            download_manager.add_task(url, filepath, priority);
            std::cout << "Task added." << std::endl;
        }
        else if (choice == 2)
        {
            int index;
            std::cout << "Task index: ";
            std::cin >> index;

            if (download_manager.pause_task(index))
            {
                std::cout << "Task paused." << std::endl;
            }
            else
            {
                std::cout << "Failed to pause task." << std::endl;
            }
        }
        else if (choice == 3)
        {
            int index;
            std::cout << "Task index: ";
            std::cin >> index;

            if (download_manager.resume_task(index))
            {
                std::cout << "Task resumed." << std::endl;
            }
            else
            {
                std::cout << "Failed to resume task." << std::endl;
            }
        }
        else if (choice == 4)
        {
            int index;
            std::cout << "Task index: ";
            std::cin >> index;

            if (download_manager.remove_task(index))
            {
                std::cout << "Task removed." << std::endl;
            }
            else
            {
                std::cout << "Failed to remove task." << std::endl;
            }
        }
        else if (choice == 5)
        {
            download_manager.start();
            break;
        }
        else if (choice == 6)
        {
            break;
        }
        else
        {
            std::cout << "Invalid choice, please try again." << std::endl;
        }
    }

    return 0;
}

*/