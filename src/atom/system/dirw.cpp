/*
 * dirw.cpp
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

#include "dirw.hpp"

#include "atom/log/loguru.hpp"

FolderMonitor::FolderMonitor(const std::string &folderPath)
    : m_folderPath(folderPath), m_isMonitoring(false) {}

void FolderMonitor::StartMonitoring()
{
    if (m_isMonitoring)
    {
        LOG_F(ERROR, "Folder monitor is already running.");
        return;
    }

    m_isMonitoring = true;

    // 创建监视线程
    m_monitorThread = std::thread([this]()
                                  {
            while (m_isMonitoring) {
                MonitorFolderChanges();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } });
}

void FolderMonitor::StopMonitoring()
{
    if (!m_isMonitoring)
    {
        LOG_F(WARNING, "Folder monitor is not running.");
        return;
    }

    m_isMonitoring = false;

    // 等待监视线程退出
    if (m_monitorThread.joinable())
    {
        m_monitorThread.join();
    }
}

void FolderMonitor::RegisterFileChangeEventCallback(FileChangeEventCallback callback)
{
    m_fileChangeEventCallback = callback;
}

void FolderMonitor::MonitorFolderChanges()
{
#ifdef _WIN32
    // Windows平台下的文件夹监视器实现
    HANDLE hDir = CreateFile(m_folderPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (hDir == INVALID_HANDLE_VALUE)
    {
        LOG_F(ERROR, "Failed to open folder: {}", m_folderPath);
        return;
    }

    DWORD dwBytesReturned;
    FILE_NOTIFY_INFORMATION buffer[1024];

    BOOL result = ReadDirectoryChangesW(hDir, &buffer, sizeof(buffer), TRUE,
                                        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME, &dwBytesReturned, NULL, NULL);

    if (!result)
    {
        LOG_F(ERROR, "Failed to start monitoring folder: {}", m_folderPath);
        CloseHandle(hDir);
        return;
    }

    while (result && m_isMonitoring)
    {
        DWORD offset = 0;
        FILE_NOTIFY_INFORMATION *pInfo = nullptr;

        do
        {
            pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(reinterpret_cast<char *>(&buffer) + offset);

            std::wstring fileName(pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
            std::string utf8FileName(fileName.begin(), fileName.end());

            std::string filePath = m_folderPath + "\\" + utf8FileName;

            if (pInfo->Action == FILE_ACTION_MODIFIED)
            {
                if (m_fileChangeEventCallback)
                {
                    m_fileChangeEventCallback(filePath);
                }
            }
            else if (pInfo->Action == FILE_ACTION_ADDED || pInfo->Action == FILE_ACTION_RENAMED_NEW_NAME)
            {
                if (m_fileChangeEventCallback)
                {
                    m_fileChangeEventCallback(filePath);
                }
            }
            else if (pInfo->Action == FILE_ACTION_REMOVED || pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME)
            {
                // 文件被删除或重命名
                // 可以在此处执行相应的处理操作
            }

            offset += pInfo->NextEntryOffset;
        } while (pInfo->NextEntryOffset != 0 && m_isMonitoring);

        result = ReadDirectoryChangesW(hDir, &buffer, sizeof(buffer), TRUE,
                                       FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME, &dwBytesReturned, NULL, NULL);
    }

    CloseHandle(hDir);
#else
    // Linux平台下的文件夹监视器实现
    int fd = inotify_init();
    if (fd == -1)
    {
        LOG_F(ERROR, "Failed to initialize inotify.");
        return;
    }

    int wd = inotify_add_watch(fd, m_folderPath.c_str(),
                               IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE | IN_MOVED_FROM | IN_MOVED_TO);

    if (wd == -1)
    {
        LOG_F(ERROR, "Failed to add watch for folder: ", m_folderPath);
        close(fd);
        return;
    }

    char buffer[4096];

    while (m_isMonitoring)
    {
        ssize_t len = read(fd, buffer, sizeof(buffer));
        if (len == -1)
        {
            LOG_F(ERROR, "Failed to read events from inotify.");
            break;
        }

        const struct inotify_event *event = nullptr;
        for (char *ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event->len)
        {
            event = reinterpret_cast<const struct inotify_event *>(ptr);

            std::string fileName(event->name);
            std::string filePath = m_folderPath + "/" + fileName;

            if (event->mask & IN_MODIFY)
            {
                if (m_fileChangeEventCallback)
                {
                    m_fileChangeEventCallback(filePath);
                }
            }
            else if (event->mask & (IN_CREATE | IN_MOVED_TO))
            {
                if (m_fileChangeEventCallback)
                {
                    m_fileChangeEventCallback(filePath);
                }
            }
            else if (event->mask & (IN_DELETE | IN_MOVED_FROM))
            {
                // 文件被删除或移动
                // 可以在此处执行相应的处理操作
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
#endif
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
