/*
 * exception.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Better Exception Library

**************************************************/

#include "exception.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if ENABLE_CPPTRACE
#include <cpptrace/cpptrace.hpp>
#endif

namespace atom::error {
auto Exception::what() const noexcept -> const char* {
    if (full_message_.empty()) {
        std::ostringstream oss;
        oss << "[" << getCurrentTime() << "] ";
        oss << "Exception at " << file_ << ":" << line_ << " in " << func_
            << "()";
        oss << " (thread " << thread_id_ << ")";
        oss << "\n\tMessage: " << message_;
#if ENABLE_CPPTRACE
        oss << "\n\tStack trace:\n"
            << cpptrace::generate()
#else
        oss << "\n\tStack trace:\n" << stack_trace_.toString();
#endif
                   full_message_ = oss.str();
    }
    return full_message_.c_str();
}

auto Exception::getFile() const -> std::string { return file_; }
auto Exception::getLine() const -> int { return line_; }
auto Exception::getFunction() const -> std::string { return func_; }
auto Exception::getMessage() const -> std::string { return message_; }
auto Exception::getThreadId() const -> std::thread::id { return thread_id_; }

auto Exception::getCurrentTime() const -> std::string {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm tmTime{};
#if defined(_MSC_VER)
    localtime_s(&tm_time, &time_t);
#else
    localtime_r(&timeT, &tmTime);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
}  // namespace atom::error
