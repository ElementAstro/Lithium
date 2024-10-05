/*
 * downloader.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "downloader.hpp"

#include <curl/curl.h>
#include <atomic>
#include <chrono>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::web {
class DownloadManager::Impl {
public:
    explicit Impl(const std::string& task_file);
    ~Impl();

    void addTask(const std::string& url, const std::string& filepath,
                 int priority);
    auto removeTask(size_t index) -> bool;
    void start(size_t thread_count, size_t download_speed);
    void pauseTask(size_t index);
    void resumeTask(size_t index);
    void cancelTask(size_t index);
    auto getDownloadedBytes(size_t index) -> size_t;
    void setThreadCount(size_t thread_count);
    void setMaxRetries(size_t retries);
    void onDownloadComplete(const std::function<void(size_t)>& callback);
    void onProgressUpdate(const std::function<void(size_t, double)>& callback);

    struct DownloadTask {
        std::string url;
        std::string filepath;
        bool completed{false};
        bool paused{false};
        bool cancelled{false};
        size_t downloadedBytes{0};
        int priority{0};
        size_t retries{0};
    };

private:
    auto getNextTaskIndex() -> std::optional<size_t>;
    auto getNextTask() -> std::optional<DownloadTask>;
    void downloadTask(DownloadTask& task, size_t download_speed);
    void run(size_t download_speed);
    void saveTaskListToFile();

    std::string taskFile_;
    std::vector<DownloadTask> tasks_;
    std::priority_queue<DownloadTask> taskQueue_;
    std::mutex mutex_;
    std::atomic<bool> running_{false};
    std::chrono::system_clock::time_point startTime_;
    size_t maxRetries_{3};
    size_t threadCount_;
    std::function<void(size_t)> onComplete_;
    std::function<void(size_t, double)> onProgress_;
};

// 写入回调函数，带进度监控
auto writeDataWithProgress(void* buffer, size_t size, size_t nmemb,
                           void* userp) -> size_t {
    auto* taskInfo = static_cast<DownloadManager::Impl::DownloadTask*>(userp);
    size_t bytesWritten = size * nmemb;
    taskInfo->downloadedBytes += bytesWritten;

    return bytesWritten;
}

DownloadManager::Impl::Impl(const std::string& task_file)
    : taskFile_(task_file), threadCount_(std::thread::hardware_concurrency()) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // 读取任务列表
    std::ifstream infile(taskFile_);
    if (!infile) {
        LOG_F(ERROR, "Failed to open task file {}", taskFile_);
        THROW_EXCEPTION("Failed to open task file.");
    }
    while (infile >> std::ws && !infile.eof()) {
        std::string url;
        std::string filepath;
        infile >> url >> filepath;
        if (!url.empty() && !filepath.empty()) {
            tasks_.push_back({url, filepath});
        }
    }
}

DownloadManager::Impl::~Impl() {
    saveTaskListToFile();
    curl_global_cleanup();
}

void DownloadManager::Impl::addTask(const std::string& url,
                                    const std::string& filepath, int priority) {
    std::ofstream outfile(taskFile_, std::ios_base::app);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open task file {}", taskFile_);
        THROW_EXCEPTION("Failed to open task file.");
    }
    outfile << url << " " << filepath << std::endl;
    tasks_.push_back({url, filepath, false, false, false, 0, priority});
}

auto DownloadManager::Impl::removeTask(size_t index) -> bool {
    if (index >= tasks_.size()) {
        return false;
    }
    tasks_[index].completed = true;
    return true;
}

void DownloadManager::Impl::cancelTask(size_t index) {
    if (index >= tasks_.size()) {
        return;
    }
    tasks_[index].cancelled = true;
    tasks_[index].paused = true;  // 停止该任务的下载
}

void DownloadManager::Impl::start(size_t thread_count, size_t download_speed) {
    running_ = true;
    std::vector<std::jthread> threads;
    threadCount_ = thread_count;
    for (size_t i = 0; i < threadCount_; ++i) {
        threads.emplace_back(&DownloadManager::Impl::run, this, download_speed);
    }

    for (auto& thread : threads) {
        thread.join();  // 等待所有线程完成
    }
}

void DownloadManager::Impl::pauseTask(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for pauseTask.");
        return;
    }
    tasks_[index].paused = true;
}

void DownloadManager::Impl::resumeTask(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for resumeTask.");
        return;
    }
    tasks_[index].paused = false;
}

auto DownloadManager::Impl::getDownloadedBytes(size_t index) -> size_t {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for getDownloadedBytes.");
        return 0;
    }
    return tasks_[index].downloadedBytes;
}

void DownloadManager::Impl::setThreadCount(size_t thread_count) {
    threadCount_ = thread_count;
}

void DownloadManager::Impl::setMaxRetries(size_t retries) {
    maxRetries_ = retries;
}

void DownloadManager::Impl::onDownloadComplete(
    const std::function<void(size_t)>& callback) {
    onComplete_ = callback;
}

void DownloadManager::Impl::onProgressUpdate(
    const std::function<void(size_t, double)>& callback) {
    onProgress_ = callback;
}

auto DownloadManager::Impl::getNextTaskIndex() -> std::optional<size_t> {
    std::unique_lock<std::mutex> lock(mutex_);
    for (size_t i = 0; i < tasks_.size(); ++i) {
        if (!tasks_[i].completed && !tasks_[i].paused && !tasks_[i].cancelled) {
            return i;
        }
    }
    return std::nullopt;
}

auto DownloadManager::Impl::getNextTask() -> std::optional<DownloadTask> {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!taskQueue_.empty()) {
        auto task = taskQueue_.top();
        taskQueue_.pop();
        return task;
    }

    auto index = getNextTaskIndex();
    if (index) {
        return tasks_[*index];
    }

    return std::nullopt;
}

void DownloadManager::Impl::run(size_t download_speed) {
    while (running_) {
        auto taskOpt = getNextTask();
        if (!taskOpt) {
            break;  // 没有任务可执行时退出
        }
        auto& task = *taskOpt;

        if (task.completed || task.paused || task.cancelled) {
            continue;
        }

        // 记录任务开始的时间
        startTime_ = std::chrono::system_clock::now();

        // 处理下载任务
        downloadTask(task, download_speed);

        if (task.completed && onComplete_) {
            onComplete_(task.priority);  // 下载完成后触发回调
        }
    }
}

void DownloadManager::Impl::downloadTask(DownloadTask& task,
                                         size_t download_speed) {
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        LOG_F(ERROR, "Failed to initialize curl for {}", task.url);
        return;
    }

    std::ofstream outfile(task.filepath, std::ios::binary | std::ios::app);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open file {}", task.filepath);
        curl_easy_cleanup(curl);
        return;
    }

    // 设置 URL
    curl_easy_setopt(curl, CURLOPT_URL, task.url.c_str());

    // 设置写入数据回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataWithProgress);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &task);

    // 设置断点续传
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM, task.downloadedBytes);

    // 设置下载速度限制
    if (download_speed > 0) {
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE,
                         static_cast<curl_off_t>(download_speed));
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_F(ERROR, "Download failed: {}", curl_easy_strerror(res));

        // 错误处理，重试机制
        if (task.retries < maxRetries_) {
            LOG_F(INFO, "Retrying task {} ({} retries left)", task.url,
                  maxRetries_ - task.retries);
            task.retries++;
            downloadTask(task, download_speed);  // 重试下载任务
        } else {
            LOG_F(ERROR, "Max retries reached for task {}", task.url);
        }
    } else {
        curl_off_t totalSize;
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &totalSize);
        task.downloadedBytes += static_cast<size_t>(totalSize);
        task.completed = true;

        // 下载进度更新回调
        if (onProgress_) {
            double progress =
                static_cast<double>(task.downloadedBytes) / totalSize * 100.0;
            onProgress_(task.priority, progress);  // 传递任务索引和进度百分比
        }
    }

    curl_easy_cleanup(curl);
}

void DownloadManager::Impl::saveTaskListToFile() {
    std::ofstream outfile(taskFile_);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open task file {}", taskFile_);
        return;
    }

    for (const auto& task : tasks_) {
        outfile << task.url << " " << task.filepath << std::endl;
    }
}

// DownloadManager public functions
DownloadManager::DownloadManager(const std::string& task_file)
    : impl_(std::make_unique<Impl>(task_file)) {}
DownloadManager::~DownloadManager() = default;
void DownloadManager::addTask(const std::string& url,
                              const std::string& filepath, int priority) {
    impl_->addTask(url, filepath, priority);
}
auto DownloadManager::removeTask(size_t index) -> bool {
    return impl_->removeTask(index);
}
void DownloadManager::start(size_t thread_count, size_t download_speed) {
    impl_->start(thread_count, download_speed);
}
void DownloadManager::pauseTask(size_t index) { impl_->pauseTask(index); }
void DownloadManager::resumeTask(size_t index) { impl_->resumeTask(index); }
auto DownloadManager::getDownloadedBytes(size_t index) -> size_t {
    return impl_->getDownloadedBytes(index);
}
void DownloadManager::cancelTask(size_t index) { impl_->cancelTask(index); }
void DownloadManager::setThreadCount(size_t thread_count) {
    impl_->setThreadCount(thread_count);
}
void DownloadManager::setMaxRetries(size_t retries) {
    impl_->setMaxRetries(retries);
}
void DownloadManager::onDownloadComplete(
    const std::function<void(size_t)>& callback) {
    impl_->onDownloadComplete(callback);
}
void DownloadManager::onProgressUpdate(
    const std::function<void(size_t, double)>& callback) {
    impl_->onProgressUpdate(callback);
}

}  // namespace atom::web
