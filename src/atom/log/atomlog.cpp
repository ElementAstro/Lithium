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

namespace atom::log {
Logger::Logger(const fs::path& file_name, LogLevel min_level,
               size_t max_file_size, int max_files)
    : file_name(file_name),
      min_level(min_level),
      max_file_size(max_file_size),
      max_files(max_files),
      worker([this] { run(); }) {
    rotateLogFile();
}

Logger::~Logger() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        finished = true;
    }
    cv.notify_one();
    if (log_file.is_open()) {
        log_file.close();
    }
}

void Logger::setThreadName(const std::string& name) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    thread_names[std::this_thread::get_id()] = name;
}

void Logger::setLevel(LogLevel level) { min_level = level; }

void Logger::setPattern(const std::string& pattern) { this->pattern = pattern; }

void Logger::registerSink(const std::shared_ptr<Logger>& logger) {
    sinks.push_back(logger);
}

void Logger::removeSink(const std::shared_ptr<Logger>& logger) {
    sinks.erase(std::remove(sinks.begin(), sinks.end(), logger), sinks.end());
}

void Logger::clearSinks() { sinks.clear(); }

std::string Logger::getThreadName() {
    auto id = std::this_thread::get_id();
    if (thread_names.find(id) != thread_names.end()) {
        return thread_names[id];
    }
    return std::to_string(std::hash<std::thread::id>{}(id));
}

std::string Logger::logLevelToString(LogLevel level) {
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

std::string Logger::formatMessage(LogLevel level, const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto tm_now = fmt::localtime(time_t_now);
    auto thread_name = getThreadName();
    return fmt::format(
        fmt::runtime(pattern), fmt::arg("Y", tm_now.tm_year + 1900),
        fmt::arg("m", tm_now.tm_mon + 1), fmt::arg("d", tm_now.tm_mday),
        fmt::arg("H", tm_now.tm_hour), fmt::arg("M", tm_now.tm_min),
        fmt::arg("S", tm_now.tm_sec),
        fmt::arg("e", std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch())
                              .count() %
                          1000),
        fmt::arg("l", logLevelToString(level)), fmt::arg("t", thread_name),
        fmt::arg("v", msg));
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < min_level) {
        return;
    }

    auto formatted_msg = formatMessage(level, msg);

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        log_queue.push(formatted_msg);
    }
    cv.notify_one();

    for (const auto& sink : sinks) {
        sink->log(level, msg);
    }
}

void Logger::run() {
    std::string msg;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [this] { return !log_queue.empty() || finished; });

            if (finished && log_queue.empty()) {
                break;
            }

            msg = log_queue.front();
            log_queue.pop();
        }  // Release lock before I/O operation

        log_file << msg << std::endl;

        if (log_file.tellp() >= static_cast<std::streampos>(max_file_size)) {
            rotateLogFile();
        }
    }
}

void Logger::rotateLogFile() {
    std::lock_guard<std::mutex> lock(queue_mutex);  // Lock during rotation
    if (log_file.is_open()) {
        log_file.close();
    }

    if (max_files > 0) {
        auto extension = file_name.extension();
        auto stem = file_name.stem();

        for (int i = max_files - 1; i > 0; --i) {
            auto src =
                file_name.parent_path() /
                (stem.string() + "." + std::to_string(i) + extension.string());
            auto dst = file_name.parent_path() /
                       (stem.string() + "." + std::to_string(i + 1) +
                        extension.string());

            if (fs::exists(src)) {
                if (fs::exists(dst)) {
                    fs::remove(dst);
                }
                fs::rename(src, dst);
            }
        }

        auto dst = file_name.parent_path() /
                   (stem.string() + ".1" + extension.string());
        if (fs::exists(file_name)) {
            if (fs::exists(dst)) {
                fs::remove(dst);
            }
            fs::rename(file_name, dst);
        }
    }

    log_file.open(file_name, std::ios::out | std::ios::app);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open log file: " +
                                 file_name.string());
    }
}

}  // namespace atom::log
