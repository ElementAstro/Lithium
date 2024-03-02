/*
 * system.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: System

**************************************************/

#ifndef ATOM_SYSTEM_SYSTEM_HPP
#define ATOM_SYSTEM_SYSTEM_HPP

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Atom::System {
/**
 * @brief Process information.
 * 进程信息
 */
struct ProcessInfo {
    int processID;
    int parentProcessID;
    int basePriority;
    std::string executableFile;
};

/**
 * @brief Check whether the specified software is installed.
 * 检查指定软件是否已安装
 *
 * @param software_name The name of the software. 软件名称
 * @return true if the software is installed.
 *         如果软件已安装，则返回 true
 * @return false if the software is not installed or an error occurred.
 *         如果软件未安装或发生错误，则返回 false
 */
bool CheckSoftwareInstalled(const std::string &software_name);

/**
 * @brief Check whether the specified file exists.
 * 检查指定文件是否存在
 *
 * @param fileName The name of the file. 文件名称
 * @param fileExt The extension of the file. 文件扩展名
 * @return true if the file exists.
 *         如果文件存在，则返回 true
 * @return false if the file does not exist or an error occurred.
 *         如果文件不存在或发生错误，则返回 false
 */
bool checkExecutableFile(const std::string &fileName,
                         const std::string &fileExt);

/**
 * @brief Check whether the current user has root/administrator privileges.
 * 检查当前用户是否具有根/管理员权限
 *
 * @return true if the current user has root/administrator privileges.
 *         如果当前用户具有根/管理员权限，则返回 true
 * @return false if the current user does not have root/administrator
 * privileges. 如果当前用户没有根/管理员权限，则返回 false
 */
bool IsRoot();

/**
 * @brief Get the current user name.
 * 获取当前用户名
 *
 * @return The current user name.
 *         当前用户名
 */
std::string GetCurrentUsername();

// Max: Surely, we will not recieve any signal after we call this function.

/**
 * @brief Shutdown the system.
 * 关闭系统
 *
 * @return true if the system is successfully shutdown.
 *         如果系统成功关闭，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
bool Shutdown();

/**
 * @brief Reboot the system.
 * 重启系统
 *
 * @return true if the system is successfully rebooted.
 *         如果系统成功重启，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
bool Reboot();

/**
 * @brief Get the process information and file address.
 * 获取进程信息以及对应文件地址
 *
 * @return A vector of pairs containing the process information (name and
 * process ID) and its file address.
 *         包含进程信息（名称和进程ID）以及对应文件地址的一对对的向量
 */
std::vector<std::pair<std::string, std::string>> GetProcessInfo();

/**
 * @brief Check if there are duplicate processes with the same program name.
 * 检查是否存在同一程序名的重复进程。
 *
 * This function checks if there are multiple instances of a process running
 * with the same program name. It compares the program name of each running
 * process with the specified program name.
 *
 * @param program_name The name of the program to check for duplicates.
 * 要检查重复项的程序名称。
 */
bool CheckDuplicateProcess(const std::string &program_name);

/**
 * @brief Check if the specified process is running.
 * 检查指定的进程是否正在运行
 *
 * @param processName The name of the process to check. 要检查的进程名称。
 * @return true if the process is running.
 *         如果进程正在运行，则返回 true
 * @return false if the process is not running.
 *         如果进程没有运行，则返回 false
 */
bool isProcessRunning(const std::string &processName);

/**
 * @brief Get the process details.
 * 获取进程详细信息
 *
 * @return A vector of process details.
 *         进程详细信息的向量
 */
std::vector<ProcessInfo> GetProcessDetails();

/**
 * @brief Get the process information by process ID.
 * 获取进程信息
 *
 * @param processID The process ID. 进程 ID。
 * @param processInfo The process information. 进程信息。
 * @return true if the process information is successfully retrieved.
 *         如果进程信息成功检索，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
#ifdef _WIN32
bool GetProcessInfoByID(DWORD processID, ProcessInfo &processInfo);
#else
bool GetProcessInfoByID(int processID, ProcessInfo &processInfo);
#endif

/**
 * @brief Get the process information by process name.
 * 获取进程信息
 *
 * @param processName The process name. 进程名称。
 * @param processInfo The process information. 进程信息。
 * @return true if the process information is successfully retrieved.
 *         如果进程信息成功检索，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
bool GetProcessInfoByName(const std::string &processName,
                          ProcessInfo &processInfo);
}  // namespace Atom::System

#endif
