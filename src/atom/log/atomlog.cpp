/*
 * atomlog.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Logger for Atom

**************************************************/

#include "atomlog.hpp"

#include <algorithm>
#include <condition_variable>
#include <fstream>
#include <queue>
#include <sstream>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <syslog.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif

#include "atom/error/exception.hpp"
#include "atom/utils/time.hpp"

namespace atom::log {

class LoggerImpl {
public:
    LoggerImpl(const fs::path& file_name_, LogLevel min_level,
               size_t max_file_size, int max_files)
        : file_name_(file_name_),
          max_file_size_(max_file_size),
          max_files_(max_files),
          min_level_(min_level),
          worker_(&LoggerImpl::run, this) {
        rotateLogFile();
    }

    ~LoggerImpl() {
        {
            std::lock_guard lock(queue_mutex_);
            finished_ = true;
        }
        cv_.notify_one();
        worker_.request_stop();
        if (log_file_.is_open()) {
            log_file_.close();
        }

#ifdef __linux__ || __APPLE__
        if (system_logging_enabled_) {
            closelog();
        }
#endif
    }

    void setThreadName(const std::string& name) {
        std::lock_guard lock(queue_mutex_);
        thread_names_[std::this_thread::get_id()] = name;
    }

    void setLevel(LogLevel level) { min_level_ = level; }

    void setPattern(const std::string& pattern) { this->pattern_ = pattern; }

    void registerSink(const std::shared_ptr<LoggerImpl>& logger) {
        sinks_.push_back(logger);
    }

