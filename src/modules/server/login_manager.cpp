/*
 * login_manager.cpp
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

#include "login_manager.hpp"
#include <mutex>
#include <thread>

#include "loguru/loguru.hpp"

#include <openssl/evp.h>

LoginManager::LoginManager()
{
    openssl_key = "lithium_server";
}

bool LoginManager::registerUser(const std::string &username, const std::string &password)
{
    std::unique_lock<std::shared_mutex> lock(users_mutex);
    if (users.find(username) == users.end())
    { // 检查用户是否已存在
        std::string encrypted_password = encryptPassword(password);
        users[username] = encrypted_password;
        LOG_F(INFO, "User registered successfully: %s", username.c_str());
        return true;
    }
    return false;
}

bool LoginManager::loginUser(const std::string &username, const std::string &password, bool rememberMe)
{
    std::shared_lock<std::shared_mutex> lock(users_mutex);
    auto user_iter = users.find(username);
    if (user_iter != users.end() && verifyPassword(password, user_iter->second))
    {
        loggedInUser = username;
        LOG_F(INFO, "User logged in successfully: %s", username.c_str());
        if (rememberMe)
        {
            rememberUser = true;
        }

        return true;
    }
    return false;
}

bool LoginManager::isLoggedIn()
{
    return !loggedInUser.empty();
}

std::string LoginManager::getCurrentUser() const
{
    return loggedInUser;
}

bool LoginManager::logoutUser()
{
    std::unique_lock<std::shared_mutex> lock(users_mutex);
    if (!loggedInUser.empty())
    {
        LOG_F(INFO, "User logged out: %s", loggedInUser.c_str());
        loggedInUser.clear();
        rememberUser = false;
        return true;
    }
    return false;
}

bool LoginManager::resetPassword(const std::string &username, const std::string &newPassword)
{
    std::unique_lock<std::shared_mutex> lock(users_mutex);
    auto user_iter = users.find(username);
    if (user_iter != users.end())
    {
        std::string encrypted_password = encryptPassword(newPassword);
        user_iter->second = encrypted_password;
        LOG_F(INFO, "Password reset successfully for user: %s", username.c_str());
        return true;
    }
    return false;
}

bool LoginManager::updateUserInformation(const std::string &username, const std::string &newInformation)
{
    std::shared_lock<std::shared_mutex> lock(users_mutex);
    auto user_iter = users.find(username);
    if (user_iter != users.end())
    {
        LOG_F(INFO, "User information updated: %s", username.c_str());
        // 在此处更新用户信息
        return true;
    }
    return false;
}

bool LoginManager::forceLogoutInactiveUsers(int timeoutSeconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(timeoutSeconds));
    std::unique_lock<std::shared_mutex> lock(users_mutex);
    if (!loggedInUser.empty())
    {
        LOG_F(INFO, "Forced logout for inactive user: ", loggedInUser.c_str());

        loggedInUser.clear();
        rememberUser = false;
        return true;
    }
    return false;
}

bool LoginManager::hasAccess(const std::string &username, const std::string &requiredPermission)
{
    std::shared_lock<std::shared_mutex> lock(users_mutex);
    auto user_iter = users.find(username);
    if (user_iter != users.end())
    {
        LOG_F(INFO, "Access granted for user: %s", username.c_str());
        return true;
    }
    return false;
}

std::string LoginManager::encryptPassword(const std::string &password)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        std::string encrypted_password;

        if (ctx != nullptr)
        {
            if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                                   reinterpret_cast<const unsigned char *>(openssl_key.data()), nullptr) == 1)
            {
                int encrypted_length = password.size() + 16;
                encrypted_password.resize(encrypted_length);

                int final_length = 0;
                if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(&encrypted_password[0]), &encrypted_length,
                                      reinterpret_cast<const unsigned char *>(password.data()), password.size()) == 1 &&
                    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&encrypted_password[0]) + encrypted_length,
                                        &final_length) == 1)
                {
                    encrypted_password.resize(encrypted_length + final_length);
                }
                else
                {
                    LOG_F(ERROR,"Encryption failed");
                    encrypted_password.clear();
                }
            }
            else
            {
                LOG_F(ERROR,"Failed to initialize encryption context");
            }

            EVP_CIPHER_CTX_free(ctx);
        }

        return encrypted_password;
}

bool LoginManager::verifyPassword(const std::string &input_password, const std::string &stored_encrypted_password)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        std::string decrypted_password;

        if (ctx != nullptr)
        {
            if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                                   reinterpret_cast<const unsigned char *>(openssl_key.data()), nullptr) == 1)
            {
                int decrypted_length = stored_encrypted_password.size();
                decrypted_password.resize(decrypted_length);

                int final_length = 0;
                if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(&decrypted_password[0]), &decrypted_length,
                                      reinterpret_cast<const unsigned char *>(stored_encrypted_password.data()),
                                      stored_encrypted_password.size()) == 1 &&
                    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&decrypted_password[0]) + decrypted_length, &final_length) == 1)
                {
                    decrypted_password.resize(decrypted_length + final_length);
                }
                else
                {
                    LOG_F(ERROR,"Decryption failed");
                    decrypted_password.clear();
                }
            }
            else
            {
                LOG_F(ERROR,"Failed to initialize decryption context");
            }

            EVP_CIPHER_CTX_free(ctx);
        }

        return (input_password == decrypted_password);
}