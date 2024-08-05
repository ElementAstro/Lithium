/*
 * user.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Some system functions to get user information.

**************************************************/

#ifndef ATOM_SYSTEM_USER_HPP
#define ATOM_SYSTEM_USER_HPP

#include <string>
#include <vector>

#include "macro.hpp"

namespace atom::system {
/**
 * @brief Get user groups.
 * @return User groups.
 */
ATOM_NODISCARD auto getUserGroups() -> std::vector<std::wstring>;

/**
 * @brief Get user name.
 * @return User name.
 */
ATOM_NODISCARD auto getUsername() -> std::string;

/**
 * @brief Get host name.
 * @return Host name.
 */
ATOM_NODISCARD auto getHostname() -> std::string;

/**
 * @brief Get user id.
 * @return User id.
 */
ATOM_NODISCARD auto getUserId() -> int;

/**
 * @brief Get group id.
 * @return Group id.
 */
ATOM_NODISCARD auto getGroupId() -> int;

/**
 * @brief Get user profile directory.
 * @return User profile directory.
 */
ATOM_NODISCARD auto getHomeDirectory() -> std::string;

/**
 * @brief Get current working directory.
 * @return Current working directory.
 */
ATOM_NODISCARD auto getCurrentWorkingDirectory() -> std::string;

/**
 * @brief Get login shell.
 * @return Login shell.
 */
ATOM_NODISCARD auto getLoginShell() -> std::string;

#ifdef _WIN32
/**
 * @brief Get user profile directory.
 * @return User profile directory.
 */
auto getUserProfileDirectory() -> std::string;
#endif

/**
 * @brief Retrieves the login name of the user.
 *
 * This function retrieves the login name of the user associated with the
 * current process.
 *
 * @return The login name of the user.
 */
auto getLogin() -> std::string;

/**
 * @brief Check whether the current user has root/administrator privileges.
 * 检查当前用户是否具有根/管理员权限
 *
 * @return true if the current user has root/administrator privileges.
 *         如果当前用户具有根/管理员权限，则返回 true
 * @return false if the current user does not have root/administrator
 * privileges. 如果当前用户没有根/管理员权限，则返回 false
 */
auto isRoot() -> bool;
}  // namespace atom::system

#endif