    void removeSink(const std::shared_ptr<LoggerImpl>& logger) {
        sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), logger),
                     sinks_.end());
    }

    void clearSinks() { sinks_.clear(); }

    void enableSystemLogging(bool enable) {
        system_logging_enabled_ = enable;

#ifdef _WIN32
        if (system_logging_enabled_) {
            h_event_log_ = RegisterEventSource(nullptr, L"AtomLogger");
        } else if (h_event_log_) {
            DeregisterEventSource(h_event_log_);
            h_event_log_ = nullptr;
        }
#elif defined(__linux__) || defined(__APPLE__)
        if (system_logging_enabled_) {
            openlog("AtomLogger", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        }
#elif defined(__ANDROID__)
        // Android logging does not require initialization
#endif
    }

    void log(LogLevel level, const std::string& msg) {
        if (level < min_level_) {
            return;
        }

        auto formattedMsg = formatMessage(level, msg);

        {
            std::lock_guard lock(queue_mutex_);
            log_queue_.push(formattedMsg);
        }
        cv_.notify_one();

        // Send to system log if enabled
        if (system_logging_enabled_) {
            logToSystem(level, formattedMsg);
        }

        for (const auto& sink : sinks_) {
            sink->log(level, msg);
        }
    }

private:
    fs::path file_name_;
    std::ofstream log_file_;
    std::queue<std::string> log_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    bool finished_ = false;
    std::jthread worker_;
    size_t max_file_size_;
    int max_files_;
    LogLevel min_level_;
    int file_index_ = 0;
    std::unordered_map<std::thread::id, std::string> thread_names_;
    std::string pattern_ = "[{}][{}][{}] {v}";
    std::vector<std::shared_ptr<LoggerImpl>> sinks_;
    bool system_logging_enabled_{};
#ifdef _WIN32
    HANDLE h_event_log_ = nullptr;
#endif

    void rotateLogFile() {
        std::lock_guard lock(queue_mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }

        if (max_files_ > 0) {
            auto extension = file_name_.extension();
            auto stem = file_name_.stem();

            for (int i = max_files_ - 1; i > 0; --i) {
                auto src = file_name_.parent_path() /
                           (stem.string() + "." + std::to_string(i) +
                            extension.string());
                auto dst = file_name_.parent_path() /
                           (stem.string() + "." + std::to_string(i + 1) +
                            extension.string());

                if (fs::exists(src)) {
                    if (fs::exists(dst)) {
                        fs::remove(dst);
                    }
                    fs::rename(src, dst);
                }
            }

            auto dst = file_name_.parent_path() /
                       (stem.string() + ".1" + extension.string());
            if (fs::exists(file_name_)) {
                if (fs::exists(dst)) {
                    fs::remove(dst);
                }
                fs::rename(file_name_, dst);
            }
        }

        log_file_.open(file_name_, std::ios::out | std::ios::app);
        if (!log_file_.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Failed to open log file: " +
                                    file_name_.string());
        }
    }

    auto getThreadName() -> std::string {
        auto id = std::this_thread::get_id();
        if (thread_names_.find(id) != thread_names_.end()) {
            return thread_names_[id];
        }
        return std::to_string(std::hash<std::thread::id>{}(id));
    }

    auto logLevelToString(LogLevel level) -> std::string {
        switch (level) {
            case LogLevel::TRACE:
                return "TRACE";
            case LogLevel::DEBUG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARN:
                return "WARN";
            case LogLevel::ERROR:
                return "ERROR";
            case LogLevel::CRITICAL:
                return "CRITICAL";
            default:
                return "UNKNOWN";
        }
    }

    auto formatMessage(LogLevel level, const std::string& msg) -> std::string {
        auto currentTime = utils::getChinaTimestampString();
        auto threadName = getThreadName();
        std::ostringstream sss;
        sss << "[" << currentTime << "]"
            << "[" << logLevelToString(level) << "]"
            << "[" << threadName << "]"
            << "[" << msg << "]";
        return sss.str();
    }

    void run(std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            std::string msg;
            {
                std::unique_lock lock(queue_mutex_);
                cv_.wait(lock,
                         [this] { return !log_queue_.empty() || finished_; });

                if (finished_ && log_queue_.empty()) {
                    break;
                }

                msg = log_queue_.front();
                log_queue_.pop();
            }  // Release lock before I/O operation

            log_file_ << msg << std::endl;

            if (log_file_.tellp() >=
                static_cast<std::streampos>(max_file_size_)) {
                rotateLogFile();
            }
        }
    }

    void logToSystem(LogLevel level, const std::string& msg) {
#ifdef _WIN32
        if (h_event_log_) {
            WORD event_type;
            switch (level) {
                case LogLevel::ERROR:
                case LogLevel::CRITICAL:
                    event_type = EVENTLOG_ERROR_TYPE;
                    break;
                case LogLevel::WARN:
                    event_type = EVENTLOG_WARNING_TYPE;
                    break;
                case LogLevel::INFO:
                case LogLevel::DEBUG:
                case LogLevel::TRACE:
                default:
                    event_type = EVENTLOG_INFORMATION_TYPE;
                    break;
            }

            LPCWSTR messages[] = {std::wstring(msg.begin(), msg.end()).c_str()};
            ReportEventW(h_event_log_, event_type, 0, 0, nullptr, 1, 0,
                         messages, nullptr);
        }
#elif defined(__linux__) || defined(__APPLE__)
        int priority;
        switch (level) {
            case LogLevel::CRITICAL:
                priority = LOG_CRIT;
                break;
            case LogLevel::ERROR:
                priority = LOG_ERR;
                break;
            case LogLevel::WARN:
                priority = LOG_WARNING;
                break;
            case LogLevel::INFO:
                priority = LOG_INFO;
                break;
            case LogLevel::DEBUG:
            case LogLevel::TRACE:
            default:
                priority = LOG_DEBUG;
                break;
        }

        syslog(priority, "%s", msg.c_str());
#elif defined(__ANDROID__)
        int priority;
        switch (level) {
            case LogLevel::CRITICAL:
                priority = ANDROID_LOG_FATAL;
                break;
            case LogLevel::ERROR:
                priority = ANDROID_LOG_ERROR;
                break;
            case LogLevel::WARN:
                priority = ANDROID_LOG_WARN;
                break;
            case LogLevel::INFO:
                priority = ANDROID_LOG_INFO;
                break;
            case LogLevel::DEBUG:
                priority = ANDROID_LOG_DEBUG;
                break;
            case LogLevel::TRACE:
            default:
                priority = ANDROID_LOG_VERBOSE;
                break;
        }

        __android_log_print(priority, "AtomLogger", "%s", msg.c_str());
#endif
    }
};

// `Logger` class method implementations

Logger::Logger(const fs::path& file_name, LogLevel min_level,
               size_t max_file_size, int max_files)
    : impl_(std::make_unique<LoggerImpl>(file_name, min_level, max_file_size,
                                         max_files)) {}

Logger::~Logger() = default;

void Logger::setThreadName(const std::string& name) {
    impl_->setThreadName(name);
}

void Logger::setLevel(LogLevel level) { impl_->setLevel(level); }

void Logger::setPattern(const std::string& pattern) {
    impl_->setPattern(pattern);
}

void Logger::registerSink(const std::shared_ptr<Logger>& logger) {
    impl_->registerSink(logger->impl_);
}

void Logger::removeSink(const std::shared_ptr<Logger>& logger) {
    impl_->removeSink(logger->impl_);
}

void Logger::clearSinks() { impl_->clearSinks(); }

void Logger::enableSystemLogging(bool enable) {
    impl_->enableSystemLogging(enable);
}

void Logger::log(LogLevel level, const std::string& msg) {
    impl_->log(level, msg);
}

}  // namespace atom::log
