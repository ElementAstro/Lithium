/*
 * downloader.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-9

Description: Downloader

**************************************************/

#include "downloader.hpp"

#include <chrono>
#include <fstream>
#include <thread>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "cpp_httplib/httplib.h"

namespace atom::web {
DownloadManager::DownloadManager(const std::string &task_file)
    : task_file_(task_file) {
    try {
        std::ifstream infile(task_file_);
        if (!infile) {
            LOG_F(ERROR, "Failed to open task file {}", task_file_);
            THROW_EXCEPTION("Failed to open task file.");
        }
        while (infile >> std::ws && !infile.eof()) {
            std::string url, filepath;
            infile >> url >> filepath;
            if (!url.empty() && !filepath.empty()) {
                tasks_.push_back({url, filepath});
            }
        }
        infile.close();
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Error: {}", e.what());
        THROW_EXCEPTION(fmt::format("Error: {}", e.what()).c_str());
    }
}

DownloadManager::~DownloadManager() { save_task_list_to_file(); }

void DownloadManager::add_task(const std::string &url,
                               const std::string &filepath, int priority) {
    try {
        std::ofstream outfile(task_file_, std::ios_base::app);
        if (!outfile) {
            LOG_F(ERROR, "Failed to open task file {}", task_file_);
            THROW_EXCEPTION("Failed to open task file.");
        }
        outfile << url << " " << filepath << std::endl;
        outfile.close();
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Error: {}", e.what());
        THROW_EXCEPTION(fmt::format("Error: {}", e.what()).c_str());
    }
    tasks_.push_back({url, filepath, false, false, 0, priority});
}

bool DownloadManager::remove_task(size_t index) {
    if (index >= tasks_.size()) {
        return false;
    }
    tasks_[index].completed = true;
    return true;
}

void DownloadManager::start(size_t thread_count, size_t download_speed) {
    running_ = true;
#if _cplusplus >= 202002L
    std::vector<std::jthread> threads;
#else
    std::vector<std::thread> threads;
#endif
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back(&DownloadManager::run, this, download_speed);
    }
    for (auto &thread : threads) {
        thread.join();
    }
}

void DownloadManager::pause_task(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds!");
        return;
    }

    tasks_[index].paused = true;
    DLOG_F(INFO, "Paused task {} - {}", tasks_[index].url,
           tasks_[index].filepath);
}

void DownloadManager::resume_task(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds!");
        return;
    }

    tasks_[index].paused = false;

    // 如果任务未完成，则重新下载
    if (!tasks_[index].completed) {
        DLOG_F(INFO, "Resumed task {} - {}", tasks_[index].url,
               tasks_[index].filepath);
    }
}

size_t DownloadManager::get_downloaded_bytes(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds!");
        return 0;
    }

    return tasks_[index].downloaded_bytes;
}

std::optional<size_t> DownloadManager::get_next_task_index() {
    std::unique_lock<std::mutex> lock(mutex_);
    for (size_t i = 0; i < tasks_.size(); ++i) {
        if (!tasks_[i].completed && !tasks_[i].paused) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<DownloadTask> DownloadManager::get_next_task() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!task_queue_.empty()) {
        auto task = task_queue_.top();
        task_queue_.pop();
        return task;
    }

    auto index = get_next_task_index();
    if (index) {
        return tasks_[*index];
    }

    return std::nullopt;
}

void DownloadManager::run(size_t download_speed) {
    while (running_) {
        auto task = get_next_task();
        if (!task) {
            break;
        }

        if (task->completed || task->paused) {
            continue;
        }
        start_time_ = std::chrono::system_clock::now();
        download_task(*task, download_speed);
    }
}

void DownloadManager::download_task(DownloadTask &task, size_t download_speed) {
    httplib::Client cli(task.url);
    auto res = cli.Get("/");
    if (!res || res->status != 200) {
        LOG_F(ERROR, "Failed to download {}", task.url);
        return;
    }

    std::ofstream outfile(task.filepath,
                          std::ofstream::binary | std::ofstream::app);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open file {}", task.filepath);
        return;
    }

    // 断点续传：从已经下载的字节数开始写入文件
    outfile.seekp(task.downloaded_bytes);

    constexpr size_t kBufferSize = 1024 * 1024;  // 1MB 缓存
    size_t buffer_size = kBufferSize;
    if (download_speed > 0) {
        // 如果有下载速度限制，根据时间计算出当前应该下载的字节数
        auto bytes_per_ms = static_cast<double>(download_speed) / 1000.0;
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now() - start_time_)
                              .count();
        auto bytes_to_download =
            static_cast<size_t>(elapsed_ms * bytes_per_ms) -
            task.downloaded_bytes;
        if (bytes_to_download < buffer_size) {
            buffer_size = bytes_to_download;
        }
    }

    size_t total_bytes_read = 0;

    while (!res->body.empty() && !task.completed && !task.paused) {
        size_t bytes_to_write = res->body.size() - task.downloaded_bytes;
        if (bytes_to_write > buffer_size) {
            bytes_to_write = buffer_size;
        }

        outfile.write(res->body.c_str() + task.downloaded_bytes,
                      bytes_to_write);
        auto bytes_written =
            static_cast<size_t>(outfile.tellp()) - task.downloaded_bytes;
        task.downloaded_bytes += bytes_written;
        total_bytes_read += bytes_written;

        // 下载速度控制：根据需要下载的字节数和已经下载的字节数计算出需要等待的时间
        if (download_speed > 0) {
            auto bytes_per_ms = static_cast<double>(download_speed) / 1000.0;
            auto elapsed_ms =
                static_cast<double>(total_bytes_read) / bytes_per_ms;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<long long>(elapsed_ms)));
        }

        // 如果下载完成，则标记任务为已完成
        if (task.downloaded_bytes >= res->body.size()) {
            task.completed = true;
        }
    }

    outfile.close();

    if (task.completed) {
        DLOG_F(INFO, "Downloaded file {}", task.filepath);
    }
}

void DownloadManager::save_task_list_to_file() {
    try {
        std::ofstream outfile(task_file_);
        if (!outfile) {
            DLOG_F(INFO, "Failed to open task file {}", task_file_);
            throw std::runtime_error("Failed to open task file.");
        }
        for (const auto &task : tasks_) {
            outfile << task.url << " " << task.filepath << std::endl;
        }
        outfile.close();
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Error: {}", e.what());
        THROW_EXCEPTION("Error: ", e.what());
    }
}
}  // namespace atom::web
