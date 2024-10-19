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
    explicit Impl(std::string task_file);
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

        auto operator<(const DownloadTask& other) const -> bool {
            return priority < other.priority;
        }
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
    size_t threadCount_{std::thread::hardware_concurrency()};
    std::function<void(size_t)> onComplete_;
    std::function<void(size_t, double)> onProgress_;
};

// 写入回调函数，带进度监控
auto writeDataWithProgress([[maybe_unused]] void* buffer, size_t size,
                           size_t nmemb, void* userp) -> size_t {
    auto* taskInfo = static_cast<DownloadManager::Impl::DownloadTask*>(userp);
    size_t bytesWritten = size * nmemb;
    taskInfo->downloadedBytes += bytesWritten;
    LOG_F(INFO, "Downloaded {} bytes for task {}", bytesWritten, taskInfo->url);
    return bytesWritten;
}

DownloadManager::Impl::Impl(std::string task_file)
    : taskFile_(std::move(task_file)) {
    LOG_F(INFO, "Initializing DownloadManager with task file: {}", taskFile_);
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
            tasks_.emplace_back(DownloadTask{url, filepath});
            LOG_F(INFO, "Loaded task: URL = {}, Filepath = {}", url, filepath);
        }
    }
}

DownloadManager::Impl::~Impl() {
    LOG_F(INFO, "Destroying DownloadManager");
    saveTaskListToFile();
    curl_global_cleanup();
}

void DownloadManager::Impl::addTask(const std::string& url,
                                    const std::string& filepath, int priority) {
    LOG_F(INFO, "Adding task: URL = {}, Filepath = {}, Priority = {}", url,
          filepath, priority);
    std::ofstream outfile(taskFile_, std::ios_base::app);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open task file {}", taskFile_);
        THROW_EXCEPTION("Failed to open task file.");
    }
    outfile << url << " " << filepath << std::endl;
    tasks_.emplace_back(
        DownloadTask{url, filepath, false, false, false, 0, priority});
}

auto DownloadManager::Impl::removeTask(size_t index) -> bool {
    LOG_F(INFO, "Removing task at index: {}", index);
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for removeTask.");
        return false;
    }
    tasks_[index].completed = true;
    return true;
}

void DownloadManager::Impl::cancelTask(size_t index) {
    LOG_F(INFO, "Cancelling task at index: {}", index);
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for cancelTask.");
        return;
    }
    tasks_[index].cancelled = true;
    tasks_[index].paused = true;  // 停止该任务的下载
}

void DownloadManager::Impl::start(size_t thread_count, size_t download_speed) {
    LOG_F(INFO,
          "Starting download manager with {} threads and download speed limit "
          "of {} bytes/sec",
          thread_count, download_speed);
    running_ = true;
    std::vector<std::jthread> threads;
    threadCount_ = thread_count;
    for (size_t i = 0; i < threadCount_; ++i) {
        threads.emplace_back(&DownloadManager::Impl::run, this, download_speed);
    }

    for (auto& thread : threads) {
        thread.join();  // 等待所有线程完成
    }
    LOG_F(INFO, "Download manager stopped");
}

void DownloadManager::Impl::pauseTask(size_t index) {
    LOG_F(INFO, "Pausing task at index: {}", index);
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for pauseTask.");
        return;
    }
    tasks_[index].paused = true;
}

void DownloadManager::Impl::resumeTask(size_t index) {
    LOG_F(INFO, "Resuming task at index: {}", index);
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for resumeTask.");
        return;
    }
    tasks_[index].paused = false;
}

auto DownloadManager::Impl::getDownloadedBytes(size_t index) -> size_t {
    LOG_F(INFO, "Getting downloaded bytes for task at index: {}", index);
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for getDownloadedBytes.");
        return 0;
    }
    return tasks_[index].downloadedBytes;
}

void DownloadManager::Impl::setThreadCount(size_t thread_count) {
    LOG_F(INFO, "Setting thread count to: {}", thread_count);
    threadCount_ = thread_count;
}

void DownloadManager::Impl::setMaxRetries(size_t retries) {
    LOG_F(INFO, "Setting max retries to: {}", retries);
    maxRetries_ = retries;
}

void DownloadManager::Impl::onDownloadComplete(
    const std::function<void(size_t)>& callback) {
    LOG_F(INFO, "Setting onDownloadComplete callback");
    onComplete_ = callback;
}

void DownloadManager::Impl::onProgressUpdate(
    const std::function<void(size_t, double)>& callback) {
    LOG_F(INFO, "Setting onProgressUpdate callback");
    onProgress_ = callback;
}

auto DownloadManager::Impl::getNextTaskIndex() -> std::optional<size_t> {
    LOG_F(INFO, "Getting next task index");
    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < tasks_.size(); ++i) {
        if (!tasks_[i].completed && !tasks_[i].paused && !tasks_[i].cancelled) {
            LOG_F(INFO, "Next task index: {}", i);
            return i;
        }
    }
    LOG_F(INFO, "No next task index found");
    return std::nullopt;
}

