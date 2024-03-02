/*
 * os.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-4

Description: Some useful system functions from Python.

**************************************************/

#ifndef ATOM_SYSTEM_OS_HPP
#define ATOM_SYSTEM_OS_HPP

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Atom::System {
struct Utsname {
    std::string sysname;   // 操作系统名称
    std::string nodename;  // 网络中的主机名
    std::string release;   // 操作系统发行版本
    std::string version;   // 操作系统内部版本
    std::string machine;   // 硬件标识
};

/**
 * @brief Recursively walks through a directory and its subdirectories.
 *
 * This function traverses a directory and its subdirectories, calling the
 * specified callback function for each file encountered.
 *
 * @param root The root path of the directory to walk.
 */
void walk(const fs::path &root);

/**
 * @brief Recursively walks through a directory and its subdirectories, applying
 * a callback function to each file.
 *
 * This function traverses a directory and its subdirectories, calling the
 * specified callback function for each file encountered.
 *
 * @param root     The root path of the directory to walk.
 * @param callback The callback function to execute for each file.
 */
void fwalk(const fs::path &root,
           const std::function<void(const fs::path &)> &callback);

/**
 * @brief Retrieves the environment variables as a key-value map.
 *
 * This function retrieves all the environment variables and their corresponding
 * values, and returns them as an unordered map.
 *
 * @return An unordered map containing the environment variables and their
 * values.
 */
std::unordered_map<std::string, std::string> Environ();

/**
 * @brief Returns the name of the controlling terminal.
 *
 * This function returns the name of the controlling terminal associated with
 * the current process.
 *
 * @return The name of the controlling terminal.
 */
std::string ctermid();

/**
 * @brief Retrieves the priority of the current process.
 *
 * This function retrieves the priority of the current process.
 *
 * @return The priority of the current process.
 */
int getpriority();

/**
 * @brief Retrieves the login name of the user.
 *
 * This function retrieves the login name of the user associated with the
 * current process.
 *
 * @return The login name of the user.
 */
std::string getlogin();

/**
 * @brief Retrieves the operating system name.
 *
 * This function retrieves the operating system name.
 *
 * @return The operating system name.
 */
Utsname uname();

}  // namespace Atom::System

#endif