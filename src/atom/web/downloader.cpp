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
    Impl(const std::string& task_file);
    ~Impl();

    void add_task(const std::string& url, const std::string& filepath,
                  int priority);
    bool remove_task(size_t index);
    void start(size_t thread_count, size_t download_speed);
    void pause_task(size_t index);
    void resume_task(size_t index);
    void cancel_task(size_t index);
    size_t get_downloaded_bytes(size_t index);
    void set_thread_count(size_t thread_count);
    void set_max_retries(size_t retries);
    void on_download_complete(const std::function<void(size_t)>& callback);
    void on_progress_update(
        const std::function<void(size_t, double)>& callback);

private:
    struct DownloadTask {
        std::string url;
        std::string filepath;
        bool completed{false};
        bool paused{false};
        bool cancelled{false};
        size_t downloaded_bytes{0};
        int priority{0};
        size_t retries{0};
    };

    std::optional<size_t> get_next_task_index();
    std::optional<DownloadTask> get_next_task();
    void download_task(DownloadTask& task, size_t download_speed);
    void run(size_t download_speed);
    void save_task_list_to_file();

    std::string task_file_;
    std::vector<DownloadTask> tasks_;
    std::priority_queue<DownloadTask> task_queue_;
    std::mutex mutex_;
    std::atomic<bool> running_{false};
    std::chrono::system_clock::time_point start_time_;
    size_t max_retries_{3};
    size_t thread_count_;
    std::function<void(size_t)> on_complete_;
    std::function<void(size_t, double)> on_progress_;
};

// 写入回调函数，带进度监控
size_t write_data_with_progress(void* buffer, size_t size, size_t nmemb,
                                void* userp) {
    auto* task_info = static_cast<DownloadManager::Impl::DownloadTask*>(userp);
    size_t bytes_written = size * nmemb;
    task_info->downloaded_bytes += bytes_written;

    return bytes_written;
}

DownloadManager::Impl::Impl(const std::string& task_file)
    : task_file_(task_file),
      thread_count_(std::thread::hardware_concurrency()) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // 读取任务列表
    std::ifstream infile(task_file_);
    if (!infile) {
        LOG_F(ERROR, "Failed to open task file {}", task_file_);
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
    save_task_list_to_file();
    curl_global_cleanup();
}

void DownloadManager::Impl::add_task(const std::string& url,
                                     const std::string& filepath,
                                     int priority) {
    std::ofstream outfile(task_file_, std::ios_base::app);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open task file {}", task_file_);
        THROW_EXCEPTION("Failed to open task file.");
    }
    outfile << url << " " << filepath << std::endl;
    tasks_.push_back({url, filepath, false, false, false, 0, priority});
}

bool DownloadManager::Impl::remove_task(size_t index) {
    if (index >= tasks_.size()) {
        return false;
    }
    tasks_[index].completed = true;
    return true;
}

void DownloadManager::Impl::cancel_task(size_t index) {
    if (index >= tasks_.size()) {
        return;
    }
    tasks_[index].cancelled = true;
    tasks_[index].paused = true;  // 停止该任务的下载
}

void DownloadManager::Impl::start(size_t thread_count, size_t download_speed) {
    running_ = true;
    std::vector<std::jthread> threads;
    thread_count_ = thread_count;
    for (size_t i = 0; i < thread_count_; ++i) {
        threads.emplace_back(&DownloadManager::Impl::run, this, download_speed);
    }

    for (auto& thread : threads) {
        thread.join();  // 等待所有线程完成
    }
}

void DownloadManager::Impl::pause_task(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for pause_task.");
        return;
    }
    tasks_[index].paused = true;
}

void DownloadManager::Impl::resume_task(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for resume_task.");
        return;
    }
    tasks_[index].paused = false;
}

size_t DownloadManager::Impl::get_downloaded_bytes(size_t index) {
    if (index >= tasks_.size()) {
        LOG_F(ERROR, "Index out of bounds for get_downloaded_bytes.");
        return 0;
    }
    return tasks_[index].downloaded_bytes;
}

void DownloadManager::Impl::set_thread_count(size_t thread_count) {
    thread_count_ = thread_count;
}