auto DownloadManager::Impl::getNextTask() -> std::optional<DownloadTask> {
    LOG_F(INFO, "Getting next task");
    std::unique_lock lock(mutex_);
    if (!taskQueue_.empty()) {
        auto task = taskQueue_.top();
        taskQueue_.pop();
        LOG_F(INFO, "Next task from queue: URL = {}, Filepath = {}", task.url,
              task.filepath);
        return task;
    }

    if (auto index = getNextTaskIndex()) {
        LOG_F(INFO, "Next task from index: URL = {}, Filepath = {}",
              tasks_[*index].url, tasks_[*index].filepath);
        return tasks_[*index];
    }

    LOG_F(INFO, "No next task found");
    return std::nullopt;
}

void DownloadManager::Impl::run(size_t download_speed) {
    LOG_F(INFO,
          "Running download manager with download speed limit of {} bytes/sec",
          download_speed);
    while (running_) {
        auto taskOpt = getNextTask();
        if (!taskOpt) {
            LOG_F(INFO, "No task to execute, exiting run loop");
            break;  // 没有任务可执行时退出
        }
        auto& task = *taskOpt;

        if (task.completed || task.paused || task.cancelled) {
            LOG_F(INFO,
                  "Skipping task: URL = {}, Filepath = {}, Completed = {}, "
                  "Paused = {}, Cancelled = {}",
                  task.url, task.filepath, task.completed, task.paused,
                  task.cancelled);
            continue;
        }

        // 记录任务开始的时间
        startTime_ = std::chrono::system_clock::now();
        LOG_F(INFO, "Starting download task: URL = {}, Filepath = {}", task.url,
              task.filepath);

        // 处理下载任务
        downloadTask(task, download_speed);

        if (task.completed && onComplete_) {
            LOG_F(INFO, "Download task completed: URL = {}, Filepath = {}",
                  task.url, task.filepath);
            onComplete_(task.priority);  // 下载完成后触发回调
        }
    }
    LOG_F(INFO, "Exiting run loop");
}

void DownloadManager::Impl::downloadTask(DownloadTask& task,
                                         size_t download_speed) {
    LOG_F(INFO, "Downloading task: URL = {}, Filepath = {}", task.url,
          task.filepath);
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
    LOG_F(INFO, "Download task finished: URL = {}, Filepath = {}", task.url,
          task.filepath);
}

void DownloadManager::Impl::saveTaskListToFile() {
    LOG_F(INFO, "Saving task list to file: {}", taskFile_);
    std::ofstream outfile(taskFile_);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open task file {}", taskFile_);
        return;
    }

    for (const auto& task : tasks_) {
        outfile << task.url << " " << task.filepath << std::endl;
        LOG_F(INFO, "Saved task: URL = {}, Filepath = {}", task.url,
              task.filepath);
    }
}

// DownloadManager public functions
DownloadManager::DownloadManager(const std::string& task_file)
    : impl_(std::make_unique<Impl>(task_file)) {
    LOG_F(INFO, "DownloadManager constructor called with task file: {}",
          task_file);
}
DownloadManager::~DownloadManager() {
    LOG_F(INFO, "DownloadManager destructor called");
}
void DownloadManager::addTask(const std::string& url,
                              const std::string& filepath, int priority) {
    LOG_F(INFO,
          "DownloadManager::addTask called with URL = {}, Filepath = {}, "
          "Priority = {}",
          url, filepath, priority);
    impl_->addTask(url, filepath, priority);
}
auto DownloadManager::removeTask(size_t index) -> bool {
    LOG_F(INFO, "DownloadManager::removeTask called with index: {}", index);
    return impl_->removeTask(index);
}
void DownloadManager::start(size_t thread_count, size_t download_speed) {
    LOG_F(INFO,
          "DownloadManager::start called with thread count = {}, download "
          "speed = {}",
          thread_count, download_speed);
    impl_->start(thread_count, download_speed);
}
void DownloadManager::pauseTask(size_t index) {
    LOG_F(INFO, "DownloadManager::pauseTask called with index: {}", index);
    impl_->pauseTask(index);
}
void DownloadManager::resumeTask(size_t index) {
    LOG_F(INFO, "DownloadManager::resumeTask called with index: {}", index);
    impl_->resumeTask(index);
}
auto DownloadManager::getDownloadedBytes(size_t index) -> size_t {
    LOG_F(INFO, "DownloadManager::getDownloadedBytes called with index: {}",
          index);
    return impl_->getDownloadedBytes(index);
}
void DownloadManager::cancelTask(size_t index) {
    LOG_F(INFO, "DownloadManager::cancelTask called with index: {}", index);
    impl_->cancelTask(index);
}
void DownloadManager::setThreadCount(size_t thread_count) {
    LOG_F(INFO, "DownloadManager::setThreadCount called with thread count: {}",
          thread_count);
    impl_->setThreadCount(thread_count);
}
void DownloadManager::setMaxRetries(size_t retries) {
    LOG_F(INFO, "DownloadManager::setMaxRetries called with retries: {}",
          retries);
    impl_->setMaxRetries(retries);
}
void DownloadManager::onDownloadComplete(
    const std::function<void(size_t)>& callback) {
    LOG_F(INFO, "DownloadManager::onDownloadComplete called");
    impl_->onDownloadComplete(callback);
}
void DownloadManager::onProgressUpdate(
    const std::function<void(size_t, double)>& callback) {
    LOG_F(INFO, "DownloadManager::onProgressUpdate called");
    impl_->onProgressUpdate(callback);
}

}  // namespace atom::web