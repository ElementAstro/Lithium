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

#include <vector>
#include <string>

namespace Atom::System
{
    /**
     * @brief Get user groups.
     * @return User groups.
    */
    [[nodiscard]] std::vector<std::wstring> GetUserGroups();
    
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
}

#endif