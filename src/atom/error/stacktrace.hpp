/*
 * stacktrace.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Enhanced StackTrace with more details

**************************************************/

#ifndef ATOM_ERROR_STACKTRACE_HPP
#define ATOM_ERROR_STACKTRACE_HPP

#include <memory>
#include <string>
#include <vector>

namespace atom::error {

/**
 * @brief Class for capturing and representing a stack trace.
 *
 * This class captures the stack trace of the current
 * execution context and represents it as a string, including
 * file names, line numbers, and symbols if available.
 */
class StackTrace {
public:
    /**
     * @brief Default constructor.
     *
     * Constructs a StackTrace object and captures the current stack trace.
     */
    StackTrace();

    /**
     * @brief Get the string representation of the stack trace.
     *
     * @return A string representing the captured stack trace.
     */
    [[nodiscard]] auto toString() const -> std::string;

private:
    /**
     * @brief Capture the current stack trace.
     *
     * This method captures the current stack trace based on the operating
     * system.
     */
    void capture();

#ifdef _WIN32
    std::vector<void*> frames_; /**< Vector to store stack frames on Windows. */
#elif defined(__APPLE__) || defined(__linux__)
    std::unique_ptr<char*, decltype(&free)> symbols_{
        nullptr,
        &free}; /**< Pointer to store stack symbols on macOS or Linux. */
    std::vector<void*>
        frames_;         /**< Vector to store raw stack frame pointers. */
    int num_frames_ = 0; /**< Number of stack frames captured. */
#endif
};

}  // namespace atom::error

#endif
