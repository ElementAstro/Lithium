#ifndef ATOM_SYSTEM_PROCESS_HPP
#define ATOM_SYSTEM_PROCESS_HPP

#include "process_info.hpp"

namespace atom::system {
/**
 * @brief Gets information about all processes.
 * @return A vector of pairs containing process IDs and names.
 */
auto getAllProcesses() -> std::vector<std::pair<int, std::string>>;

/**
 * @brief Gets information about a process by its PID.
 * @param pid The process ID.
 * @return A Process struct containing information about the process.
 */
[[nodiscard("The process info is not used")]]
auto getProcessInfoByPid(int pid) -> Process;
/**
 * @brief Gets information about the current process.
 * @return A Process struct containing information about the current process.
 */
[[nodiscard("The process info is not used")]] auto getSelfProcessInfo()
    -> Process;

/**
 * @brief Returns the name of the controlling terminal.
 *
 * This function returns the name of the controlling terminal associated with
 * the current process.
 *
 * @return The name of the controlling terminal.
 */
[[nodiscard]] auto ctermid() -> std::string;

/**
 * @brief Checks if a process is running by its name.
 *
 * This function checks if a process with the specified name is currently
 * running.
 *
 * @param processName The name of the process to check.
 * @return bool True if the process is running, otherwise false.
 */
auto isProcessRunning(const std::string &processName) -> bool;

/**
 * @brief Returns the parent process ID of a given process.
 *
 * This function retrieves the parent process ID (PPID) of a specified process.
 * If the process is not found or an error occurs, the function returns -1.
 *
 * @param processId The process ID of the target process.
 * @return int The parent process ID if found, otherwise -1.
 */
auto getParentProcessId(int processId) -> int;

/**
 * @brief Creates a process as a specified user.
 *
 * This function creates a new process using the specified user credentials.
 * It logs in the user, duplicates the user token, and creates a new process
 * with the specified command. This function is only available on Windows.
 *
 * @param command The command to be executed by the new process.
 * @param username The username of the user account.
 * @param domain The domain of the user account.
 * @param password The password of the user account.
 * @return bool True if the process is created successfully, otherwise false.
 */
auto createProcessAsUser(const std::string &command,
                          const std::string &username,
                          const std::string &domain,
                          const std::string &password) -> bool;

/**
 * @brief Gets the process IDs of processes with the specified name.
 * @param processName The name of the process.
 * @return A vector of process IDs.
 */
auto getProcessIdByName(const std::string &processName) -> std::vector<int>;

#ifdef _WIN32
auto getWindowsPrivileges(int pid) -> PrivilegesInfo;
#endif
}  // namespace atom::system

#endif