void DownloadManager::Impl::set_max_retries(size_t retries) {
    max_retries_ = retries;
}

void DownloadManager::Impl::on_download_complete(
    const std::function<void(size_t)>& callback) {
    on_complete_ = callback;
}

void DownloadManager::Impl::on_progress_update(
    const std::function<void(size_t, double)>& callback) {
    on_progress_ = callback;
}

std::optional<size_t> DownloadManager::Impl::get_next_task_index() {
    std::unique_lock<std::mutex> lock(mutex_);
    for (size_t i = 0; i < tasks_.size(); ++i) {
        if (!tasks_[i].completed && !tasks_[i].paused && !tasks_[i].cancelled) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<DownloadManager::Impl::DownloadTask>
DownloadManager::Impl::get_next_task() {
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

void DownloadManager::Impl::run(size_t download_speed) {
    while (running_) {
        auto task_opt = get_next_task();
        if (!task_opt) {
            break;  // 没有任务可执行时退出
        }
        auto& task = *task_opt;

        if (task.completed || task.paused || task.cancelled) {
            continue;
        }

        // 记录任务开始的时间
        start_time_ = std::chrono::system_clock::now();

        // 处理下载任务
        download_task(task, download_speed);

        if (task.completed && on_complete_) {
            on_complete_(task.priority);  // 下载完成后触发回调
        }
    }
}

void DownloadManager::Impl::download_task(DownloadTask& task,
                                          size_t download_speed) {
    CURL* curl = curl_easy_init();
    if (!curl) {
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_with_progress);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &task);

    // 设置断点续传
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM, task.downloaded_bytes);

    // 设置下载速度限制
    if (download_speed > 0) {
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE,
                         static_cast<curl_off_t>(download_speed));
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_F(ERROR, "Download failed: {}", curl_easy_strerror(res));

        // 错误处理，重试机制
        if (task.retries < max_retries_) {
            LOG_F(INFO, "Retrying task {} ({} retries left)", task.url,
                  max_retries_ - task.retries);
            task.retries++;
            download_task(task, download_speed);  // 重试下载任务
        } else {
            LOG_F(ERROR, "Max retries reached for task {}", task.url);
        }
    } else {
        double total_size;
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &total_size);
        task.downloaded_bytes += static_cast<size_t>(total_size);
        task.completed = true;

        // 下载进度更新回调
        if (on_progress_) {
            double progress =
                static_cast<double>(task.downloaded_bytes) / total_size * 100.0;
            on_progress_(task.priority, progress);  // 传递任务索引和进度百分比
        }
    }

    curl_easy_cleanup(curl);
}

void DownloadManager::Impl::save_task_list_to_file() {
    std::ofstream outfile(task_file_);
    if (!outfile) {
        LOG_F(ERROR, "Failed to open task file {}", task_file_);
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
void DownloadManager::add_task(const std::string& url,
                               const std::string& filepath, int priority) {
    impl_->add_task(url, filepath, priority);
}
bool DownloadManager::remove_task(size_t index) {
    return impl_->remove_task(index);
}
void DownloadManager::start(size_t thread_count, size_t download_speed) {
    impl_->start(thread_count, download_speed);
}
void DownloadManager::pause_task(size_t index) { impl_->pause_task(index); }
void DownloadManager::resume_task(size_t index) { impl_->resume_task(index); }
size_t DownloadManager::get_downloaded_bytes(size_t index) {
    return impl_->get_downloaded_bytes(index);
}
void DownloadManager::cancel_task(size_t index) { impl_->cancel_task(index); }
void DownloadManager::set_thread_count(size_t thread_count) {
    impl_->set_thread_count(thread_count);
}
void DownloadManager::set_max_retries(size_t retries) {
    impl_->set_max_retries(retries);
}
void DownloadManager::on_download_complete(
    const std::function<void(size_t)>& callback) {
    impl_->on_download_complete(callback);
}
void DownloadManager::on_progress_update(
    const std::function<void(size_t, double)>& callback) {
    impl_->on_progress_update(callback);
}

}  // namespace atom::web
