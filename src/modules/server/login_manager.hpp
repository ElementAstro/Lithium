/*
 * login_manager.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-6-17

Description: Login and Register Manager

**************************************************/

#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>

/**
 * @brief 管理用户登录的类
 */
class LoginManager
{
public:
    /**
     * @brief 构造函数
     */
    LoginManager();

    /**
     * @brief 注册新用户
     * @param username 用户名
     * @param password 密码
     * @return 注册成功返回true，否则返回false
     */
    bool registerUser(const std::string &username, const std::string &password);

    /**
     * @brief 用户登录
     * @param username 用户名
     * @param password 密码
     * @param rememberMe 是否记住用户
     * @return 登录成功返回true，否则返回false
     */
    bool loginUser(const std::string &username, const std::string &password, bool rememberMe);

    /**
     * @brief 检查用户是否已登录
     * @return 如果已登录返回true，否则返回false
     */
    bool isLoggedIn();

    /**
     * @brief 获取当前登录用户
     * @return 当前登录用户的用户名，若无用户登录则返回空字符串
     */
    std::string getCurrentUser() const;

    /**
     * @brief 注销当前登录用户
     * @return 注销成功返回true，否则返回false
     */
    bool logoutUser();

    /**
     * @brief 重置指定用户的密码
     * @param username 用户名
     * @param newPassword 新密码
     * @return 重置成功返回true，否则返回false
     */
    bool resetPassword(const std::string &username, const std::string &newPassword);

    /**
     * @brief 更新指定用户的信息
     * @param username 用户名
     * @param newInformation 新信息
     * @return 更新成功返回true，否则返回false
     */
    bool updateUserInformation(const std::string &username, const std::string &newInformation);

    /**
     * @brief 强制登出超时用户
     * @param timeoutSeconds 超时时间（秒）
     * @return 登出成功返回true，否则返回false
     */
    bool forceLogoutInactiveUsers(int timeoutSeconds);

    /**
     * @brief 检查用户是否具有指定权限
     * @param username 用户名
     * @param requiredPermission 需要的权限
     * @return 如果具有指定权限返回true，否则返回false
     */
    bool hasAccess(const std::string &username, const std::string &requiredPermission);

private:
    std::unordered_map<std::string, std::string> users; // 存储用户名和加密密码的映射
    std::string loggedInUser;                           // 当前已登录的用户名
    bool rememberUser;                                  // 是否记住当前用户
    std::shared_mutex users_mutex;                      // 用于保护users和loggedInUser的互斥锁
    std::string openssl_key;                            // 加密密码所需的密钥

    /**
     * @brief 加密密码
     * @param password 待加密的密码
     * @return 加密后的密码
     */
    std::string encryptPassword(const std::string &password);

    /**
     * @brief 验证密码
     * @param input_password 输入的密码
     * @param stored_encrypted_password 存储的加密密码
     * @return 如果密码验证成功返回true，否则返回false
     */
    bool verifyPassword(const std::string &input_password, const std::string &stored_encrypted_password);
};
