// atomlog.cpp
/*
 * atomlog.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Enhanced Logger Implementation for Atom with C++20 Features

**************************************************/

#include "atomlog.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <format>
#include <fstream>
#include <memory>
#include <queue>
#include <shared_mutex>
#include <sstream>
#include <thread>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#undef ERROR
#elif defined(__linux__) || defined(__APPLE__)
#include <syslog.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif

#include "atom/error/exception.hpp"
#include "atom/utils/time.hpp"

namespace atom::log {

class Logger::LoggerImpl : public std::enable_shared_from_this<LoggerImpl> {
public:
    LoggerImpl(fs::path file_name_, LogLevel min_level, size_t max_file_size,
               int max_files)
        : file_name_(std::move(file_name_)),
          max_file_size_(max_file_size),
          max_files_(max_files),
          min_level_(min_level),
          system_logging_enabled_(false) {
        rotateLogFile();
        worker_ = std::jthread(&LoggerImpl::run, this);
    }

    ~LoggerImpl() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            finished_ = true;
        }
        cv_.notify_one();
        if (log_file_.is_open()) {
            log_file_.close();
        }

#if defined(__linux__) || defined(__APPLE__)
        if (system_logging_enabled_) {
            closelog();
        }
#endif
    }

    void setThreadName(const std::string& name) {
        std::lock_guard<std::mutex> lock(thread_mutex_);
        thread_names_[std::this_thread::get_id()] = name;
    }

    void setLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(level_mutex_);
        min_level_ = level;
    }

    void setPattern(const std::string& pattern) {
        std::lock_guard<std::mutex> lock(pattern_mutex_);
        pattern_ = pattern;
    }

    void registerSink(const std::shared_ptr<LoggerImpl>& logger) {
        if (logger.get() == this) {
            // 防止注册自身以避免递归调用
            return;
        }
        std::lock_guard<std::mutex> lock(sinks_mutex_);
        sinks_.emplace_back(logger);
    }

    void removeSink(const std::shared_ptr<LoggerImpl>& logger) {
        std::lock_guard<std::mutex> lock(sinks_mutex_);
        sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), logger),
                     sinks_.end());
    }

    void clearSinks() {
        std::lock_guard<std::mutex> lock(sinks_mutex_);
        sinks_.clear();
    }

    void enableSystemLogging(bool enable) {
        std::lock_guard<std::mutex> lock(system_log_mutex_);
        system_logging_enabled_ = enable;

#ifdef _WIN32
        if (system_logging_enabled_) {
            h_event_log_ = RegisterEventSourceW(nullptr, L"AtomLogger");
        } else if (h_event_log_) {
            DeregisterEventSource(h_event_log_);
            h_event_log_ = nullptr;
        }
#elif defined(__linux__) || defined(__APPLE__)
        if (system_logging_enabled_) {
            openlog("AtomLogger", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        }
#endif
    }

    void registerCustomLogLevel(const std::string& name, int severity) {
        std::lock_guard<std::mutex> lock(custom_level_mutex_);
        custom_levels_[name] = severity;
    }

    void log(LogLevel level, const std::string& msg) {
        if (static_cast<int>(level) < static_cast<int>(min_level_)) {
            return;
        }

        auto formattedMsg = formatMessage(level, msg);

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            log_queue_.push(formattedMsg);
        }
        cv_.notify_one();

        // 如果启用了系统日志，发送到系统日志
        if (system_logging_enabled_) {
            logToSystem(level, formattedMsg);
        }

        // 分发到所有注册的日志接收器
        dispatchToSinks(level, msg);
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
    std::unordered_map<std::thread::id, std::string> thread_names_;
    std::string pattern_ = "[{}][{}][{}] {}";
    std::vector<std::shared_ptr<LoggerImpl>> sinks_;
    bool system_logging_enabled_ = false;

#ifdef _WIN32
    HANDLE h_event_log_ = nullptr;
#endif

    std::mutex thread_mutex_;
    std::mutex pattern_mutex_;
    std::mutex sinks_mutex_;
    std::mutex system_log_mutex_;
    std::mutex level_mutex_;
    std::mutex custom_level_mutex_;
    std::unordered_map<std::string, int> custom_levels_;

    void rotateLogFile() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }

        if (max_files_ > 0) {
            auto extension = file_name_.extension();
            auto stem = file_name_.stem();

            for (int i = max_files_ - 1; i > 0; --i) {
                auto src = file_name_.parent_path() /
                           std::format("{}.{}{}", stem.string(), i,
                                       extension.string());
                auto dst = file_name_.parent_path() /
                           std::format("{}.{}{}", stem.string(), i + 1,
                                       extension.string());

                if (fs::exists(src)) {
                    if (fs::exists(dst)) {
                        fs::remove(dst);
                    }
                    fs::rename(src, dst);
                }
            }

            auto dst = file_name_.parent_path() /
                       std::format("{}.1{}", stem.string(), extension.string());
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
        std::lock_guard<std::mutex> lock(thread_mutex_);
        auto thread_id = std::this_thread::get_id();
        if (thread_names_.contains(thread_id)) {
            return thread_names_[thread_id];
        }
        return std::to_string(std::hash<std::thread::id>{}(thread_id));
    }

    static auto logLevelToString(LogLevel level) -> std::string {
        using enum LogLevel;
        switch (level) {
            case TRACE:
                return "TRACE";
            case DEBUG:
                return "DEBUG";
            case INFO:
                return "INFO";
            case WARN:
                return "WARN";
            case ERROR:
                return "ERROR";
            case CRITICAL:
                return "CRITICAL";
            default:
                return "UNKNOWN";
        }
    }

    auto formatMessage(LogLevel level, const std::string& msg) -> std::string {
        auto currentTime = utils::getChinaTimestampString();
        auto threadName = getThreadName();

        std::shared_lock<std::mutex> patternLock(pattern_mutex_);
        return std::vformat(pattern_, std::make_format_args(
                                          currentTime, logLevelToString(level),
                                          threadName, msg));
    }

    void run(std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            std::string msg;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock,
                         [this] { return !log_queue_.empty() || finished_; });

                if (finished_ && log_queue_.empty()) {
                    break;
                }

                msg = log_queue_.front();
                log_queue_.pop();
            }

            log_file_ << msg << std::endl;

            if (log_file_.tellp() >=
                static_cast<std::streampos>(max_file_size_)) {
                rotateLogFile();
            }
        }
    }

    void logToSystem(LogLevel level, const std::string& msg) const {
#ifdef _WIN32
        if (h_event_log_) {
            using enum LogLevel;
            WORD eventType;
            switch (level) {
                case ERROR:
                case CRITICAL:
                    eventType = EVENTLOG_ERROR_TYPE;
                    break;
                case WARN:
                    eventType = EVENTLOG_WARNING_TYPE;
                    break;
                case INFO:
                case DEBUG:
                case TRACE:
                default:
                    eventType = EVENTLOG_INFORMATION_TYPE;
                    break;
            }

            std::wstring wideMsg = std::wstring(msg.begin(), msg.end());
            LPCWSTR messages[] = {wideMsg.c_str()};
            ReportEventW(h_event_log_, eventType, 0, 0, nullptr, 1, 0, messages,
                         nullptr);
        }
#elif defined(__linux__) || defined(__APPLE__)
        if (system_logging_enabled_) {
            using enum LogLevel;
            int priority;
            switch (level) {
                case CRITICAL:
                    priority = LOG_CRIT;
                    break;
                case ERROR:
                    priority = LOG_ERR;
                    break;
                case WARN:
                    priority = LOG_WARNING;
                    break;
                case INFO:
                    priority = LOG_INFO;
                    break;
                case DEBUG:
                case TRACE:
                default:
                    priority = LOG_DEBUG;
                    break;
            }

            syslog(priority, "%s", msg.c_str());
        }
#elif defined(__ANDROID__)
        if (system_logging_enabled_) {
            using enum LogLevel;
            int priority;
            switch (level) {
                case CRITICAL:
                    priority = ANDROID_LOG_FATAL;
                    break;
                case ERROR:
                    priority = ANDROID_LOG_ERROR;
                    break;
                case WARN:
                    priority = ANDROID_LOG_WARN;
                    break;
                case INFO:
                    priority = ANDROID_LOG_INFO;
                    break;
                case DEBUG:
                    priority = ANDROID_LOG_DEBUG;
                    break;
                case TRACE:
                default:
                    priority = ANDROID_LOG_VERBOSE;
                    break;
            }

            __android_log_print(priority, "AtomLogger", "%s", msg.c_str());
        }
#endif
    }

    void dispatchToSinks(LogLevel level, const std::string& msg) {
        std::shared_lock<std::mutex> lock(sinks_mutex_);
        for (const auto& sink : sinks_) {
            sink->log(level, msg);
        }
    }

    auto getCustomLogLevel(const std::string& name) -> LogLevel {
        std::shared_lock<std::mutex> lock(custom_level_mutex_);
        if (custom_levels_.find(name) != custom_levels_.end()) {
            return static_cast<LogLevel>(custom_levels_.at(name));
        }
        return LogLevel::INFO;
    }
};

// `Logger` 类的方法实现

Logger::Logger(const fs::path& file_name, LogLevel min_level,
               size_t max_file_size, int max_files)
    : impl_(std::make_shared<LoggerImpl>(file_name, min_level, max_file_size,
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
    if (logger) {
        impl_->registerSink(logger->impl_);
    }
}

void Logger::removeSink(const std::shared_ptr<Logger>& logger) {
    if (logger) {
        impl_->removeSink(logger->impl_);
    }
}

void Logger::clearSinks() { impl_->clearSinks(); }

void Logger::enableSystemLogging(bool enable) {
    impl_->enableSystemLogging(enable);
}

void Logger::registerCustomLogLevel(const std::string& name, int severity) {
    impl_->registerCustomLogLevel(name, severity);
}

void Logger::log(LogLevel level, const std::string& msg) {
    impl_->log(level, msg);
}

}  // namespace atom::log