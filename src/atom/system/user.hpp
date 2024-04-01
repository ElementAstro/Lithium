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

namespace Atom::System {
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
}  // namespace Atom::System

#endif