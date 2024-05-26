/*
 * sandbox.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A sandbox for alone componnents, such as executables.

**************************************************/

#ifndef LITHIUM_ADDON_SANDBOX_HPP
#define LITHIUM_ADDON_SANDBOX_HPP

#include <string>
#include <vector>

namespace lithium {
/**
 * @brief Sandbox class for running programs with time and memory limits in a
 * restricted environment.
 */
class Sandbox {
public:
    /**
     * @brief Default constructor for Sandbox class.
     */
    Sandbox() = default;

    /**
     * @brief Default destructor for Sandbox class.
     */
    ~Sandbox() = default;

    /**
     * @brief Sets the time limit for program execution.
     * @param timeLimitMs The time limit in milliseconds.
     * @return True if the time limit is set successfully, false otherwise.
     */
    bool setTimeLimit(int timeLimitMs);

    /**
     * @brief Sets the memory limit for program execution.
     * @param memoryLimitKb The memory limit in kilobytes.
     * @return True if the memory limit is set successfully, false otherwise.
     */
    bool setMemoryLimit(long memoryLimitKb);

    /**
     * @brief Sets the root directory for the sandbox environment.
     * @param rootDirectory The root directory path.
     * @return True if the root directory is set successfully, false otherwise.
     */
    bool setRootDirectory(const std::string& rootDirectory);

    /**
     * @brief Sets the user ID for running the program in the sandbox.
     * @param userId The user ID.
     * @return True if the user ID is set successfully, false otherwise.
     */
    bool setUserId(int userId);

    /**
     * @brief Sets the path to the program to be executed in the sandbox.
     * @param programPath The path to the program.
     * @return True if the program path is set successfully, false otherwise.
     */
    bool setProgramPath(const std::string& programPath);

    /**
     * @brief Sets the arguments for the program to be executed in the sandbox.
     * @param programArgs The vector of program arguments.
     * @return True if the program arguments are set successfully, false
     * otherwise.
     */
    bool setProgramArgs(const std::vector<std::string>& programArgs);

    /**
     * @brief Runs the program in the sandbox environment.
     * @return True if the program runs successfully within the time and memory
     * limits, false otherwise.
     */
    bool run();

    /**
     * @brief Retrieves the actual time used by the program during execution.
     * @return The time used in milliseconds.
     */
    [[nodiscard]] int getTimeUsed() const { return m_timeUsed; }

    /**
     * @brief Retrieves the actual memory used by the program during execution.
     * @return The memory used in kilobytes.
     */
    [[nodiscard]] long getMemoryUsed() const { return m_memoryUsed; }

private:
    int m_timeLimit =
        0; /**< Time limit for program execution in milliseconds. */
    long m_memoryLimit =
        0; /**< Memory limit for program execution in kilobytes. */
    std::string
        m_rootDirectory; /**< Root directory for the sandbox environment. */
    int m_userId = 0;    /**< User ID for running the program in the sandbox. */
    std::string m_programPath; /**< Path to the program to be executed. */
    std::vector<std::string> m_programArgs; /**< Program arguments. */
    int m_timeUsed =
        0; /**< Actual time used by the program during execution. */
    long m_memoryUsed =
        0; /**< Actual memory used by the program during execution. */
};

#endif
}
