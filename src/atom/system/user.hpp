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

namespace atom::system {
/**
 * @brief Get user groups.
 * @return User groups.
 */
[[nodiscard]] std::vector<std::wstring> getUserGroups();

/**
 * @brief Get user name.
 * @return User name.
 */
[[nodiscard]] std::string getUsername();

/**
 * @brief Get host name.
 * @return Host name.
 */
[[nodiscard]] std::string getHostname();

/**
 * @brief Get user id.
 * @return User id.
 */
[[nodiscard]] int getUserId();

/**
 * @brief Get group id.
 * @return Group id.
 */
[[nodiscard]] int getGroupId();

/**
 * @brief Get user profile directory.
 * @return User profile directory.
 */
[[nodiscard]] std::string getHomeDirectory();

/**
 * @brief Get login shell.
 * @return Login shell.
 */
[[nodiscard]] std::string getLoginShell();

/**
 * @brief Retrieves the login name of the user.
 *
 * This function retrieves the login name of the user associated with the
 * current process.
 *
 * @return The login name of the user.
 */
std::string getLogin();

/**
 * @brief Check whether the current user has root/administrator privileges.
 * 检查当前用户是否具有根/管理员权限
 *
 * @return true if the current user has root/administrator privileges.
 *         如果当前用户具有根/管理员权限，则返回 true
 * @return false if the current user does not have root/administrator
 * privileges. 如果当前用户没有根/管理员权限，则返回 false
 */
bool isRoot();
}  // namespace atom::system

#endif
