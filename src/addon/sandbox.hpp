/*
 * sandbox.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A sandbox for isolated components, such as executables.

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
    Sandbox() = default;
    ~Sandbox() = default;

    auto setTimeLimit(int timeLimitMs) -> bool;
    auto setMemoryLimit(long memoryLimitKb) -> bool;
    auto setRootDirectory(const std::string& rootDirectory) -> bool;
    auto setUserId(int userId) -> bool;
    auto setProgramPath(const std::string& programPath) -> bool;
    auto setProgramArgs(const std::vector<std::string>& programArgs) -> bool;
    auto run() -> bool;

    [[nodiscard]] int getTimeUsed() const { return m_timeUsed; }
    [[nodiscard]] long getMemoryUsed() const { return m_memoryUsed; }

private:
    int m_timeLimit{0};
    long m_memoryLimit{0};
    std::string m_rootDirectory;
    int m_userId{0};
    std::string m_programPath;
    std::vector<std::string> m_programArgs;
    int m_timeUsed{0};
    long m_memoryUsed{0};

#ifdef _WIN32
    bool setWindowsLimits(PROCESS_INFORMATION& processInfo);
#else
    bool setUnixLimits();
#endif
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_SANDBOX_HPP
