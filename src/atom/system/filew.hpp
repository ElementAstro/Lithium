/*
 * filww.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-10-27

Description: File Watcher

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/inotify.h>
#include <pthread.h>
#endif

/**
 * @brief Enumerates the types of file events that can be monitored.
 */
enum class FileEventType
{
    kCreated,  /**< A new file was created. */
    kModified, /**< An existing file was modified. */
    kDeleted   /**< A file was deleted. */
};

/**
 * @brief Represents a file event that occurred.
 */
struct FileEvent
{
    std::string path;   /**< The path of the file that triggered the event. */
    FileEventType type; /**< The type of the event that occurred. */
};

/**
 * @brief Defines a callback function that is invoked when a file event occurs.
 */
using FileEventHandler = std::function<void(const FileEvent &)>;

/**
 * @brief Monitors files and directories for changes.
 */
class FileMonitor
{
public:
    /**
     * @brief Constructor for the FileMonitor class.
     */
    FileMonitor();

    /**
     * @brief Destructor for the FileMonitor class.
     */
    ~FileMonitor();

    /**
     * @brief Adds a watch for a file or directory.
     *
     * @param path The path to the file or directory to watch.
     * @param handler The callback function to invoke when a file event occurs.
     *
     * @return True if the watch was successfully added, false otherwise.
     */
    bool AddWatch(const std::string &path, const FileEventHandler &handler);

    /**
     * @brief Removes a watch for a file or directory.
     *
     * @param path The path to the file or directory to unwatch.
     *
     * @return True if the watch was successfully removed, false otherwise.
     */
    bool RemoveWatch(const std::string &path);

private:
    /**
     * @brief Contains information about a watch.
     */
    struct WatchInfo;

    /**
     * @brief The type of handle used to identify a watch.
     */
    using WatchHandle = decltype(CreateWatch(""));

    /**
     * @brief Creates a new watch for a file or directory.
     *
     * @param path The path to the file or directory to watch.
     *
     * @return The handle to the new watch, or INVALID_HANDLE_VALUE on failure.
     */
    WatchHandle CreateWatch(const std::string &path);

    /**
     * @brief Destroys a watch.
     *
     * @param handle The handle to the watch to destroy.
     */
    void DestroyWatch(WatchHandle handle);

    /**
     * @brief The main monitoring loop.
     */
    void MonitorLoop();

#ifdef _WIN32
    /**
     * @brief The callback function for the Windows implementation.
     *
     * @param lpParameter A pointer to the WatchInfo structure associated with the watch that triggered the event.
     * @param TimerOrWaitFired Not used.
     */
    static void CALLBACK Win32Callback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
#else
    /**
     * @brief The thread function for the Linux implementation.
     *
     * @param arg A pointer to the WatchInfo structure associated with the watch.
     *
     * @return Always returns nullptr.
     */
    static void *LinuxThreadFunc(void *arg);
#endif

    std::vector<WatchInfo> m_watches;                       /**< The list of watches currently being monitored. */
    std::unordered_map<WatchHandle, WatchInfo *> m_handles; /**< A map of watch handles to their associated WatchInfo structures. */
    bool m_running;                                         /**< Indicates whether the monitoring loop is running or not. */
};
