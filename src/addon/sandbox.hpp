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

#include <memory>
#include <string>
#include <vector>

namespace lithium {

class SandboxImpl;  // Forward declaration of the implementation class

/**
 * @brief Sandbox class for running programs with time and memory limits in a
 * restricted environment.
 */
class Sandbox {
public:
    Sandbox();
    ~Sandbox();

    auto setTimeLimit(int timeLimitMs) -> bool;
    auto setMemoryLimit(long memoryLimitKb) -> bool;
    auto setRootDirectory(const std::string& rootDirectory) -> bool;
    auto setUserId(int userId) -> bool;
    auto setProgramPath(const std::string& programPath) -> bool;
    auto setProgramArgs(const std::vector<std::string>& programArgs) -> bool;
    auto run() -> bool;

    [[nodiscard]] auto getTimeUsed() const -> int;
    [[nodiscard]] auto getMemoryUsed() const -> long;

private:
    std::unique_ptr<SandboxImpl> pimpl;  // Pointer to the implementation class
};

}  // namespace lithium

#endif