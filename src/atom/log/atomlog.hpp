/*
 * atomlog.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Logger for Atom

**************************************************/

#ifndef ATOM_LOG_ATOMLOG_HPP
#define ATOM_LOG_ATOMLOG_HPP

#include <condition_variable>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace atom::log {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
    OFF  // Used to disable logging
};

class Logger {
public:
    explicit Logger(const fs::path& file_name,
                    LogLevel min_level = LogLevel::TRACE,
                    size_t max_file_size = 1048576, int max_files = 10);

    ~Logger();
    Logger(const Logger&) = delete;
    auto operator=(const Logger&) -> Logger& = delete;

    template <typename... Args>
    void trace(const std::string& format, const Args&... args) {
        log(LogLevel::TRACE,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void debug(const std::string& format, const Args&... args) {
        log(LogLevel::DEBUG,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void info(const std::string& format, const Args&... args) {
        log(LogLevel::INFO,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void warn(const std::string& format, const Args&... args) {
        log(LogLevel::WARN,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void error(const std::string& format, const Args&... args) {
        log(LogLevel::ERROR,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void critical(const std::string& format, const Args&... args) {
        log(LogLevel::CRITICAL,
            std::vformat(format, std::make_format_args(args...)));
    }

    void setLevel(LogLevel level);

    void setPattern(const std::string& pattern);

    void setThreadName(const std::string& name);

    void registerSink(const std::shared_ptr<Logger>& logger);

    void removeSink(const std::shared_ptr<Logger>& logger);

    void clearSinks();

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
    std::vector<std::shared_ptr<Logger>> sinks_;

    void rotateLogFile();

    auto getThreadName() -> std::string;

    static auto logLevelToString(LogLevel level) -> std::string;

    auto formatMessage(LogLevel level, const std::string& msg) -> std::string;

    void log(LogLevel level, const std::string& msg);

    void run();
};
}  // namespace atom::log

#endif  // ATOM_LOG_ATOMLOG_HPP
