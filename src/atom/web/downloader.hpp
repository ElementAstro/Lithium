/*
 * downloader.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace atom::web {

/**
 * @class DownloadManager
 * @brief A class for managing download tasks using the Pimpl idiom to hide
 * implementation details.
 */
class DownloadManager {
public:
    /**
     * @brief Constructs a DownloadManager.
     * @param task_file The file path to save the download task list.
     */
    explicit DownloadManager(const std::string& task_file);

    /**
     * @brief Destructor to release resources.
     */
    ~DownloadManager();

    // Disable copy constructor and copy assignment operator
    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;

    // Enable move constructor and move assignment operator
    DownloadManager(DownloadManager&&) noexcept = default;
    DownloadManager& operator=(DownloadManager&&) noexcept = default;

    /**
     * @brief Adds a download task.
     * @param url The download URL.
     * @param filepath The local file path to save the downloaded file.
     * @param priority The priority of the download task, higher numbers
     * indicate higher priority.
     */
    void addTask(const std::string& url, const std::string& filepath,
                 int priority = 0);

    /**
     * @brief Removes a download task.
     * @param index The index of the task in the task list to remove.
     * @return True if the task was successfully removed, false otherwise.
     */
    auto removeTask(size_t index) -> bool;

    /**
     * @brief Starts the download tasks.
     * @param thread_count The number of download threads, defaults to the
     * number of CPU cores.
     * @param download_speed The download speed limit in bytes per second, 0
     * means no limit.
     */
    void start(size_t thread_count = std::thread::hardware_concurrency(),
               size_t download_speed = 0);

    /**
     * @brief Pauses a download task.
     * @param index The index of the task in the task list to pause.
     */
    void pauseTask(size_t index);

    /**
     * @brief Resumes a paused download task.
     * @param index The index of the task in the task list to resume.
     */
    void resumeTask(size_t index);

    /**
     * @brief Gets the number of bytes downloaded for a task.
     * @param index The index of the task in the task list.
     * @return The number of bytes downloaded.
     */
    auto getDownloadedBytes(size_t index) -> size_t;

    /**
     * @brief Cancels a download task.
     * @param index The index of the task in the task list to cancel.
     */
    void cancelTask(size_t index);

    /**
     * @brief Dynamically adjusts the number of download threads.
     * @param thread_count The new number of download threads.
     */
    void setThreadCount(size_t thread_count);

    /**
     * @brief Sets the maximum number of retries for download errors.
     * @param retries The maximum number of retries for each task on failure.
     */
    void setMaxRetries(size_t retries);

    /**
     * @brief Registers a callback function to be called when a download is
     * complete.
     * @param callback The callback function, which takes the task index as a
     * parameter.
     */
    void onDownloadComplete(const std::function<void(size_t)>& callback);

    /**
     * @brief Registers a callback function to be called when the download
     * progress is updated.
     * @param callback The callback function, which takes the task index and
     * download percentage as parameters.
     */
    void onProgressUpdate(const std::function<void(size_t, double)>& callback);

    /**
     * @class Impl
     * @brief The implementation class for DownloadManager, used to hide
     * implementation details.
     */
    class Impl;

private:
    std::unique_ptr<Impl> impl_;  ///< Pointer to the implementation, using
                                  ///< Pimpl idiom to hide details.
};

}  // namespace atom::web