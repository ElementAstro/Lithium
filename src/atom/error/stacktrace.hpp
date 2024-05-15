/*
 * stacktrace.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: StackTrace

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
 * This class provides functionality to capture the stack trace of the current
 * execution context and represent it as a string. It supports different
 * implementations for different operating systems.
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
    std::string toString() const;

private:
    /**
     * @brief Capture the current stack trace.
     *
     * This method captures the current stack trace based on the operating
     * system.
     */
    void capture();

#ifdef _WIN32
    std::vector<void*> frames; /**< Vector to store stack frames on Windows. */
#elif defined(__APPLE__) || defined(__linux__)
    std::unique_ptr<char*, decltype(&free)> symbols{
        nullptr,
        &free}; /**< Pointer to store stack symbols on macOS or Linux. */
    int num_frames = 0; /**< Number of stack frames captured. */
#endif
};
}  // namespace atom::error

#endif