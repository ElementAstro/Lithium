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

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#if ENABLE_CPPTRACE
#include <cpptrace/cpptrace.hpp>
#endif

namespace atom::error {
const char* Exception::what() const noexcept {
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

const std::string& Exception::getFile() const { return file_; }
int Exception::getLine() const { return line_; }
const std::string& Exception::getFunction() const { return func_; }
const std::string& Exception::getMessage() const { return message_; }
std::thread::id Exception::getThreadId() const { return thread_id_; }

std::string Exception::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&time));
    return buffer;
}
}  // namespace atom::error
