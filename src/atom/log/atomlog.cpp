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
#include <sstream>

#include "atom/error/exception.hpp"
#include "atom/utils/time.hpp"

namespace atom::log {
Logger::Logger(const fs::path& file_name_, LogLevel min_level,
               size_t max_file_size, int max_files_)
    : file_name_(file_name_),
      worker_([this] { run(); }),
      max_file_size_(max_file_size),
      max_files_(max_files_),
      min_level_(min_level) {
    rotateLogFile();
}

Logger::~Logger() {
    {
        std::lock_guard lock(queue_mutex_);
        finished_ = true;
    }
    cv_.notify_one();
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::setThreadName(const std::string& name) {
    std::lock_guard lock(queue_mutex_);
    thread_names_[std::this_thread::get_id()] = name;
}

void Logger::setLevel(LogLevel level) { min_level_ = level; }

void Logger::setPattern(const std::string& pattern) {
    this->pattern_ = pattern;
}

void Logger::registerSink(const std::shared_ptr<Logger>& logger) {
    sinks_.push_back(logger);
}

void Logger::removeSink(const std::shared_ptr<Logger>& logger) {
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), logger),
                 sinks_.end());
}

void Logger::clearSinks() { sinks_.clear(); }

auto Logger::getThreadName() -> std::string {
    auto id = std::this_thread::get_id();
    if (thread_names_.find(id) != thread_names_.end()) {
        return thread_names_[id];
    }
    return std::to_string(std::hash<std::thread::id>{}(id));
}

auto Logger::logLevelToString(LogLevel level) -> std::string {
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

auto Logger::formatMessage(LogLevel level,
                           const std::string& msg) -> std::string {
    auto currentTime = utils::getChinaTimestampString();
    auto threadName = getThreadName();
    std::ostringstream sss;
    sss << "[" << currentTime << "]"
        << "[" << logLevelToString(level) << "]"
        << "[" << threadName << "]"
        << "[" << msg << "]";
    return sss.str();
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < min_level_) {
        return;
    }

    auto formattedMsg = formatMessage(level, msg);

    {
        std::lock_guard lock(queue_mutex_);
        log_queue_.push(formattedMsg);
    }
    cv_.notify_one();

    for (const auto& sink : sinks_) {
        sink->log(level, msg);
    }
}

void Logger::run() {
    std::string msg;
    while (true) {
        {
            std::unique_lock lock(queue_mutex_);
            cv_.wait(lock, [this] { return !log_queue_.empty() || finished_; });

            if (finished_ && log_queue_.empty()) {
                break;
            }

            msg = log_queue_.front();
            log_queue_.pop();
        }  // Release lock before I/O operation

        log_file_ << msg << std::endl;

        if (log_file_.tellp() >= static_cast<std::streampos>(max_file_size_)) {
            rotateLogFile();
        }
    }
}

void Logger::rotateLogFile() {
    std::lock_guard lock(queue_mutex_);  // Lock during rotation
    if (log_file_.is_open()) {
        log_file_.close();
    }

    if (max_files_ > 0) {
        auto extension = file_name_.extension();
        auto stem = file_name_.stem();

        for (int i = max_files_ - 1; i > 0; --i) {
            auto src =
                file_name_.parent_path() /
                (stem.string() + "." + std::to_string(i) + extension.string());
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
}  // namespace atom::log