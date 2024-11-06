#ifndef ATOM_SYSTEM_PROCESS_INFO_HPP
#define ATOM_SYSTEM_PROCESS_INFO_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "atom/macro.hpp"

namespace fs = std::filesystem;

namespace atom::system {
/**
 * @struct Process
 * @brief Represents a system process with detailed information.
 */
struct Process {
    int pid;              ///< Process ID.
    std::string name;     ///< Process name.
    std::string command;  ///< Command used to start the process.
    std::string output;   ///< Process output.
    fs::path path;        ///< Path to the process executable.
    std::string status;   ///< Process status.
#if defined(_WIN32)
    void *handle;  ///< Handle to the process (Windows only).
#endif
    bool isBackground;  ///< Indicates if the process runs in the background.
} ATOM_ALIGNAS(128);

/**
 * @struct PrivilegesInfo
 * @brief Contains privileges information of a user.
 */
struct PrivilegesInfo {
    std::string username;
    std::string groupname;
    std::vector<std::string> privileges;
    bool isAdmin;
} ATOM_ALIGNAS(128);
}  // namespace atom::system

#endif
