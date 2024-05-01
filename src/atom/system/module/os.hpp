/*
 * os.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - OS Information

**************************************************/

#ifndef ATOM_SYSTEM_MODULE_OS_HPP
#define ATOM_SYSTEM_MODULE_OS_HPP

#include <string>

namespace atom::system {
/**
 * @brief Represents information about the operating system.
 */
struct OperatingSystemInfo {
    std::string osName;        /**< The name of the operating system. */
    std::string osVersion;     /**< The version of the operating system. */
    std::string kernelVersion; /**< The version of the kernel. */
    std::string architecture; /**< The architecture of the operating system. */
    std::string compiler; /**< The compiler used to compile the operating system. */

    OperatingSystemInfo() = default;

    std::string toJson() const;
};

/**
 * @brief Retrieves the information about the operating system.
 * @return The `OperatingSystemInfo` struct containing the operating system
 * information.
 */
OperatingSystemInfo getOperatingSystemInfo();

}  // namespace atom::system

#endif