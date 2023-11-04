/*
 * filww.cpp
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

Description: File Watcher

**************************************************/

#include "filew.hpp"

#ifdef _WIN32
#include <locale>
#include <codecvt>
#else
#include <unistd.h>
#endif

#include "loguru/loguru.hpp"

enum class FileEventType
{
    kCreated,
    kModified,
    kDeleted,
};

struct FileEvent
{
    std::string path;
    FileEventType type;
};

using FileEventHandler = std::function<void(const FileEvent &)>;

struct FileMonitor::WatchInfo
{
    std::string path;
    FileEventHandler handler;
    WatchHandle handle;

#ifdef _WIN32
    std::vector<char> buffer;
#endif
};

FileMonitor::FileMonitor() : m_running(true)
{
}

FileMonitor::~FileMonitor()
{
    for (auto &watch_info : m_watches)
    {
#ifdef _WIN32
        UnregisterWait(watch_info.handle);
        CloseHandle(watch_info.handle);
#else
        DestroyWatch(watch_info.handle);
#endif
    }
}

bool FileMonitor::AddWatch(const std::string &path, const FileEventHandler &handler)
{
    auto handle = CreateWatch(path);
#ifdef _WIN32
    if (handle == INVALID_HANDLE_VALUE)
#else
    if (handle == -1)
#endif
    {
        return false;
    }

    WatchInfo watch_info;
    watch_info.path = path;
    watch_info.handler = handler;
    watch_info.handle = handle;

    m_watches.push_back(watch_info);
    m_handles[handle] = &m_watches.back();

    return true;
}

bool FileMonitor::RemoveWatch(const std::string &path)
{
    auto it = std::find_if(m_watches.begin(), m_watches.end(),
                           [&](const WatchInfo &watch_info)
                           { return watch_info.path == path; });

    if (it == m_watches.end())
    {
        return false;
    }

#ifdef _WIN32
    UnregisterWait(it->handle);
    CloseHandle(it->handle);
#else
    DestroyWatch(it->handle);
#endif

    m_handles.erase(it->handle);
    m_watches.erase(it);

    return true;
}

FileMonitor::WatchHandle FileMonitor::CreateWatch(const std::string &path)
{
#ifdef _WIN32
    int size = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
    std::vector<wchar_t> buffer(size);
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, buffer.data(), size);
    LPCWSTR lpPath = buffer.data();
    auto handle = FindFirstChangeNotificationW(
        lpPath,
        FALSE,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return INVALID_HANDLE_VALUE;
    }

    RegisterWaitForSingleObject(
        &handle,
        handle,
        &FileMonitor::Win32Callback,
        &m_handles[handle],
        INFINITE,
        WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);

    return handle;
#else
    auto handle = inotify_init();
    if (handle == -1)
    {
        return -1;
    }

    auto watch_descriptor = inotify_add_watch(
        handle,
        path.c_str(),
        IN_CREATE | IN_MOVED_TO | IN_MODIFY | IN_DELETE | IN_MOVED_FROM);

    if (watch_descriptor == -1)
    {
        close(handle);
        return -1;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, nullptr, &FileMonitor::LinuxThreadFunc, &m_handles[watch_descriptor]);
    pthread_detach(thread_id);

    return watch_descriptor;
#endif
}

void FileMonitor::DestroyWatch(WatchHandle handle)
{
#ifdef _WIN32
    FindCloseChangeNotification(handle);
#else
    inotify_rm_watch(handle, IN_ALL_EVENTS);
    close(handle);
#endif
}

void FileMonitor::MonitorLoop()
{
    while (m_running)
    {
        // Do nothing.
    }
}

#ifdef _WIN32
void CALLBACK FileMonitor::Win32Callback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    auto watch_info = static_cast<WatchInfo *>(lpParameter);

    DWORD bytes_returned;
    FILE_NOTIFY_INFORMATION *info;

    while (ReadDirectoryChangesW(
        watch_info->handle,
        &watch_info->buffer[0],
        static_cast<DWORD>(watch_info->buffer.size()),
        FALSE,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,
        &bytes_returned,
        nullptr,
        nullptr))
    {
        info = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(&watch_info->buffer[0]);

        do
        {
            std::wstring file_name(info->FileName, info->FileNameLength / 2);

            FileEvent event;
            event.path = std::string(watch_info->path.begin(), watch_info->path.end()) + "\\" + std::string(file_name.begin(), file_name.end());

            switch (info->Action)
            {
            case FILE_ACTION_ADDED:
            case FILE_ACTION_RENAMED_NEW_NAME:
                event.type = FileEventType::kCreated;
                break;
            case FILE_ACTION_MODIFIED:
                event.type = FileEventType::kModified;
                break;
            case FILE_ACTION_REMOVED:
            case FILE_ACTION_RENAMED_OLD_NAME:
                event.type = FileEventType::kDeleted;
                break;
            default:
                continue;
            }

            watch_info->handler(event);
        } while ((info = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(
                      reinterpret_cast<char *>(info) + info->NextEntryOffset)));
    }
}
#else
void *FileMonitor::LinuxThreadFunc(void *arg)
{
    auto watch_info = static_cast<WatchInfo *>(arg);

    char buffer[1024];
    ssize_t len;

    while ((len = read(watch_info->handle, buffer, sizeof(buffer))) > 0)
    {
        char *ptr = buffer;
        while (ptr < buffer + len)
        {
            auto event = reinterpret_cast<inotify_event *>(ptr);

            FileEvent file_event;
            file_event.path = watch_info->path + "/" + event->name;

            switch (event->mask & (IN_CREATE | IN_MOVED_TO | IN_MODIFY | IN_DELETE | IN_MOVED_FROM))
            {
            case IN_CREATE:
            case IN_MOVED_TO:
                file_event.type = FileEventType::kCreated;
                break;
            case IN_MODIFY:
                file_event.type = FileEventType::kModified;
                break;
            case IN_DELETE:
            case IN_MOVED_FROM:
                file_event.type = FileEventType::kDeleted;
                break;
            default:
                continue;
            }

            watch_info->handler(file_event);

            ptr += sizeof(inotify_event) + event->len;
        }
    }

    return nullptr;
}
#endif
