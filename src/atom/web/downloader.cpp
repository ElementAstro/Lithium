#include "downloader.hpp"

#include <atomic>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#ifdef USE_ASIO
#include <asio.hpp>
#endif

#include "atom/log/loguru.hpp"
#include "atom/macro.hpp"
#include "atom/web/curl.hpp"

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

private:
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
    } ATOM_ALIGNAS(128);

    void downloadTask(DownloadTask& task, size_t download_speed);
    void run(size_t download_speed);
    void saveTaskListToFile();
    void loadTaskListFromFile();

    std::string taskFile_;
    std::vector<DownloadTask> tasks_;
    std::priority_queue<DownloadTask> taskQueue_;
    std::mutex mutex_;
    std::atomic<bool> running_{false};
    size_t maxRetries_{3};
    size_t threadCount_{std::thread::hardware_concurrency()};
    std::function<void(size_t)> onComplete_;
    std::function<void(size_t, double)> onProgress_;

#ifdef USE_ASIO
    asio::io_context io_context_;
    std::vector<std::thread> io_threads_;
#endif
};

DownloadManager::Impl::Impl(std::string task_file)
    : taskFile_(std::move(task_file)) {
    LOG_F(INFO, "Initializing DownloadManager with task file: {}", taskFile_);
    loadTaskListFromFile();
}

DownloadManager::Impl::~Impl() {
    running_ = false;

#ifdef USE_ASIO
    io_context_.stop();
    for (auto& thread : io_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
#endif

    saveTaskListToFile();
    LOG_F(INFO, "DownloadManager destroyed");
}

void DownloadManager::Impl::addTask(const std::string& url,
                                    const std::string& filepath, int priority) {
    std::lock_guard lock(mutex_);
    tasks_.emplace_back(
        DownloadTask{url, filepath, false, false, false, 0, priority});
    taskQueue_.push(tasks_.back());
    saveTaskListToFile();
}

auto DownloadManager::Impl::removeTask(size_t index) -> bool {
    std::lock_guard lock(mutex_);
    if (index >= tasks_.size()) {
        return false;
    }
    tasks_.erase(tasks_.begin() + index);
    // 重新构建任务队列
    std::priority_queue<DownloadTask> newQueue;
    for (const auto& task : tasks_) {
        newQueue.push(task);
    }
    taskQueue_ = std::move(newQueue);
    saveTaskListToFile();
    return true;
}

void DownloadManager::Impl::start(size_t thread_count, size_t download_speed) {
    {
        std::lock_guard lock(mutex_);
        if (running_) {
            return;
        }
        running_ = true;
    }
    threadCount_ = thread_count;

#ifdef USE_ASIO
    for (size_t i = 0; i < threadCount_; ++i) {
        io_threads_.emplace_back([this]() { io_context_.run(); });
    }
    io_context_.post([this, download_speed]() { run(download_speed); });
#else
    for (size_t i = 0; i < threadCount_; ++i) {
        std::thread([this, download_speed]() { run(download_speed); }).detach();
    }
#endif
}

void DownloadManager::Impl::pauseTask(size_t index) {
    std::lock_guard lock(mutex_);
    if (index < tasks_.size()) {
        tasks_[index].paused = true;
    }
}

void DownloadManager::Impl::resumeTask(size_t index) {
    std::lock_guard lock(mutex_);
    if (index < tasks_.size()) {
        tasks_[index].paused = false;
        taskQueue_.push(tasks_[index]);
    }
}

void DownloadManager::Impl::cancelTask(size_t index) {
    std::lock_guard lock(mutex_);
    if (index < tasks_.size()) {
        tasks_[index].cancelled = true;
    }
}

size_t DownloadManager::Impl::getDownloadedBytes(size_t index) {
    std::lock_guard lock(mutex_);
    if (index < tasks_.size()) {
        return tasks_[index].downloadedBytes;
    }
    return 0;
}

void DownloadManager::Impl::setThreadCount(size_t thread_count) {
    std::lock_guard lock(mutex_);
    threadCount_ = thread_count;
    // 如果需要，调整线程池（未实现）
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

void DownloadManager::Impl::downloadTask(DownloadTask& task,
                                         size_t download_speed) {
    CurlWrapper curl;
    curl.setUrl(task.url)
        .setRequestMethod("GET")
        .onResponse([&](const std::string& data) {
            std::ofstream ofs(task.filepath, std::ios::binary | std::ios::app);
            ofs.write(data.c_str(), data.size());
            task.downloadedBytes += data.size();
            if (onProgress_) {
                onProgress_(task.priority,
                            static_cast<double>(task.downloadedBytes));
            }
        })
        .onError([&](CURLcode code) {
            LOG_F(ERROR, "Download error for URL {}: {}", task.url.c_str(),
                  static_cast<int>(code));
            if (task.retries < maxRetries_) {
                task.retries++;
                taskQueue_.push(task);
            } else {
                if (onComplete_) {
                    onComplete_(task.priority);
                }
            }
        })
        .setMaxDownloadSpeed(download_speed);

#ifdef USE_ASIO
    io_context_.post([this, &task, download_speed]() {
        curl.perform();
        task.completed = true;
        if (onComplete_) {
            onComplete_(task.priority);
        }
    });
#else
    curl.perform();
    task.completed = true;
    if (onComplete_) {
        onComplete_(task.priority);
    }
#endif
}

void DownloadManager::Impl::run(size_t download_speed) {
    while (running_) {
        DownloadTask task;
        {
            std::lock_guard lock(mutex_);
            if (taskQueue_.empty()) {
                break;
            }
            task = taskQueue_.top();
            taskQueue_.pop();
        }
        if (task.paused || task.cancelled || task.completed) {
            continue;
        }
        downloadTask(task, download_speed);
    }
}

void DownloadManager::Impl::saveTaskListToFile() {
    std::ofstream ofs(taskFile_, std::ios::trunc);
    for (const auto& task : tasks_) {
        ofs << task.url << " " << task.filepath << " " << task.priority << "\n";
    }
}

void DownloadManager::Impl::loadTaskListFromFile() {
    std::ifstream ifs(taskFile_);
    std::string url;
    std::string filepath;
    int priority;
    while (ifs >> url >> filepath >> priority) {
        tasks_.emplace_back(
            DownloadTask{url, filepath, false, false, false, 0, priority});
        taskQueue_.push(tasks_.back());
    }
}

DownloadManager::DownloadManager(const std::string& task_file)
    : impl_(std::make_unique<Impl>(task_file)) {}

DownloadManager::~DownloadManager() = default;

void DownloadManager::addTask(const std::string& url,
                              const std::string& filepath, int priority) {
    impl_->addTask(url, filepath, priority);
}

bool DownloadManager::removeTask(size_t index) {
    return impl_->removeTask(index);
}

void DownloadManager::start(size_t thread_count, size_t download_speed) {
    impl_->start(thread_count, download_speed);
}

void DownloadManager::pauseTask(size_t index) { impl_->pauseTask(index); }

void DownloadManager::resumeTask(size_t index) { impl_->resumeTask(index); }

size_t DownloadManager::getDownloadedBytes(size_t index) {
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
